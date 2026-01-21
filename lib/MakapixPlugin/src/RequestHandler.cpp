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
 * @file   RequestHandler.cpp
 * @brief  Makapix request handler
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "RequestHandler.h"
#include <MqttService.h>
#include <Logging.h>
#include <ArduinoJson.h>
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

void RequestHandler::init(MakapixOnResponseCallback onResponseCallback)
{
    m_onResponseCallback = onResponseCallback;
}

void RequestHandler::configure(const String& playerKey, uint8_t mqttInstance)
{
    /* Abort any pending request. */
    abort();

    m_playerKey    = playerKey;
    m_mqttInstance = mqttInstance;
}

void RequestHandler::process()
{
    /* Check for request timeout. */
    if ((true == m_requestTimer.isTimerRunning()) &&
        (true == m_requestTimer.isTimeout()))
    {
        MqttService& mqttService = MqttService::getInstance();
        String       responseTopic;

        MqttTopic::getResponseTopic(m_playerKey, responseTopic, m_lastRequestId);

        LOG_WARNING("Request timeout occurred.");

        /* Remove response subscription on timeout. */
        LOG_DEBUG("Unsubscribe from %s", responseTopic.c_str());
        (void)mqttService.unsubscribe(m_mqttInstance, responseTopic.c_str());
        m_requestTimer.stop();
    }
}

bool RequestHandler::request(const char* channelName, const char* userHandle, const char* sortOrder, uint32_t page, uint32_t limit)
{
    bool isSuccessful = false;

    if ((nullptr != channelName) &&
        (nullptr != sortOrder))
    {

        MqttService& mqttService = MqttService::getInstance();
        String       responseTopic;

        LOG_INFO("Request page %u with limit %u for channel %s", page, limit, channelName);

        /* Abort any pending request. */
        abort();

        ++m_lastRequestId;
        MqttTopic::getResponseTopic(m_playerKey, responseTopic, m_lastRequestId);

        LOG_DEBUG("Subscribe to %s", responseTopic.c_str());

        /* First subscribe for the possible response. */
        isSuccessful = mqttService.subscribe(
            m_mqttInstance,
            responseTopic.c_str(),
            [this](const String& topic, const uint8_t* payload, size_t size) {
                this->mqttTopicCallback(topic, payload, size);
            });

        if (false == isSuccessful)
        {
            --m_lastRequestId;
        }
        else
        {
            const size_t        JSON_DOC_SIZE = 2048U;
            DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
            String              requestTopic;
            JsonArray           jsonCriteria           = jsonDoc.createNestedArray("criteria");
            JsonObject          jsonCriteriaWidthMax   = jsonCriteria.createNestedObject();
            JsonObject          jsonCriteriaHeightMax  = jsonCriteria.createNestedObject();
            JsonObject          jsonCriteriaFileFormat = jsonCriteria.createNestedObject();
            JsonArray           jsonFileFormats;
            String              requestPayload;

            LOG_DEBUG("Send request with id: %u", m_lastRequestId);

            MqttTopic::getRequestTopic(m_playerKey, requestTopic, m_lastRequestId);

            jsonDoc["request_id"]   = String(m_lastRequestId);
            jsonDoc["request_type"] = "query_posts";
            jsonDoc["player_key"]   = m_playerKey;
            jsonDoc["channel"]      = channelName;

            if (nullptr != userHandle)
            {
                jsonDoc["user_handle"] = userHandle;
            }

            jsonDoc["sort"] = sortOrder;

            if (0 == strcmp(sortOrder, "random"))
            {
                jsonDoc["random_seed"] = millis();
            }

            jsonDoc["cursor"]               = String(page);
            jsonDoc["limit"]                = limit;

            jsonCriteriaWidthMax["field"]   = "width";
            jsonCriteriaWidthMax["op"]      = "lte";
            jsonCriteriaWidthMax["value"]   = CONFIG_LED_MATRIX_WIDTH;

            jsonCriteriaHeightMax["field"]  = "height";
            jsonCriteriaHeightMax["op"]     = "lte";
            jsonCriteriaHeightMax["value"]  = CONFIG_LED_MATRIX_HEIGHT;

            jsonCriteriaFileFormat["field"] = "file_format";
            jsonCriteriaFileFormat["op"]    = "in";
            jsonFileFormats                 = jsonCriteriaFileFormat.createNestedArray("value");
            (void)jsonFileFormats.add("bmp");
            (void)jsonFileFormats.add("gif");

            if (0U < serializeJson(jsonDoc.as<JsonObject>(), requestPayload))
            {
                m_requestTimer.start(REQUEST_TIMEOUT);

                isSuccessful = mqttService.publish(m_mqttInstance, requestTopic.c_str(), requestPayload.c_str());
            }

            if (false == isSuccessful)
            {
                LOG_DEBUG("Unsubscribe from %s", responseTopic.c_str());

                /* Remove response subscription on failure. */
                (void)mqttService.unsubscribe(m_mqttInstance, responseTopic.c_str());
                m_requestTimer.stop();
            }
        }
    }

    return isSuccessful;
}

void RequestHandler::abort()
{
    /* Any pending request? */
    if (true == m_requestTimer.isTimerRunning())
    {
        MqttService& mqttService = MqttService::getInstance();
        String       pendingResponseTopic;

        MqttTopic::getResponseTopic(m_playerKey, pendingResponseTopic, m_lastRequestId);

        /* Abort it by removing subscription. */
        (void)mqttService.unsubscribe(m_mqttInstance, pendingResponseTopic.c_str());
        m_requestTimer.stop();
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void RequestHandler::mqttTopicCallback(const String& topic, const uint8_t* payload, size_t size)
{
    MqttService& mqttService = MqttService::getInstance();
    String       expectedTopicPrefix;

    m_requestTimer.stop();

    MqttTopic::getTopicPrefix(m_playerKey, expectedTopicPrefix);

    if (false == topic.startsWith(expectedTopicPrefix))
    {
        LOG_WARNING("Received MQTT topic does not match expected prefix.");
    }
    else
    {
        const size_t         JSON_DOC_SIZE = 2048U;
        DynamicJsonDocument  jsonDoc(JSON_DOC_SIZE);
        DeserializationError error = deserializeJson(jsonDoc, payload, size);

        if (DeserializationError::Ok != error)
        {
            LOG_WARNING("MQTT payload contains invalid JSON.");
        }
        else if (nullptr == m_onResponseCallback)
        {
            LOG_WARNING("No response callback registered.");
        }
        else
        {
            m_onResponseCallback(jsonDoc);
        }
    }

    LOG_DEBUG("Unsubscribe from %s", topic.c_str());
    (void)mqttService.unsubscribe(m_mqttInstance, topic.c_str());
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
