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
 * @file   CommandHandler.cpp
 * @brief  Makapix command handler
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "CommandHandler.h"

#include <MqttService.h>
#include <Logging.h>
#include <Version.h>
#include <ClockDrv.h>
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

void CommandHandler::init(
    MakapixNextArtworkCallback nextArtworkCallback,
    MakapixPrevArtworkCallback prevArtworkCallback,
    MakapixPlayChannelCallback playChannelCallback,
    MakapixShowArtworkCallback showArtworkCallback)
{
    m_nextArtworkCallback = nextArtworkCallback;
    m_prevArtworkCallback = prevArtworkCallback;
    m_playChannelCallback = playChannelCallback;
    m_showArtworkCallback = showArtworkCallback;
}

void CommandHandler::configure(const String& playerKey, uint8_t mqttInstance)
{
    unsubscribe();

    m_playerKey    = playerKey;
    m_mqttInstance = mqttInstance;
}

void CommandHandler::process()
{
    MqttService&     mqttService  = MqttService::getInstance();
    MqttTypes::State currentState = mqttService.getState(m_mqttInstance);

    /* Handle MQTT states.*/
    switch (currentState)
    {
    case MqttTypes::STATE_IDLE:
        /* Nothing to do. */
        break;

    case MqttTypes::STATE_DISCONNECTED:
        /* Nothing to do. */
        break;

    case MqttTypes::STATE_CONNECTED:
        /* Subscribe to MQTT makapix topics and notify about status immediately after
         * MQTT connection is established.
         */
        if (MqttTypes::STATE_CONNECTED != m_mqttConnectionState)
        {
            subscribe();
            notifyStatusUpdate(true);
            m_statusTimer.start(STATUS_UPDATE_PERIOD);
        }

        /* Time to send periodic status update? */
        if ((true == m_statusTimer.isTimerRunning()) &&
            (true == m_statusTimer.isTimeout()))
        {
            notifyStatusUpdate(true);
            m_statusTimer.restart();
        }

        /* Time to send view update? */
        if ((true == m_viewUpdateTimer.m_timer.isTimerRunning()) &&
            (true == m_viewUpdateTimer.isTimeout()))
        {
            if (true == notifyViewUpdate())
            {
                m_viewUpdateTimer.startNext();
            }
        }
        break;
    };

    m_mqttConnectionState = currentState;
}

void CommandHandler::subscribe()
{
    MqttService& mqttService = MqttService::getInstance();

    if ((MqttTypes::STATE_CONNECTED == mqttService.getState(m_mqttInstance)) &&
        (false == m_playerKey.isEmpty()))
    {
        bool   isSuccessful;
        String commandTopic;

        MqttTopic::getCommandTopic(m_playerKey, commandTopic);

        isSuccessful = mqttService.subscribe(m_mqttInstance,
            commandTopic,
            [this](const String& topic, const uint8_t* payload, size_t size) {
                this->mqttTopicCallback(topic, payload, size);
            });

        if (true == isSuccessful)
        {
            m_statusTimer.start(STATUS_UPDATE_PERIOD);
        }
    }
}

void CommandHandler::unsubscribe()
{
    MqttService& mqttService = MqttService::getInstance();

    m_statusTimer.stop();

    if ((MqttTypes::STATE_CONNECTED == mqttService.getState(m_mqttInstance)) &&
        (false == m_playerKey.isEmpty()))
    {
        String commandTopic;

        MqttTopic::getCommandTopic(m_playerKey, commandTopic);

        notifyStatusUpdate(false);
        mqttService.unsubscribe(m_mqttInstance, commandTopic);
    }
}

void CommandHandler::setPostId(uint32_t postId)
{
    if (m_artworkPostId != postId)
    {
        m_artworkPostId = postId;
        m_viewUpdateTimer.start();
        notifyStatusUpdate(true);
    }
}

