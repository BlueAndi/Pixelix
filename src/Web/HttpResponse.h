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
 * @brief  HTTP response
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup web
 *
 * @{
 */

#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <WString.h>
#include <LinkedList.hpp>

#include "HttpHeader.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Http response
 */
class HttpResponse
{
public:

    /**
     * Construct a empty response.
     */
    HttpResponse() :
        m_httpVersion(),
        m_statusCode(0U),
        m_reasonPhrase(),
        m_headers(),
        m_payload(nullptr),
        m_size(0U),
        m_wrIndex(0U)
    {
    }

    /**
     * Destroys the response.
     */
    ~HttpResponse()
    {
        clear();
    }

    /**
     * Construct a response by copy a other response.
     *
     * @param[in] rsp   Response
     */
    HttpResponse(const HttpResponse& rsp) :
        m_httpVersion(),
        m_statusCode(0U),
        m_reasonPhrase(),
        m_headers(),
        m_payload(nullptr),
        m_size(0U),
        m_wrIndex(0U)
    {
        *this = rsp;
    }

    /**
     * Assign a different response.
     *
     * @param[in] rsp   Response
     *
     * @return Response
     */
    HttpResponse& operator=(const HttpResponse& rsp);

    /**
     * Clear response.
     */
    void clear();

    /**
     * Add status line during parsing the response.
     *
     * @param[in] line  Status line
     */
    void addStatusLine(const String& line);

    /**
     * Add header during parsing the response.
     *
     * @param[in] line  Single header line
     */
    void addHeader(const String& line);

    /**
     * Extend payload size in bytes.
     *
     * @param[in] size  Size in bytes
     */
    void extendPayload(size_t size);

    /**
     * Add a complete payload or add it several times partly.
     *
     * @param[in] payload   Complete or partly payload
     * @param[in] size      Payload size in byte
     */
    void addPayload(const uint8_t* payload, size_t size);

    /**
     * Get HTTP version.
     *
     * @return HTTP version
     */
    String getHttpVersion() const;

    /**
     * Get status code.
     *
     * @return Status code
     */
    uint16_t getStatusCode() const;

    /**
     * Get reason phrase.
     *
     * @return Reason phrase
     */
    String getReasonPhrase() const;

    /**
     * Get header field value.
     *
     * @param[in] name  Field name
     */
    String getHeader(const String& name);

    /**
     * Get payload.
     *
     * @param[out] size Payload size in byte
     *
     * @return Payload buffer
     */
    const uint8_t* getPayload(size_t& size) const;

private:

    String                      m_httpVersion;  /**< HTTP version */
    uint16_t                    m_statusCode;   /**< Status code */
    String                      m_reasonPhrase; /**< Reason phrase */
    DLinkedList<HttpHeader*>    m_headers;      /**< List of headers */
    uint8_t*                    m_payload;      /**< Payload */
    size_t                      m_size;         /**< Payload size in byte */
    size_t                      m_wrIndex;      /**< Payload write index */

    /**
     * Clear headers.
     */
    void clearHeaders();

    /**
     * Clears the payload.
     */
    void clearPayload();
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* HTTP_RESPONSE_H */

/** @} */