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
 * @file   HttpResponseSink.cpp
 * @brief  HTTP response sink stream
 * @author Andreas Merkle (web@blue-andi.de)
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "HttpResponseSink.h"

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

HttpResponseSink::HttpResponseSink(IHttpResponseHandler* handler, WorkerResponse& response) :
    m_handler(handler),
    m_response(response),
    m_index(0U)
{
}

HttpResponseSink::~HttpResponseSink()
{
}

size_t HttpResponseSink::write(uint8_t data)
{
    return write(&data, 1U);
}

size_t HttpResponseSink::write(const uint8_t* buffer, size_t size)
{
    if ((nullptr == buffer) || (0U == size))
    {
        return 0U;
    }

    if (nullptr != m_handler)
    {
        m_handler->onResponse(m_index, false, buffer, size);
    }
    else
    {
        m_response.append(const_cast<uint8_t*>(buffer), size);
    }

    ++m_index;

    return size;
}

int HttpResponseSink::available()
{
    return 0;
}

int HttpResponseSink::read()
{
    return -1;
}

int HttpResponseSink::peek()
{
    return -1;
}

void HttpResponseSink::flush()
{
    /* Nothing to do. */
}

void HttpResponseSink::finalize()
{
    if (nullptr != m_handler)
    {
        m_handler->onResponse(m_index, true, nullptr, 0U);
    }
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