bool CommandHandler::notifyStatusUpdate(bool isOnline)
{
    bool         isSuccessful = false;
    MqttService& mqttService  = MqttService::getInstance();

    if ((MqttTypes::STATE_CONNECTED == mqttService.getState(m_mqttInstance)) &&
        (false == m_playerKey.isEmpty()))
    {
        const size_t        JSON_DOC_SIZE = 512U;
        DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
        JsonObject          jsonObj = jsonDoc.to<JsonObject>();
        String              payload;

        jsonObj["player_key"]       = m_playerKey;
        jsonObj["status"]           = (true == isOnline) ? "online" : "offline";
        jsonObj["current_post_id"]  = m_artworkPostId;
        jsonObj["firmware_version"] = Version::getSoftwareVersion();

        if (0U < serializeJson(jsonObj, payload))
        {
            String statusTopic;

            MqttTopic::getStatusTopic(m_playerKey, statusTopic);

            if (true == mqttService.publish(m_mqttInstance, statusTopic.c_str(), payload.c_str(), payload.length()))
            {
                isSuccessful = true;
            }
        }
    }

    return isSuccessful;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void CommandHandler::mqttTopicCallback(const String& topic, const uint8_t* payload, size_t size)
{
    String expectedTopic;

    MqttTopic::getCommandTopic(m_playerKey, expectedTopic);

    if (false == topic.equals(expectedTopic))
    {
        LOG_WARNING("Received MQTT topic does not match expected prefix.");
    }
    else
    {
        const size_t         JSON_DOC_SIZE = 1024U;
        DynamicJsonDocument  jsonDoc(JSON_DOC_SIZE);
        DeserializationError error = deserializeJson(jsonDoc, payload, size);

        if (DeserializationError::Ok != error)
        {
            LOG_WARNING("MQTT payload contains invalid JSON.");
        }
        else
        {
            onCommand(jsonDoc);
        }
    }
}

void CommandHandler::onCommand(const JsonDocument& jsonDoc)
{
    JsonObjectConst  jsonObj         = jsonDoc.as<JsonObjectConst>();
    JsonVariantConst jsonCommandType = jsonObj["command_type"];
    JsonVariantConst jsonPayload     = jsonObj["payload"];

    if (true == jsonCommandType.is<String>())
    {
        const char* commandType = jsonCommandType.as<const char*>();

        if (0 == strcmp(commandType, "swap_next"))
        {
            swapNext();
        }
        else if (0 == strcmp(commandType, "swap_back"))
        {
            swapBack();
        }
        else if (0 == strcmp(commandType, "show_artwork"))
        {
            showArtwork(jsonPayload.as<JsonObjectConst>());
        }
        else if (0 == strcmp(commandType, "play_channel"))
        {
            playChannel(jsonPayload.as<JsonObjectConst>());
        }
        else
        {
            LOG_WARNING("MQTT command_type \"%s\" is unknown.", commandType);
        }
    }
    else
    {
        LOG_WARNING("MQTT command_type not found or invalid type.");
    }
}

void CommandHandler::swapNext()
{
    if (nullptr == m_nextArtworkCallback)
    {
        LOG_WARNING("Next artwork callback not set.");
    }
    else
    {
        m_nextArtworkCallback();
    }
}

void CommandHandler::swapBack()
{
    if (nullptr == m_prevArtworkCallback)
    {
        LOG_WARNING("Previous artwork callback not set.");
    }
    else
    {
        m_prevArtworkCallback();
    }
}

void CommandHandler::showArtwork(const JsonObjectConst& jsonPayload)
{
    JsonVariantConst jsonPostId       = jsonPayload["post_id"];
    JsonVariantConst jsonStorageKey   = jsonPayload["storage_key"];
    JsonVariantConst jsonStorageShard = jsonPayload["storage_shard"];
    JsonVariantConst jsonNativeFormat = jsonPayload["native_format"];
    JsonVariantConst jsonCanvas       = jsonPayload["canvas"];

    if (false == jsonPostId.is<uint32_t>())
    {
        LOG_WARNING("MQTT payload post_id not found or invalid type.");
    }
    else if (false == jsonStorageKey.is<const char*>())
    {
        LOG_WARNING("MQTT payload storage_key not found or invalid type.");
    }
    else if (false == jsonStorageShard.is<const char*>())
    {
        LOG_WARNING("MQTT payload storage_shard not found or invalid type.");
    }
    else if (false == jsonNativeFormat.is<const char*>())
    {
        LOG_WARNING("MQTT payload native_format not found or invalid type.");
    }
    else if (false == jsonCanvas.is<const char*>())
    {
        LOG_WARNING("MQTT payload canvas not found or invalid type.");
    }
    else
    {
        uint32_t    postId       = jsonPostId.as<uint32_t>();
        const char* storageKey   = jsonStorageKey.as<const char*>();
        const char* storageShard = jsonStorageShard.as<const char*>();
        const char* nativeFormat = jsonNativeFormat.as<const char*>();
        const char* canvas       = jsonCanvas.as<const char*>();
        uint16_t    width        = 0U;
        uint16_t    height       = 0U;

        getWidthHeight(canvas, width, height);

        if ((CONFIG_LED_MATRIX_WIDTH < width) ||
            (CONFIG_LED_MATRIX_HEIGHT < height))
        {
            LOG_WARNING("Canvas size %ux%u not supported.", width, height);
        }
        else
        {
            /* Add to internal playlist. */
            int32_t playlistIdx = m_playlist.add(postId, storageKey, storageShard, nativeFormat, DEFAULT_DWELL_TIME_MS, true);

            if (0 > playlistIdx)
            {
                LOG_WARNING("Failed to add artwork to playlist.");
            }
            else
            {
                LOG_INFO("Artwork %s added to playlist.", storageKey);
                LOG_INFO("Select artwork %s in playlist.", storageKey);

                /* Select it in the playlist to show it immediately. */
                m_playlist.select(static_cast<uint8_t>(playlistIdx));

                if (nullptr == m_showArtworkCallback)
                {
                    LOG_WARNING("Show artwork callback not set.");
                }
                else
                {
                    m_showArtworkCallback();
                }
            }
        }
    }
}

void CommandHandler::playChannel(const JsonObjectConst& jsonPayload)
{
    JsonVariantConst jsonChannelName = jsonPayload["channel_name"];

    if (false == jsonChannelName.is<const char*>())
    {
        LOG_WARNING("MQTT payload channel_name not found or invalid type.");
    }
    else if (nullptr == m_playChannelCallback)
    {
        LOG_WARNING("Play channel callback not set.");
    }
    else
    {
        const char* channelName = jsonChannelName.as<const char*>();

        m_playChannelCallback(channelName);
    }
}

void CommandHandler::getWidthHeight(const char* canvas, uint16_t& width, uint16_t& height) const
{
    width  = 0U;
    height = 0U;

    if (nullptr != canvas)
    {
        /* Example: "64x64" */
        char* endPtr = nullptr;
        long  lWidth = strtol(canvas, &endPtr, 10);

        if ((endPtr != canvas) && ('x' == (*endPtr)))
        {
            long lHeight;

            ++endPtr; /* Skip 'x' character. */
            lHeight = strtol(endPtr, nullptr, 10);

            if ((endPtr != canvas) && (0 < lWidth) && (0 < lHeight))
            {
                width  = static_cast<uint16_t>(lWidth);
                height = static_cast<uint16_t>(lHeight);
            }
        }
    }
}

bool CommandHandler::notifyViewUpdate()
{
    bool isSuccessful = false;

    if (false == m_playerKey.isEmpty())
    {
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

        if (0U < serializeJson(jsonObj, payload))
        {
            MqttService& mqttService = MqttService::getInstance();
            String       statusTopic;

            MqttTopic::getStatusTopic(m_playerKey, statusTopic);

            if (true == mqttService.publish(m_mqttInstance, statusTopic.c_str(), payload.c_str(), payload.length()))
            {
                isSuccessful = true;
            }
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
