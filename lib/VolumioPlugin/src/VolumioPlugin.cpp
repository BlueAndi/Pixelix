/* MIT License
 *
 * Copyright (c) 2019 - 2023 Andreas Merkle <web@blue-andi.de>
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

#include <Logging.h>
#include <ArduinoJson.h>
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

/* Initialize image path for standard icon. */
const char* VolumioPlugin::IMAGE_PATH_STD_ICON      = "/plugins/VolumioPlugin/volumio.bmp";

/* Initialize image path for "stop" icon. */
const char* VolumioPlugin::IMAGE_PATH_STOP_ICON     = "/plugins/VolumioPlugin/volumioStop.bmp";

/* Initialize image path for "play" icon. */
const char* VolumioPlugin::IMAGE_PATH_PLAY_ICON     = "/plugins/VolumioPlugin/volumioPlay.bmp";

/* Initialize image path for "pause" icon. */
const char* VolumioPlugin::IMAGE_PATH_PAUSE_ICON    = "/plugins/VolumioPlugin/volumioPause.bmp";

/* Initialize plugin topic. */
const char* VolumioPlugin::TOPIC_CONFIG             = "/host";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void VolumioPlugin::getTopics(JsonArray& topics) const
{
    (void)topics.add(TOPIC_CONFIG);
}

bool VolumioPlugin::getTopic(const String& topic, JsonObject& value) const
{
    bool isSuccessful = false;

    if (0U != topic.equals(TOPIC_CONFIG))
    {
        getConfiguration(value);
        isSuccessful = true;
    }

    return isSuccessful;
}

bool VolumioPlugin::setTopic(const String& topic, const JsonObjectConst& value)
{
    bool isSuccessful = false;

    if (0U != topic.equals(TOPIC_CONFIG))
    {
        const size_t        JSON_DOC_SIZE           = 512U;
        DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
        JsonObject          jsonCfg                 = jsonDoc.to<JsonObject>();
        JsonVariantConst    jsonHost                = value["host"];

        /* The received configuration may not contain all single key/value pair.
         * Therefore read first the complete internal configuration and
         * overwrite them with the received ones.
         */
        getConfiguration(jsonCfg);

        /* Note:
         * Check only for the key/value pair availability.
         * The type check will follow in the setConfiguration().
         */

        if (false == jsonHost.isNull())
        {
            jsonCfg["host"] = jsonHost.as<String>();
            isSuccessful = true;
        }

        if (true == isSuccessful)
        {
            JsonObjectConst jsonCfgConst = jsonCfg;

            isSuccessful = setConfiguration(jsonCfgConst);

            if (true == isSuccessful)
            {
                requestStoreToPersistentMemory();
            }
        }
    }

    return isSuccessful;
}

bool VolumioPlugin::hasTopicChanged(const String& topic)
{
    MutexGuard<MutexRecursive>  guard(m_mutex);
    bool                        hasTopicChanged = m_hasTopicChanged;

    /* Only a single topic, therefore its not necessary to check. */
    PLUGIN_NOT_USED(topic);

    m_hasTopicChanged = false;

    return hasTopicChanged;
}

void VolumioPlugin::start(uint16_t width, uint16_t height)
{
    uint16_t                    tcHeight        = 0U;
    MutexGuard<MutexRecursive>  guard(m_mutex);

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

    /* The text canvas is left aligned to the icon canvas and aligned to the
     * top. Consider that below the text canvas the music position is shown.
     */
    tcHeight = height - 2U;
    m_textCanvas.setPosAndSize(ICON_WIDTH, 0, width - ICON_WIDTH, tcHeight);
    (void)m_textCanvas.addWidget(m_textWidget);

    /* The text widget inside the text canvas is left aligned on x-axis and
     * aligned to the center of y-axis.
     */
    if (tcHeight > m_textWidget.getFont().getHeight())
    {
        uint16_t diffY = height - m_textWidget.getFont().getHeight();
        uint16_t offsY = diffY / 2U;

        m_textWidget.move(0, offsY);
    }

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
    else
    {
        /* Remember current timestamp to detect updates of the configuration in the
         * filesystem without using the plugin API.
         */
        updateTimestampLastUpdate();
    }

    m_cfgReloadTimer.start(CFG_RELOAD_PERIOD);

    initHttpClient();

    m_offlineTimer.start(OFFLINE_PERIOD);
}

void VolumioPlugin::stop()
{
    String                      configurationFilename = getFullPathToConfiguration();
    MutexGuard<MutexRecursive>  guard(m_mutex);

    m_cfgReloadTimer.stop();
    m_offlineTimer.stop();
    m_requestTimer.stop();

    if (false != FILESYSTEM.remove(configurationFilename))
    {
        LOG_INFO("File %s removed", configurationFilename.c_str());
    }
}

