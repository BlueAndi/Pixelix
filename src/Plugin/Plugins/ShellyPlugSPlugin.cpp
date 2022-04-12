/* MIT License
 *
 * Copyright (c) 2019 - 2022 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Shelly PlugS plugin.
 * @author Yann Le Glaz <yann_le@web.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "AsyncHttpClient.h"
#include "ClockDrv.h"
#include "Settings.h"
#include "ShellyPlugSPlugin.h"
#include "RestApi.h"
#include "time.h"
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
const char* ShellyPlugSPlugin::IMAGE_PATH   = "/images/plug.bmp";

/* Initialize plugin topic. */
const char* ShellyPlugSPlugin::TOPIC        = "/ipAddress";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void ShellyPlugSPlugin::getTopics(JsonArray& topics) const
{
    (void)topics.add(TOPIC);
}

bool ShellyPlugSPlugin::getTopic(const String& topic, JsonObject& value) const
{
    bool isSuccessful = false;

    if (0U != topic.equals(TOPIC))
    {
        String  ipAddress   = getIPAddress();

        value["ipAddress"] = ipAddress;

        isSuccessful = true;
    }

    return isSuccessful;
}

bool ShellyPlugSPlugin::setTopic(const String& topic, const JsonObject& value)
{
    bool isSuccessful = false;

    if (0U != topic.equals(TOPIC))
    {
        String      ipAddress;
        JsonVariant jsonSet     = value["set"];

        if (false == jsonSet.isNull())
        {
            ipAddress = jsonSet.as<String>();
            isSuccessful = true;
        }

        if (true == isSuccessful)
        {
            setIPAddress(ipAddress);
        }
    }

    return isSuccessful;
}

void ShellyPlugSPlugin::start(uint16_t width, uint16_t height)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_iconCanvas.setPosAndSize(0, 0, ICON_WIDTH, ICON_HEIGHT);
    (void)m_iconCanvas.addWidget(m_bitmapWidget);

    (void)m_bitmapWidget.load(FILESYSTEM, IMAGE_PATH);

    m_textCanvas.setPosAndSize(ICON_WIDTH, 0, width - ICON_WIDTH, height);
    (void)m_textCanvas.addWidget(m_textWidget);

    /* Try to load configuration. If there is no configuration available, a default configuration
     * will be created.
     */
    if (false == loadConfiguration())
    {
        if (false == saveConfiguration())
        {
            LOG_WARNING("Failed to create initial configuration file %s.", getFullPathToConfiguration().c_str());
        }
    }

    initHttpClient();

    return;
}

void ShellyPlugSPlugin::stop()
{
    String                      configurationFilename = getFullPathToConfiguration();
    MutexGuard<MutexRecursive>  guard(m_mutex);

    m_requestTimer.stop();

    if (false != FILESYSTEM.remove(configurationFilename))
    {
        LOG_INFO("File %s removed", configurationFilename.c_str());
    }

    return;
}

void ShellyPlugSPlugin::process(bool isConnected)
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

    return;
}

void ShellyPlugSPlugin::update(YAGfx& gfx)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    gfx.fillScreen(ColorDef::BLACK);
    m_iconCanvas.update(gfx);
    m_textCanvas.update(gfx);
    
    return;
}

void ShellyPlugSPlugin::setIPAddress(const String& ipAddress)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    if (ipAddress != m_ipAddress)
    {
        m_ipAddress = ipAddress;

        (void)saveConfiguration();
    }

    return;
}

