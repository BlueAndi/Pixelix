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
 * @file   HttpService.cpp
 * @brief  HTTP service
 * @author Andreas Merkle (web@blue-andi.de)
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "HttpService.h"
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
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

bool HttpService::start()
{
    bool isSuccessful = true;

    /* Is service already running? */
    if (true == m_isRunning)
    {
        /* Nothing to do. */
        ;
    }
    else
    {
        if (false == m_workerQueues.requestQueue.create(WORKER_REQUEST_QUEUE_LENGTH))
        {
            isSuccessful = false;
        }
        else if (false == m_workerQueues.responseQueue.create(WORKER_RESPONSE_QUEUE_LENGTH))
        {
            isSuccessful = false;
        }
        else if (false == m_workerQueues.abortJobQueue.create(WORKER_ABORT_JOB_QUEUE_LENGTH))
        {
            isSuccessful = false;
        }
        else if (false == m_workerQueues.abortedJobQueue.create(WORKER_ABORTED_JOB_QUEUE_LENGTH))
        {
            isSuccessful = false;
        }
        else if (false == m_worker.start(&m_workerQueues))
        {
            isSuccessful = false;
        }
        else
        {
            ;
        }
    }

    if (false == isSuccessful)
    {
        stop();
    }
    else if (true == m_isRunning)
    {
        LOG_WARNING("HTTP service is already started.");
    }
    else
    {
        m_isRunning = true;
        LOG_INFO("HTTP service started.");
    }
    return m_isRunning;
}

void HttpService::stop()
{
    (void)m_worker.stop();

    m_requestList.clear();
    m_responseList.clear();

    m_workerQueues.responseQueue.destroy();
    m_workerQueues.requestQueue.destroy();
    m_workerQueues.abortJobQueue.destroy();
    m_workerQueues.abortedJobQueue.destroy();

    m_mutex.destroy();

    if (true == m_isRunning)
    {
        m_isRunning = false;
        LOG_INFO("HTTP service stopped.");
    }
}

void HttpService::process()
{
    MutexGuard<Mutex> guard(m_mutex);

    if (true == m_isRunning)
    {
        WorkerResponse workerRsp;

        /* Handle received HTTP responses. */
        if (true == m_workerQueues.responseQueue.receive(&workerRsp, 0U))
        {
            m_responseList.push_back(workerRsp);

            /* Clear active job id. */
            m_activeJobId = INVALID_HTTP_JOB_ID;
        }

        /* Handle pending HTTP requests. */
        if (INVALID_HTTP_JOB_ID == m_activeJobId)
        {
            /* Check for new requests to process. */
            if (false == m_requestList.empty())
            {
                WorkerRequest request = m_requestList.front();

                /* Send request to worker task. */
                if (false == m_workerQueues.requestQueue.sendToBack(request, portMAX_DELAY))
                {
                    LOG_WARNING("Sending HTTP request job id %u to worker failed, request queue full.", request.jobId);
                }
                else
                {
                    m_activeJobId = request.jobId;

                    /* Remove the request from the list. */
                    (void)m_requestList.erase(m_requestList.begin());
                }
            }
        }
    }
}

HttpJobId HttpService::get(const char* url, IHttpResponseHandler* handler)
{
    HttpJobId         jobId = INVALID_HTTP_JOB_ID;
    MutexGuard<Mutex> guard(m_mutex);

    if (true == m_isRunning)
    {
        WorkerRequest request;

        request.jobId   = generateJobId();
        request.url     = url;
        request.method  = HTTP_METHOD_GET;
        request.payload = nullptr;
        request.size    = 0U;
        request.handler = handler;

        m_requestList.push_back(request);

        jobId = request.jobId;
    }

    return jobId;
}

HttpJobId HttpService::post(const char* url, const uint8_t* payload, size_t size, IHttpResponseHandler* handler)
{
    HttpJobId         jobId = INVALID_HTTP_JOB_ID;
    MutexGuard<Mutex> guard(m_mutex);

    if (true == m_isRunning)
    {
        WorkerRequest request;

        request.jobId   = generateJobId();
        request.url     = url;
        request.method  = HTTP_METHOD_POST;
        request.payload = payload;
        request.size    = size;
        request.handler = handler;

        m_requestList.push_back(request);

        jobId = request.jobId;
    }

    return jobId;
}

bool HttpService::getResponse(HttpJobId jobId, HttpRsp& response)
{
    bool              isAvailable = false;
    MutexGuard<Mutex> guard(m_mutex);

    if (true == m_isRunning)
    {
        WorkerResponse workerRsp;

        /* Search for the response with the given job id. */
        for (auto it = m_responseList.begin(); it != m_responseList.end(); ++it)
        {
            if (jobId == it->jobId)
            {
                workerRsp = *it;

                /* Remove the response from the list. */
                (void)m_responseList.erase(it);
                isAvailable = true;
                break;
            }
        }
    }

    return isAvailable;
}

void HttpService::abortJob(HttpJobId jobId)
{
    MutexGuard<Mutex> guard(m_mutex);

    if (true == m_isRunning)
    {
        bool isAborted = false;

        /* Is in the pending request queue? */
        for (auto it = m_requestList.begin(); it != m_requestList.end(); ++it)
        {
            if (jobId == it->jobId)
            {
                /* Remove the request from the list. */
                (void)m_requestList.erase(it);
                isAborted = true;
                break;
            }
        }

        if (false == isAborted)
        {
            /* Is it the currently active job? */
            if (jobId == m_activeJobId)
            {
                HttpJobId abortedJobId = INVALID_HTTP_JOB_ID;

                (void)m_workerQueues.abortJobQueue.sendToBack(jobId, portMAX_DELAY);
                (void)m_workerQueues.abortedJobQueue.receive(&abortedJobId, portMAX_DELAY);

                if (jobId == abortedJobId)
                {
                    isAborted     = true;
                    m_activeJobId = INVALID_HTTP_JOB_ID;
                }
            }
        }

        if (false == isAborted)
        {
            /* Is in the response list? */
            for (auto it = m_responseList.begin(); it != m_responseList.end(); ++it)
            {
                if (jobId == it->jobId)
                {
                    /* Remove the response from the list. */
                    (void)m_responseList.erase(it);
                    isAborted = true;
                    break;
                }
            }
        }
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

HttpJobId HttpService::generateJobId()
{
    /* Increment job ID counter and avoid invalid job ID.
     * Its a simple wrap around counter, because the chance of job ID collision
     * is very low in this application.
     */
    ++m_jobIdCounter;

    if (INVALID_HTTP_JOB_ID == m_jobIdCounter)
    {
        ++m_jobIdCounter;
    }

    return m_jobIdCounter;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
