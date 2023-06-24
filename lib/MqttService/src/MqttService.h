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
 * @brief  MQTT service
 * @author Andreas Merkle <web@blue-andi.de>
 * 
 * @addtogroup service
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
#include <WiFi.h>
#include <PubSubClient.h>
#include <KeyValueString.h>
#include <functional>
#include <vector>
#include <SimpleTimer.hpp>

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
     * Topic callback prototype.
     */
    typedef std::function<void(const String& topic, const uint8_t* payload, size_t size)>    TopicCallback;

    /**
     * MQTT service states.
     */
    enum State
    {
        STATE_DISCONNECTED = 0, /**< No connection to a MQTT broker */
        STATE_CONNECTED,        /**< Connected with a MQTT broker */
        STATE_IDLE              /**< Service is idle */
    };

    /**
     * Get the audio service instance.
     * 
     * @return Audio service instance
     */
    static MqttService& getInstance()
    {
        static MqttService instance; /* idiom */

        return instance;
    }

    /**
     * Start the service.
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
     * @return MQTT connection state
     */
    State getState() const;

    /**
     * Publish a message for a topic.
     * 
     * @param[in] topic Message topic
     * @param[in] msg   Message itself
     * 
     * @return If successful published, it will return true otherwise false.
     */
    bool publish(const String& topic, const String& msg);

    /**
     * Publish a message for a topic.
     * 
     * @param[in] topic Message topic
     * @param[in] msg   Message itself
     * 
     * @return If successful published, it will return true otherwise false.
     */
    bool publish(const char* topic, const char* msg);

    /**
     * Subscribe for a topic. The callback will be called every time a message
     * is received for the topic.
     * 
     * @param[in] topic     The topic which to subscribe for.
     * @param[in] callback  The callback which to call for any received topic message.
     * 
     * @return If successful subscribed, it will return true otherwise false.
     */
    bool subscribe(const String& topic, TopicCallback callback);

    /**
     * Subscribe for a topic. The callback will be called every time a message
     * is received for the topic.
     * 
     * @param[in] topic     The topic which to subscribe for.
     * @param[in] callback  The callback which to call for any received topic message.
     * 
     * @return If successful subscribed, it will return true otherwise false.
     */
    bool subscribe(const char* topic, TopicCallback callback);

    /**
     * Unsubscribe topic.
     * 
     * @param[in] topic The topic which to unsubscribe.
     */
    void unsubscribe(const String& topic);

    /**
     * Unsubscribe topic.
     * 
     * @param[in] topic The topic which to unsubscribe.
     */
    void unsubscribe(const char* topic);

private:

    /**
     * Subscriber information
     */
    struct Subscriber
    {
        String          topic;      /**< The subscriber topic */
        TopicCallback   callback;   /**< The subscriber callback */
    };

    /**
     * This type defines a list of subscribers.
     */
    typedef std::vector<Subscriber*>    SubscriberList;

    /** MQTT port */
    static const uint16_t   MQTT_PORT                   = 1883U;

    /** MQTT broker URL key */
    static const char*      KEY_MQTT_BROKER_URL;

    /** MQTT broker URL name */
    static const char*      NAME_MQTT_BROKER_URL;

    /** MQTT broker URL default value */
    static const char*      DEFAULT_MQTT_BROKER_URL;

    /** MQTT broker URL min. length */
    static const size_t     MIN_VALUE_MQTT_BROKER_URL   = 0U;

    /** MQTT broker URL max. length */
    static const size_t     MAX_VALUE_MQTT_BROKER_URL   = 64U;

    /**
     * Reconnect period in ms.
     */
    static const uint32_t   RECONNECT_PERIOD            = SIMPLE_TIMER_SECONDS(10U);

    /**
     * Max. MQTT client buffer size in byte.
     * Received MQTT messages greather than this will be skipped.
     */
    static const size_t     MAX_BUFFER_SIZE             = 2048U;

    KeyValueString          m_mqttBrokerUrlSetting; /**< URL of the MQTT broker setting */
    String                  m_url;                  /**< URL of the MQTT broker */
    String                  m_user;                 /**< MQTT authentication: user name */
    String                  m_password;             /**< MQTT authentication: password */
    String                  m_hostname;             /**< MQTT hostname */
    WiFiClient              m_wifiClient;           /**< WiFi client */
    PubSubClient            m_mqttClient;           /**< MQTT client */
    State                   m_state;                /**< Connection state */
    SubscriberList          m_subscriberList;       /**< List of subscribers */
    SimpleTimer             m_reconnectTimer;       /**< Timer used for periodically reconnecting. */

    /**
     * Constructs the service instance.
     */
    MqttService() :
        IService(),
        m_mqttBrokerUrlSetting(KEY_MQTT_BROKER_URL, NAME_MQTT_BROKER_URL, DEFAULT_MQTT_BROKER_URL, MIN_VALUE_MQTT_BROKER_URL, MAX_VALUE_MQTT_BROKER_URL),
        m_url(),
        m_user(),
        m_password(),
        m_hostname(),
        m_wifiClient(),
        m_mqttClient(m_wifiClient),
        m_state(STATE_DISCONNECTED),
        m_subscriberList(),
        m_reconnectTimer()
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
     * Handles the DISCONNECTED state.
     */
    void disconnectedState();

    /**
     * Handles the CONNECTED state.
     */
    void connectedState();

    /**
     * Handles the IDLE state.
     */
    void idleState();

    /**
     * MQTT receive callback.
     * 
     * @param[in] topic     The topic name.
     * @param[in] payload   The payload of the topic.
     * @param[in] length    Payload length in byte.
     */
    void rxCallback(char* topic, uint8_t* payload, uint32_t length);

    /**
     * Resubscribe all topics.
     */
    void resubscribe();

    /**
     * Parse MQTT broker URL and derive the raw URL, the user and password.
     * 
     * @param[in] mqttBrokerUrl The MQTT broker URL.
     */
    void parseMqttBrokerUrl(const String& mqttBrokerUrl);
};

/******************************************************************************
 * Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* MQTT_SERVICE_H */

/** @} */