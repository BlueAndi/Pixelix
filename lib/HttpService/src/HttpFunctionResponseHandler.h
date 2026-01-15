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
 * @file   HttpFunctionResponseHandler.h
 * @brief  HTTP function response handler
 * @author Andreas Merkle (web@blue-andi.de)
 *
 * @addtogroup HTTP_SERVICE
 *
 * @{
 */

#ifndef HTTP_FUNCTION_RESPONSE_HANDLER_H
#define HTTP_FUNCTION_RESPONSE_HANDLER_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <functional>
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
 * HTTP response handler that calls a function when a response chunk is available.
 */
class HttpFunctionResponseHandler : public IHttpResponseHandler
{
public:

    /**
     * Type definition for the response function.
     *
     * @param[in] index     Index of the response chunk, starting from 0.
     * @param[in] isFinal   Indicates that this is the final chunk.
     * @param[in] payload   Payload of the HTTP response.
     * @param[in] size      Size of the payload in byte.
     */
    typedef std::function<void(uint32_t index, bool isFinal, const uint8_t* payload, size_t size)> ResponseFunction;

    /**
     * Constructs the function response handler.
     *
     * @param[in] function  Function to call when a response chunk is available.
     */
    HttpFunctionResponseHandler(ResponseFunction function) :
        m_function(function)
    {
    }

    /**
     * Destroys the function response handler.
     */
    ~HttpFunctionResponseHandler()
    {
    }

    /**
     * This method will be called when a HTTP response is available.
     *
     * @param[in] index     Index of the response chunk, starting from 0.
     * @param[in] isFinal   Indicates that this is the final chunk.
     * @param[in] payload   Payload of the HTTP response.
     * @param[in] size      Size of the payload in byte.
     */
    void onResponse(uint32_t index, bool isFinal, const uint8_t* payload, size_t size) final
    {
        if (nullptr != m_function)
        {
            m_function(index, isFinal, payload, size);
        }
    }

private:

    ResponseFunction m_function; /**< Function to call when a response chunk is available. */

    /**
     * Disable default constructor.
     */
    HttpFunctionResponseHandler()                                                      = delete;

    /**
     * Disable copy constructor.
     *
     * @param[in] handler   Handler to copy.
     */
    HttpFunctionResponseHandler(const HttpFunctionResponseHandler& handler)            = delete;

    /**
     * Disable assignment operator.
     *
     * @param[in] handler   Handler to assign.
     *
     * @return Assigned handler.
     */
    HttpFunctionResponseHandler& operator=(const HttpFunctionResponseHandler& handler) = delete;
};

/******************************************************************************
 * Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* HTTP_FUNCTION_RESPONSE_HANDLER_H */

/** @} */