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
 * @file   HttpRsp.h
 * @brief  HTTP response
 * @author Andreas Merkle (web@blue-andi.de)
 *
 * @addtogroup HTTP_SERVICE
 *
 * @{
 */

#ifndef HTTP_RSP_H
#define HTTP_RSP_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <stdlib.h>
#include <HTTPClient.h>
#include <PsAllocator.hpp>

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * This type defines a HTTP response structure.
 */
struct HttpRsp
{
    t_http_codes statusCode; /**< HTTP status code of the response. */
    uint8_t*     payload;    /**< Payload of the HTTP response. */
    size_t       size;       /**< Size of the payload in byte. */

    /**
     * Construct a new HTTP response object.
     */
    HttpRsp() :
        statusCode(HTTP_CODE_SERVICE_UNAVAILABLE),
        payload(nullptr),
        size(0U)
    {
    }

    /**
     * Move constructor.
     *
     * @param[in] other Source HTTP response.
     */
    HttpRsp(HttpRsp&& other) noexcept :
        statusCode(other.statusCode),
        payload(other.payload),
        size(other.size)
    {
        other.payload = nullptr;
        other.size    = 0U;
    }

    /**
     * Move assignment operator.
     *
     * @param[in] other Source HTTP response.
     *
     * @return Reference to this response.
     */
    HttpRsp& operator=(HttpRsp&& other) noexcept
    {
        if (this != &other)
        {
            releasePayload();

            statusCode    = other.statusCode;
            payload       = other.payload;
            size          = other.size;
            other.payload = nullptr;
            other.size    = 0U;
        }

        return *this;
    }

    /**
     * Destroys the HTTP response object.
     */
    ~HttpRsp()
    {
        releasePayload();
    }

    /**
     * Release the payload memory.
     *
     * Note, the payload ownership was taken over from the HTTP service worker thread.
     */
    void releasePayload()
    {
        if (nullptr != payload)
        {
            PsAllocator allocator;
            allocator.deallocate(payload);

            payload = nullptr;
            size    = 0U;
        }
    }

    /* Disable copy constructor and copy assignment operator */
    HttpRsp(const HttpRsp&)            = delete;
    HttpRsp& operator=(const HttpRsp&) = delete;
};

/******************************************************************************
 * Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* HTTP_RSP_H */

/** @} */