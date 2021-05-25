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
 * @brief  VOLUMIO plugin
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "VolumioPlugin.h"
#include "RestApi.h"
#include "FileSystem.h"

#include <Logging.h>
#include <ArduinoJson.h>
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

/* Initialize image path for standard icon. */
const char* VolumioPlugin::IMAGE_PATH_STD_ICON      = "/images/volumio.bmp";

/* Initialize image path for "stop" icon. */
const char* VolumioPlugin::IMAGE_PATH_STOP_ICON     = "/images/volumioStop.bmp";

/* Initialize image path for "play" icon. */
const char* VolumioPlugin::IMAGE_PATH_PLAY_ICON     = "/images/volumioPlay.bmp";

/* Initialize image path for "pause" icon. */
const char* VolumioPlugin::IMAGE_PATH_PAUSE_ICON    = "/images/volumioPause.bmp";

/* Initialize plugin topic. */
const char* VolumioPlugin::TOPIC                    = "/host";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void VolumioPlugin::getTopics(JsonArray& topics) const
{
    (void)topics.add(TOPIC);
}

bool VolumioPlugin::getTopic(const String& topic, JsonObject& value) const
{
    bool isSuccessful = false;

    if (0U != topic.equals(TOPIC))
    {
        String  host    = getHost();

        value["host"] = host;

        isSuccessful = true;
    }

    return isSuccessful;
}

bool VolumioPlugin::setTopic(const String& topic, const JsonObject& value)
{
    bool isSuccessful = false;

    if (0U != topic.equals(TOPIC))
    {
        String  host;

        if (false == value["host"].isNull())
        {
            host = value["host"].as<String>();
            isSuccessful = true;
        }

        if (true == isSuccessful)
        {
            setHost(host);
        }
    }

    return isSuccessful;
}


void VolumioPlugin::start()
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
        /* If a request fails, show standard icon and a '?' */
        (void)m_bitmapWidget.load(FILESYSTEM, IMAGE_PATH_STD_ICON);
        m_textWidget.setFormatStr("\\calign?");

        m_requestTimer.start(UPDATE_PERIOD_SHORT);
    }
    else
    {
        m_requestTimer.start(UPDATE_PERIOD);
    }

    m_offlineTimer.start(OFFLINE_PERIOD);

    unlock();

    return;
}

void VolumioPlugin::stop()
{
    String configurationFilename = getFullPathToConfiguration();

    lock();

    m_offlineTimer.stop();
    m_requestTimer.stop();

    if (false != FILESYSTEM.remove(configurationFilename))
    {
        LOG_INFO("File %s removed", configurationFilename.c_str());
    }

    unlock();

    return;
}

void VolumioPlugin::process()
{
    lock();

    if ((true == m_requestTimer.isTimerRunning()) &&
        (true == m_requestTimer.isTimeout()))
    {
        if (false == startHttpRequest())
        {
            /* If a request fails, show standard icon and a '?' */
            (void)m_bitmapWidget.load(FILESYSTEM, IMAGE_PATH_STD_ICON);
            m_textWidget.setFormatStr("\\calign?");

            m_requestTimer.start(UPDATE_PERIOD_SHORT);
        }
        else
        {
            m_requestTimer.start(UPDATE_PERIOD);
        }
    }

    /* If VOLUMIO is offline, disable the plugin. */
    if ((true == m_offlineTimer.isTimerRunning()) &&
        (true == m_offlineTimer.isTimeout()) &&
        (true == isEnabled()))
    {
        LOG_INFO("VOLUMIO not present, going offline.");
        disable();
    }

    unlock();

    return;
}

