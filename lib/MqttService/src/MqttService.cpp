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
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "MqttService.h"

#include <Logging.h>
#include <SettingsService.h>

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

/* Initialize MQTT service variables */
const char* MqttService::KEY_MQTT_BROKER_URL        = "mqtt_broker_url";
const char* MqttService::NAME_MQTT_BROKER_URL       = "MQTT broker URL";
const char* MqttService::DEFAULT_MQTT_BROKER_URL    = "";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

bool MqttService::start()
{
    bool                isSuccessful    = true;
    SettingsService&    settings        = SettingsService::getInstance();

    if (false == settings.registerSetting(&m_mqttBrokerUrlSetting))
    {
        LOG_ERROR("Couldn't register MQTT broker URL setting.");
        isSuccessful = false;
    }
    else if (false == settings.open(true))
    {
        LOG_ERROR("Couldn't open settings.");
        isSuccessful = false;
    }
    else
    {
        String mqttBrokerUrl = m_mqttBrokerUrlSetting.getValue();

        /* Determine URL, user and password. */
        parseMqttBrokerUrl(mqttBrokerUrl);

        m_hostname = settings.getHostname().getValue();

        settings.close();

        if (false == m_url.isEmpty())
        {
            (void)m_mqttClient.setServer(m_url.c_str(), MQTT_PORT);
            (void)m_mqttClient.setCallback([this](char* topic, uint8_t* payload, uint32_t length) {
                this->rxCallback(topic, payload, length);
            });
            (void)m_mqttClient.setBufferSize(MAX_BUFFER_SIZE);

            m_state = STATE_DISCONNECTED;
        }
        else
        {
            m_state = STATE_IDLE;
        }
    }

    if (false == isSuccessful)
    {
        stop();
    }
    else
    {
        LOG_INFO("MQTT service started.");
    }

    return isSuccessful;
}

void MqttService::stop()
{
    SettingsService& settings = SettingsService::getInstance();

    settings.unregisterSetting(&m_mqttBrokerUrlSetting);
    m_mqttClient.disconnect();
    m_state = STATE_IDLE;
    m_reconnectTimer.stop();

    LOG_INFO("MQTT service stopped.");
}

void MqttService::process()
{
    switch(m_state)
    {
    case STATE_DISCONNECTED:
        disconnectedState();
        break;

    case STATE_CONNECTED:
        connectedState();
        break;

    case STATE_IDLE:
        idleState();
        break;

    default:
        break;
    }
}

MqttService::State MqttService::getState() const
{
    return m_state;
}

bool MqttService::publish(const String& topic, const String& msg)
{
    return publish(topic.c_str(), msg.c_str());
}

bool MqttService::publish(const char* topic, const char* msg)
{
    return m_mqttClient.publish(topic, msg);
}

bool MqttService::subscribe(const String& topic, TopicCallback callback)
{
    return subscribe(topic.c_str(), callback);
}

bool MqttService::subscribe(const char* topic, TopicCallback callback)
{
    bool isSuccessful = false;

    if (nullptr != topic)
    {
        SubscriberList::const_iterator it;

        /* Register a topic only once! */
        for(it = m_subscriberList.begin(); it != m_subscriberList.end(); ++it)
        {
            if (nullptr != (*it))
            {
                if (0 == strcmp((*it)->topic.c_str(), topic))
                {
                    break;
                }
            }
        }

        if (it == m_subscriberList.end())
        {
            Subscriber* subscriber = new(std::nothrow) Subscriber;

            if (nullptr != subscriber)
            {
                subscriber->topic       = topic;
                subscriber->callback    = callback;

                if (false == m_mqttClient.connected())
                {
                    m_subscriberList.push_back(subscriber);
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
                        m_subscriberList.push_back(subscriber);
                        isSuccessful = true;
                    }
                }

                if (false == isSuccessful)
                {
                    delete subscriber;
                    subscriber = nullptr;
                }
            }
        }
    }

    return isSuccessful;
}

void MqttService::unsubscribe(const String& topic)
{
    unsubscribe(topic.c_str());
}

