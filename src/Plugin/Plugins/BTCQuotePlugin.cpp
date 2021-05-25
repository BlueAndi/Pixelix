/* MIT License
 *
 * Copyright (c) 2019 - 2021 Andreas Merkle <web@blue-andi.de>
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
#include "AsyncHttpClient.h"
#include "BTCQuotePlugin.h"
#include "FileSystem.h"

#include <ArduinoJson.h>
#include <Logging.h>
#include <JsonFile.h>

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

/* Initialize image path. */
const char* BTCQuotePlugin::BTC_USD_IMAGE_PATH     = "/images/BTC_USD.bmp";

void BTCQuotePlugin::active(YAGfx& gfx)
{
    lock();

    gfx.fillScreen(ColorDef::BLACK);

    if (nullptr == m_iconCanvas)
    {
        m_iconCanvas = new Canvas(ICON_WIDTH, ICON_HEIGHT, 0, 0);

        if (nullptr != m_iconCanvas)
        {
            (void)m_iconCanvas->addWidget(m_bitmapWidget);

            /* Load  icon from filesystem. */
            (void)m_bitmapWidget.load(FILESYSTEM, BTC_USD_IMAGE_PATH);

            m_iconCanvas->update(gfx);
        }
    }

    if (nullptr == m_textCanvas)
    {
        m_textCanvas = new Canvas(gfx.getWidth() - ICON_WIDTH, gfx.getHeight(), ICON_WIDTH, 0);

        if (nullptr != m_textCanvas)
        {
            (void)m_textCanvas->addWidget(m_textWidget);

            m_textCanvas->update(gfx);
        }
    }

    unlock();

    return;
}

void BTCQuotePlugin::inactive()
{
    return;
}

void BTCQuotePlugin::update(YAGfx& gfx)
{
    lock();

    gfx.fillScreen(ColorDef::BLACK);

    if (nullptr != m_iconCanvas)
    {
        m_iconCanvas->update(gfx);
    }

    if (nullptr != m_textCanvas)
    {
        m_textCanvas->update(gfx);
    }

    unlock();

    return;
}

void BTCQuotePlugin::start()
{
    lock();

    initHttpClient();
    if (false == startHttpRequest())
    {
        m_requestTimer.start(UPDATE_PERIOD_SHORT);
    }
    else
    {
        m_requestTimer.start(UPDATE_PERIOD);
    }

    unlock();

    return;
}

void BTCQuotePlugin::stop()
{
    lock();

    m_requestTimer.stop();

    unlock();

    return;
}

void BTCQuotePlugin::process()
{
    lock();

    if ((true == m_requestTimer.isTimerRunning()) &&
        (true == m_requestTimer.isTimeout()))
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

    unlock();

    return;
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
    m_client.regOnResponse([this](const HttpResponse& rsp){
        size_t                          payloadSize             = 0U;
        const char*                     payload                 = reinterpret_cast<const char*>(rsp.getPayload(payloadSize));
        const size_t                    JSON_DOC_SIZE           = 512U;
        DynamicJsonDocument             jsonDoc(JSON_DOC_SIZE);
        const size_t                    FILTER_SIZE             = 128U;
        StaticJsonDocument<FILTER_SIZE> filter;
        DeserializationError            error;

        filter["bpi"]["USD"]["rate_float"]      = true;
        filter["bpi"]["USD"]["rate"]            = true;

        if (true == filter.overflowed())
        {
            LOG_ERROR("Less memory for filter available.");
        }

        error = deserializeJson(jsonDoc, payload, payloadSize, DeserializationOption::Filter(filter));

        if (DeserializationError::Ok != error.code())
        {
            LOG_ERROR("Invalid JSON message received: %s", error.c_str());
        }
        else
        {
            m_relevantResponsePart = jsonDoc["bpi"]["USD"]["rate"].as<String>() + " $/BTC";
            m_relevantResponsePart.replace(",", "'");               // beautify to european(?) standard formatting ' for 1000s
            
            LOG_INFO("BTC/USD to print %s", m_relevantResponsePart.c_str());

            lock();
            m_textWidget.setFormatStr(m_relevantResponsePart);
            unlock();

            if (true == jsonDoc.overflowed())
            {
                LOG_ERROR("JSON document has less memory available.");
            }
            else
            {
                LOG_INFO("JSON document size: %u", jsonDoc.memoryUsage());
            }
        }
    });
}

void BTCQuotePlugin::lock() const
{
    if (nullptr != m_xMutex)
    {
        (void)xSemaphoreTakeRecursive(m_xMutex, portMAX_DELAY);
    }

    return;
}

void BTCQuotePlugin::unlock() const
{
    if (nullptr != m_xMutex)
    {
        (void)xSemaphoreGiveRecursive(m_xMutex);
    }

    return;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
