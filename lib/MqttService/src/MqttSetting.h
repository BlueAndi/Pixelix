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
 * @file   MqttSetting.h
 * @brief  MQTT setting
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup MQTT_SERVICE
 *
 * @{
 */

#ifndef MQTT_SETTING_H
#define MQTT_SETTING_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <ArduinoJson.h>

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
 * Single MQTT setting.
 */
class MqttSetting
{
public:

    /**
     * Constructs a MQTT setting.
     */
    MqttSetting();

    /**
     * Copy constructs a MQTT setting.
     *
     * @param[in] other MQTT setting to copy.
     */
    MqttSetting(const MqttSetting& other);

    /**
     * Destroys a MQTT setting.
     */
    ~MqttSetting()
    {
        clear();
    }

    /**
     * Copy assignment operator.
     *
     * @param[in] other MQTT setting to copy.
     *
     * @return Reference to this MQTT setting.
     */
    MqttSetting& operator=(const MqttSetting& other);

    /**
     * Move assignment operator.
     *
     * @param[in] other MQTT setting to move.
     *
     * @return Reference to this MQTT setting.
     */
    MqttSetting& operator=(MqttSetting&& other) noexcept;

    /**
     * Clear MQTT setting to default values.
     */
    void clear();

    /**
     * Convert setting to JSON.
     *
     * @param[out] jsonSetting JSON object destination.
     */
    void toJson(JsonObject& jsonSetting) const;

    /**
     * Convert from JSON to setting.
     *
     * @param[in] jsonSetting  JSON object source.
     *
     * @return If successful, it will return true otherwise false.
     */
    bool fromJson(const JsonObjectConst& jsonSetting);
    /**
     * Is timer enabled?
     *
     * @return If enabled, it will return true otherwise false.
     */
    bool isEnabled() const
    {
        return m_isEnabled;
    }

    /**
     * Use TLS for connection?
     *
     * @return If TLS is used, it will return true otherwise false.
     */
    bool useTls() const
    {
        return m_useTls;
    }

    /**
     * Get broker host.
     *
     * @return Broker host.
     */
    String getBroker() const
    {
        return m_broker;
    }

    /**
     * Get broker port.
     *
     * @return Broker port.
     */
    uint16_t getPort() const
    {
        return m_port;
    }

    /**
     * Get user name for authentication.
     *
     * @return User name.
     */
    String getUser() const
    {
        return m_user;
    }

    /**
     * Get password for authentication.
     *
     * @return Password.
     */
    String getPassword() const
    {
        return m_password;
    }

    /**
     * Get root CA certificate for TLS connection.
     * If no certificate is set, it returns nullptr.
     *
     * @return Root CA certificate.
     */
    const char* getRootCaCert() const
    {
        return m_rootCaCert;
    }

    /**
     * Get client certificate for TLS connection.
     * If no certificate is set, it returns nullptr.
     *
     * @return Client certificate.
     */
    const char* getClientCert() const
    {
        return m_clientCert;
    }

    /**
     * Get client key for TLS connection.
     * If no key is set, it returns nullptr.
     *
     * @return Client key.
     */
    const char* getClientKey() const
    {
        return m_clientKey;
    }

private:

    /** MQTT port */
    static const uint16_t MQTT_PORT = 1883U;

    bool                  m_isEnabled;  /**< Is MQTT enabled? */
    bool                  m_useTls;     /**< Use TLS for connection? */
    String                m_broker;     /**< MQTT broker host. */
    uint16_t              m_port;       /**< MQTT broker port. */
    String                m_user;       /**< User name for authentication. */
    String                m_password;   /**< Password for authentication. */
    char*                 m_rootCaCert; /**< Root CA certificate for TLS connection. */
    char*                 m_clientCert; /**< Client certificate for TLS connection. */
    char*                 m_clientKey;  /**< Client key for TLS connection. */
};

/******************************************************************************
 * Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* MQTT_SETTING_H */

/** @} */