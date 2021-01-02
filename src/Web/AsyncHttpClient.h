/* MIT License
 *
 * Copyright (c) 2019 - 2021 Andreas Merkle <web@blue-andi.de>
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

#include "HttpResponse.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Asynchronous HTTP client
 *
 * Used RFCs:
 * - RFC2616 (obsolete, because of RFC7230)
 * - RFC7230
 */
class AsyncHttpClient
{
public:

    /**
     * Prototype of HTTP response callback for a complete received response.
     */
    typedef std::function<void(const HttpResponse& rsp)> OnResponse;

    /**
     * Prototype of HTTP response callback for a closed connection.
     */
    typedef std::function<void()> OnClosed;

    /**
     * Prototype of HTTP response callback in case a error happened.
     */
    typedef std::function<void()> OnError;

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
     * Note, calling this will clear user defined headers and URL encoded parameters.
     *
     * @param[in] url   URL
     *
     * @return If successful parsed, it will return true otherwise false.
     */
    bool begin(const String& url);

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
    void setHttpVersion(bool useHttp10);

    /**
     * Keep connection alive or close it after a request.
     *
     * @param[in] keepAlive Keep alive (true) or close (false) it.
     */
    void setKeepAlive(bool keepAlive);

    /**
     * Add header to request header.
     *
     * @param[in] name  Name
     * @param[in] value Value
     */
    void addHeader(const String& name, const String& value);

    /**
     * Clear user defined request headers.
     */
    void clearHeader();

    /**
     * Add parameter to request (application/x-www-form-urlencoded).
     *
     * Note: Don't use a user payload for the request, because it
     * will skip the added parameters. Only parameters can be sent
     * or a user payload.
     *
     * @param[in] name  Parameter name
     * @param[in] value Parameter value
     */
    void addPar(const String& name, const String& value);

    /**
     * Clear URL encoded parameters.
     */
    void clearPar();

    /**
     * Register callback function on response reception.
     *
     * @param[in] onResponse    Callback
     */
    void regOnResponse(const OnResponse& onResponse);

    /**
     * Register callback function on closed connection.
     *
     * @param[in] onClosed  Callback
     */
    void regOnClosed(const OnClosed& onClosed);

    /**
     * Register callback function on error.
     *
     * @param[in] onError   Callback
     */
    void regOnError(const OnError& onError);
    
    /**
     * Send GET request to host.
     *
     * @return If request is successful sent, it will return true otherwise false.
     */
    bool GET();

    /**
     * Send POST request to host.
     *
     * @param[in] payload   Payload, which must be kept alive until response is available!
     * @param[in] size      Payload size in byte
     *
     * @return If request is successful sent, it will return true otherwise false.
     */
    bool POST(const uint8_t* payload = nullptr, size_t size = 0U);

    /**
     * Send POST request to host.
     *
     * @param[in] payload   Payload, which must be kept alive until response is available!
     *
     * @return If request is successful sent, it will return true otherwise false.
     */
    bool POST(const String& payload);

private:

    /**
     * HTTP response parts.
     */
    enum ResponsePart
    {
        RESPONSE_PART_STATUS_LINE = 0,  /**< Response status line */
        RESPONSE_PART_HEADER,           /**< Response headers */
        RESPONSE_PART_BODY              /**< Response body */
    };

    /**
     * Supported HTTP transfer codings
     */
    enum TransferCoding
    {
        TRANSFER_CODING_IDENTITY = 0,   /**< Identity */
        TRANSFER_CODING_CHUNCKED        /**< Chunked */
    };

    /**
     * Chunk body parts
     */
    enum ChunkBodyPart
    {
        CHUNK_SIZE = 0, /**< Chunk or last chunk size */
        CHUNK_DATA,     /**< Chunk data */
        CHUNK_DATA_END, /**< Chunk data end */
        TRAILER         /**< Trailer */
    };

    /** HTTP port */
    static const uint16_t   HTTP_PORT   = 80U;

    /** HTTPS port */
    static const uint16_t   HTTPS_PORT  = 443U;

    AsyncClient     m_tcpClient;            /**< Asynchronous TCP client */
    OnResponse      m_onRspCallback;        /**< Callback which to call for a complete response. */
    OnClosed        m_onClosedCallback;     /**< Callback which to call for a closed connection. */
    OnError         m_onErrorCallback;      /**< Callback which to call for a connection error. */
    String          m_hostname;             /**< Server hostname */
    uint16_t        m_port;                 /**< Server port */
    String          m_base64Authorization;  /**< Authorization BASE64 encoded */
    String          m_uri;                  /**< Request URI */
    String          m_headers;              /**< Additional request headers */
    bool            m_isReqOpen;            /**< Is a request open? */
    String          m_method;               /**< Request method, e.g. GET, PUT, etc. */
    String          m_userAgent;            /**< User agent */
    bool            m_isHttpVer10;          /**< Use HTTP/1.0 (true) instead of HTTP/1.1 (false) */
    bool            m_isKeepAlive;          /**< Keep connection alive or not? */
    String          m_urlEncodedPars;       /**< URL encoded paramters (application/x-www-form-urlencoded) */
    const uint8_t*  m_payload;              /**< Request payload */
    size_t          m_payloadSize;          /**< Request payload size in byte */

