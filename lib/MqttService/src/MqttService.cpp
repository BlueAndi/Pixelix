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
 * @file   MqttService.cpp
 * @brief  MQTT service
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "MqttService.h"

#include <FileSystem.h>
#include <JsonFile.h>
#include <TopicHandlerService.h>
#include <SettingsService.h>
#include <Util.h>
#include <Logging.h>

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

/* Initialize constant values. */
const char* MqttService::FILE_NAME = "/configuration/mqttService.json";
const char* MqttService::TOPIC     = "mqtt";
const char* MqttService::ENTITY_ID = "mqttService";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

bool MqttService::start()
{
    bool isSuccessful = true;

    /* Service already started? */
    if (true == m_isRunning)
    {
        /* Nothing to do. */
        ;
    }
    else if (false == m_mutex.create())
    {
        isSuccessful = false;
    }
    else
    {
        SettingsService& settings = SettingsService::getInstance();

        if (false == settings.open(true))
        {
            m_deviceId = settings.getHostname().getDefault();
        }
        else
        {
            m_deviceId = settings.getHostname().getValue();

            settings.close();
        }

        if (false == loadSettings())
        {
            (void)saveSettings();
        }

        /* Start MQTT broker connections. */
        connectAllBrokers();
    }

    if (false == isSuccessful)
    {
        stop();
    }
    else if (true == m_isRunning)
    {
        LOG_WARNING("MQTT service is already started.");
    }
    else
    {
        m_isRunning = true;
        LOG_INFO("MQTT service started.");
    }

    return isSuccessful;
}

void MqttService::stop()
{
    TopicHandlerService& topicHandlerService = TopicHandlerService::getInstance();

    topicHandlerService.unregisterTopic(m_deviceId, ENTITY_ID, TOPIC);

    /* Stop MQTT broker connections. */
    disconnectAllBrokers();

    m_mutex.destroy();

    if (true == m_isRunning)
    {
        m_isRunning = false;
        LOG_INFO("MQTT service stopped.");
    }
}

void MqttService::process()
{
    if (true == m_isRunning)
    {
        /* Register settings topic if not done yet.
         * This is done here to have the service started before the topic handler
         * service tries to get or set any topic value.
         */
        if (false == m_isSettingsTopicRegistered)
        {
            TopicHandlerService&        topicHandlerService = TopicHandlerService::getInstance();
            JsonObjectConst             jsonExtra; /* Empty */
            ITopicHandler::GetTopicFunc getTopicFunc =
                [this](const String& topic, JsonObject& jsonValue) -> bool {
                return this->getTopic(topic, jsonValue);
            };
            TopicHandlerService::HasChangedFunc hasChangedFunc =
                [this](const String& topic) -> bool {
                return this->hasTopicChanged(topic);
            };
            ITopicHandler::SetTopicFunc setTopicFunc =
                [this](const String& topic, const JsonObjectConst& jsonValue) -> bool {
                return this->setTopic(topic, jsonValue);
            };

            topicHandlerService.registerTopic(m_deviceId, ENTITY_ID, TOPIC, jsonExtra, getTopicFunc, hasChangedFunc, setTopicFunc, nullptr);

            m_isSettingsTopicRegistered = true;
        }

        for (size_t idx = 0U; idx < MAX_MQTT_COUNT; ++idx)
        {
            m_brokerConnections[idx].process();
        }
    }
}

MqttTypes::State MqttService::getState(uint8_t instance) const
{
    MqttTypes::State state = MqttTypes::STATE_IDLE;

    if ((true == m_isRunning) &&
        (MAX_MQTT_COUNT > instance))
    {
        state = m_brokerConnections[instance].getState();
    }

    return state;
}

bool MqttService::publish(uint8_t instance, const String& topic, const String& msg, bool retained)
{
    bool isSuccessful = false;

    if ((true == m_isRunning) &&
        (MAX_MQTT_COUNT > instance))
    {
        isSuccessful = m_brokerConnections[instance].publish(topic, msg, retained);
    }

    return isSuccessful;
}

