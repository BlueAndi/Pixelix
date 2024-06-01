/* MIT License
 *
 * Copyright (c) 2019 - 2024 Andreas Merkle <web@blue-andi.de>
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

/* Initialize plugin topic. */
const char* VolumioPlugin::TOPIC_CONFIG = "/host";

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
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_view.init(width, height);

    PluginWithConfig::start(width, height);

    initHttpClient();

    m_offlineTimer.start(OFFLINE_PERIOD);
}

void VolumioPlugin::stop()
{
    MutexGuard<MutexRecursive>  guard(m_mutex);

    m_offlineTimer.stop();
    m_requestTimer.stop();

    PluginWithConfig::stop();
}

void VolumioPlugin::process(bool isConnected)
{
    Msg                         msg;
    MutexGuard<MutexRecursive>  guard(m_mutex);

    PluginWithConfig::process(isConnected);
    
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
                m_view.setFormatText("\\calign?");

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
                m_view.setFormatText("\\calign?");

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
                m_view.setFormatText("\\calign?");

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
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_view.getProgressBar().setProgress(m_pos);
    m_view.update(gfx);
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

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
    switch(state)
    {
    case STATE_UNKNOWN:
        m_view.showIcon(_VolumioPlugin::View::ICON_STD);
        break;

    case STATE_STOP:
        m_view.showIcon(_VolumioPlugin::View::ICON_STOP);
        break;

    case STATE_PLAY:
        m_view.showIcon(_VolumioPlugin::View::ICON_PLAY);
        break;

    case STATE_PAUSE:
        m_view.showIcon(_VolumioPlugin::View::ICON_PAUSE);
        break;
        
    default:
        m_view.showIcon(_VolumioPlugin::View::ICON_STD);
        state = STATE_UNKNOWN;
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
            StaticJsonDocument<FILTER_SIZE> jsonFilterDoc;

            jsonFilterDoc["artist"]     = true;
            jsonFilterDoc["duration"]   = true;
            jsonFilterDoc["seek"]       = true;
            jsonFilterDoc["service"]    = true;
            jsonFilterDoc["status"]     = true;
            jsonFilterDoc["title"]      = true;
            
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
        m_view.setFormatText(infoOnDisplay);

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
