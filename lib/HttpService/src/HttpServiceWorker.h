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
 * @file   HttpServiceWorker.h
 * @brief  HTTP service worker
 * @author Andreas Merkle (web@blue-andi.de)
 *
 * @addtogroup HTTP_SERVICE
 *
 * @{
 */

#ifndef HTTP_SERVICE_WORKER_H
#define HTTP_SERVICE_WORKER_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <Task.hpp>
#include "HttpServiceTypes.h"

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
 * The HTTP service worker handles the HTTP requests in the context of a task.
 */
class HttpServiceWorker : public Task<WorkerData>
{
public:

    /**
     * Constructs the HTTP service worker.
     */
    HttpServiceWorker() :
        Task<WorkerData>("HttpServiceWorkerTask", DEFAULT_STACK_SIZE)
    {
        ;
    }

    /**
     * Destroys the HTTP service worker.
     */
    ~HttpServiceWorker()
    {
        /* Nothing to do. */
    }

private:

    /** Default stack size for the worker task. */
    static const size_t DEFAULT_STACK_SIZE                        = 8192U;

    /** Maximum wait time for mutex operations. */
    static const TickType_t MAX_WAIT_TIME                         = pdMS_TO_TICKS(100U);

    /**
     * Copy constructor not allowed.
     *
     * @param[in] worker    Worker to copy.
     */
    HttpServiceWorker(const HttpServiceWorker& worker)            = delete;

    /**
     * Assignment operator not allowed.
     *
     * @param[in] worker    Worker to assign.
     *
     * @return Reference to this worker.
     */
    HttpServiceWorker& operator=(const HttpServiceWorker& worker) = delete;

    /**
     * Process the worker task.
     *
     * @param[in] data  Worker data used for communication.
     */
    void process(WorkerData* data) final;

    /**
     * Perform the HTTP request.
     *
     * @param[in]  request      Worker request.
     * @param[out] response     Worker response.
     * @param[in]  mutex        Mutex to protect the worker data.
     * @param[in]  jobToAbort   Job id to abort.
     */
    void performHttpRequest(const WorkerRequest& request, WorkerResponse& response, Mutex& mutex, HttpJobId& jobToAbort);

    /**
     * Handle the HTTP response.
     *
     * @param[in]  httpClient   HTTP client which holds the response.
     * @param[in]  handler      Optional response handler which will be called when the response is available.
     * @param[out] response     Worker response.
     */
    void handleHttpResponse(HTTPClient& httpClient, IHttpResponseHandler* handler, WorkerResponse& response);
};

/******************************************************************************
 * Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* HTTP_SERVICE_WORKER_H */

/** @} */