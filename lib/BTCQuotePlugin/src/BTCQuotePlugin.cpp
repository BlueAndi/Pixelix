/* MIT License
 *
 * Copyright (c) 2019 - 2025 Andreas Merkle <web@blue-andi.de>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*******************************************************************************
    DESCRIPTION
*******************************************************************************/
/**
 * @brief  Gets Bitcoin USD exchange rate
 * @author Flavio Curti <fcu-github@no-way.org>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "BTCQuotePlugin.h"

#include <FileSystem.h>
#include <ArduinoJson.h>
#include <Logging.h>
#include <JsonFile.h>
#include <HttpStatus.h>

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and classes
 *****************************************************************************/

/******************************************************************************
 * Prototypes
 *****************************************************************************/

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void BTCQuotePlugin::start(uint16_t width, uint16_t height)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_view.init(width, height);

    initHttpClient();
}

void BTCQuotePlugin::stop()
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_requestTimer.stop();
}

void BTCQuotePlugin::process(bool isConnected)
{
    Msg                         msg;
    MutexGuard<MutexRecursive>  guard(m_mutex);

    /* Only if a network connection is established the required information
     * shall be periodically requested via REST API.
     */
    if (false == m_requestTimer.isTimerRunning())
    {
        if (true == isConnected)
        {
            if (false == startHttpRequest())
            {
                m_requestTimer.start(UPDATE_PERIOD_SHORT);
            }
            else
            {
                m_requestTimer.start(UPDATE_PERIOD);
            }
        }
    }
    else
    {
        /* If the connection is lost, stop periodically requesting information
         * via REST API.
         */
        if (false == isConnected)
        {
            m_requestTimer.stop();
        }
        /* Network connection is available and next request may be necessary for
         * information update.
         */
        else if (true == m_requestTimer.isTimeout())
        {
            if (false == startHttpRequest())
            {
                m_requestTimer.start(UPDATE_PERIOD_SHORT);
            }
            else
            {
                m_requestTimer.start(UPDATE_PERIOD);
            }
        }
    }

    if (true == m_taskProxy.receive(msg))
    {
        switch(msg.type)
        {
        case MSG_TYPE_INVALID:
            /* Should never happen. */
            break;

        case MSG_TYPE_RSP:
            if (nullptr != msg.rsp)
            {
                handleWebResponse(*msg.rsp);
                delete msg.rsp;
                msg.rsp = nullptr;
            }
            break;

        default:
            /* Should never happen. */
            break;
        }
    }
}

void BTCQuotePlugin::update(YAGfx& gfx)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_view.update(gfx);
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

bool BTCQuotePlugin::startHttpRequest()
{
    bool    status  = false;
    String  url     = String("http://api.coindesk.com/v1/bpi/currentprice/USD.json");

    if (true == m_client.begin(url))
    {
        if (false == m_client.GET())
        {
            LOG_WARNING("GET %s failed.", url.c_str());
        }
        else
        {
            LOG_INFO("GET %s success.", url.c_str());
            status = true;
        }
    }

    return status;
}

void BTCQuotePlugin::initHttpClient()
{
    /* Note: All registered callbacks are running in a different task context!
     *       Therefore it is not allowed to access a member here directly.
     *       The processing must be deferred via task proxy.
     */
    m_client.regOnResponse(
        [this](const HttpResponse& rsp)
        {
            handleAsyncWebResponse(rsp);
        }
    );
}

void BTCQuotePlugin::handleAsyncWebResponse(const HttpResponse& rsp)
{
    if (HttpStatus::STATUS_CODE_OK == rsp.getStatusCode())
    {
        const size_t            JSON_DOC_SIZE   = 512U;
        DynamicJsonDocument*    jsonDoc         = new(std::nothrow) DynamicJsonDocument(JSON_DOC_SIZE);

        if (nullptr != jsonDoc)
        {
            bool                            isSuccessful    = false;
            size_t                          payloadSize     = 0U;
            const void*                     vPayload        = rsp.getPayload(payloadSize);
            const char*                     payload         = static_cast<const char*>(vPayload);
            const size_t                    FILTER_SIZE     = 128U;
            StaticJsonDocument<FILTER_SIZE> jsonFilterDoc;

            jsonFilterDoc["bpi"]["USD"]["rate_float"]   = true;
            jsonFilterDoc["bpi"]["USD"]["rate"]         = true;

            if (true == jsonFilterDoc.overflowed())
            {
                LOG_ERROR("Less memory for filter available.");
            }
            else if ((nullptr == payload) ||
                     (0U == payloadSize))
            {
                LOG_ERROR("No payload.");
            }
            else
            {
                DeserializationError error = deserializeJson(*jsonDoc, payload, payloadSize, DeserializationOption::Filter(jsonFilterDoc));

                if (DeserializationError::Ok != error.code())
                {
                    LOG_ERROR("Invalid JSON message received: %s", error.c_str());
                }
                else
                {
                    Msg msg;

                    msg.type    = MSG_TYPE_RSP;
                    msg.rsp     = jsonDoc;

                    isSuccessful = this->m_taskProxy.send(msg);
                }
            }

            if (false == isSuccessful)
            {
                delete jsonDoc;
                jsonDoc = nullptr;
            }
        }
    }
}

void BTCQuotePlugin::handleWebResponse(const DynamicJsonDocument& jsonDoc)
{
    JsonVariantConst    jsonBpi     = jsonDoc["bpi"];
    JsonVariantConst    jsonUsd     = jsonBpi["USD"];
    JsonVariantConst    jsonRate    = jsonUsd["rate"];

    if (false == jsonRate.isNull())
    {
        m_relevantResponsePart = jsonRate.as<String>() + " $/BTC";
        m_relevantResponsePart.replace(",", "'");  /* Beautify to european(?) standard formatting ' for 1000s */
        
        LOG_INFO("BTC/USD to print %s", m_relevantResponsePart.c_str());

        m_view.setFormatText(m_relevantResponsePart);
    }
}

void BTCQuotePlugin::clearQueue()
{
    Msg msg;

    while(true == m_taskProxy.receive(msg))
    {
        if (MSG_TYPE_RSP == msg.type)
        {
            delete msg.rsp;
            msg.rsp = nullptr;
        }
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
