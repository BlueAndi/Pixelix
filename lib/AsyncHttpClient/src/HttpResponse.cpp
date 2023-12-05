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
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "HttpResponse.h"
#include <new>

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
        DLinkedListConstIterator<HttpHeader*>   it(rsp.m_headers);

        m_httpVersion   = rsp.m_httpVersion;
        m_statusCode    = rsp.m_statusCode;
        m_reasonPhrase  = rsp.m_reasonPhrase;

        clearPayload();

        if (nullptr != rsp.m_payload)
        {
            m_payload = new(std::nothrow) uint8_t[rsp.m_size];

            if (nullptr == m_payload)
            {
                m_size = 0U;
                m_wrIndex = 0U;
            }
            else
            {
                memcpy(m_payload, rsp.m_payload, rsp.m_size);
                m_size = rsp.m_size;
                m_wrIndex = rsp.m_wrIndex;
            }
        }

        clearHeaders();

        if (true == it.first())
        {
            do
            {
                HttpHeader* header = new(std::nothrow) HttpHeader;

                if (nullptr != header)
                {
                    *header = **it.current();
                    (void)m_headers.append(header);
                }
            }
            while(true == it.next());
        }
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
    const char* SP          = " ";
    int         idx         = 0;
    int         begin       = 0;
    String      statusCode;

    /* Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF */

    /* HTTP-Version */
    idx             = line.indexOf(SP);
    m_httpVersion   = line.substring(begin, idx);

    /* Overstep all spaces */
    while(('\0' != line[idx]) && (' ' == line[idx]))
    {
        ++idx;
    }
    begin = idx;

    /* Status-Code */
    idx             = line.indexOf(SP, begin);
    statusCode      = line.substring(begin, idx);
    m_statusCode    = statusCode.toInt();

    /* Overstep all spaces */
    while(('\0' != line[idx]) && (' ' == line[idx]))
    {
        ++idx;
    }
    begin = idx;

    /* Reason-Phrase */
    m_reasonPhrase  = line.substring(begin);
}

void HttpResponse::addHeader(const String& line)
{
    HttpHeader* header = new(std::nothrow) HttpHeader(line);

    if (nullptr != header)
    {
        m_headers.append(header);
    }
}

void HttpResponse::extendPayload(size_t size)
{
    uint8_t* tmp = m_payload;

    m_payload = new(std::nothrow) uint8_t[m_size + size];

    if (nullptr != m_payload)
    {
        m_size += size;
    }

    if ((nullptr != m_payload) &&
        (nullptr != tmp))
    {
        memcpy(m_payload, tmp, m_wrIndex);
    }

    if (nullptr != tmp)
    {
        delete[] tmp;
    }
}

void HttpResponse::addPayload(const uint8_t* payload, size_t size)
{
    if ((nullptr == m_payload) ||
        ((m_size - m_wrIndex) < size))
    {
        extendPayload(size);
    }

    if ((nullptr != m_payload) &&
        ((m_size - m_wrIndex) >= size))
    {
        memcpy(&m_payload[m_wrIndex], payload, size);
        m_wrIndex += size;
    }
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
    String                              value;
    DLinkedListIterator<HttpHeader*>    it(m_headers);

    if (true == it.first())
    {
        HttpHeader* hdr     = nullptr;
        bool        isFound = false;

        do
        {
            hdr = *it.current();

            if ((nullptr != hdr) &&
                (0U != hdr->getName().equalsIgnoreCase(name)))
            {
                value = hdr->getValue();
                isFound = true;
            }
        }
        while((false == isFound) &&
                (true == it.next()));
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
    DLinkedListIterator<HttpHeader*> it(m_headers);

    while(true == it.first())
    {
        HttpHeader* header = *it.current();

        it.remove();
        delete header;
    }
}

void HttpResponse::clearPayload()
{
    if (nullptr != m_payload)
    {
        delete[] m_payload;
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
