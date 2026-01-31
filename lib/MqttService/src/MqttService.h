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
 * @file   MqttService.h
 * @brief  MQTT service
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup MQTT_SERVICE
 *
 * @{
 */

#ifndef MQTT_SERVICE_H
#define MQTT_SERVICE_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <IService.hpp>
#include <Mutex.hpp>

#include "MqttTypes.h"
#include "MqttBrokerConnection.h"
#include "MqttSetting.h"

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * The MQTT service provides access via MQTT.
 */
class MqttService : public IService
{
public:

    /**
     * Get the MQTT service instance.
     *
     * @return MQTT service instance
     */
    static MqttService& getInstance()
    {
        static MqttService instance; /* idiom */

        return instance;
    }

    /**
     * Start the service.
     *
     * @return If successful started, it will return true otherwise false.
     */
    bool start() final;

    /**
     * Stop the service.
     */
    void stop() final;

    /**
     * Process the service.
     */
    void process() final;

    /**
     * Get current MQTT connection state.
     *
     * @param[in] instance MQTT instance index.
     *
     * @return MQTT connection state
     */
    MqttTypes::State getState(uint8_t instance) const;

    /**
     * Publish a message for a topic.
     *
     * @param[in] instance  MQTT instance index.
     * @param[in] topic     Message topic
     * @param[in] msg       Message itself
     * @param[in] retained  Retained message? Default is false.
     *
     * @return If successful published, it will return true otherwise false.
     */
    bool publish(uint8_t instance, const String& topic, const String& msg, bool retained = false);

    /**
     * Publish a message for a topic.
     *
     * @param[in] instance  MQTT instance index.
     * @param[in] topic     Message topic
     * @param[in] msg       Message itself
     * @param[in] retained  Retained message? Default is false.
     *
     * @return If successful published, it will return true otherwise false.
     */
    bool publish(uint8_t instance, const char* topic, const char* msg, bool retained = false);

    /**
     * Subscribe for a topic. The callback will be called every time a message
     * is received for the topic.
     *
     * @param[in] instance  MQTT instance index.
     * @param[in] topic     The topic which to subscribe for.
     * @param[in] callback  The callback which to call for any received topic message.
     *
     * @return If successful subscribed, it will return true otherwise false.
     */
    bool subscribe(uint8_t instance, const String& topic, MqttTypes::TopicCallback callback);

    /**
     * Subscribe for a topic. The callback will be called every time a message
     * is received for the topic.
     *
     * @param[in] instance  MQTT instance index.
     * @param[in] topic     The topic which to subscribe for.
     * @param[in] callback  The callback which to call for any received topic message.
     *
     * @return If successful subscribed, it will return true otherwise false.
     */
    bool subscribe(uint8_t instance, const char* topic, MqttTypes::TopicCallback callback);

    /**
     * Unsubscribe topic.
     *
     * @param[in] instance  MQTT instance index.
     * @param[in] topic     The topic which to unsubscribe.
     */
    void unsubscribe(uint8_t instance, const String& topic);

    /**
     * Unsubscribe topic.
     *
     * @param[in] instance  MQTT instance index.
     * @param[in] topic     The topic which to unsubscribe.
     */
    void unsubscribe(uint8_t instance, const char* topic);

    /**
     * Primary MQTT instance index.
     */
    static const uint8_t PRIMARY_MQTT_INST = 0U;

    /**
     * Maximum MQTT instance count.
     */
    static const uint8_t MAX_MQTT_COUNT    = 1U;

private:

    static const char*   FILE_NAME; /**< File name of the MQTT settings. */
    static const char*   TOPIC;     /**< Topic for MQTT settings. */
    static const char*   ENTITY_ID; /**< Entity id for MQTT settings. */

    Mutex                m_mutex;                             /**< Mutex to protect the settings. */
    String               m_deviceId;                          /**< Device id. */
    MqttSetting          m_settings[MAX_MQTT_COUNT];          /**< MQTT settings. */
    bool                 m_hasSettingsChanged;                /**< Has any MQTT setting changed since last request? */
    MqttBrokerConnection m_brokerConnections[MAX_MQTT_COUNT]; /**< MQTT broker connections. */
    bool                 m_isSettingsTopicRegistered;         /**< Is settings topic registered? */
    bool                 m_isRunning;                         /**< Is service running? */

    /**
     * Constructs the service instance.
     */
    MqttService() :
        IService(),
        m_mutex(),
        m_deviceId(),
        m_settings(),
        m_hasSettingsChanged(true),
        m_brokerConnections(),
        m_isSettingsTopicRegistered(false),
        m_isRunning(false)
    {
    }

    /**
     * Destroys the service instance.
     */
    ~MqttService()
    {
        /* Never called. */
    }

    /* An instance shall not be copied. */
    MqttService(const MqttService& service);
    MqttService& operator=(const MqttService& service);

    /**
     * Clear all MQTT settings.
     */
    void clear();

    /**
     * Load MQTT settings from file.
     *
     * @return If successful, it will return true otherwise false.
     */
    bool loadSettings();

    /**
     * Save MQTT settings to file.
     *
     * @return If successful, it will return true otherwise false.
     */
    bool saveSettings();

    /**
     * Get MQTT settings.
     *
     * @param[in]       topic       The topic name.
     * @param[in,out]   jsonValue   The JSON value.
     *
     * @return If successful, it will return true otherwise false.
     */
    bool getTopic(const String& topic, JsonObject& jsonValue);

    /**
     * Has any MQTT setting changed since last request?
     *
     * @param[in] topic The topic name.
     *
     * @return If changed, it will return true otherwise false.
     */
    bool hasTopicChanged(const String& topic);

    /**
     * Set MQTT settings.
     *
     * @param[in] topic The topic name.
     * @param[in] value The JSON value.
     *
     * @return If successful, it will return true otherwise false.
     */
    bool setTopic(const String& topic, const JsonObjectConst& value);

    /**
     * Connect all MQTT broker connections.
     */
    void connectAllBrokers();

    /**
     * Disconnect all MQTT broker connections.
     */
    void disconnectAllBrokers();
};

/******************************************************************************
 * Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* MQTT_SERVICE_H */

/** @} */