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
 * @file   HTTPClient.cpp
 * @brief  HTTP client for the native environment
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "HTTPClient.h"

#include <Arduino.h>
#include <stdlib.h>
#include <string.h>

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

/** Default port of a plain HTTP connection. */
static const uint16_t PORT_HTTP  = 80U;

/** Default port of a HTTP connection over TLS. */
static const uint16_t PORT_HTTPS = 443U;

/******************************************************************************
 * Public Methods
 *****************************************************************************/

HTTPClient::HTTPClient() :
    m_client(nullptr),
    m_host(),
    m_port(PORT_HTTP),
    m_uri("/"),
    m_isSecure(false),
    m_headers(),
    m_followRedirects(HTTPC_DISABLE_FOLLOW_REDIRECTS),
    m_redirectLimit(10U),
    m_timeout(DEFAULT_TIMEOUT),
    m_size(-1),
    m_isChunked(false),
    m_location()
{
}

HTTPClient::~HTTPClient()
{
    end();
}

bool HTTPClient::begin(WiFiClient& client, const String& url)
{
    bool isSuccessful = false;

    end();

    if (true == parseUrl(url))
    {
        m_client = &client;

        m_client->setTimeout(m_timeout);

        isSuccessful = true;
    }

    return isSuccessful;
}

void HTTPClient::end()
{
    if (nullptr != m_client)
    {
        m_client->stop();
        m_client = nullptr;
    }

    m_headers.clear();
    m_location.clear();
    m_size      = -1;
    m_isChunked = false;
}

int HTTPClient::GET()
{
    return request("GET", nullptr, 0U);
}

int HTTPClient::POST(const uint8_t* payload, size_t size)
{
    return request("POST", payload, size);
}

int HTTPClient::POST(const String& payload)
{
    return request("POST", reinterpret_cast<const uint8_t*>(payload.c_str()), payload.length());
}

void HTTPClient::addHeader(const String& name, const String& value)
{
    m_headers += name;
    m_headers += ": ";
    m_headers += value;
    m_headers += "\r\n";
}

int HTTPClient::writeToStream(Stream* stream)
{
    int written = 0;

    if (nullptr == stream)
    {
        written = HTTPC_ERROR_NO_HTTP_SERVER;
    }
    else if (nullptr == m_client)
    {
        written = HTTPC_ERROR_NOT_CONNECTED;
    }
    else if (true == m_isChunked)
    {
        bool isFinished = false;

        while (false == isFinished)
        {
            String line;

            if (false == readLine(line))
            {
                written    = HTTPC_ERROR_CONNECTION_LOST;
                isFinished = true;
            }
            else
            {
                /* The chunk size is hexadecimal and may be followed by a
                 * chunk extension, which is separated by a semicolon.
                 */
                long chunkSize = strtol(line.c_str(), nullptr, 16);

                if (0 >= chunkSize)
                {
                    isFinished = true;
                }
                else
                {
                    uint8_t buffer[256U];
                    long    remaining = chunkSize;

                    while ((0 < remaining) &&
                           (false == isFinished))
                    {
                        size_t requested = sizeof(buffer);
                        int    received  = 0;

                        if (static_cast<long>(requested) > remaining)
                        {
                            requested = static_cast<size_t>(remaining);
                        }

                        received = m_client->read(buffer, requested);

                        if (0 >= received)
                        {
                            written    = HTTPC_ERROR_CONNECTION_LOST;
                            isFinished = true;
                        }
                        else
                        {
                            written   += static_cast<int>(stream->write(buffer, static_cast<size_t>(received)));
                            remaining -= received;
                        }
                    }

                    if (false == isFinished)
                    {
                        /* The chunk is terminated by a empty line. */
                        String terminator;

                        (void)readLine(terminator);
                    }
                }
            }
        }
    }
    else
    {
        uint8_t buffer[256U];
        int     remaining  = m_size;
        bool    isFinished = false;

        while (false == isFinished)
        {
            size_t requested = sizeof(buffer);
            int    received  = 0;

            /* A known content length limits the number of bytes to read,
             * otherwise it is read until the server closes the connection.
             */
            if (0 <= remaining)
            {
                if (0 == remaining)
                {
                    isFinished = true;
                    continue;
                }

                if (static_cast<int>(requested) > remaining)
                {
                    requested = static_cast<size_t>(remaining);
                }
            }

            received = m_client->read(buffer, requested);

            if (0 < received)
            {
                written += static_cast<int>(stream->write(buffer, static_cast<size_t>(received)));

                if (0 <= remaining)
                {
                    remaining -= received;
                }
            }
            else if (0 > received)
            {
                isFinished = true;
            }
            /* Nothing received. Without a content length the payload ends as
             * soon as the server closes the connection.
             */
            else if (0U == m_client->connected())
            {
                isFinished = true;
            }
            else
            {
                /* The server is still connected, try again. */
                ;
            }
        }
    }

    return written;
}

