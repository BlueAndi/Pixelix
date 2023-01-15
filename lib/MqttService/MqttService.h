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

#ifndef __MQTT_SERVICE_H__
#define __MQTT_SERVICE_H__

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <IService.hpp>
#include <WiFi.h>
#include <PubSubClient.h>
#include <KeyValueString.h>

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

private:

    /**
     * MQTT service states.
     */
    enum State
    {
        STATE_DISCONNECTED = 0, /**< No connection to a MQTT broker */
        STATE_CONNECTED,        /**< Connected with a MQTT broker */
        STATE_IDLE              /**< Service is idle */
    };

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

    KeyValueString  m_mqttBrokerUrlSetting; /**< URL of the MQTT broker setting */
    String          m_mqttBrokerUrl;        /**< URL of the MQTT broker */
    String          m_hostname;             /**< MQTT hostname */
    WiFiClient      m_wifiClient;           /**< WiFi client */
    PubSubClient    m_mqttClient;           /**< MQTT client */
    State           m_state;                /**< Connection state */

    /**
     * Constructs the service instance.
     */
    MqttService() :
        IService(),
        m_mqttBrokerUrlSetting(KEY_MQTT_BROKER_URL, NAME_MQTT_BROKER_URL, DEFAULT_MQTT_BROKER_URL, MIN_VALUE_MQTT_BROKER_URL, MAX_VALUE_MQTT_BROKER_URL),
        m_mqttBrokerUrl(),
        m_hostname(),
        m_wifiClient(),
        m_mqttClient(m_wifiClient),
        m_state(STATE_DISCONNECTED)
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
     * MQTT receive callback.
     * 
     * @param[in] topic     The topic name.
     * @param[in] payload   The payload of the topic.
     * @param[in] length    Payload length in byte.
     */
    void rxCallback(char* topic, uint8_t* payload, uint32_t length);
};

/******************************************************************************
 * Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __MQTT_SERVICE_H__ */

/** @} */