void VolumioPlugin::process(bool isConnected)
{
    Msg                         msg;
    MutexGuard<MutexRecursive>  guard(m_mutex);

    /* Configuration in persistent memory updated? */
    if ((true == m_cfgReloadTimer.isTimerRunning()) &&
        (true == m_cfgReloadTimer.isTimeout()))
    {
        if (true == isConfigurationUpdated())
        {
            m_reloadConfigReq = true;
        }

        m_cfgReloadTimer.restart();
    }

    if (true == m_storeConfigReq)
    {
        if (false == saveConfiguration())
        {
            LOG_WARNING("Failed to save configuration: %s", getFullPathToConfiguration().c_str());
        }

        m_storeConfigReq = false;
    }
    else if (true == m_reloadConfigReq)
    {
        LOG_INFO("Reload configuration: %s", getFullPathToConfiguration().c_str());

        if (true == loadConfiguration())
        {
            updateTimestampLastUpdate();
        }

        m_reloadConfigReq = false;
    }
    else
    {
        ;
    }
    
    /* Only if a network connection is established the required information
     * shall be periodically requested via REST API.
     */
    if (false == m_requestTimer.isTimerRunning())
    {
        if (true == isConnected)
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
    gfx.drawHLine(tcX, gfx.getHeight() - 1, posWidth, posColor);
    PLUGIN_NOT_USED(tcY);
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void VolumioPlugin::requestStoreToPersistentMemory()
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_storeConfigReq = true;
}

void VolumioPlugin::getConfiguration(JsonObject& jsonCfg) const
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    jsonCfg["host"] = m_volumioHost;
}

bool VolumioPlugin::setConfiguration(JsonObjectConst& jsonCfg)
{
    bool                status      = false;
    JsonVariantConst    jsonHost    = jsonCfg["host"];

    if (false == jsonHost.is<String>())
    {
        LOG_WARNING("Host not found or invalid type.");
    }
    else
    {
        MutexGuard<MutexRecursive> guard(m_mutex);

        m_volumioHost = jsonHost.as<String>();

        /* Force update on display */
        m_requestTimer.start(UPDATE_PERIOD_SHORT);

        m_hasTopicChanged = true;

        status = true;
    }

    return status;
}

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

    if (false == m_volumioHost.isEmpty())
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
            handleAsyncWebResponse(rsp);
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

void VolumioPlugin::handleAsyncWebResponse(const HttpResponse& rsp)
{
    if (HttpStatus::STATUS_CODE_OK == rsp.getStatusCode())
    {
        const size_t            JSON_DOC_SIZE   = 512U;
        DynamicJsonDocument*    jsonDoc         = new(std::nothrow) DynamicJsonDocument(JSON_DOC_SIZE);

        if (nullptr != jsonDoc)
        {
            size_t                          payloadSize = 0U;
            const void*                     vPayload    = rsp.getPayload(payloadSize);
            const char*                     payload     = static_cast<const char*>(vPayload);
            const size_t                    FILTER_SIZE = 128U;
            StaticJsonDocument<FILTER_SIZE> filter;

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
            else if ((nullptr == payload) ||
                     (0U == payloadSize))
            {
                LOG_ERROR("No payload.");
            }
            else
            {
                DeserializationError error = deserializeJson(*jsonDoc, payload, payloadSize, DeserializationOption::Filter(filter));

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
    }
}

void VolumioPlugin::handleWebResponse(DynamicJsonDocument& jsonDoc)
{
    JsonVariantConst    jsonStatus  = jsonDoc["status"];
    JsonVariantConst    jsonTitle   = jsonDoc["title"];
    JsonVariantConst    jsonSeek    = jsonDoc["seek"];
    JsonVariantConst    jsonService = jsonDoc["service"];

    if (false == jsonStatus.is<String>())
    {
        LOG_WARNING("JSON status type mismatch or missing.");
    }
    else if (false == jsonTitle.is<String>())
    {
        LOG_WARNING("JSON title type mismatch or missing.");
    }
    else if (false == jsonSeek.is<uint32_t>())
    {
        LOG_WARNING("JSON seek type mismatch or missing.");
    }
    else if (false == jsonService.is<String>())
    {
        LOG_WARNING("JSON service type mismatch or missing.");
    }
    else
    {
        JsonVariantConst    jsonArtist      = jsonDoc["artist"];
        JsonVariantConst    jsonDuration    = jsonDoc["duration"];
        String              status          = jsonStatus.as<String>();
        String              artist;
        String              title           = jsonTitle.as<String>();
        uint32_t            seekValue       = jsonSeek.as<uint32_t>();
        String              service         = jsonService.as<String>();
        String              infoOnDisplay;
        uint32_t            pos             = 0U;
        VolumioState        state           = STATE_UNKNOWN;

        /* Artist may exist */
        if (true == jsonArtist.is<String>())
        {
            artist = jsonArtist.as<String>();
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
        if (true == jsonDuration.is<uint32_t>())
        {
            uint32_t duration = jsonDuration.as<uint32_t>();

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
