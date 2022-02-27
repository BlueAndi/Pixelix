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

void VolumioPlugin::start(uint16_t width, uint16_t height)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_iconCanvas.setPosAndSize(0, 0, ICON_WIDTH, ICON_HEIGHT);

    (void)m_iconCanvas.addWidget(m_stdIconWidget);
    (void)m_iconCanvas.addWidget(m_stopIconWidget);
    (void)m_iconCanvas.addWidget(m_playIconWidget);
    (void)m_iconCanvas.addWidget(m_pauseIconWidget);

    /* Load all icons from filesystem now, to prevent filesystem
     * access during active/inactive/update methods.
     */
    (void)m_stdIconWidget.load(FILESYSTEM, IMAGE_PATH_STD_ICON);
    (void)m_stopIconWidget.load(FILESYSTEM, IMAGE_PATH_STOP_ICON);
    (void)m_playIconWidget.load(FILESYSTEM, IMAGE_PATH_PLAY_ICON);
    (void)m_pauseIconWidget.load(FILESYSTEM, IMAGE_PATH_PAUSE_ICON);

    /* Disable all, except the standard icon. */
    m_stopIconWidget.disable();
    m_playIconWidget.disable();
    m_pauseIconWidget.disable();

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
    if (false == startHttpRequest())
    {
        /* If a request fails, show standard icon and a '?' */
        changeState(STATE_UNKNOWN);
        m_textWidget.setFormatStr("\\calign?");

        m_requestTimer.start(UPDATE_PERIOD_SHORT);
    }
    else
    {
        m_requestTimer.start(UPDATE_PERIOD);
    }

    m_offlineTimer.start(OFFLINE_PERIOD);

    return;
}

void VolumioPlugin::stop()
{
    String                      configurationFilename = getFullPathToConfiguration();
    MutexGuard<MutexRecursive>  guard(m_mutex);

    m_offlineTimer.stop();
    m_requestTimer.stop();

    if (false != FILESYSTEM.remove(configurationFilename))
    {
        LOG_INFO("File %s removed", configurationFilename.c_str());
    }

    return;
}

void VolumioPlugin::process()
{
    Msg                         msg;
    MutexGuard<MutexRecursive>  guard(m_mutex);

    if ((true == m_requestTimer.isTimerRunning()) &&
        (true == m_requestTimer.isTimeout()))
    {
        if (false == startHttpRequest())
        {
            /* If a request fails, show standard icon and a '?' */
            changeState(STATE_UNKNOWN);
            m_textWidget.setFormatStr("\\calign?");

            m_requestTimer.start(UPDATE_PERIOD_SHORT);
        }
        else
        {
            m_requestTimer.start(UPDATE_PERIOD);
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

        case MSG_TYPE_CONN_CLOSED:
            LOG_INFO("Connection closed.");

            if (true == m_isConnectionError)
            {
                /* If a request fails, show standard icon and a '?' */
                changeState(STATE_UNKNOWN);
                m_textWidget.setFormatStr("\\calign?");

                m_requestTimer.start(UPDATE_PERIOD_SHORT);
            }
            m_isConnectionError = false;
            break;

        case MSG_TYPE_CONN_ERROR:
            LOG_WARNING("Connection error.");
            m_isConnectionError = true;
            break;

        default:
            /* Should never happen. */
            break;
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

    return;
}

void VolumioPlugin::update(YAGfx& gfx)
{
    MutexGuard<MutexRecursive>  guard(m_mutex);
    int16_t                     tcX         = 0;
    int16_t                     tcY         = 0;
    uint16_t                    posWidth    = m_textCanvas.getWidth() * m_pos / 100U;
    Color                       posColor    = ColorDef::RED;

    gfx.fillScreen(ColorDef::BLACK);
    m_iconCanvas.update(gfx);

    m_textCanvas.getPos(tcX, tcY);
    m_textCanvas.update(gfx);

    /* Draw a nice line to represent the current music position. */
    gfx.drawHLine(tcX, m_textCanvas.getHeight() - 1, posWidth, posColor);

    return;
}

String VolumioPlugin::getHost() const
{
    String                      host;
    MutexGuard<MutexRecursive>  guard(m_mutex);

    host = m_volumioHost;

    return host;
}

void VolumioPlugin::setHost(const String& host)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    if (host != m_volumioHost)
    {
        m_volumioHost = host;
        (void)saveConfiguration();
    }

    return;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void VolumioPlugin::changeState(VolumioState state)
{
    /* Disable current icon */
    switch(m_state)
    {
    case STATE_UNKNOWN:
        m_stdIconWidget.disable();
        break;

    case STATE_STOP:
        m_stopIconWidget.disable();
        break;

    case STATE_PLAY:
        m_playIconWidget.disable();
        break;

    case STATE_PAUSE:
        m_pauseIconWidget.disable();
        break;

    default:
        break;
    }

    /* Enable new icon */
    switch(state)
    {
    case STATE_UNKNOWN:
        m_stdIconWidget.enable();
        break;

    case STATE_STOP:
        m_stopIconWidget.enable();
        break;

    case STATE_PLAY:
        m_playIconWidget.enable();
        break;

    case STATE_PAUSE:
        m_pauseIconWidget.enable();
        break;
        
    default:
        break;
    }

    m_state = state;
}

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

    m_client.regOnClosed(
        [this]()
        {
            Msg msg;

            msg.type = MSG_TYPE_CONN_CLOSED;

            (void)this->m_taskProxy.send(msg);
        }
    );

    m_client.regOnError(
        [this]()
        {
            Msg msg;

            msg.type = MSG_TYPE_CONN_ERROR;

            (void)this->m_taskProxy.send(msg);
        }
    );
}

void VolumioPlugin::handleWebResponse(DynamicJsonDocument& jsonDoc)
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
        String          status          = jsonDoc["status"].as<String>();
        String          artist;
        String          title           = jsonDoc["title"].as<String>();
        uint32_t        seekValue       = jsonDoc["seek"].as<uint32_t>();
        String          service         = jsonDoc["service"].as<String>();
        String          infoOnDisplay;
        uint32_t        pos             = 0U;
        VolumioState    state           = STATE_UNKNOWN;

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
            state = STATE_STOP;
        }
        else if (status == "play")
        {
            state = STATE_PLAY;
        }
        else if (status == "pause")
        {
            state = STATE_PAUSE;
        }
        else
        {
            state = STATE_UNKNOWN;
        }

        changeState(state);
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
    }
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

void VolumioPlugin::clearQueue()
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
