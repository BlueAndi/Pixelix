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
 * @file   HTTPClient.h
 * @brief  HTTP client for the native environment
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * Counterpart of the Arduino HTTPClient library. It provides the part of the
 * interface which the HttpService uses, not more. The transport is the
 * WiFiClient of the HalNative library.
 *
 * @addtogroup HTTP_CLIENT_NATIVE
 *
 * @{
 */

#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <stddef.h>
#include <Stream.h>
#include <WString.h>
#include <WiFiClient.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** HTTP status codes, like the Arduino HTTPClient declares them. */
typedef enum
{
    HTTP_CODE_CONTINUE              = 100, /**< Continue */
    HTTP_CODE_SWITCHING_PROTOCOLS   = 101, /**< Switching protocols */
    HTTP_CODE_OK                    = 200, /**< Ok */
    HTTP_CODE_CREATED               = 201, /**< Created */
    HTTP_CODE_ACCEPTED              = 202, /**< Accepted */
    HTTP_CODE_NO_CONTENT            = 204, /**< No content */
    HTTP_CODE_MOVED_PERMANENTLY     = 301, /**< Moved permanently */
    HTTP_CODE_FOUND                 = 302, /**< Found */
    HTTP_CODE_SEE_OTHER             = 303, /**< See other */
    HTTP_CODE_NOT_MODIFIED          = 304, /**< Not modified */
    HTTP_CODE_TEMPORARY_REDIRECT    = 307, /**< Temporary redirect */
    HTTP_CODE_PERMANENT_REDIRECT    = 308, /**< Permanent redirect */
    HTTP_CODE_BAD_REQUEST           = 400, /**< Bad request */
    HTTP_CODE_UNAUTHORIZED          = 401, /**< Unauthorized */
    HTTP_CODE_FORBIDDEN             = 403, /**< Forbidden */
    HTTP_CODE_NOT_FOUND             = 404, /**< Not found */
    HTTP_CODE_INTERNAL_SERVER_ERROR = 500, /**< Internal server error */
    HTTP_CODE_NOT_IMPLEMENTED       = 501, /**< Not implemented */
    HTTP_CODE_BAD_GATEWAY           = 502, /**< Bad gateway */
    HTTP_CODE_SERVICE_UNAVAILABLE   = 503  /**< Service unavailable */

} t_http_codes;

/** Behaviour in case the server answers with a redirect. */
typedef enum
{
    HTTPC_DISABLE_FOLLOW_REDIRECTS = 0, /**< Don't follow a redirect. */
    HTTPC_STRICT_FOLLOW_REDIRECTS,      /**< Follow a redirect of a GET or HEAD request only. */
    HTTPC_FORCE_FOLLOW_REDIRECTS        /**< Follow a redirect of every request. */

} followRedirects_t;

/** No connection to the server. */
#define HTTPC_ERROR_CONNECTION_REFUSED (-1)

/** Out of memory. */
#define HTTPC_ERROR_SEND_HEADER_FAILED (-2)

/** The payload couldn't be sent. */
#define HTTPC_ERROR_SEND_PAYLOAD_FAILED (-3)

/** The client is not connected. */
#define HTTPC_ERROR_NOT_CONNECTED (-4)

/** The connection was lost during the request. */
#define HTTPC_ERROR_CONNECTION_LOST (-5)

/** The server didn't answer in time. */
#define HTTPC_ERROR_READ_TIMEOUT (-11)

/** The answer of the server is no valid HTTP response. */
#define HTTPC_ERROR_NO_HTTP_SERVER (-7)

/** The url is invalid. */
#define HTTPC_ERROR_INVALID_URL (-12)

/** Too many redirects. */
#define HTTPC_ERROR_TOO_LESS_RAM (-8)

/**
 * HTTP client, which performs a request in a blocking way.
 */
class HTTPClient
{
public:

    /**
     * Constructs the HTTP client.
     */
    HTTPClient();

    /**
     * Destroys the HTTP client and closes a pending connection.
     */
    ~HTTPClient();

    /**
     * Prepare a request to the given url.
     *
     * @param[in] client    Transport, which is used for the request.
     * @param[in] url       Url of the request.
     *
     * @return If the url is valid, it will return true otherwise false.
     */
    bool begin(WiFiClient& client, const String& url);

    /**
     * Finish the request and close the connection.
     */
    void end();

    /**
     * Perform a GET request.
     *
     * @return HTTP status code or a negative value in case of an error.
     */
    int GET();

    /**
     * Perform a POST request.
     *
     * @param[in] payload   Payload of the request. May be nullptr.
     * @param[in] size      Payload size in byte.
     *
     * @return HTTP status code or a negative value in case of an error.
     */
    int POST(const uint8_t* payload, size_t size);

    /**
     * Perform a POST request.
     *
     * @param[in] payload   Payload of the request.
     *
     * @return HTTP status code or a negative value in case of an error.
     */
    int POST(const String& payload);

    /**
     * Write the payload of the response to the given stream.
     *
     * @param[out] stream   Stream, which the payload is written to.
     *
     * @return Number of written bytes or a negative value in case of an error.
     */
    int writeToStream(Stream* stream);

    /**
     * Get the payload size of the response.
     *
     * @return Payload size in byte or -1 if it is unknown.
     */
    int getSize() const
    {
        return m_size;
    }

    /**
     * Shall a redirect of the server be followed?
     *
     * @param[in] follow    Behaviour in case of a redirect.
     */
    void setFollowRedirects(followRedirects_t follow)
    {
        m_followRedirects = follow;
    }

    /**
     * Set the max. number of redirects, which are followed.
     *
     * @param[in] limit Max. number of redirects.
     */
    void setRedirectLimit(uint16_t limit)
    {
        m_redirectLimit = limit;
    }

    /**
     * Set the max. time to wait for the answer of the server.
     *
     * @param[in] timeout   Timeout in ms.
     */
    void setTimeout(uint16_t timeout)
    {
        m_timeout = timeout;
    }

    /**
     * Add a header to the request.
     *
     * @param[in] name  Name of the header.
     * @param[in] value Value of the header.
     */
    void addHeader(const String& name, const String& value);

    /**
     * Get the description of the given error.
     *
     * @param[in] error Error code, see HTTPC_ERROR_*.
     *
     * @return Description of the error.
     */
    String errorToString(int error);

private:

    /** Max. size of a single header line in byte. */
    static const size_t MAX_LINE_LENGTH   = 512U;

    /** Max. time to wait for the answer of the server in ms. */
    static const uint16_t DEFAULT_TIMEOUT = 5000U;

    WiFiClient*           m_client;          /**< Transport, not owned by the client. */
    String                m_host;            /**< Host of the url. */
    uint16_t              m_port;            /**< Port of the url. */
    String                m_uri;             /**< Path and query of the url. */
    bool                  m_isSecure;        /**< Is a TLS connection required? */
    String                m_headers;         /**< Additional headers of the request. */
    followRedirects_t     m_followRedirects; /**< Behaviour in case of a redirect. */
    uint16_t              m_redirectLimit;   /**< Max. number of redirects, which are followed. */
    uint16_t              m_timeout;         /**< Max. time to wait for the answer of the server in ms. */
    int                   m_size;            /**< Payload size of the response in byte, -1 if unknown. */
    bool                  m_isChunked;       /**< Is the payload chunked transfer encoded? */
    String                m_location;        /**< Location header of a redirect. */

    /**
     * Split the given url into its parts.
     *
     * @param[in] url   Url, which to split.
     *
     * @return If the url is valid, it will return true otherwise false.
     */
    bool parseUrl(const String& url);

    /**
     * Send the request to the server.
     *
     * @param[in] method    HTTP method, e.g. "GET".
     * @param[in] payload   Payload of the request. May be nullptr.
     * @param[in] size      Payload size in byte.
     *
     * @return If successful sent, it will return true otherwise false.
     */
    bool sendRequest(const char* method, const uint8_t* payload, size_t size);

    /**
     * Read the status line and the headers of the response.
     *
     * @return HTTP status code or a negative value in case of an error.
     */
    int readResponseHead();

    /**
     * Read a single line from the response, without the line ending.
     *
     * @param[out] line Line, which was read.
     *
     * @return If a complete line was read, it will return true otherwise false.
     */
    bool readLine(String& line);

    /**
     * Perform a request and follow the redirects of the server.
     *
     * @param[in] method    HTTP method, e.g. "GET".
     * @param[in] payload   Payload of the request. May be nullptr.
     * @param[in] size      Payload size in byte.
     *
     * @return HTTP status code or a negative value in case of an error.
     */
    int request(const char* method, const uint8_t* payload, size_t size);

    /**
     * Is the given status code a redirect, which shall be followed?
     *
     * @param[in] statusCode    HTTP status code of the response.
     * @param[in] method        HTTP method of the request.
     *
     * @return If the redirect shall be followed, it will return true otherwise false.
     */
    bool isRedirectToFollow(int statusCode, const char* method) const;

    HTTPClient(const HTTPClient& client);
    HTTPClient& operator=(const HTTPClient& client);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* HTTPCLIENT_H */

/** @} */
