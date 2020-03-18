/* MIT License
 *
 * Copyright (c) 2019 - 2020 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Asynchronous HTTP client
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "AsyncHttpClient.h"

#include <Util.h>
#include <Logging.h>
#include <base64.h>

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

AsyncHttpClient::AsyncHttpClient() :
    m_tcpClient(),
    m_hostname(),
    m_port(0U),
    m_base64Authorization(),
    m_uri(),
    m_headers(),
    m_isReqOpen(false),
    m_type()
{
    m_tcpClient.onConnect(  [this](void* arg, AsyncClient* client)
                            {
                                UTIL_NOT_USED(arg);

                                onConnect(client);
                            });

    m_tcpClient.onDisconnect(   [this](void* arg, AsyncClient* client)
                                {
                                    UTIL_NOT_USED(arg);

                                    onDisconnect(client);
                                });

    m_tcpClient.onError(    [this](void* arg, AsyncClient* client, int8_t error)
                            {
                                UTIL_NOT_USED(arg);

                                onError(client, error);
                            });

    m_tcpClient.onData( [this](void* arg, AsyncClient* client, void* data, size_t len)
                        {
                            UTIL_NOT_USED(arg);

                            onData(client, static_cast<uint8_t*>(data), len);
                        });
}

AsyncHttpClient::~AsyncHttpClient()
{
}

bool AsyncHttpClient::begin(String url)
{
    bool    status = true;
    int     index = url.indexOf(':');

    if (0 > index)
    {
        LOG_ERROR("Failed to parse protocol.");
        status = false;
    }
    else
    {
        String protocol = url.substring(0, index);  /* Get protocol http or https */

        /* Determine port from protocol */
        if (protocol == "http")
        {
            m_port = 80U;
        }
        else if (protocol == "https")
        {
            m_port = 443U;
        }
        else
        {
            status = false;
            LOG_ERROR("Unknown protocol \"%s\".", protocol.c_str());
        }

        if (true == status)
        {
            String host;

            /* Remove protocol from url */
            url.remove(0U, index + 3U);

            /* Get host */
            index = url.indexOf('/');
            host = url.substring(0U, index);

            /* Remove host from url */
            url.remove(0, index);

            /* Get authorization */
            index = host.indexOf('@');

            if (0 > index)
            {
                m_base64Authorization.clear();
            }
            else
            {
                String auth = host.substring(0U, index);

                /* Remove auth from host */
                host.remove(0, index + 1U);

                m_base64Authorization = base64::encode(auth);
            }

            /* Get port */
            index = host.indexOf(':');

            if (0 > index)
            {
                m_hostname = host;
            }
            else
            {
                long port = 0;

                m_hostname = host.substring(0U, index);

                /* Remove hostname from host */
                host.remove(0U, index + 1U);

                port = host.toInt();

                if ((0 > port) ||
                    (UINT16_MAX < port))
                {
                    status = false;
                }
                else
                {
                    m_port = static_cast<uint16_t>(port);
                }
            }

            if (true == status)
            {
                m_uri = url;

                LOG_INFO("[HTTP client] host: %s port: %u uri: %s", m_hostname.c_str(), m_port, m_uri.c_str());
            }
        }

        if (false == status)
        {
            clear();
        }
    }

    return status;
}

void AsyncHttpClient::end()
{
    disconnect();
    clear();
}

bool AsyncHttpClient::connect()
{
    return m_tcpClient.connect(m_hostname.c_str(), m_port);
}

void AsyncHttpClient::disconnect()
{
    m_tcpClient.close();
}

bool AsyncHttpClient::isConnected()
{
    return m_tcpClient.connected();
}

bool AsyncHttpClient::isDisconnected()
{
    return m_tcpClient.disconnected();
}

bool AsyncHttpClient::sendRequest(const char* type, uint8_t* payload, size_t size)
{
    bool status = true;

    if (false == isConnected())
    {
        m_type = type;
        /* m_payload = payload */
        /* m_payloadSize = size */
        m_isReqOpen = true;

        status = connect();
    }
    else
    {
        m_isReqOpen = false;
        status = sendHeader(type);
    }

    return status;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void AsyncHttpClient::onConnect(AsyncClient* client)
{
    LOG_INFO("Client connected.");

    if (true == m_isReqOpen)
    {
        (void)sendRequest(m_type.c_str(), NULL, 0U);
    }
}

void AsyncHttpClient::onDisconnect(AsyncClient* client)
{
    LOG_INFO("Client disconnected.");
}

void AsyncHttpClient::onError(AsyncClient* client, int8_t error)
{
    LOG_INFO("Client error occurred.");
}

void AsyncHttpClient::onData(AsyncClient* client, uint8_t* data, size_t len)
{
    size_t index = 0U;
    String rsp;

    for(index = 0U; index < len; ++index)
    {
        rsp += static_cast<char>(data[index]);
    }

    LOG_INFO("Client rsp: %s", rsp.c_str());
}

void AsyncHttpClient::clear()
{
    m_hostname.clear();
    m_port = 0U;
    m_base64Authorization.clear();
    m_uri.clear();
    m_headers.clear();

    return;
}

bool AsyncHttpClient::sendHeader(const char* type)
{
    String      header;
    const char* PROTOCOL            = "HTTP";
    const char* PROTOCOL_VERSION    = "1.0";

    header += type;
    header += " / ";
    header += PROTOCOL;
    header += "/";
    header += PROTOCOL_VERSION;
    header += "\r\nHost: ";
    header += m_hostname;
    header += "\r\n\r\n";

    return (header.length() == m_tcpClient.write(header.c_str()));
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
