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
 * @file   MqttSetting.cpp
 * @brief  MQTT setting
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "MqttSetting.h"

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

MqttSetting::MqttSetting() :
    m_isEnabled(false),
    m_useTls(false),
    m_broker(),
    m_port(MQTT_PORT),
    m_user(),
    m_password(),
    m_rootCaCert(nullptr),
    m_clientCert(nullptr),
    m_clientKey(nullptr)
{
}

MqttSetting::MqttSetting(const MqttSetting& other) :
    m_isEnabled(other.m_isEnabled),
    m_useTls(other.m_useTls),
    m_broker(other.m_broker),
    m_port(other.m_port),
    m_user(other.m_user),
    m_password(other.m_password),
    m_rootCaCert(nullptr),
    m_clientCert(nullptr),
    m_clientKey(nullptr)
{
    if (nullptr != other.m_rootCaCert)
    {
        m_rootCaCert = new (std::nothrow) char[strlen(other.m_rootCaCert) + 1U];

        if (nullptr != m_rootCaCert)
        {
            (void)strcpy(m_rootCaCert, other.m_rootCaCert);
        }
    }

    if (nullptr != other.m_clientCert)
    {
        m_clientCert = new (std::nothrow) char[strlen(other.m_clientCert) + 1U];

        if (nullptr != m_clientCert)
        {
            (void)strcpy(m_clientCert, other.m_clientCert);
        }
    }

    if (nullptr != other.m_clientKey)
    {
        m_clientKey = new (std::nothrow) char[strlen(other.m_clientKey) + 1U];

        if (nullptr != m_clientKey)
        {
            (void)strcpy(m_clientKey, other.m_clientKey);
        }
    }
}

MqttSetting& MqttSetting::operator=(const MqttSetting& other)
{
    if (this != &other)
    {
        m_isEnabled = other.m_isEnabled;
        m_useTls    = other.m_useTls;
        m_broker    = other.m_broker;
        m_port      = other.m_port;
        m_user      = other.m_user;
        m_password  = other.m_password;

        if (nullptr != m_rootCaCert)
        {
            delete[] m_rootCaCert;
            m_rootCaCert = nullptr;
        }

        if (nullptr != other.m_rootCaCert)
        {
            m_rootCaCert = new (std::nothrow) char[strlen(other.m_rootCaCert) + 1U];

            if (nullptr != m_rootCaCert)
            {
                (void)strcpy(m_rootCaCert, other.m_rootCaCert);
            }
        }

        if (nullptr != m_clientCert)
        {
            delete[] m_clientCert;
            m_clientCert = nullptr;
        }

        if (nullptr != other.m_clientCert)
        {
            m_clientCert = new (std::nothrow) char[strlen(other.m_clientCert) + 1U];

            if (nullptr != m_clientCert)
            {
                (void)strcpy(m_clientCert, other.m_clientCert);
            }
        }

        if (nullptr != m_clientKey)
        {
            delete[] m_clientKey;
            m_clientKey = nullptr;
        }

        if (nullptr != other.m_clientKey)
        {
            m_clientKey = new (std::nothrow) char[strlen(other.m_clientKey) + 1U];

            if (nullptr != m_clientKey)
            {
                (void)strcpy(m_clientKey, other.m_clientKey);
            }
        }
    }

    return *this;
}

MqttSetting& MqttSetting::operator=(MqttSetting&& other) noexcept
{
    if (this != &other)
    {
        m_isEnabled    = other.m_isEnabled;
        m_useTls       = other.m_useTls;
        m_broker       = other.m_broker;
        m_port         = other.m_port;
        m_user         = other.m_user;
        m_password     = other.m_password;
        m_rootCaCert   = other.m_rootCaCert;
        m_clientCert   = other.m_clientCert;
        m_clientKey    = other.m_clientKey;

        other.m_rootCaCert = nullptr;
        other.m_clientCert = nullptr;
        other.m_clientKey  = nullptr;
    }

    return *this;
}

void MqttSetting::clear()
{
    m_isEnabled = false;
    m_useTls    = false;
    m_broker    = "";
    m_port      = MQTT_PORT;
    m_user      = "";
    m_password  = "";

    if (nullptr != m_rootCaCert)
    {
        delete[] m_rootCaCert;
        m_rootCaCert = nullptr;
    }

    if (nullptr != m_clientCert)
    {
        delete[] m_clientCert;
        m_clientCert = nullptr;
    }

    if (nullptr != m_clientKey)
    {
        delete[] m_clientKey;
        m_clientKey = nullptr;
    }
}

