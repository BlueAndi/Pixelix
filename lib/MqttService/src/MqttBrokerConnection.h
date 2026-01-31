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
 * @file   MqttBrokerConnection.h
 * @brief  MQTT broker connection handler
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup MQTT_SERVICE
 *
 * @{
 */

#ifndef MQTT_BROKER_CONNECTION_H
#define MQTT_BROKER_CONNECTION_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <IService.hpp>
#include <PubSubClient.h>
#include <KeyValueString.h>
#include <vector>
#include <SimpleTimer.hpp>
#include <WiFiClient.h>
#include "MqttTypes.h"

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
 * The MQTT broker connection handler. It manages the connection to a MQTT broker.
 * Provides publish and subscribe functionality. The subscribe functionality
 * supports one subscriber per topic.
 */
class MqttBrokerConnection
{
public:

    /**
     * Constructs the MqttBrokerConnection instance.
     */
    MqttBrokerConnection() :
        m_clientId(),
        m_url(),
        m_user(),
        m_password(),
        m_port(MQTT_PORT),
        m_willTopic(),
        m_birthPayload(),
        m_lastWillPayload(),
        m_wifiClient(nullptr),
        m_mqttClient(),
        m_state(MqttTypes::STATE_IDLE),
        m_subscriberList(),
        m_reconnectTimer()
    {
    }

    /**
     * Destroys the MqttBrokerConnection instance.
     */
    ~MqttBrokerConnection()
    {
        disconnect();

        if (nullptr != m_wifiClient)
        {
            delete m_wifiClient;
            m_wifiClient = nullptr;
        }
    }

    /**
     * Process the connection by calling periodically this method.
     */
    void process();

    /**
     * Set will topic and payloads for the MQTT birth and last will message.
     * Must be set before connect() is called.
     *
     * @param[in] willTopic         Will topic.
     * @param[in] birthPayload      Birth payload.
     * @param[in] lastWillPayload   Last will payload.
     */
    void setLastWillTopic(const String& willTopic, const String& birthPayload, const String& lastWillPayload)
    {
        m_willTopic       = willTopic;
        m_birthPayload    = birthPayload;
        m_lastWillPayload = lastWillPayload;
    }

    /**
     * Setup the MQTT client with TLS configuration.
     * Must be called before connect() is called.
     * If a connection is already established, it will be disconnected.
     *
     * @param[in] useTls        Use TLS connection.
     * @param[in] rootCaCert    Root CA certificate.
     * @param[in] clientCert    Client certificate.
     * @param[in] clientKey     Client key.
     *
     * @return If successful, it will return true otherwise false.
     */
    bool setupClient(bool useTls, const char* rootCaCert, const char* clientCert, const char* clientKey);

    /**
     * Connect to the MQTT broker.
     *
     * @param[in] clientId      The MQTT client identifier.
     * @param[in] broker        The MQTT broker host.
     * @param[in] port          The MQTT broker port. Default is 1883.
     * @param[in] user          The user name. Default is empty.
     * @param[in] password      The password. Default is empty.
     *
     * @return If successful connected, it will return true otherwise false.
     */
    bool connect(const String& clientId, const String& broker, uint16_t port = MQTT_PORT, const String& user = "", const String& password = "");

    /**
     * Disconnect from the MQTT broker.
     */
    void disconnect();

    /**
     * Get current MQTT connection state.
     *
     * @return MQTT connection state
     */
    MqttTypes::State getState() const;

    /**
     * Publish a message for a topic.
     *
     * @param[in] topic     Message topic
     * @param[in] msg       Message itself
     * @param[in] retained  Retained message? Default is false.
     *
     * @return If successful published, it will return true otherwise false.
     */
    bool publish(const String& topic, const String& msg, bool retained = false);

    /**
     * Publish a message for a topic.
     *
     * @param[in] topic     Message topic
     * @param[in] msg       Message itself
     * @param[in] retained  Retained message? Default is false.
     *
     * @return If successful published, it will return true otherwise false.
     */
    bool publish(const char* topic, const char* msg, bool retained = false);

    /**
     * Subscribe for a topic. The callback will be called every time a message
     * is received for the topic.
     *
     * @param[in] topic     The topic which to subscribe for.
     * @param[in] callback  The callback which to call for any received topic message.
     *
     * @return If successful subscribed, it will return true otherwise false.
     */
    bool subscribe(const String& topic, MqttTypes::TopicCallback callback);

    /**
     * Subscribe for a topic. The callback will be called every time a message
     * is received for the topic.
     *
     * @param[in] topic     The topic which to subscribe for.
     * @param[in] callback  The callback which to call for any received topic message.
     *
     * @return If successful subscribed, it will return true otherwise false.
     */
    bool subscribe(const char* topic, MqttTypes::TopicCallback callback);

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
        String                   topic;    /**< The subscriber topic */
        MqttTypes::TopicCallback callback; /**< The subscriber callback */

        /**
         * Constructs the Subscriber instance.
         *
         * @param[in] topic     The subscriber topic.
         * @param[in] callback  The subscriber callback.
         */
        Subscriber(const String& topic, MqttTypes::TopicCallback callback) :
            topic(topic),
            callback(callback)
        {
        }

        /**
         * Default constructor is not supported.
         */
        Subscriber() = delete;
    };

    /**
     * This type defines a list of subscribers.
     */
    typedef std::vector<Subscriber> SubscriberList;

    /** MQTT port */
    static const uint16_t MQTT_PORT         = 1883U;

    /**
     * MQTT socket timeout in s. Keep it low to improve systems responsiveness.
     * Otherwise a reconnect may take a while and blocks any other activities.
     * The same for reading and writing to the socket.
     */
    static const uint16_t MQTT_SOCK_TIMEOUT = 1U;

    /**
     * Reconnect period in ms.
     */
    static const uint32_t RECONNECT_PERIOD  = SIMPLE_TIMER_SECONDS(10U);

    /**
     * Max. MQTT client buffer size in byte.
     * Received MQTT messages greather than this will be skipped.
     */
    static const size_t MAX_BUFFER_SIZE     = 2048U;


    String              m_clientId;        /**< MQTT client identifier */
    String              m_url;             /**< URL of the MQTT broker */
    String              m_user;            /**< MQTT authentication: user name */
    String              m_password;        /**< MQTT authentication: password */
    uint16_t            m_port;            /**< MQTT port */
    String              m_willTopic;       /**< Will topic */
    String              m_birthPayload;    /**< Birth payload */
    String              m_lastWillPayload; /**< Last will payload */
    WiFiClient*         m_wifiClient;      /**< WiFi client */
    PubSubClient        m_mqttClient;      /**< MQTT client */
    MqttTypes::State    m_state;           /**< Connection state */
    SubscriberList      m_subscriberList;  /**< List of subscribers */
    SimpleTimer         m_reconnectTimer;  /**< Timer used for periodically reconnecting. */

    /* An instance shall not be copied. */
    MqttBrokerConnection(const MqttBrokerConnection& service);
    MqttBrokerConnection& operator=(const MqttBrokerConnection& service);

    /**
     * Handles the DISCONNECTED state.
     */
    void disconnectedState();

    /**
     * Handles the CONNECTED state.
     */
    void connectedState();

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
     * Log the current MQTT client state.
     */
    void logMqttClientState();
};

/******************************************************************************
 * Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* MQTT_BROKER_CONNECTION_H */

/** @} */