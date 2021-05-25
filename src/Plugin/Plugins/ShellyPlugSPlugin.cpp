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
        String  ipAddress;

        if (false == value["set"].isNull())
        {
            ipAddress = value["set"].as<String>();
            isSuccessful = true;
        }

        if (true == isSuccessful)
        {
            setIPAddress(ipAddress);
        }
    }

    return isSuccessful;
}

void ShellyPlugSPlugin::active(YAGfx& gfx)
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
            (void)m_bitmapWidget.load(FILESYSTEM, IMAGE_PATH);

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

void ShellyPlugSPlugin::inactive()
{
    return;
}

void ShellyPlugSPlugin::update(YAGfx& gfx)
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

void ShellyPlugSPlugin::start()
{
    lock();

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

void ShellyPlugSPlugin::stop()
{
    String configurationFilename = getFullPathToConfiguration();

    lock();

    m_requestTimer.stop();

    if (false != FILESYSTEM.remove(configurationFilename))
    {
        LOG_INFO("File %s removed", configurationFilename.c_str());
    }

    unlock();

    return;
}

void ShellyPlugSPlugin::process()
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

void ShellyPlugSPlugin::setIPAddress(const String& ipAddress)
{
    lock();

    if (ipAddress != m_ipAddress)
    {
        m_ipAddress = ipAddress;

        (void)saveConfiguration();
    }
    unlock();

    return;
}

String ShellyPlugSPlugin::getIPAddress() const
{
    String ipAddress;

    lock();
    ipAddress = m_ipAddress;
    unlock();

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
    bool    status  = false;
    String  url     = String("http://") + m_ipAddress + "/meter/0/";
    wl_status_t connectionStatus    = WiFi.status();

    if (WL_CONNECTED == connectionStatus)
    {
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
    m_client.regOnResponse([this](const HttpResponse& rsp){
        size_t                          payloadSize             = 0U;
        const char*                     payload                 = reinterpret_cast<const char*>(rsp.getPayload(payloadSize));
        const size_t                    JSON_DOC_SIZE           = 512U;
        DynamicJsonDocument             jsonDoc(JSON_DOC_SIZE);
        const size_t                    FILTER_SIZE             = 128U;
        StaticJsonDocument<FILTER_SIZE> filter;
        DeserializationError            error;

        filter["power"] = true;

        if (true == filter.overflowed())
        {
            LOG_ERROR("Less memory for filter available.");
        }
        
        error = deserializeJson(jsonDoc, payload, payloadSize, DeserializationOption::Filter(filter));

        if (DeserializationError::Ok != error.code())
        {
            LOG_WARNING("JSON parse error: %s", error.c_str());
        }
        else if (false == jsonDoc["power"].is<float>())
        {
            LOG_WARNING("JSON power type missmatch or missing.");
        }
        else
        {
            float       powerRaw                = jsonDoc["power"].as<float>();
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
            
            lock();
            m_textWidget.setFormatStr(power);
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

    m_client.regOnError([this]() {
        LOG_WARNING("Connection error happened.");
    });
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
    else if (false == jsonDoc["shellyPlugSIP"].is<String>())
    {
        LOG_WARNING("shellyPlugSIP not found or invalid type.");
        status = false;
    }
    else
    {
        m_ipAddress = jsonDoc["shellyPlugSIP"].as<String>();
    }

    return status;
}

void ShellyPlugSPlugin::lock() const
{
    if (nullptr != m_xMutex)
    {
        (void)xSemaphoreTakeRecursive(m_xMutex, portMAX_DELAY);
    }

    return;
}

void ShellyPlugSPlugin::unlock() const
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
