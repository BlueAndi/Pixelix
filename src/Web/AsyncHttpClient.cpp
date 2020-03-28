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
    m_onRspCallback(nullptr),
    m_hostname(),
    m_port(0U),
    m_base64Authorization(),
    m_uri(),
    m_headers(),
    m_isReqOpen(false),
    m_method(),
    m_userAgent("AsyncHttpClient"),
    m_isHttpVer10(false),
    m_isKeepAlive(false),
    m_payload(nullptr),
    m_payloadSize(0U),
    m_rspPart(RESPONSE_PART_STATUS_LINE),
    m_rsp(),
    m_rspLine(),
    m_transferCoding(TRANSFER_CODING_IDENTITY),
    m_contentLength(0U),
    m_contentIndex(0U),
    m_chunkSize(0U),
    m_chunkIndex(0U),
    m_chunkBodyPart(CHUNK_SIZE)
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

bool AsyncHttpClient::begin(const String& url)
{
    bool    status  = true;
    int     index   = url.indexOf(':');
    int     begin   = 0;

    if (0 > index)
    {
        LOG_ERROR("Failed to parse protocol.");
        status = false;
    }
    else
    {
        /* Get protocol http or https */
        String protocol = url.substring(begin, index);
        begin = index + 3; /* Overstep '://' too. */

        /* Determine port from protocol */
        if (protocol == "http")
        {
            m_port = HTTP_PORT;
        }
        else if (protocol == "https")
        {
            m_port = HTTPS_PORT;
        }
        else
        {
            status = false;
            LOG_ERROR("Unknown protocol \"%s\".", protocol.c_str());
        }

        if (true == status)
        {
            String host;

            /* Get host */
            index = url.indexOf('/', begin);
            host = url.substring(begin, index);
            begin = index;

            /* Get authorization */
            index = host.indexOf('@', begin);

            if (0 > index)
            {
                m_base64Authorization.clear();
            }
            else
            {
                String auth = host.substring(begin, index);
                begin = index + 1;

                m_base64Authorization = base64::encode(auth);
            }

            /* Get port */
            index = host.indexOf(':', begin);

            if (0 > index)
            {
                m_hostname = host;
            }
            else
            {
                long port = 0;

                m_hostname = host.substring(begin, index);
                begin = index + 1;

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

                LOG_INFO("Host: %s port: %u uri: %s", m_hostname.c_str(), m_port, m_uri.c_str());
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

void AsyncHttpClient::setHttpVersion(bool useHttp10)
{
    m_isHttpVer10 = useHttp10;
}

void AsyncHttpClient::setKeepAlive(bool keepAlive)
{
    m_isKeepAlive = keepAlive;
}

void AsyncHttpClient::regOnResponse(OnResponse onResponse)
{
    m_onRspCallback = onResponse;
}

void AsyncHttpClient::regOnClosed(OnClosed onClosed)
{
    m_onClosedCallback = onClosed;
}

bool AsyncHttpClient::GET()
{
    bool status = false;

    if (false == m_isReqOpen)
    {
        m_method        = "GET";
        m_payload       = nullptr;
        m_payloadSize   = 0U;

        if (false == isConnected())
        {
            status = connect();
            m_isReqOpen = status;
        }
        else
        {
            status = sendRequest();
            m_isReqOpen = false;
        }
    }

    return status;
}

bool AsyncHttpClient::POST(const uint8_t* payload, size_t size)
{
    bool status = false;

    if (false == m_isReqOpen)
    {
        m_method        = "POST";
        m_payload       = payload;
        m_payloadSize   = size;

        if (false == isConnected())
        {
            status = connect();
            m_isReqOpen = status;
        }
        else
        {
            status = sendRequest();
            m_isReqOpen = false;
        }
    }

    return status;
}

bool AsyncHttpClient::POST(const String& payload)
{
    bool status = false;

    if (false == m_isReqOpen)
    {
        m_method        = "POST";
        m_payload       = reinterpret_cast<const uint8_t*>(payload.c_str());
        m_payloadSize   = payload.length();

        if (false == isConnected())
        {
            status = connect();
            m_isReqOpen = status;
        }
        else
        {
            status = sendRequest();
            m_isReqOpen = false;
        }
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
    LOG_INFO("Connected.");

    /* Is there a queued request, which to send? */
    if (true == m_isReqOpen)
    {
        m_isReqOpen = false;

        if (false == sendRequest())
        {
            client->close();
        }
    }
}

void AsyncHttpClient::onDisconnect(AsyncClient* client)
{
    LOG_INFO("Disconnected.");
    clear();
    notifyClosed();
}

void AsyncHttpClient::onError(AsyncClient* client, int8_t error)
{
    LOG_INFO("Error occurred.");

    disconnect();
}

void AsyncHttpClient::onData(AsyncClient* client, const uint8_t* data, size_t len)
{
    size_t      index       = 0U;
    const char* asciiData   = reinterpret_cast<const char*>(data);
    bool        isError     = false;

    /* RFC2616 - Response = Status-Line
     *                      *(( general-header
     *                       | response-header
     *                       | entity-header ) CRLF)
     *                      CRLF
     *                      [ message-body ]
     */

    LOG_INFO("onData(): len = %u", len);

    while((len > index) && (false == isError))
    {
        switch(m_rspPart)
        {
        case RESPONSE_PART_STATUS_LINE:
            if (true == parseRspStatusLine(asciiData, len, index))
            {
                LOG_INFO("Rsp. HTTP-Version: %s", m_rsp.getHttpVersion().c_str());
                LOG_INFO("Rsp. Status-Code: %u", m_rsp.getStatusCode());
                LOG_INFO("Rsp. Reason-Phrase: %s", m_rsp.getReasonPhrase().c_str());

                m_rspPart = RESPONSE_PART_HEADER;
            }
            break;

        case RESPONSE_PART_HEADER:
            if (true == parseRspHeader(asciiData, len, index))
            {
                /* Examine response header.
                 * This is important to determine the number of following
                 * payload data and to know when the last data is
                 * received.
                 */
                if (false == handleRspHeader())
                {
                    /* Not nice, but anyway. */
                    LOG_ERROR("Header error.");
                    client->close();
                    isError = true;
                }
                m_rspPart = RESPONSE_PART_BODY;
            }
            break;

        case RESPONSE_PART_BODY:
            if (TRANSFER_CODING_CHUNCKED == m_transferCoding)
            {
                if (true == parseChunkedResponse(data, len, index))
                {
                    m_transferCoding = TRANSFER_CODING_IDENTITY;
                    m_rspPart = RESPONSE_PART_STATUS_LINE;

                    notifyResponse();
                    m_rsp.clear();
                }
            }
            else
            {
                size_t available    = len - index;
                size_t needed       = m_contentLength - m_contentIndex;
                size_t copySize     = 0U;

                if (available >= needed)
                {
                    copySize = needed;
                }
                else
                {
                    copySize = available;
                }

                m_rsp.addPayload(&data[index], copySize);
                m_contentIndex += copySize;
                index += copySize;

                if (m_contentLength <= m_contentIndex)
                {
                    m_rspPart = RESPONSE_PART_STATUS_LINE;

                    notifyResponse();
                    m_rsp.clear();
                }
            }
            break;

        default:
            LOG_FATAL("Internal error.");
            client->close();
            isError = true;
            break;
        }
    }
}

bool AsyncHttpClient::sendRequest()
{
    bool        status      = false;
    String      request;
    const char* PROTOCOL    = "HTTP";
    const char* SP          = " ";
    const char* CRLF        = "\r\n";

    /* RFC2616
     * Request = Request-Line
     *           * (( general-header
     *            | request-header
     *            | entity-header ) CRLF)
     *            CRLF
     *            [ message-body ]
     *
     */

    /* Request-Line: Method SP Request-URI SP HTTP-Version CRLF */

    /* Method */
    request += m_method;
    request += SP;

    /* Request-URI    = "*" | absoluteURI | abs_path | authority */
    if (true == m_uri.isEmpty())
    {
        request += "/";
    }
    else
    {
        request += m_uri;
    }

    request += SP;

    /* HTTP-Version */
    request += PROTOCOL;
    request += "/";

    if (false == m_isHttpVer10)
    {
        request += "1.1";
    }
    else
    {
        request += "1.0";
    }

    request += CRLF;

    /* --- Add now the request headers. --- */

    /* RFC2616 - general-header */
    /* Empty */
    request += CRLF;

    /* RFC2616 - request-header */
    request += "Host: ";
    request += m_hostname;

    if ((HTTP_PORT != m_port) &&
        (HTTPS_PORT != m_port))
    {
        request += ":";
        request += m_port;
    }

    request += CRLF;

    request += "User-Agent: ";
    request += m_userAgent;
    request += CRLF;

    request += "Connection: ";

    if (false == m_isKeepAlive)
    {
        request += "close";
    }
    else
    {
        request += "keep-alive";
    }
    request += CRLF;

    if (false == m_isHttpVer10)
    {
        /* By the client supported transfer codings. */
        request += "Accept-Encoding: ";
        request += "identity;q=1,chunked;q=0.1,*;q=0";
        request += CRLF;
    }

    if (0U < m_base64Authorization.length())
    {
        m_base64Authorization.replace("\n", "");
        request += "Authorization: Basic ";
        request += m_base64Authorization;
        request += CRLF;
    }

    if ((nullptr != m_payload) &&
        (0U < m_payloadSize))
    {
        request += "Content-Length: ";
        request += m_payloadSize;
        request += CRLF;
    }

    request += m_headers;
    request += CRLF;

    /* Send header */
    status = (request.length() == m_tcpClient.write(request.c_str(), request.length()));

    /* Send payload */
    if (true == status)
    {
        status = (m_payloadSize == m_tcpClient.write(reinterpret_cast<const char*>(m_payload), m_payloadSize, 0));
    }

    return status;
}

void AsyncHttpClient::clear()
{
    m_hostname.clear();
    m_port = 0U;
    m_base64Authorization.clear();
    m_uri.clear();
    m_headers.clear();

    m_rspPart = RESPONSE_PART_STATUS_LINE;
    m_rsp.clear();
    m_rspLine.clear();
    m_transferCoding = TRANSFER_CODING_IDENTITY;
    m_contentLength = 0U;
    m_contentIndex = 0U;
    m_chunkSize = 0U;
    m_chunkIndex = 0U;
    m_chunkBodyPart = CHUNK_SIZE;

    return;
}

void AsyncHttpClient::addHeader(const String& name, const String& value)
{
    const char* CRLF    = "\r\n";

    m_headers += name;
    m_headers += ": ";
    m_headers += value;
    m_headers += CRLF;
}

bool AsyncHttpClient::handleRspHeader()
{
    bool    isSuccess = true;
    String  value;

    value = m_rsp.getHeader("Connection");

    if (false == value.isEmpty())
    {
        /* Server closes the connection after the response? */
        if (0U != value.equalsIgnoreCase("close"))
        {
            /* Client want a permanent connection? */
            if (true == m_isKeepAlive)
            {
                LOG_WARNING("Connection can not be kept-alive.");
                m_isKeepAlive = false;
            }
        }
    }

    value = m_rsp.getHeader("Content-Length");

    if (false == value.isEmpty())
    {
        m_contentLength = value.toInt();
    }

    value = m_rsp.getHeader("Transfer-Encoding");

    if (false == value.isEmpty())
    {
        /* Only IDENTITY (default) and CHUNKED transfer coding are supported. */
        if (0U != value.equalsIgnoreCase("chunked"))
        {
            m_transferCoding = TRANSFER_CODING_CHUNCKED;
        }
        /* Unsupported transfer coding */
        else
        {
            isSuccess = false;
        }
    }

    return isSuccess;
}

bool AsyncHttpClient::parseChunkedResponseSize(const char* data, size_t len, size_t& index)
{
    const char*     CRLF        = "\r\n";
    const size_t    CRLF_LEN    = strlen(CRLF);
    bool            isSizeEOF   = false;

    while((len > index) && (false == isSizeEOF))
    {
        m_rspLine += data[index];
        ++index;

        if (0U != m_rspLine.endsWith(CRLF))
        {
            m_rspLine.remove(m_rspLine.indexOf(CRLF), CRLF_LEN);
            m_chunkSize = Util::hexToUInt32(m_rspLine);

            LOG_INFO("Chunk size is %u byte.", m_chunkSize);

            m_rspLine.clear();
            isSizeEOF = true;
        }
    }

    return isSizeEOF;
}

bool AsyncHttpClient::parseChunkedResponseChunkData(const uint8_t* data, size_t len, size_t& index)
{
    size_t  available   = len - index;
    size_t  needed      = m_chunkSize - m_chunkIndex;
    size_t  copySize    = 0U;
    bool    isDataEOF   = false;

    if (available >= needed)
    {
        copySize = needed;
    }
    else
    {
        copySize = available;
    }

    m_rsp.addPayload(&data[index], copySize);
    index += copySize;
    m_chunkIndex += copySize;

    if (m_chunkSize <= m_chunkIndex)
    {
        m_chunkIndex = 0U;
        isDataEOF = true;
    }

    return isDataEOF;
}

bool AsyncHttpClient::parseChunkedResponseChunkDataEnd(const char* data, size_t len, size_t& index)
{
    const char* CRLF        = "\r\n";
    bool        isDataEOF   = false;

    while((len > index) && (false == isDataEOF))
    {
        m_rspLine += data[index];
        ++index;

        if (0U != m_rspLine.endsWith(CRLF))
        {
            m_rspLine.clear();
            isDataEOF = true;
        }
    }

    return isDataEOF;
}

bool AsyncHttpClient::parseChunkedResponseTrailer(const char* data, size_t len, size_t& index)
{
    const char*     CRLF            = "\r\n";
    const size_t    CRLF_LEN        = strlen(CRLF);
    bool            isTrailerEOF    = false;

    while((len > index) && (false == isTrailerEOF))
    {
        m_rspLine += data[index];
        ++index;

        if (0U != m_rspLine.endsWith(CRLF))
        {
            if (CRLF_LEN < m_rspLine.length())
            {
                m_rspLine.remove(m_rspLine.length() - CRLF_LEN);

                LOG_INFO("Rsp. trailer: %s", m_rspLine.c_str());
            }
            else
            {
                LOG_INFO("Rsp. chunked transfer finished.");

                isTrailerEOF = true;
            }

            m_rspLine.clear();
        }
    }

    return isTrailerEOF;
}

bool AsyncHttpClient::parseChunkedResponse(const uint8_t* data, size_t len, size_t& index)
{
    const char* asciiData   = reinterpret_cast<const char*>(data);
    bool        isChunkEOF  = false;

    /*
     * Chunked-Body   = *chunk
     *                 last-chunk
     *                 trailer
     *                 CRLF
     *
     * chunk          = chunk-size [ chunk-extension ] CRLF
     *                 chunk-data CRLF
     * chunk-size     = 1*HEX
     * last-chunk     = 1*("0") [ chunk-extension ] CRLF
     *
     * chunk-extension= *( ";" chunk-ext-name [ "=" chunk-ext-val ] )
     * chunk-ext-name = token
     * chunk-ext-val  = token | quoted-string
     * chunk-data     = chunk-size(OCTET)
     * trailer        = *(entity-header CRLF)
     */

    while((len > index) && (false == isChunkEOF))
    {
        switch(m_chunkBodyPart)
        {
        /* Handle chunk or last-chunk size */
        case CHUNK_SIZE:
            if (true == parseChunkedResponseSize(asciiData, len, index))
            {
                if (0U == m_chunkSize)
                {
                    m_chunkBodyPart = TRAILER;
                }
                else
                {
                    m_chunkBodyPart = CHUNK_DATA;

                    /* Extend response payload */
                    m_rsp.extendPayload(m_chunkSize);
                }
            }
            break;

        /* Handle chunk data */
        case CHUNK_DATA:
            if (true == parseChunkedResponseChunkData(data, len, index))
            {
                m_chunkBodyPart = CHUNK_DATA_END;
            }
            break;

        /* Handle chunk data end (CRLF) */
        case CHUNK_DATA_END:
            if (true == parseChunkedResponseChunkDataEnd(asciiData, len, index))
            {
                m_chunkBodyPart = CHUNK_SIZE;
            }
            break;

        /* Handle trailer and CRLF */
        case TRAILER:
            if (true == parseChunkedResponseTrailer(asciiData, len, index))
            {
                m_chunkBodyPart = CHUNK_SIZE;
                isChunkEOF = true;
            }
            break;

        default:
            LOG_FATAL("Should never happen.");
            break;
        }
    }

    return isChunkEOF;
}

bool AsyncHttpClient::parseRspStatusLine(const char* data, size_t len, size_t& index)
{
    const char* CRLF            = "\r\n";
    bool        isStatusLineEOF = false;

    while((len > index) && (false == isStatusLineEOF))
    {
        m_rspLine += data[index];
        ++index;

        if (0U != m_rspLine.endsWith(CRLF))
        {
            m_rsp.addStatusLine(m_rspLine);

            isStatusLineEOF = true;
            m_rspLine.clear();
        }
    }

    return isStatusLineEOF;
}

bool AsyncHttpClient::parseRspHeader(const char* data, size_t len, size_t& index)
{
    const char*     CRLF        = "\r\n";
    const size_t    CRLF_LEN    = strlen(CRLF);
    bool            isHeaderEOF = false;

    while((len > index) && (false == isHeaderEOF))
    {
        m_rspLine += data[index];
        ++index;

        if (0U != m_rspLine.endsWith(CRLF))
        {
            /* The header will be finished, if a single CRLF is received. */
            if (CRLF_LEN < m_rspLine.length())
            {
                m_rspLine.remove(m_rspLine.length() - CRLF_LEN);

                LOG_INFO("Rsp. header: %s", m_rspLine.c_str());

                m_rsp.addHeader(m_rspLine);
            }
            else
            {
                isHeaderEOF = true;
            }

            m_rspLine.clear();
        }
    }

    return isHeaderEOF;
}

void AsyncHttpClient::notifyResponse()
{
    if (nullptr != m_onRspCallback)
    {
        m_onRspCallback(m_rsp);
    }
}

void AsyncHttpClient::notifyClosed()
{
    if (nullptr != m_onClosedCallback)
    {
        m_onClosedCallback();
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