bool MqttService::publish(uint8_t instance, const char* topic, const char* msg, bool retained)
{
    bool isSuccessful = false;

    if ((true == m_isRunning) &&
        (MAX_MQTT_COUNT > instance))
    {
        isSuccessful = m_brokerConnections[instance].publish(topic, msg, retained);
    }

    return isSuccessful;
}

bool MqttService::subscribe(uint8_t instance, const String& topic, MqttTypes::TopicCallback callback)
{
    bool isSuccessful = false;

    if ((true == m_isRunning) &&
        (MAX_MQTT_COUNT > instance))
    {
        isSuccessful = m_brokerConnections[instance].subscribe(topic, callback);
    }

    return isSuccessful;
}

bool MqttService::subscribe(uint8_t instance, const char* topic, MqttTypes::TopicCallback callback)
{
    bool isSuccessful = false;

    if ((true == m_isRunning) &&
        (MAX_MQTT_COUNT > instance))
    {
        isSuccessful = m_brokerConnections[instance].subscribe(topic, callback);
    }

    return isSuccessful;
}

void MqttService::unsubscribe(uint8_t instance, const String& topic)
{
    if ((true == m_isRunning) &&
        (MAX_MQTT_COUNT > instance))
    {
        m_brokerConnections[instance].unsubscribe(topic);
    }
}

