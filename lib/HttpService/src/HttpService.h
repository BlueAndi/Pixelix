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
 * @file   HttpService.h
 * @brief  HTTP service
 * @author Andreas Merkle (web@blue-andi.de)
 *
 * @addtogroup HTTP_SERVICE
 *
 * @{
 */

#ifndef HTTP_SERVICE_H
#define HTTP_SERVICE_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <IService.hpp>
#include <Mutex.hpp>
#include <Task.hpp>
#include <WString.h>
#include <HTTPClient.h>
#include <vector>

#include "HttpServiceTypes.h"
#include "HttpRsp.h"
#include "IHttpResponseHandler.h"
#include "HttpServiceWorker.h"

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
 * The HTTP service handles outgoing HTTP requests and their responses.
 * It is able to handle multiple requests sequentially.
 *
 * It supports HTTP and HTTPS requests.
 */
class HttpService : public IService
{
public:

    /**
     * Get the REST service instance.
     *
     * @return REST service instance
     */
    static HttpService& getInstance()
    {
        static HttpService instance; /* idiom */

        return instance;
    }

    /**
     * Start the service.
     *
     * @return If successful started, it will return true otherwise false.
     */
    bool start() final;

    /**
     * Stop the service.
     */
    void stop() final;

    /**
     * Process the service.
     */
    void process() final;

    /**
     * Send a HTTP GET request.
     *
     * If a response handler is provided, it will be called in the context of the
     * worker task when the response streams in. Its not allowed to call any
     * HttpService methods from within the response handler.
     *
     * @param[in] url       URL of the HTTP GET request.
     * @param[in] handler   Optional response handler which will be called when the response is available.
     *
     * @return HTTP job id. If the job id is INVALID_HTTP_JOB_ID, the request could not be started.
     */
    HttpJobId get(const char* url, IHttpResponseHandler* handler = nullptr);

    /**
     * Send a HTTP POST request.
     *
     * If a response handler is provided, it will be called in the context of the
     * worker task when the response streams in. Its not allowed to call any
     * HttpService methods from within the response handler.
     *
     * Attention, the payload must remain valid until the response is received.
     *
     * @param[in] url     URL of the HTTP POST request.
     * @param[in] payload Payload of the HTTP POST request.
     * @param[in] size    Size of the payload in byte.
     * @param[in] handler Optional response handler which will be called when the response is available.
     *
     * @return HTTP job id. If the job id is INVALID_HTTP_JOB_ID, the request could not be started.
     */
    HttpJobId post(const char* url, const uint8_t* payload, size_t size, IHttpResponseHandler* handler = nullptr);

    /**
     * Get the HTTP response for a previously started request.
     *
     * @param[in]  jobId    HTTP job id of the previously started request.
     * @param[out] response HTTP response.
     *
     * @return If a response is available, it will return true otherwise false.
     */
    bool getResponse(HttpJobId jobId, HttpRsp& response);

    /**
     * Abort a previously started HTTP request.
     * This method will block until the request is aborted.
     *
     * Blocking ensures that all resources allocated by the application for the request are
     * released when this method returns.
     *
     * @param[in] jobId    HTTP job id of the previously started request.
     */
    void abortJob(HttpJobId jobId);

private:

    /**
     * Type definition for a list of worker requests.
     */
    typedef std::vector<WorkerRequest> WorkerRequestList;

    /**
     * Type definition for a list of worker responses.
     */
    typedef std::vector<WorkerResponse> WorkerResponseList;

    Mutex                               m_mutex;        /**< Mutex to protect against concurrent access. */
    bool                                m_isRunning;    /**< Signals the status of the service. True means it is running, false means it is stopped. */
    WorkerRequestList                   m_requestList;  /**< List to store pending HTTP requests. */
    WorkerResponseList                  m_responseList; /**< List to store received HTTP responses. */
    HttpJobId                           m_jobIdCounter; /**< Counter to generate unique job ids. */
    HttpJobId                           m_activeJobId;  /**< Currently active job id. */
    WorkerData                          m_workerData;   /**< Data for worker task. */
    HttpServiceWorker                   m_worker;       /**< Worker context for the worker task. */

    /**
     * Constructs the service instance.
     */
    HttpService() :
        IService(),
        m_mutex(),
        m_isRunning(false),
        m_requestList(),
        m_responseList(),
        m_jobIdCounter(INVALID_HTTP_JOB_ID),
        m_activeJobId(INVALID_HTTP_JOB_ID),
        m_workerData(),
        m_worker()
    {
    }

    /**
     * Destroys the service instance.
     */
    ~HttpService()
    {
        /* Never called. */
    }

    /* An instance shall not be copied. */
    HttpService(const HttpService& service);
    HttpService& operator=(const HttpService& service);

    /**
     * Generate a new job id.
     *
     * @return New job id.
     */
    HttpJobId generateJobId();
};

/******************************************************************************
 * Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* HTTP_SERVICE_H */

/** @} */