    ResponsePart    m_rspPart;              /**< Current parsing part of the response */
    HttpResponse    m_rsp;                  /**< Response */
    String          m_rspLine;              /**< Single line, used for response parsing */
    TransferCoding  m_transferCoding;       /**< Transfer coding */
    size_t          m_contentLength;        /**< Content length in byte */
    size_t          m_contentIndex;         /**< Content index */
    size_t          m_chunkSize;            /**< Chunk size in byte */
    size_t          m_chunkIndex;           /**< Chunk body index */
    ChunkBodyPart   m_chunkBodyPart;        /**< Current part of chunked response */

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
    void onData(AsyncClient* client, const uint8_t* data, size_t len);

    /**
     * This method is called by the TCP client if ACK timeout happens.
     *
     * @param[in] client    TCP client
     * @param[in] timeout   Timeout value in ms
     */
    void onTimeout(AsyncClient* client, uint32_t timeout);

    /**
     * Send request to host.
     *
     * @return If request is successful sent, it will return true otherwise false.
     */
    bool sendRequest();

    /**
     * Clear all server related parameters.
     */
    void clear();

    /**
     * Is line terminator detected?
     *
     * @param[in]   str String
     * @param[out]  len Length of terminator
     *
     * @return If line terminator detected, it will return true otherwise false.
     */
    bool isEOL(const String& str, size_t& len);

    /**
     * Handle response header.
     *
     * @return If client is fine with the resposne header, it will return true otherwise false.
     */
    bool handleRspHeader();

    /**
     * Parse response chunked transfer chunk size.
     *
     * @param[in]       data    Data stream
     * @param[in]       len     Data size in byte
     * @param[in,out]   index   Current data index
     *
     * @return If complete parsed, it will return true otherwise false.
     */
    bool parseChunkedResponseSize(const char* data, size_t len, size_t& index);

    /**
     * Parse response chunked transfer chunk data.
     *
     * @param[in]       data    Data stream
     * @param[in]       len     Data size in byte
     * @param[in,out]   index   Current data index
     *
     * @return If complete parsed, it will return true otherwise false.
     */
    bool parseChunkedResponseChunkData(const uint8_t* data, size_t len, size_t& index);

    /**
     * Parse response chunked transfer chunk data end.
     *
     * @param[in]       data    Data stream
     * @param[in]       len     Data size in byte
     * @param[in,out]   index   Current data index
     *
     * @return If complete parsed, it will return true otherwise false.
     */
    bool parseChunkedResponseChunkDataEnd(const char* data, size_t len, size_t& index);

    /**
     * Parse response chunked transfer trailer.
     *
     * @param[in]       data    Data stream
     * @param[in]       len     Data size in byte
     * @param[in,out]   index   Current data index
     *
     * @return If complete parsed, it will return true otherwise false.
     */
    bool parseChunkedResponseTrailer(const char* data, size_t len, size_t& index);

    /**
     * Parse response chunked transfer.
     *
     * @param[in]       data    Data stream
     * @param[in]       len     Data size in byte
     * @param[in,out]   index   Current data index
     *
     * @return If chunked transfer is finished, it will return true otherwise false.
     */
    bool parseChunkedResponse(const uint8_t* data, size_t len, size_t& index);

    /**
     * Parse response status line.
     *
     * @param[in]       data    Data stream
     * @param[in]       len     Data size in byte
     * @param[in,out]   index   Current data index
     *
     * @return If status line is parsed, it will return true otherwise false.
     */
    bool parseRspStatusLine(const char* data, size_t len, size_t& index);

    /**
     * Parse response header.
     *
     * @param[in]       data    Data stream
     * @param[in]       len     Data size in byte
     * @param[in,out]   index   Current data index
     *
     * @return If all headers are parsed, it will return true otherwise false.
     */
    bool parseRspHeader(const char* data, size_t len, size_t& index);

    /**
     * This method will be called for every complete response and provides
     * it to the application, depended on whether a application callback
     * function is registered or not.
     */
    void notifyResponse();

    /**
     * This method will be called for a closed connection and notifies the
     * application, depended on whether a application callback function is
     * registered or not.
     */
    void notifyClosed();

    /**
     * This method will be called if a error happened and notifies the
     * application, depended on whether a application callback function is
     * registered or not.
     */
    void notifyError();

    /**
     * URL encode string (RFC1738 section 2.2)
     *
     * @param[in] str   String which to encode
     *
     * @return URL encoded string
     */
    String urlEncode(const String& str);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __ASYNC_HTTP_CLIENT_H__ */

/** @} */