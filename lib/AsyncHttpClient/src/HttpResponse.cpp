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
 * @file   HttpResponse.cpp
 * @brief  HTTP response
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "HttpResponse.h"

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

HttpResponse& HttpResponse::operator=(const HttpResponse& rsp)
{
    if (this != &rsp)
    {
        m_httpVersion  = rsp.m_httpVersion;
        m_statusCode   = rsp.m_statusCode;
        m_reasonPhrase = rsp.m_reasonPhrase;

        clearPayload();

        if (nullptr != rsp.m_payload)
        {
            DataAllocator allocator;

            m_payload = allocator.allocateArray(rsp.m_size);

            if (nullptr == m_payload)
            {
                m_size    = 0U;
                m_wrIndex = 0U;
            }
            else
            {
                (void)memcpy(m_payload, rsp.m_payload, rsp.m_size);
                m_size    = rsp.m_size;
                m_wrIndex = rsp.m_wrIndex;
            }
        }

        clearHeaders();
        m_headers = rsp.m_headers;
    }

    return *this;
}

void HttpResponse::clear()
{
    clearHeaders();
    clearPayload();
    m_wrIndex = 0U;
}

void HttpResponse::addStatusLine(const String& line)
{
    const char* SP    = " ";
    int         idx   = 0;
    int         begin = 0;
    String      statusCode;

    /* Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF */

    /* HTTP-Version */
    idx = line.indexOf(SP);

    /* Malformed status line? */
    if (0 > idx)
    {
        m_httpVersion.clear();
        m_statusCode = 0U;
        m_reasonPhrase.clear();
    }
    else
    {
        m_httpVersion = line.substring(begin, idx);

        /* Overstep all spaces */
        while (('\0' != line[idx]) && (' ' == line[idx]))
        {
            ++idx;
        }
        begin = idx;

        /* Status-Code */
        idx   = line.indexOf(SP, begin);

        /* Malformed status code? */
        if (0 > idx)
        {
            m_httpVersion.clear();
            m_statusCode = 0U;
            m_reasonPhrase.clear();
        }
        else
        {
            statusCode   = line.substring(begin, idx);
            m_statusCode = statusCode.toInt();

            /* Overstep all spaces */
            while (('\0' != line[idx]) && (' ' == line[idx]))
            {
                ++idx;
            }
            begin          = idx;

            /* Reason-Phrase */
            m_reasonPhrase = line.substring(begin);
        }
    }
}

void HttpResponse::addHeader(const String& line)
{
    m_headers.emplace_back(line);
}

bool HttpResponse::extendPayload(size_t size)
{
    DataAllocator allocator;
    uint8_t*      newPayload = allocator.allocateArray(m_size + size);

    if (nullptr != newPayload)
    {
        if ((nullptr != m_payload) &&
            (0U < m_wrIndex))
        {
            memcpy(newPayload, m_payload, m_wrIndex);
        }

        if (nullptr != m_payload)
        {
            allocator.deallocateArray(m_payload);
        }

        m_payload  = newPayload;
        m_size    += size;
    }

    return (nullptr != newPayload);
}

bool HttpResponse::addPayload(const uint8_t* payload, size_t size)
{
    bool isSuccess = false;

    if ((nullptr == m_payload) ||
        ((m_size - m_wrIndex) < size))
    {
        (void)extendPayload(size);
    }

    if ((nullptr != m_payload) &&
        ((m_size - m_wrIndex) >= size))
    {
        memcpy(&m_payload[m_wrIndex], payload, size);
        m_wrIndex += size;

        isSuccess  = true;
    }

    return isSuccess;
}

String HttpResponse::getHttpVersion() const
{
    return m_httpVersion;
}

uint16_t HttpResponse::getStatusCode() const
{
    return m_statusCode;
}

String HttpResponse::getReasonPhrase() const
{
    return m_reasonPhrase;
}

String HttpResponse::getHeader(const String& name)
{
    String value;

    if (0U < m_headers.size())
    {
        ListOfHeaders::const_iterator headerIt = m_headers.begin();
        bool                          isFound  = false;

        while ((headerIt != m_headers.end()) && (false == isFound))
        {
            const HttpHeader& header = *headerIt;

            if (true == header.getName().equalsIgnoreCase(name))
            {
                value   = header.getValue();
                isFound = true;
            }

            ++headerIt;
        }
    }

    return value;
}

const uint8_t* HttpResponse::getPayload(size_t& size) const
{
    size = m_size;
    return m_payload;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void HttpResponse::clearHeaders()
{
    m_headers.clear();
}

void HttpResponse::clearPayload()
{
    if (nullptr != m_payload)
    {
        DataAllocator allocator;

        allocator.deallocateArray(m_payload);
        m_payload = nullptr;
    }

    m_size = 0U;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