void VolumioPlugin::active(YAGfx& gfx)
{
    lock();

    gfx.fillScreen(ColorDef::BLACK);

    if (nullptr == m_iconCanvas)
    {
        m_iconCanvas = new Canvas(ICON_WIDTH, ICON_HEIGHT, 0, 0);

        if (nullptr != m_iconCanvas)
        {
            (void)m_iconCanvas->addWidget(m_bitmapWidget);

            /* Load icon from filesystem. */
            (void)m_bitmapWidget.load(FILESYSTEM, IMAGE_PATH_STD_ICON);

            m_iconCanvas->update(gfx);
        }
    }
    else
    {
        m_iconCanvas->update(gfx);
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
    else
    {
        m_textCanvas->update(gfx);
    }

    unlock();

    return;
}

void VolumioPlugin::inactive()
{
    /* Nothing to do. */
    return;
}

void VolumioPlugin::update(YAGfx& gfx)
{
    lock();

    gfx.fillScreen(ColorDef::BLACK);

    if (nullptr != m_iconCanvas)
    {
        m_iconCanvas->update(gfx);
    }

    if (nullptr != m_textCanvas)
    {
        int16_t     tcX         = 0;
        int16_t     tcY         = 0;
        uint16_t    posWidth    = m_textCanvas->getWidth() * m_pos / 100U;
        Color       posColor(ColorDef::RED);

        m_textCanvas->getPos(tcX, tcY);
        m_textCanvas->update(gfx);

        /* Draw a nice line to represent the current music position. */
        gfx.drawHLine(tcX, m_textCanvas->getHeight() - 1, posWidth, posColor);
    }

    unlock();

    return;
}

String VolumioPlugin::getHost() const
{
    String host;

    lock();
    host = m_volumioHost;
    unlock();

    return host;
}

void VolumioPlugin::setHost(const String& host)
{
    lock();
    m_volumioHost = host;
    (void)saveConfiguration();
    unlock();

    return;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

bool VolumioPlugin::startHttpRequest()
{
    bool status = false;

    if (0 < m_volumioHost.length())
    {
        String url = String("http://") + m_volumioHost + "/api/v1/getState";

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

void VolumioPlugin::initHttpClient()
{
    m_client.regOnResponse([this](const HttpResponse& rsp){
        size_t                          payloadSize             = 0U;
        const char*                     payload                 = reinterpret_cast<const char*>(rsp.getPayload(payloadSize));
        const size_t                    JSON_DOC_SIZE           = 512U;
        DynamicJsonDocument             jsonDoc(JSON_DOC_SIZE);
        const size_t                    FILTER_SIZE             = 128U;
        StaticJsonDocument<FILTER_SIZE> filter;
        DeserializationError            error;

        filter["artist"]    = true;
        filter["duration"]  = true;
        filter["seek"]      = true;
        filter["service"]   = true;
        filter["status"]    = true;
        filter["title"]     = true;
        
        if (true == filter.overflowed())
        {
            LOG_ERROR("Less memory for filter available.");
        }

        error = deserializeJson(jsonDoc, payload, payloadSize, DeserializationOption::Filter(filter));

        if (DeserializationError::Ok != error.code())
        {
            LOG_WARNING("JSON parse error: %s", error.c_str());
        }
        else
        {
            if (false == jsonDoc["status"].is<String>())
            {
                LOG_WARNING("JSON status type missmatch or missing.");
            }
            else if (false == jsonDoc["title"].is<String>())
            {
                LOG_WARNING("JSON title type missmatch or missing.");
            }
            else if (false == jsonDoc["seek"].is<uint32_t>())
            {
                LOG_WARNING("JSON seek type missmatch or missing.");
            }
            else if (false == jsonDoc["service"].is<String>())
            {
                LOG_WARNING("JSON service type missmatch or missing.");
            }
            else
            {
                String      status          = jsonDoc["status"].as<String>();
                String      artist;
                String      title           = jsonDoc["title"].as<String>();
                uint32_t    seekValue       = jsonDoc["seek"].as<uint32_t>();
                String      service         = jsonDoc["service"].as<String>();
                String      infoOnDisplay;
                uint32_t    pos             = 0U;

                /* Artist may exist */
                if (true == jsonDoc["artist"].is<String>())
                {
                    artist = jsonDoc["artist"].as<String>();
                }

                if (true == title.isEmpty())
                {
                    title = "\\calign-";
                }

                if (service == "mpd")
                {
                    if (true == artist.isEmpty())
                    {
                        infoOnDisplay = title;
                    }
                    else
                    {
                        infoOnDisplay = artist + " - " + title;
                    }
                }
                else if (service == "webradio")
                {
                    /* If stopped, the title contains the radio station name,
                     * otherwise the title contains the music and the artist
                     * the radio station name.
                     * 
                     * Therefore show only the title in any case.
                     */
                    infoOnDisplay = title;
                }
                else
                {
                    infoOnDisplay = title;
                }

                /* Determine position */
                if (true == jsonDoc["duration"].is<uint32_t>())
                {
                    uint32_t duration = jsonDoc["duration"].as<uint32_t>();

                    if (0U == duration)
                    {
                        pos = 0U;
                    }
                    else
                    {
                        pos = seekValue / duration;
                        pos /= 10U;

                        if (100U < pos)
                        {
                            pos = 100U;
                        }
                    }
                }
                else
                {
                    pos = 0U;
                }

                lock();

                /* Workaround for a VOLUMIO bug, which provides a wrong status. */
                if (status == "stop")
                {
                    if (m_lastSeekValue != seekValue)
                    {
                        status = "play";
                    }
                }
                m_lastSeekValue = seekValue;

                if (status == "stop")
                {
                    (void)m_bitmapWidget.load(FILESYSTEM, IMAGE_PATH_STOP_ICON);
                }
                else if (status == "play")
                {
                    (void)m_bitmapWidget.load(FILESYSTEM, IMAGE_PATH_PLAY_ICON);
                }
                else if (status == "pause")
                {
                    (void)m_bitmapWidget.load(FILESYSTEM, IMAGE_PATH_PAUSE_ICON);
                }
                else
                {
                    (void)m_bitmapWidget.load(FILESYSTEM, IMAGE_PATH_STD_ICON);
                }

                m_textWidget.setFormatStr(infoOnDisplay);

                m_pos = static_cast<uint8_t>(pos);

                /* Feed the offline timer to avoid that the plugin gets disabled. */
                m_offlineTimer.restart();

                /* Enable plugin again, if necessary. */
                if (false == isEnabled())
                {
                    LOG_INFO("VOLUMIO back again, going online.");
                    enable();
                }

                unlock();
            }
        }
    });

    m_client.regOnClosed([this]() {
        LOG_INFO("Connection closed.");

        lock();
        if (true == m_isConnectionError)
        {
            /* If a request fails, show standard icon and a '?' */
            (void)m_bitmapWidget.load(FILESYSTEM, IMAGE_PATH_STD_ICON);
            m_textWidget.setFormatStr("\\calign?");

            m_requestTimer.start(UPDATE_PERIOD_SHORT);
        }
        m_isConnectionError = false;
        unlock();
    });

    m_client.regOnError([this]() {
        LOG_WARNING("Connection error happened.");

        lock();
        m_isConnectionError = true;
        unlock();
    });
}

bool VolumioPlugin::saveConfiguration() const
{
    bool                status                  = true;
    JsonFile            jsonFile(FILESYSTEM);
    const size_t        JSON_DOC_SIZE           = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    String              configurationFilename   = getFullPathToConfiguration();

    jsonDoc["host"] = m_volumioHost;
    
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

bool VolumioPlugin::loadConfiguration()
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
    else if (false == jsonDoc["host"].is<String>())
    {
        LOG_WARNING("Host not found or invalid type.");
        status = false;
    }
    else
    {
        m_volumioHost = jsonDoc["host"].as<String>();
    }

    return status;
}

void VolumioPlugin::lock() const
{
    if (nullptr != m_xMutex)
    {
        (void)xSemaphoreTakeRecursive(m_xMutex, portMAX_DELAY);
    }

    return;
}

void VolumioPlugin::unlock() const
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
