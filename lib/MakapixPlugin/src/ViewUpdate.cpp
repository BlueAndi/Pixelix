/* MIT License
 *
 * Copyright (c) 2019 - 2026 Andreas Merkle <web@blue-andi.de>
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
 * @file   ViewUpdate.cpp
 * @brief  View update used to send view updates via MQTT periodically.
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "ViewUpdate.h"
#include <ArduinoJson.h>
#include <MqttService.h>
#include <ClockDrv.h>
#include <Logging.h>
#include "MqttTopic.h"

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

void ViewUpdate::configure(const String& playerKey, uint8_t mqttInstance)
{
    m_playerKey    = playerKey;
    m_mqttInstance = mqttInstance;
}

void ViewUpdate::process()
{
    MqttService& mqttService = MqttService::getInstance();

    if ((MqttTypes::STATE_CONNECTED == mqttService.getState(m_mqttInstance)) &&
        (false == m_playerKey.isEmpty()))
    {
        /* Time to send view update? */
        if ((true == m_viewUpdateTimer.m_timer.isTimerRunning()) &&
            (true == m_viewUpdateTimer.isTimeout()))
        {
            if (true == notifyViewUpdate())
            {
                m_viewUpdateTimer.startNext();
            }
        }
    }
}

void ViewUpdate::setPostId(uint32_t postId)
{
    if (m_artworkPostId != postId)
    {
        m_artworkPostId = postId;
        m_viewUpdateTimer.start();
    }
}

void ViewUpdate::pause()
{
    m_viewUpdateTimer.pause();
}

void ViewUpdate::play()
{
    m_viewUpdateTimer.play();
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

bool ViewUpdate::notifyViewUpdate()
{
    bool                isSuccessful  = false;
    const size_t        JSON_DOC_SIZE = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    JsonObject          jsonObj = jsonDoc.to<JsonObject>();
    String              payload;
    struct tm           tmUtc = { 0 };
    char                tUtcIso8601[25U];

    LOG_INFO("Notify view update for post id %u.", m_artworkPostId);

    (void)ClockDrv::getInstance().getTimeUtc(tmUtc);

    /* Format as ISO 8601 (YYYY-MM-DDThh:mm:ssZ) */
    strftime(tUtcIso8601, sizeof(tUtcIso8601), "%Y-%m-%dT%H:%M:%SZ", &tmUtc);

    jsonObj["player_key"] = m_playerKey;
    jsonObj["post_id"]    = m_artworkPostId;
    jsonObj["timestamp"]  = tUtcIso8601;
    jsonObj["timezone"]   = ""; /* Reserved, shall be empty. */
    jsonObj["intent"]     = "channel";
    jsonObj["play_order"] = m_channel.getSortOrder();
    jsonObj["channel"]    = m_channel.getChannelName();

    if (Channel::CHANNEL_ID_BY_USER == m_channel.getChannelId())
    {
        jsonObj["channel_user_sqid"] = m_channel.getUserSqid();
    }
    else if (Channel::CHANNEL_ID_HASHTAG == m_channel.getChannelId())
    {
        jsonObj["channel_hashtag"] = m_channel.getHashtag();
    }
    else
    {
        /* Nothing to do. */
        ;
    }

    jsonObj["request_ack"] = true;

    if (0U < serializeJson(jsonObj, payload))
    {
        MqttService& mqttService = MqttService::getInstance();
        String       viewUpdateTopic;

        MqttTopic::getViewUpdateTopic(m_playerKey, viewUpdateTopic);

        if (true == mqttService.publish(m_mqttInstance, viewUpdateTopic.c_str(), payload.c_str()))
        {
            isSuccessful = true;
        }
    }

    return isSuccessful;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