String ShellyPlugSPlugin::getIPAddress() const
{
    String                      ipAddress;
    MutexGuard<MutexRecursive>  guard(m_mutex);

    ipAddress = m_ipAddress;

    return ipAddress;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

bool ShellyPlugSPlugin::startHttpRequest()
{
    bool status = false;

    if (false == m_ipAddress.isEmpty())
    {
        String url = String("http://") + m_ipAddress + "/meter/0/";

        if (true == m_client.begin(url))
        {
            if (false == m_client.GET())
            {
                LOG_WARNING("GET %s failed.", url.c_str());
            }
            else
            {
                status = true;
            }
        }
    }

    return status;
}

void ShellyPlugSPlugin::initHttpClient()
{
    /* Note: All registered callbacks are running in a different task context!
     *       Therefore it is not allowed to access a member here directly.
     *       The processing must be deferred via task proxy.
     */
    m_client.regOnResponse(
        [this](const HttpResponse& rsp)
        {
            const size_t            JSON_DOC_SIZE   = 512U;
            DynamicJsonDocument*    jsonDoc         = new(std::nothrow) DynamicJsonDocument(JSON_DOC_SIZE);

            if (nullptr != jsonDoc)
            {
                size_t                          payloadSize = 0U;
                const char*                     payload     = reinterpret_cast<const char*>(rsp.getPayload(payloadSize));
                const size_t                    FILTER_SIZE = 128U;
                StaticJsonDocument<FILTER_SIZE> filter;
                DeserializationError            error;

                filter["power"] = true;

                if (true == filter.overflowed())
                {
                    LOG_ERROR("Less memory for filter available.");
                }
                
                error = deserializeJson(*jsonDoc, payload, payloadSize, DeserializationOption::Filter(filter));

                if (DeserializationError::Ok != error.code())
                {
                    LOG_WARNING("JSON parse error: %s", error.c_str());
                }
                else
                {
                    Msg msg;

                    msg.type    = MSG_TYPE_RSP;
                    msg.rsp     = jsonDoc;

                    if (false == this->m_taskProxy.send(msg))
                    {
                        delete jsonDoc;
                        jsonDoc = nullptr;
                    }
                }
            }
        }
    );
}

void ShellyPlugSPlugin::handleWebResponse(DynamicJsonDocument& jsonDoc)
{
    JsonVariant jsonPower = jsonDoc["power"];

    if (false == jsonPower.is<float>())
    {
        LOG_WARNING("JSON power type missmatch or missing.");
    }
    else
    {
        float       powerRaw                = jsonPower.as<float>();
        String      power;
        const char* reducePrecision;
        char        powerReducedPrecison[6] = { 0 };

        if (powerRaw < 99.99f)
        {
            reducePrecision = (powerRaw > 9.9f) ? "%.1f" : "%.2f";
        }
        else
        {
            reducePrecision = "%.0f";
        }
        
        (void)snprintf(powerReducedPrecison, sizeof(powerReducedPrecison), reducePrecision, powerRaw);

        power = "\\calign";
        power += powerReducedPrecison;
        power += " W";
        
        m_textWidget.setFormatStr(power);
    }
}

bool ShellyPlugSPlugin::saveConfiguration() const
{
    bool                status                  = true;
    JsonFile            jsonFile(FILESYSTEM);
    const size_t        JSON_DOC_SIZE           = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    String              configurationFilename   = getFullPathToConfiguration();

    jsonDoc["shellyPlugSIP"] = m_ipAddress;
    
    if (false == jsonFile.save(configurationFilename, jsonDoc))
    {
        LOG_WARNING("Failed to save file %s.", configurationFilename.c_str());
        status = false;
    }
    else
    {
        LOG_INFO("File %s saved.", configurationFilename.c_str());
    }

    return status;
}

bool ShellyPlugSPlugin::loadConfiguration()
{
    bool                status                  = true;
    JsonFile            jsonFile(FILESYSTEM);
    const size_t        JSON_DOC_SIZE           = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    String              configurationFilename   = getFullPathToConfiguration();

    if (false == jsonFile.load(configurationFilename, jsonDoc))
    {
        LOG_WARNING("Failed to load file %s.", configurationFilename.c_str());
        status = false;
    }
    else
    {
        JsonVariant jsonIP  = jsonDoc["shellyPlugSIP"];

        if (false == jsonIP.is<String>())
        {
            LOG_WARNING("shellyPlugSIP not found or invalid type.");
            status = false;
        }
        else
        {
            m_ipAddress = jsonIP.as<String>();
        }
    }

    return status;
}

void ShellyPlugSPlugin::clearQueue()
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