void MqttService::unsubscribe(uint8_t instance, const char* topic)
{
    if ((true == m_isRunning) &&
        (MAX_MQTT_COUNT > instance))
    {
        m_brokerConnections[instance].unsubscribe(topic);
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void MqttService::clear()
{
    size_t idx;

    for (idx = 0U; idx < MAX_MQTT_COUNT; ++idx)
    {
        m_settings[idx].clear();
    }
}

bool MqttService::loadSettings()
{
    bool                isSuccessful = false;
    const size_t        JSON_SIZE    = 8192U;
    DynamicJsonDocument jsonDoc(JSON_SIZE);
    JsonFile            jsonFile(FILESYSTEM);

    if (false == jsonFile.load(FILE_NAME, jsonDoc))
    {
        LOG_WARNING("Failed to load MQTT settings.");
    }
    else
    {
        JsonVariantConst jsonMqttSettings = jsonDoc["mqttSettings"];

        if (false == jsonMqttSettings.is<JsonArrayConst>())
        {
            LOG_ERROR("No MQTT settings found.");
        }
        else
        {
            JsonArrayConst jsonMqttSettingsArray = jsonMqttSettings.as<JsonArrayConst>();
            size_t         idx                   = 0U;

            clear();

            for (JsonObjectConst jsonMqttSetting : jsonMqttSettingsArray)
            {
                if (false == m_settings[idx].fromJson(jsonMqttSetting))
                {
                    LOG_WARNING("Failed to load MQTT setting %u.", idx);
                }
                else
                {
                    ++idx;
                }

                if (MAX_MQTT_COUNT <= idx)
                {
                    break;
                }
            }

            m_hasSettingsChanged = true;

            isSuccessful         = true;
        }
    }

    return isSuccessful;
}

bool MqttService::saveSettings()
{
    bool                isSuccessful = false;
    const size_t        JSON_SIZE    = 8192U;
    DynamicJsonDocument jsonDoc(JSON_SIZE);
    JsonArray           jsonMqttSettings = jsonDoc.createNestedArray("mqttSettings");
    JsonFile            jsonFile(FILESYSTEM);
    size_t              idx;

    for (idx = 0U; idx < MAX_MQTT_COUNT; ++idx)
    {
        JsonObject jsonMqttSetting = jsonMqttSettings.createNestedObject();

        m_settings[idx].toJson(jsonMqttSetting);
    }

    if (false == jsonFile.save(FILE_NAME, jsonDoc))
    {
        LOG_ERROR("Failed to save MQTT settings.");
    }
    else
    {
        isSuccessful = true;
    }

    return isSuccessful;
}

bool MqttService::getTopic(const String& topic, JsonObject& jsonValue)
{
    size_t            idx;
    JsonArray         jsonMqttSettings = jsonValue.createNestedArray("mqttSettings");
    MutexGuard<Mutex> guard(m_mutex);

    /* The callback is dedicated to a topic, therefore the
     * topic parameter is not used.
     */
    UTIL_NOT_USED(topic);

    for (idx = 0U; idx < MAX_MQTT_COUNT; ++idx)
    {
        JsonObject jsonMqttSetting = jsonMqttSettings.createNestedObject();

        m_settings[idx].toJson(jsonMqttSetting);
    }

    return true;
}

bool MqttService::hasTopicChanged(const String& topic)
{
    MutexGuard<Mutex> guard(m_mutex);
    bool              hasChanged = m_hasSettingsChanged;

    m_hasSettingsChanged         = false;

    return hasChanged;
}

bool MqttService::setTopic(const String& topic, const JsonObjectConst& jsonValue)
{
    bool              isSuccessful = false;
    size_t            idx;
    JsonVariantConst  jsonMqttSettings = jsonValue["mqttSettings"];
    MutexGuard<Mutex> guard(m_mutex);

    /* The callback is dedicated to a topic, therefore the
     * topic parameter is not used.
     */
    UTIL_NOT_USED(topic);

    if (true == jsonMqttSettings.is<JsonArrayConst>())
    {
        JsonArrayConst jsonMqttSettingsArray = jsonMqttSettings.as<JsonArrayConst>();
        size_t         count                 = (MAX_MQTT_COUNT >= jsonMqttSettingsArray.size()) ? jsonMqttSettingsArray.size() : MAX_MQTT_COUNT;
        MqttSetting    tempSetting;

        isSuccessful = true;
        for (idx = 0U; idx < count; ++idx)
        {
            if (false == tempSetting.fromJson(jsonMqttSettingsArray[idx]))
            {
                LOG_WARNING("Invalid MQTT setting %u.", idx);
                isSuccessful = false;
                break;
            }

            m_settings[idx] = std::move(tempSetting);
            tempSetting.clear();
        }

        m_hasSettingsChanged = true;
    }

    if (true == isSuccessful)
    {
        isSuccessful = saveSettings();

        if (true == isSuccessful)
        {
            disconnectAllBrokers();
            connectAllBrokers();
        }
    }

    return isSuccessful;
}

void MqttService::connectAllBrokers()
{
    for (size_t idx = 0U; idx < MAX_MQTT_COUNT; ++idx)
    {
        MqttSetting& setting = m_settings[idx];

        if ((true == setting.isEnabled()) &&
            (false == setting.getBroker().isEmpty()))
        {
            m_brokerConnections[idx].setLastWillTopic(m_deviceId + "/status", "online", "offline");

            if (false == m_brokerConnections[idx].setupClient(
                             setting.useTls(),
                             setting.getRootCaCert(),
                             setting.getClientCert(),
                             setting.getClientKey()))
            {
                LOG_WARNING("MQTT client setup for broker %s failed.", setting.getBroker().c_str());
            }
            else if (false == m_brokerConnections[idx].connect(
                                  m_deviceId,
                                  setting.getBroker(),
                                  setting.getPort(),
                                  setting.getUser(),
                                  setting.getPassword()))
            {
                LOG_WARNING("MQTT broker connection to %s failed.", setting.getBroker().c_str());
            }
            else
            {
                ;
            }
        }
    }
}

void MqttService::disconnectAllBrokers()
{
    for (size_t idx = 0U; idx < MAX_MQTT_COUNT; ++idx)
    {
        m_brokerConnections[idx].disconnect();
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
