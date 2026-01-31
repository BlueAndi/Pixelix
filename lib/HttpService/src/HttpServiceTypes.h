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
 * @file   HttpServiceTypes.h
 * @brief  HTTP service types
 * @author Andreas Merkle (web@blue-andi.de)
 *
 * @addtogroup HTTP_SERVICE
 *
 * @{
 */

#ifndef HTTP_SERVICE_TYPES_H
#define HTTP_SERVICE_TYPES_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <stddef.h>
#include <HTTPClient.h>
#include <Mutex.hpp>
#include "IHttpResponseHandler.h"

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
 * Type definition for HTTP job id.
 */
typedef uint32_t HttpJobId;

/**
 * Invalid HTTP job id.
 */
static constexpr HttpJobId INVALID_HTTP_JOB_ID = 0U;

/**
 * This type defines the HTTP methods.
 */
typedef enum
{
    HTTP_METHOD_GET, /**< HTTP GET method */
    HTTP_METHOD_POST /**< HTTP POST method */

} HttpMethod;

/**
 * This type defines a HTTP request structure.
 * This structure is used to send HTTP requests to the worker task.
 *
 * Attention, constructor and destructor of the members are not called!
 */
struct WorkerRequest
{
    HttpJobId             jobId;   /**< Job id of the HTTP request/response. */
    String                url;     /**< URL of the HTTP request. */
    HttpMethod            method;  /**< HTTP method of the request. */
    const uint8_t*        payload; /**< Payload of the HTTP request (only for POST). */
    size_t                size;    /**< Size of the payload in byte (only for POST). */
    IHttpResponseHandler* handler; /**< Optional response handler which will be called when the response is available. */

    /**
     * Constructs the HTTP request.
     */
    WorkerRequest() :
        jobId(INVALID_HTTP_JOB_ID),
        url(),
        method(HTTP_METHOD_GET),
        payload(nullptr),
        size(0U),
        handler(nullptr)
    {
    }

    /**
     * Copy constructor.
     *
     * @param[in] other   Other HTTP request to copy.
     */
    WorkerRequest(const WorkerRequest& other) :
        jobId(other.jobId),
        url(other.url),
        method(other.method),
        payload(nullptr),
        size(other.size),
        handler(other.handler)
    {
        if (nullptr != other.payload)
        {
            uint8_t* buffer = new (std::nothrow) uint8_t[size];

            if (nullptr == buffer)
            {
                size = 0U;
            }
            else
            {
                memcpy(buffer, other.payload, size);
                payload = buffer;
            }
        }
    }

    /**
     * Move constructor.
     *
     * @param[in] other   Other HTTP request to move from.
     */
    WorkerRequest(WorkerRequest&& other) noexcept :
        jobId(other.jobId),
        url(std::move(other.url)),
        method(other.method),
        payload(other.payload),
        size(other.size),
        handler(other.handler)
    {
        other.payload = nullptr;
        other.size    = 0U;
    }

    /**
     * Destroys the HTTP request.
     */
    ~WorkerRequest()
    {
        if (nullptr != payload)
        {
            delete[] payload;
            payload = nullptr;
        }
    }

    /**
     * Assignment operator.
     *
     * @param[in] other   Other HTTP request to assign.
     *
     * @return Reference to this HTTP request.
     */
    WorkerRequest& operator=(const WorkerRequest& other)
    {
        if (this != &other)
        {
            jobId   = other.jobId;
            url     = other.url;
            method  = other.method;
            handler = other.handler;

            if (nullptr != payload)
            {
                delete[] payload;
                payload = nullptr;
            }

            payload = nullptr;
            size    = other.size;

            if (nullptr != other.payload)
            {
                uint8_t* buffer = new (std::nothrow) uint8_t[size];

                if (nullptr == buffer)
                {
                    size = 0U;
                }
                else
                {
                    memcpy(buffer, other.payload, size);
                    payload = buffer;
                }
            }
        }

        return *this;
    }
};

/**
 * This type defines a worker response structure.
 * This structure is used to send back the HTTP response from the worker task.
 *
 * Attention, constructor and destructor of the members are not called!
 */
struct WorkerResponse
{
    HttpJobId    jobId;      /**< Job id of the HTTP request/response. */
    t_http_codes statusCode; /**< HTTP status code of the response. */
    uint8_t*     payload;    /**< Payload of the HTTP response. */
    size_t       size;       /**< Size of the payload in byte. */

    /**
     * Constructs the HTTP response.
     */
    WorkerResponse() :
        jobId(INVALID_HTTP_JOB_ID),
        statusCode(HTTP_CODE_OK),
        payload(nullptr),
        size(0U)
    {
    }

    /**
     * Copy constructor.
     *
     * @param[in] other   Other HTTP response to copy.
     */
    WorkerResponse(const WorkerResponse& other) :
        jobId(other.jobId),
        statusCode(other.statusCode),
        payload(nullptr),
        size(other.size)
    {
        if (nullptr != other.payload)
        {
            uint8_t* buffer = new (std::nothrow) uint8_t[size];

            if (nullptr == buffer)
            {
                size = 0U;
            }
            else
            {
                memcpy(buffer, other.payload, size);
                payload = buffer;
            }
        }
    }

    /**
     * Move constructor.
     *
     * @param[in] other   Other HTTP response to move from.
     */
    WorkerResponse(WorkerResponse&& other) noexcept :
        jobId(other.jobId),
        statusCode(other.statusCode),
        payload(other.payload),
        size(other.size)
    {
        other.payload = nullptr;
        other.size    = 0U;
    }

    /**
     * Destroys the HTTP response.
     */
    ~WorkerResponse()
    {
        if (nullptr != payload)
        {
            delete[] payload;
            payload = nullptr;
        }
    }

    /**
     * Assignment operator.
     *
     * @param[in] other   Other HTTP response to assign.
     *
     * @return Reference to this HTTP response.
     */
    WorkerResponse& operator=(const WorkerResponse& other)
    {
        if (this != &other)
        {
            jobId      = other.jobId;
            statusCode = other.statusCode;

            if (nullptr != payload)
            {
                delete[] payload;
                payload = nullptr;
            }

            payload = nullptr;
            size    = other.size;

            if (nullptr != other.payload)
            {
                uint8_t* buffer = new (std::nothrow) uint8_t[size];

                if (nullptr == buffer)
                {
                    size = 0U;
                }
                else
                {
                    memcpy(buffer, other.payload, size);
                    payload = buffer;
                }
            }
        }

        return *this;
    }

    /**
     * Append data to the payload.
     *
     * @param[in] data      Data to append.
     * @param[in] dataSize  Size of the data in byte.
     */
    void append(uint8_t* data, size_t dataSize)
    {
        if (nullptr == payload)
        {
            payload = new (std::nothrow) uint8_t[dataSize];
            if (nullptr != payload)
            {
                memcpy(payload, data, dataSize);
                size = dataSize;
            }
        }
        else
        {
            uint8_t* newPayload = new (std::nothrow) uint8_t[this->size + dataSize];

            if (nullptr != newPayload)
            {
                memcpy(newPayload, payload, this->size);
                memcpy(newPayload + this->size, data, dataSize);

                delete[] payload;
                payload  = newPayload;
                size    += dataSize;
            }
        }
    }
};

/**
 * This type defines the worker data, that are
 * task safe to be used from the worker task.
 */
typedef struct
{
    Mutex          mutex;      /**< Mutex to protect the worker data. */
    WorkerRequest  request;    /**< HTTP request. */
    WorkerResponse response;   /**< HTTP response. */
    HttpJobId      jobToAbort; /**< Job to abort. */
    HttpJobId      abortedJob; /**< Aborted job. */

} WorkerData;

/******************************************************************************
 * Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* HTTP_SERVICE_TYPES_H */

/** @} */