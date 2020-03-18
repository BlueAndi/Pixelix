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
 *
 * @addtogroup web
 *
 * @{
 */

#ifndef __ASYNC_HTTP_CLIENT_H__
#define __ASYNC_HTTP_CLIENT_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <FreeRTOS.h>
#include <AsyncTCP.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Asynchronous HTTP client
 */
class AsyncHttpClient
{
public:

    /**
     * Constructs a http client.
     */
    AsyncHttpClient();

    /**
     * Destroys the http client instance.
     */
    ~AsyncHttpClient();

    /**
     * Parse all necessary parameters from URL and prepare for sending requests.
     *
     * @param[in] url   URL
     *
     * @return If successful parsed, it will return true otherwise false.
     */
    bool begin(String url);

    /**
     * Disconnect and clear all parameters.
     */
    void end();

    /**
     * Establish TCP connection.
     *
     * @return If the connection procedure is pending, it will return true otherwise false.
     */
    bool connect();

    /**
     * Disconnect TCP connection.
     */
    void disconnect();

    /**
     * Is connection established?
     *
     * @return If connection is established, it will return true otherwise false.
     */
    bool isConnected();

    /**
     * Is connection disconnected?
     *
     * @return If connection is disconnected, it will return true otherwise false.
     */
    bool isDisconnected();

    /**
     * Use HTTP/1.0 instead of HTTP/1.1
     *
     * @param[in] useHttp10 Use HTTP/1.0 (true) or HTTP/1.1 (false)
     */
    void useHttp10(bool useHttp10);

    /**
     * Send GET request to host.
     *
     * @param[in] method    Request method, e.g. GET, PUT, etc.
     * @param[in] payload   Request payload buffer
     * @param[in] size      Payload size in byte
     *
     * @return If request is successful sent, it will return true otherwise false.
     */
    bool sendRequest(const char* method, uint8_t* payload, size_t size);

private:

    /** HTTP port */
    static const uint16_t   HTTP_PORT   = 80U;

    /** HTTPS port */
    static const uint16_t   HTTPS_PORT  = 443U;

    AsyncClient m_tcpClient;            /**< Asynchronous TCP client */
    String      m_hostname;             /**< Server hostname */
    uint16_t    m_port;                 /**< Server port */
    String      m_base64Authorization;  /**< Authorization BASE64 encoded */
    String      m_uri;                  /**< Request URI */
    String      m_headers;              /**< Request headers */
    bool        m_isReqOpen;            /**< Is a request open? */
    String      m_method;               /**< Request method, e.g. GET, PUT, etc. */
    String      m_userAgent;            /**< User agent */
    bool        m_useHttp10;            /**< Use HTTP/1.0 instead of HTTP/1.1 */

    AsyncHttpClient(const AsyncHttpClient& client);
    AsyncHttpClient& operator=(const AsyncHttpClient& client);

    /**
     * This method is called by the TCP client if a connection is successful established.
     *
     * @param[in] client    TCP client
     */
    void onConnect(AsyncClient* client);

    /**
     * This method is called by the TCP client if a connection is disconnected.
     *
     * @param[in] client    TCP client
     */
    void onDisconnect(AsyncClient* client);

    /**
     * This method is called by the TCP client if a error occurred.
     *
     * @param[in] client    TCP client
     * @param[in] error     Error id
     */
    void onError(AsyncClient* client, int8_t error);

    /**
     * This method is called by the TCP client if data is received.
     *
     * @param[in] client    TCP client
     * @param[in] data      Data stream
     * @param[in] len       Data size in byte
     */
    void onData(AsyncClient* client, uint8_t* data, size_t len);

    /**
     * Clear all server related parameters.
     */
    void clear();

    /**
     * Send request header to server.
     *
     * @param[in] method    Request method, e.g. GET, PUT, etc.
     * @param[in] host      Hostname of server
     * @param[in] port      Port
     *
     * @return If successful requested, it will return true otherwise false.
     */
    bool sendHeader(const char* method);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __ASYNC_HTTP_CLIENT_H__ */

/** @} */