void MqttSetting::toJson(JsonObject& jsonSetting) const
{
    jsonSetting["enabled"]  = m_isEnabled;
    jsonSetting["useTls"]   = m_useTls;
    jsonSetting["broker"]   = m_broker;
    jsonSetting["port"]     = m_port;
    jsonSetting["user"]     = m_user;
    jsonSetting["password"] = m_password;

    if (nullptr == m_rootCaCert)
    {
        jsonSetting["rootCaCert"] = "";
    }
    else
    {
        jsonSetting["rootCaCert"] = m_rootCaCert;
    }

    if (nullptr == m_clientCert)
    {
        jsonSetting["clientCert"] = "";
    }
    else
    {
        jsonSetting["clientCert"] = m_clientCert;
    }

    if (nullptr == m_clientKey)
    {
        jsonSetting["clientKey"] = "";
    }
    else
    {
        jsonSetting["clientKey"] = m_clientKey;
    }
}

bool MqttSetting::fromJson(const JsonObjectConst& jsonSetting)
{
    bool             isSuccessful   = false;
    JsonVariantConst jsonEnabled    = jsonSetting["enabled"];
    JsonVariantConst jsonUseTls     = jsonSetting["useTls"];
    JsonVariantConst jsonBroker     = jsonSetting["broker"];
    JsonVariantConst jsonPort       = jsonSetting["port"];
    JsonVariantConst jsonUser       = jsonSetting["user"];
    JsonVariantConst jsonPassword   = jsonSetting["password"];
    JsonVariantConst jsonRootCaCert = jsonSetting["rootCaCert"];
    JsonVariantConst jsonClientCert = jsonSetting["clientCert"];
    JsonVariantConst jsonClientKey  = jsonSetting["clientKey"];

    if ((false == jsonEnabled.isNull()) &&
        (false == jsonUseTls.isNull()) &&
        (false == jsonBroker.isNull()) &&
        (false == jsonPort.isNull()) &&
        (false == jsonUser.isNull()) &&
        (false == jsonPassword.isNull()) &&
        (false == jsonRootCaCert.isNull()) &&
        (false == jsonClientCert.isNull()) &&
        (false == jsonClientKey.isNull()))
    {
        const char* rootCaCert = jsonRootCaCert.as<const char*>();
        const char* clientCert = jsonClientCert.as<const char*>();
        const char* clientKey  = jsonClientKey.as<const char*>();

        clear();

        if (true == jsonEnabled.is<String>())
        {
            m_isEnabled = jsonEnabled.as<String>().equalsIgnoreCase("true");
        }
        else if (true == jsonEnabled.is<bool>())
        {
            m_isEnabled = jsonEnabled.as<bool>();
        }
        else
        {
            m_isEnabled = false;
        }

        if (true == jsonUseTls.is<String>())
        {
            m_useTls = jsonUseTls.as<String>().equalsIgnoreCase("true");
        }
        else if (true == jsonUseTls.is<bool>())
        {
            m_useTls = jsonUseTls.as<bool>();
        }
        else
        {
            m_useTls = false;
        }

        m_broker   = jsonBroker.as<const char*>();
        m_port     = jsonPort.as<uint16_t>();
        m_user     = jsonUser.as<const char*>();
        m_password = jsonPassword.as<const char*>();

        if (nullptr != rootCaCert)
        {
            const size_t ROOT_CA_CERT_LEN = strlen(rootCaCert);

            if (nullptr != m_rootCaCert)
            {
                delete[] m_rootCaCert;
                m_rootCaCert = nullptr;
            }

            m_rootCaCert = new (std::nothrow) char[ROOT_CA_CERT_LEN + 1U];

            if (nullptr != m_rootCaCert)
            {
                (void)strcpy(m_rootCaCert, rootCaCert);
            }
        }

        if (nullptr != clientCert)
        {
            const size_t CLIENT_CERT_LEN = strlen(clientCert);

            if (nullptr != m_clientCert)
            {
                delete[] m_clientCert;
                m_clientCert = nullptr;
            }

            m_clientCert = new (std::nothrow) char[CLIENT_CERT_LEN + 1U];

            if (nullptr != m_clientCert)
            {
                (void)strcpy(m_clientCert, clientCert);
            }
        }

        if (nullptr != clientKey)
        {
            const size_t CLIENT_KEY_LEN = strlen(clientKey);

            if (nullptr != m_clientKey)
            {
                delete[] m_clientKey;
                m_clientKey = nullptr;
            }

            m_clientKey = new (std::nothrow) char[CLIENT_KEY_LEN + 1U];

            if (nullptr != m_clientKey)
            {
                (void)strcpy(m_clientKey, clientKey);
            }
        }

        isSuccessful = true;
    }

    return isSuccessful;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
