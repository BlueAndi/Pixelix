/* MIT License
 *
 * Copyright (c) 2019 - 2025 Andreas Merkle <web@blue-andi.de>
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
 * @file   HttpServiceWorker.cpp
 * @brief  HTTP service worker
 * @author Andreas Merkle (web@blue-andi.de)
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "HttpServiceWorker.h"
#include <Logging.h>

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

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void HttpServiceWorker::process(WorkerData* data)
{
    if (nullptr != data)
    {
        /* Check for next HTTP request. */
        if (true == data->mutex.take(MAX_WAIT_TIME))
        {
            /* HTTP request available and previous response processed? */
            if ((INVALID_HTTP_JOB_ID != data->request.jobId) &&
                (INVALID_HTTP_JOB_ID == data->response.jobId))
            {
                WorkerResponse response;

                /* Response must have the same job id as the request. */
                response.jobId = data->request.jobId;

                /* Check if the job is aborted. */
                if (data->jobToAbort == data->request.jobId)
                {
                    LOG_INFO("HTTP job id %u is aborted before performing the request.", data->request.jobId);

                    data->jobToAbort = INVALID_HTTP_JOB_ID;
                    data->abortedJob = data->request.jobId;
                }
                else
                {
                    /* Release mutex before performing the HTTP request. */
                    (void)data->mutex.give();

                    /* Perform the HTTP request. */
                    performHttpRequest(data->request, response, data->mutex, data->jobToAbort);

                    /* Wait until mutex is available to store the response. */
                    (void)data->mutex.take(portMAX_DELAY);

                    /* Check if the job is aborted meanwhile. */
                    if (data->jobToAbort == data->request.jobId)
                    {
                        LOG_INFO("HTTP job id %u is aborted after performing the request.", data->request.jobId);

                        data->jobToAbort = INVALID_HTTP_JOB_ID;
                        data->abortedJob = data->request.jobId;
                    }
                    else
                    {
                        /* Send back the HTTP response. */
                        data->response = response;
                    }
                }

                /* Clear the request. */
                data->request = WorkerRequest();
            }

            (void)data->mutex.give();
        }
    }
}

void HttpServiceWorker::performHttpRequest(const WorkerRequest& request, WorkerResponse& response, Mutex& mutex, HttpJobId& jobToAbort)
{
    const char* PREFIX_HTTPS = "https://";
    WiFiClient* wifiClient   = nullptr;

    /* HTTP over TLS required? */
    if (true == request.url.startsWith(PREFIX_HTTPS))
    {
        WiFiClientSecure* secureClient = new WiFiClientSecure();

        if (nullptr != secureClient)
        {
            wifiClient = secureClient;
            secureClient->setInsecure();
        }
    }
    /* HTTP over plain TCP. */
    else
    {
        wifiClient = new WiFiClient();
    }

    if (nullptr == wifiClient)
    {
        LOG_WARNING("HTTP request to URL %s failed, no heap memory available.", request.url.c_str());
        response.statusCode = HTTP_CODE_INTERNAL_SERVER_ERROR;
    }
    else
    {
        HTTPClient httpClient;

        httpClient.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
        httpClient.setRedirectLimit(5U);

        if (false == httpClient.begin(*wifiClient, request.url))
        {
            LOG_WARNING("HTTP request failed to %s", request.url.c_str());
            response.statusCode = HTTP_CODE_SERVICE_UNAVAILABLE;
        }
        else
        {
            int httpClientRet = 0;

            LOG_DEBUG("HTTP client connected.");

            switch (request.method)
            {
            case HTTP_METHOD_GET:
                httpClientRet = httpClient.GET();
                break;

            case HTTP_METHOD_POST:
                httpClientRet = httpClient.POST(const_cast<uint8_t*>(request.payload), request.size);
                break;

            default:
                LOG_WARNING("HTTP request to URL %s failed, unsupported HTTP method %d.", request.url.c_str(), request.method);
                response.statusCode = HTTP_CODE_NOT_IMPLEMENTED;
                break;
            }

            if (0 > httpClientRet)
            {
                LOG_WARNING("HTTP request to URL %s failed, error: %s", request.url.c_str(), httpClient.errorToString(httpClientRet).c_str());
                response.statusCode = HTTP_CODE_BAD_REQUEST;
            }
            else
            {
                response.statusCode = static_cast<t_http_codes>(httpClientRet);

                LOG_DEBUG("HTTP client received response with status code %d.", httpClientRet);

                (void)mutex.take(portMAX_DELAY);

                if (HTTP_CODE_OK == response.statusCode)
                {
                    if (request.jobId != jobToAbort)
                    {
                        handleHttpResponse(httpClient, request.handler, response);
                    }
                }

                (void)mutex.give();
            }

            httpClient.end();
        }

        delete wifiClient;
        wifiClient = nullptr;
    }
}

void HttpServiceWorker::handleHttpResponse(HTTPClient& httpClient, IHttpResponseHandler* handler, WorkerResponse& response)
{
    int32_t     contentLength = httpClient.getSize(); /* Get size of the payload. If no Content-Length header present, size will be -1. */
    uint8_t     buffer[1024U];
    WiFiClient& stream = httpClient.getStream();
    uint32_t    index  = 0U;

    while ((true == httpClient.connected()) && ((0 < contentLength) || (-1 == contentLength)))
    {
        int32_t toRead = stream.available();

        if (0 < toRead)
        {
            char*   cBuffer = static_cast<char*>(static_cast<void*>(buffer));
            int32_t read    = stream.readBytes(cBuffer, (toRead < sizeof(buffer)) ? toRead : sizeof(buffer));

            if (0 < read)
            {
                if (0 < contentLength)
                {
                    if (read > contentLength)
                    {
                        contentLength = 0;
                    }
                    else
                    {
                        contentLength -= read;
                    }
                }

                ++index;

                /* If a response handler is provided, call it to process the received payload chunk. */
                if (nullptr != handler)
                {
                    bool isFinal = (0 == contentLength);

                    handler->onResponse(index, isFinal, buffer, static_cast<size_t>(read));
                }
                else
                {
                    /* Append data to the response payload. */
                    response.append(buffer, static_cast<size_t>(read));
                }
            }
        }

        /* Give other tasks a chance to run. */
        delay(1U);
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
