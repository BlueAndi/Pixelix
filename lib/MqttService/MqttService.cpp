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
const char* MqttService::HELLO_WORLD                = "Obi Wan Kenobi";

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
        m_mqttBrokerUrl = m_mqttBrokerUrlSetting.getValue();
        m_hostname      = settings.getHostname().getValue();

        settings.close();

        if (false == m_mqttBrokerUrl.isEmpty())
        {
            m_mqttClient.setServer(m_mqttBrokerUrl.c_str(), MQTT_PORT);
            m_mqttClient.setCallback([this](char* topic, uint8_t* payload, uint32_t length) {
                this->rxCallback(topic, payload, length);
            });
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
}

void MqttService::process()
{
    switch(m_state)
    {
    case STATE_DISCONNECTED:
        if (true == WiFi.isConnected())
        {
            if (true == m_mqttClient.connect(m_hostname.c_str()))
            {
                LOG_INFO("Connection to MQTT broker established.");

                m_state = STATE_CONNECTED;

                (void)m_mqttClient.publish(m_hostname.c_str(), HELLO_WORLD);
            }
        }
        break;

    case STATE_CONNECTED:
        if (false == m_mqttClient.connected())
        {
            LOG_INFO("Connection to MQTT broker disconnected.");
            m_state = STATE_DISCONNECTED;
        }
        break;

    case STATE_IDLE:
        /* Nothing to do. */
        break;

    default:
        break;
    }
}

bool MqttService::publish(const String& topic, const String& msg)
{
    return publish(topic.c_str(), msg.c_str());
}

bool MqttService::publish(const char* topic, const char* msg)
{
    return m_mqttClient.publish(topic, msg);
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void MqttService::rxCallback(char* topic, uint8_t* payload, uint32_t length)
{
    LOG_DEBUG("MQTT Rx: %s", topic);
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