void MqttService::unsubscribe(const char* topic)
{
    if (nullptr != topic)
    {
        SubscriberList::iterator it = m_subscriberList.begin();

        while(m_subscriberList.end() != it)
        {
            if (nullptr != (*it))
            {
                if (0 == strcmp((*it)->topic.c_str(), topic))
                {
                    Subscriber* subscriber = *it;

                    m_mqttClient.unsubscribe(subscriber->topic.c_str());

                    (void)m_subscriberList.erase(it);
                    delete subscriber;

                    break;
                }
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

void MqttService::disconnectedState()
{
    if (true == WiFi.isConnected())
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
            bool    isConnected = false;
            String  willTopic   = m_hostname + "/status";

            /* Authentication necessary? */
            if (false == m_user.isEmpty())
            {
                LOG_INFO("Connect to %s as %s with %s.", m_url.c_str(), m_user.c_str(), m_hostname.c_str());

                isConnected = m_mqttClient.connect(m_hostname.c_str(), m_user.c_str(), m_password.c_str(), willTopic.c_str(), 0, true, "offline");
            }
            /* Connect anonymous */
            else
            {
                LOG_INFO("Connect anyonymous to %s with %s.", m_url.c_str(), m_hostname.c_str());

                isConnected = m_mqttClient.connect(m_hostname.c_str(), nullptr, nullptr, willTopic.c_str(), 0, true, "offline");
            }

            /* Connection to broker failed? */
            if (false == isConnected)
            {
                /* Try to reconnect later. */
                m_reconnectTimer.restart();
            }
            /* Connection to broker successful. */
            else
            {
                LOG_INFO("Connection to MQTT broker established.");

                m_state = STATE_CONNECTED;
                m_reconnectTimer.stop();

                /* Provide online status */
                (void)m_mqttClient.publish(willTopic.c_str(), "online", true);
                
                resubscribe();
            }
        }
    }
}

void MqttService::connectedState()
{
    /* Connection with broker lost? */
    if (false == m_mqttClient.loop())
    {
        LOG_INFO("Connection to MQTT broker disconnected.");
        m_state = STATE_DISCONNECTED;

        /* Try to reconnect later. */
        m_reconnectTimer.restart();
    }
}

void MqttService::idleState()
{
    /* Nothing to do. */
}

void MqttService::rxCallback(char* topic, uint8_t* payload, uint32_t length)
{
    SubscriberList::const_iterator it;

    for(it = m_subscriberList.begin(); it != m_subscriberList.end(); ++it)
    {
        if (nullptr != (*it))
        {
            if (0 == strcmp((*it)->topic.c_str(), topic))
            {
                Subscriber* subscriber = *it;

                subscriber->callback(topic, payload, length);
                break;
            }
        }
    }
}

void MqttService::resubscribe()
{
    SubscriberList::const_iterator it;

    for(it = m_subscriberList.begin(); it != m_subscriberList.end(); ++it)
    {
        if (nullptr != (*it))
        {
            Subscriber* subscriber = *it;

            if (false == m_mqttClient.subscribe(subscriber->topic.c_str()))
            {
                LOG_WARNING("MQTT topic subscription not possible: %s", subscriber->topic.c_str());
            }
        }
    }
}

void MqttService::parseMqttBrokerUrl(const String& mqttBrokerUrl)
{
    int32_t idx = mqttBrokerUrl.indexOf("://");

    /* The MQTT broker URL format:
     * [mqtt://][<USER>:<PASSWORD>@]<BROKER-URL>
     */
    m_url = mqttBrokerUrl;

    /* Remove protocol, we don't care about. */
    if (0 <= idx)
    {
        m_url.remove(0U, idx + 3);
    }

    /* User and passwort */
    idx = m_url.indexOf("@");

    m_user.clear();
    m_password.clear();

    if (0 <= idx)
    {
        int32_t dividerIdx = m_url.indexOf(":");

        /* Only user name with empty password? */
        if (0 > dividerIdx)
        {
            m_user = m_url.substring(0U, idx);
        }
        /* At least one character for a user name must exist. */
        else if (0 < dividerIdx)
        {
            m_user = m_url.substring(0U, dividerIdx);

            /* Password not empty? */
            if (idx > (dividerIdx + 1))
            {
                m_password = m_url.substring(dividerIdx + 1, idx);
            }
        }

        m_url.remove(0U, idx + 1);
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
