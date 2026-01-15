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
 * @file   MqttBrokerConnection.cpp
 * @brief  MQTT broker connection handler
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "MqttBrokerConnection.h"

#include <Logging.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>

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

void MqttBrokerConnection::process()
{
    switch (m_state)
    {
    case MqttTypes::STATE_IDLE:
        /* Do nothing */
        break;

    case MqttTypes::STATE_DISCONNECTED:
        disconnectedState();
        break;

    case MqttTypes::STATE_CONNECTED:
        connectedState();
        break;

    default:
        break;
    }
}

bool MqttBrokerConnection::setupClient(bool useTls, const char* rootCaCert, const char* clientCert, const char* clientKey)
{
    bool isSuccessful = false;

    if (nullptr != m_wifiClient)
    {
        disconnect();

        delete m_wifiClient;
        m_wifiClient = nullptr;
    }

    if (nullptr == m_wifiClient)
    {
        if (false == useTls)
        {
            m_wifiClient = new (std::nothrow) WiFiClient();
        }
        else
        {
            WiFiClientSecure* secureClient = new (std::nothrow) WiFiClientSecure();

            if (nullptr != secureClient)
            {
                if (nullptr == rootCaCert)
                {
                    secureClient->setInsecure();
                }
                else
                {
                    secureClient->setCACert(rootCaCert);
                }

                if ((nullptr != clientCert) && (nullptr != clientKey))
                {
                    secureClient->setCertificate(clientCert);
                    secureClient->setPrivateKey(clientKey);
                }

                m_wifiClient = secureClient;
            }
        }

        if (nullptr != m_wifiClient)
        {
            isSuccessful = true;
        }
    }

    return isSuccessful;
}

bool MqttBrokerConnection::connect(const String& clientId, const String& broker, uint16_t port, const String& user, const String& password)
{
    bool isSuccessful = false;

    if (nullptr != m_wifiClient)
    {
        m_clientId = clientId;
        m_url      = broker;
        m_port     = port;
        m_user     = user;
        m_password = password;
        m_state    = MqttTypes::STATE_DISCONNECTED;

        (void)m_mqttClient.setClient(*m_wifiClient);
        (void)m_mqttClient.setServer(m_url.c_str(), m_port);
        (void)m_mqttClient.setCallback([this](char* topic, uint8_t* payload, uint32_t length) {
            this->rxCallback(topic, payload, length);
        });
        (void)m_mqttClient.setSocketTimeout(MQTT_SOCK_TIMEOUT);

        if (true == m_mqttClient.setBufferSize(MAX_BUFFER_SIZE))
        {
            /* The connection establishment takes part during process() method. */
            isSuccessful = true;
        }
    }

    return isSuccessful;
}

void MqttBrokerConnection::disconnect()
{
    if (nullptr != m_wifiClient)
    {
        /* Provide offline status */
        (void)m_mqttClient.publish(m_willTopic.c_str(), m_lastWillPayload.c_str(), true);
        (void)m_mqttClient.disconnect();

        m_state = MqttTypes::STATE_IDLE;
    }
}

MqttTypes::State MqttBrokerConnection::getState() const
{
    return m_state;
}

bool MqttBrokerConnection::publish(const String& topic, const String& msg, bool retained)
{
    return publish(topic.c_str(), msg.c_str(), retained);
}

bool MqttBrokerConnection::publish(const char* topic, const char* msg, bool retained)
{
    bool isSuccessful = false;

    if (nullptr != m_wifiClient)
    {
        isSuccessful = m_mqttClient.publish(topic, msg, retained);
    }

    return isSuccessful;
}

bool MqttBrokerConnection::subscribe(const String& topic, MqttTypes::TopicCallback callback)
{
    return subscribe(topic.c_str(), callback);
}

bool MqttBrokerConnection::subscribe(const char* topic, MqttTypes::TopicCallback callback)
{
    bool isSuccessful = false;

    if ((nullptr != m_wifiClient) && (nullptr != topic))
    {
        SubscriberList::const_iterator it;

        /* Register a topic only once! */
        for (it = m_subscriberList.begin(); it != m_subscriberList.end(); ++it)
        {
            if (0 == strcmp((*it).topic.c_str(), topic))
            {
                break;
            }
        }

        if (it == m_subscriberList.end())
        {
            if (false == m_mqttClient.connected())
            {
                m_subscriberList.emplace_back(topic, callback);
                isSuccessful = true;
            }
            else
            {
                if (false == m_mqttClient.subscribe(topic))
                {
                    LOG_WARNING("MQTT topic subscription not possible: %s", topic);
                }
                else
                {
                    m_subscriberList.emplace_back(topic, callback);
                    isSuccessful = true;
                }
            }
        }
    }

    return isSuccessful;
}

void MqttBrokerConnection::unsubscribe(const String& topic)
{
    unsubscribe(topic.c_str());
}