String HTTPClient::errorToString(int error)
{
    String description;

    switch (error)
    {
    case HTTPC_ERROR_CONNECTION_REFUSED:
        description = "connection refused";
        break;

    case HTTPC_ERROR_SEND_HEADER_FAILED:
        description = "send header failed";
        break;

    case HTTPC_ERROR_SEND_PAYLOAD_FAILED:
        description = "send payload failed";
        break;

    case HTTPC_ERROR_NOT_CONNECTED:
        description = "not connected";
        break;

    case HTTPC_ERROR_CONNECTION_LOST:
        description = "connection lost";
        break;

    case HTTPC_ERROR_NO_HTTP_SERVER:
        description = "no HTTP server";
        break;

    case HTTPC_ERROR_READ_TIMEOUT:
        description = "read timeout";
        break;

    case HTTPC_ERROR_INVALID_URL:
        description = "invalid url";
        break;

    case HTTPC_ERROR_TOO_LESS_RAM:
        description = "too many redirects";
        break;

    default:
        description = "unknown error";
        break;
    }

    return description;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

bool HTTPClient::parseUrl(const String& url)
{
    const char* PREFIX_HTTP  = "http://";
    const char* PREFIX_HTTPS = "https://";
    bool        isSuccessful = false;
    String      rest;

    if (0U != url.startsWith(String(PREFIX_HTTPS)))
    {
        m_isSecure   = true;
        m_port       = PORT_HTTPS;
        rest         = url.substring(strlen(PREFIX_HTTPS));
        isSuccessful = true;
    }
    else if (0U != url.startsWith(String(PREFIX_HTTP)))
    {
        m_isSecure   = false;
        m_port       = PORT_HTTP;
        rest         = url.substring(strlen(PREFIX_HTTP));
        isSuccessful = true;
    }
    else
    {
        /* Guard: no supported scheme. */
    }

    if (true == isSuccessful)
    {
        int slashIndex = rest.indexOf('/');

        if (0 > slashIndex)
        {
            m_uri = "/";
        }
        else
        {
            m_uri = rest.substring(slashIndex);
            rest  = rest.substring(0, slashIndex);
        }

        /* A port may be appended to the host, separated by a colon. */
        int colonIndex = rest.indexOf(':');

        if (0 > colonIndex)
        {
            m_host = rest;
        }
        else
        {
            m_host = rest.substring(0, colonIndex);
            m_port = static_cast<uint16_t>(rest.substring(colonIndex + 1).toInt());
        }

        if (true == m_host.isEmpty())
        {
            isSuccessful = false;
        }
    }

    return isSuccessful;
}

bool HTTPClient::sendRequest(const char* method, const uint8_t* payload, size_t size)
{
    bool   isSuccessful = false;
    String head;

    head  = method;
    head += " ";
    head += m_uri;
    head += " HTTP/1.1\r\n";
    head += "Host: ";
    head += m_host;

    if (((false == m_isSecure) && (PORT_HTTP != m_port)) ||
        ((true == m_isSecure) && (PORT_HTTPS != m_port)))
    {
        head += ":";
        head += m_port;
    }

    head += "\r\n";
    head += "User-Agent: Pixelix\r\n";

    /* The response is read until the server closes the connection, therefore
     * no keep-alive is requested.
     */
    head += "Connection: close\r\n";
    head += m_headers;

    if ((nullptr != payload) &&
        (0U < size))
    {
        head += "Content-Length: ";
        head += size;
        head += "\r\n";
    }

    head += "\r\n";

    if (head.length() == m_client->write(reinterpret_cast<const uint8_t*>(head.c_str()), head.length()))
    {
        if ((nullptr == payload) ||
            (0U == size))
        {
            isSuccessful = true;
        }
        else if (size == m_client->write(payload, size))
        {
            isSuccessful = true;
        }
        else
        {
            /* Guard: payload not completely sent. */
        }
    }

    return isSuccessful;
}

bool HTTPClient::readLine(String& line)
{
    bool   isComplete = false;
    size_t length     = 0U;

    line.clear();

    while ((false == isComplete) &&
           (MAX_LINE_LENGTH > length))
    {
        int data = m_client->read();

        if (0 > data)
        {
            /* No data anymore, the connection may be closed. */
            break;
        }
        else if ('\n' == data)
        {
            isComplete = true;
        }
        else if ('\r' == data)
        {
            /* Skipped, the line ending is \r\n. */
            ;
        }
        else
        {
            line += static_cast<char>(data);
            ++length;
        }
    }

    return isComplete;
}

int HTTPClient::readResponseHead()
{
    int    statusCode = HTTPC_ERROR_NO_HTTP_SERVER;
    String line;

    m_size      = -1;
    m_isChunked = false;
    m_location.clear();

    if (false == readLine(line))
    {
        statusCode = HTTPC_ERROR_CONNECTION_LOST;
    }
    else if (0U == line.startsWith(String("HTTP/1.")))
    {
        statusCode = HTTPC_ERROR_NO_HTTP_SERVER;
    }
    else
    {
        int spaceIndex = line.indexOf(' ');

        if (0 > spaceIndex)
        {
            statusCode = HTTPC_ERROR_NO_HTTP_SERVER;
        }
        else
        {
            bool isHeadComplete = false;

            statusCode          = static_cast<int>(line.substring(spaceIndex + 1).toInt());

            while (false == isHeadComplete)
            {
                if (false == readLine(line))
                {
                    isHeadComplete = true;
                }
                /* A empty line separates the head from the payload. */
                else if (true == line.isEmpty())
                {
                    isHeadComplete = true;
                }
                else
                {
                    int    colonIndex = line.indexOf(':');
                    String name;
                    String value;

                    if (0 <= colonIndex)
                    {
                        name  = line.substring(0, colonIndex);
                        value = line.substring(colonIndex + 1);
                        value.trim();
                        name.toLowerCase();

                        if (name == String("content-length"))
                        {
                            m_size = static_cast<int>(value.toInt());
                        }
                        else if (name == String("transfer-encoding"))
                        {
                            value.toLowerCase();

                            if (0 <= value.indexOf(String("chunked")))
                            {
                                m_isChunked = true;
                            }
                        }
                        else if (name == String("location"))
                        {
                            m_location = value;
                        }
                        else
                        {
                            /* Not of interest. */
                            ;
                        }
                    }
                }
            }
        }
    }

    return statusCode;
}

bool HTTPClient::isRedirectToFollow(int statusCode, const char* method) const
{
    bool isToFollow = false;

    if (true == m_location.isEmpty())
    {
        /* Guard: without a location there is nothing to follow. */
    }
    else if ((HTTP_CODE_MOVED_PERMANENTLY != statusCode) &&
             (HTTP_CODE_FOUND != statusCode) &&
             (HTTP_CODE_SEE_OTHER != statusCode) &&
             (HTTP_CODE_TEMPORARY_REDIRECT != statusCode) &&
             (HTTP_CODE_PERMANENT_REDIRECT != statusCode))
    {
        /* Guard: no redirect. */
    }
    else if (HTTPC_FORCE_FOLLOW_REDIRECTS == m_followRedirects)
    {
        isToFollow = true;
    }
    else if (HTTPC_STRICT_FOLLOW_REDIRECTS == m_followRedirects)
    {
        /* Only a GET or a HEAD request is repeated, everything else may not be
         * idempotent.
         */
        isToFollow = (0 == strcmp(method, "GET")) || (0 == strcmp(method, "HEAD"));
    }
    else
    {
        /* Guard: redirects are disabled. */
    }

    return isToFollow;
}

int HTTPClient::request(const char* method, const uint8_t* payload, size_t size)
{
    int      statusCode = HTTPC_ERROR_NOT_CONNECTED;
    uint16_t redirects  = 0U;
    bool     isFinished = false;

    if (nullptr == m_client)
    {
        return HTTPC_ERROR_NOT_CONNECTED;
    }

    while (false == isFinished)
    {
        isFinished = true;

        m_client->stop();

        if (0 == m_client->connect(m_host.c_str(), m_port))
        {
            statusCode = HTTPC_ERROR_CONNECTION_REFUSED;
        }
        else if (false == sendRequest(method, payload, size))
        {
            statusCode = HTTPC_ERROR_SEND_HEADER_FAILED;
        }
        else
        {
            statusCode = readResponseHead();

            if (true == isRedirectToFollow(statusCode, method))
            {
                if (m_redirectLimit <= redirects)
                {
                    statusCode = HTTPC_ERROR_TOO_LESS_RAM;
                }
                /* The location may be relative to the current host, in that
                 * case only the uri changes.
                 */
                else if (0U != m_location.startsWith(String("/")))
                {
                    m_uri = m_location;
                    ++redirects;
                    isFinished = false;
                }
                else if (false == parseUrl(m_location))
                {
                    statusCode = HTTPC_ERROR_INVALID_URL;
                }
                else
                {
                    ++redirects;
                    isFinished = false;
                }
            }
        }
    }

    return statusCode;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