void MqttBrokerConnection::unsubscribe(const char* topic)
{
    if ((nullptr != m_wifiClient) && (nullptr != topic))
    {
        SubscriberList::iterator it = m_subscriberList.begin();

        while (m_subscriberList.end() != it)
        {
            if (0 == strcmp((*it).topic.c_str(), topic))
            {
                (void)m_mqttClient.unsubscribe((*it).topic.c_str());

                (void)m_subscriberList.erase(it);

                /* Only one subscriber per topic, therefore break. */
                break;
            }

            ++it;
        }
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void MqttBrokerConnection::disconnectedState()
{
    if ((nullptr != m_wifiClient) && (true == WiFi.isConnected()))
    {
        bool connectNow = false;

        /* Connect immediately after service is started initially? */
        if (false == m_reconnectTimer.isTimerRunning())
        {
            connectNow = true;

            m_reconnectTimer.start(RECONNECT_PERIOD);
        }
        else if (true == m_reconnectTimer.isTimeout())
        {
            connectNow = true;
        }
        else
        {
            ;
        }

        if (true == connectNow)
        {
            bool isConnected = false;

            /* Authentication necessary? */
            if (false == m_user.isEmpty())
            {
                LOG_INFO("Connect to %s:%u", m_url.c_str(), m_port);
                LOG_INFO("User     : %s", m_user.c_str());
                LOG_INFO("Client id: %s", m_clientId.c_str());

                isConnected = m_mqttClient.connect(m_clientId.c_str(), m_user.c_str(), m_password.c_str(), m_willTopic.c_str(), 0, true, m_lastWillPayload.c_str());
            }
            /* Connect anonymous */
            else
            {
                LOG_INFO("Connect to %s:%u", m_url.c_str(), m_port);
                LOG_INFO("User     : <anonymous>");
                LOG_INFO("Client id: %s", m_clientId.c_str());

                isConnected = m_mqttClient.connect(m_clientId.c_str(), nullptr, nullptr, m_willTopic.c_str(), 0, true, m_lastWillPayload.c_str());
            }

            /* Connection to broker failed? */
            if (false == isConnected)
            {
                logMqttClientState();

                /* Try to reconnect later. */
                m_reconnectTimer.restart();
            }
            /* Connection to broker successful. */
            else
            {
                LOG_INFO("Connection to MQTT broker established.");

                m_state = MqttTypes::STATE_CONNECTED;
                m_reconnectTimer.stop();

                /* Provide online status */
                (void)m_mqttClient.publish(m_willTopic.c_str(), m_birthPayload.c_str(), true);

                resubscribe();
            }
        }
    }
}

void MqttBrokerConnection::connectedState()
{
    /* Connection with broker lost? */
    if ((nullptr != m_wifiClient) && (false == m_mqttClient.loop()))
    {
        LOG_INFO("Connection to MQTT broker disconnected.");
        m_state = MqttTypes::STATE_DISCONNECTED;

        /* Try to reconnect later. */
        m_reconnectTimer.restart();
    }
}

void MqttBrokerConnection::rxCallback(char* topic, uint8_t* payload, uint32_t length)
{
    SubscriberList::const_iterator it;

    for (it = m_subscriberList.begin(); it != m_subscriberList.end(); ++it)
    {
        if (0 == strcmp((*it).topic.c_str(), topic))
        {
            (*it).callback(topic, payload, length);
            break;
        }
    }
}

void MqttBrokerConnection::resubscribe()
{
    SubscriberList::const_iterator it;

    for (it = m_subscriberList.begin(); it != m_subscriberList.end(); ++it)
    {
        if (false == m_mqttClient.subscribe((*it).topic.c_str()))
        {
            LOG_WARNING("MQTT topic subscription not possible: %s", (*it).topic.c_str());
        }
    }
}

void MqttBrokerConnection::logMqttClientState()
{
    int32_t rc = m_mqttClient.state();

    switch (rc)
    {
    case MQTT_CONNECTION_TIMEOUT:
        LOG_ERROR("MQTT client state: CONNECTION_TIMEOUT");
        break;

    case MQTT_CONNECTION_LOST:
        LOG_ERROR("MQTT client state: CONNECTION_LOST");
        break;

    case MQTT_CONNECT_FAILED:
        LOG_ERROR("MQTT client state: CONNECT_FAILED");
        break;

    case MQTT_DISCONNECTED:
        LOG_INFO("MQTT client state: DISCONNECTED");
        break;

    case MQTT_CONNECTED:
        LOG_INFO("MQTT client state: CONNECTED");
        break;

    case MQTT_CONNECT_BAD_PROTOCOL:
        LOG_ERROR("MQTT client state: CONNECT_BAD_PROTOCOL");
        break;

    case MQTT_CONNECT_BAD_CLIENT_ID:
        LOG_ERROR("MQTT client state: CONNECT_BAD_CLIENT_ID");
        break;

    case MQTT_CONNECT_UNAVAILABLE:
        LOG_ERROR("MQTT client state: CONNECT_UNAVAILABLE");
        break;

    case MQTT_CONNECT_BAD_CREDENTIALS:
        LOG_ERROR("MQTT client state: CONNECT_BAD_CREDENTIALS");
        break;

    case MQTT_CONNECT_UNAUTHORIZED:
        LOG_ERROR("MQTT client state: CONNECT_UNAUTHORIZED");
        break;

    default:
        LOG_ERROR("MQTT client state: UNKNOWN (%d)", rc);
        break;
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
