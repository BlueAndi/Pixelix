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
#include <MemUtil.h>
#include <utility>

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

/**
 * Hard limit for the largest allocatable internal heap block required to start
 * a new outgoing HTTP request.
 *
 * This keeps background polling (e.g. weather/plugin updates) from adding extra
 * pressure when web page loads already fragment the internal heap.
 */
static const size_t MIN_FREE_BLOCK_FOR_OUTGOING_HTTP_REQUEST = 20U * 1024U;

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
        if (false == m_workerData.mutex.create())
        {
            isSuccessful = false;
        }
        else if (false == m_worker.start(&m_workerData))
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

    m_workerData.mutex.destroy();

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
        if (true == m_workerData.mutex.take(0U))
        {
            /* Handle received HTTP responses. */
            if (INVALID_HTTP_JOB_ID != m_workerData.response.jobId)
            {
                m_responseList.emplace_back(std::move(m_workerData.response));

                /* Clear worker response. */
                m_workerData.response = WorkerResponse();

                /* Clear active job id. */
                m_activeJobId         = INVALID_HTTP_JOB_ID;
            }

            /* Handle pending HTTP requests. */
            if (INVALID_HTTP_JOB_ID == m_activeJobId)
            {
                /* Check for new requests to process. */
                if (false == m_requestList.empty())
                {
                    m_workerData.request = std::move(m_requestList.front());
                    m_activeJobId        = m_workerData.request.jobId;

                    /* Remove the request from the list. */
                    (void)m_requestList.erase(m_requestList.begin());
                }
            }

            (void)m_workerData.mutex.give();
        }
    }
}

HttpJobId HttpService::get(const char* url, IHttpResponseHandler* handler)
{
    HttpJobId         jobId = INVALID_HTTP_JOB_ID;
    MutexGuard<Mutex> guard(m_mutex);

    if ((true == m_isRunning) && (nullptr != url))
    {
        size_t largestFreeBlock = MemUtil::getLargestFreeBlockSize();

        if (MIN_FREE_BLOCK_FOR_OUTGOING_HTTP_REQUEST <= largestFreeBlock)
        {
            WorkerRequest request;

            request.jobId   = generateJobId();
            request.url     = url;
            request.method  = HTTP_METHOD_GET;
            request.payload = nullptr;
            request.size    = 0U;
            request.handler = handler;

            m_requestList.emplace_back(std::move(request));
            jobId = m_requestList.back().jobId;
        }
        else
        {
            LOG_WARNING("Outgoing HTTP GET skipped, largest free block too low (%u byte, min %u byte)",
                largestFreeBlock,
                MIN_FREE_BLOCK_FOR_OUTGOING_HTTP_REQUEST);
        }
    }

    return jobId;
}

HttpJobId HttpService::post(const char* url, const uint8_t* payload, size_t size, IHttpResponseHandler* handler)
{
    HttpJobId         jobId = INVALID_HTTP_JOB_ID;
    MutexGuard<Mutex> guard(m_mutex);

    if ((true == m_isRunning) && (nullptr != url))
    {
        size_t largestFreeBlock = MemUtil::getLargestFreeBlockSize();

        if (MIN_FREE_BLOCK_FOR_OUTGOING_HTTP_REQUEST <= largestFreeBlock)
        {
            WorkerRequest request;

            request.jobId   = generateJobId();
            request.url     = url;
            request.method  = HTTP_METHOD_POST;
            request.handler = handler;

            if (true == request.setPayload(payload, size))
            {
                m_requestList.emplace_back(std::move(request));
                jobId = m_requestList.back().jobId;
            }
        }
        else
        {
            LOG_WARNING("Outgoing HTTP POST skipped, largest free block too low (%u byte, min %u byte)",
                largestFreeBlock,
                MIN_FREE_BLOCK_FOR_OUTGOING_HTTP_REQUEST);
        }
    }

    return jobId;
}

bool HttpService::getResponse(HttpJobId jobId, HttpRsp& response)
{
    bool              isAvailable = false;
    MutexGuard<Mutex> guard(m_mutex);

    if ((true == m_isRunning) &&
        (INVALID_HTTP_JOB_ID != jobId))
    {
        WorkerResponse workerRsp;

        /* Search for the response with the given job id. */
        for (WorkerResponseList::iterator it = m_responseList.begin(); it != m_responseList.end(); ++it)
        {
            if (jobId == it->jobId)
            {
                workerRsp = std::move(*it);

                /* Remove the response from the list. */
                (void)m_responseList.erase(it);
                isAvailable = true;
                break;
            }
        }

        if (true == isAvailable)
        {
            /* Ensure that no memory is leaked. */
            response.releasePayload();

            response.statusCode = workerRsp.statusCode;

            /* Move memory ownership from worker response to HTTP response. */
            response.payload    = workerRsp.payload;
            response.size       = workerRsp.size;
            workerRsp.payload   = nullptr;
            workerRsp.size      = 0U;
        }
    }

    return isAvailable;
}

void HttpService::abortJob(HttpJobId jobId)
{
    MutexGuard<Mutex> guard(m_mutex);

    if ((true == m_isRunning) &&
        (INVALID_HTTP_JOB_ID != jobId))
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

                (void)m_workerData.mutex.take(portMAX_DELAY);
                m_workerData.jobToAbort = jobId;
                (void)m_workerData.mutex.give();

                /* Wait till the job is aborted. */
                while (1)
                {
                    /* The worker holds the mutex, e.g. while handling the HTTP response. Sleep at least 1 ms, otherwise
                     * the idle task of this core gets starved.
                     */
                    if (true == m_workerData.mutex.take(1U))
                    {
                        /* Aborted? */
                        if (INVALID_HTTP_JOB_ID != m_workerData.abortedJob)
                        {
                            isAborted               = true;
                            m_workerData.abortedJob = INVALID_HTTP_JOB_ID;
                            (void)m_workerData.mutex.give();
                            break;
                        }

                        /* The worker already finished the job and is idle, so it will never consume jobToAbort. Discard the pending
                         * response and treat the job as aborted, otherwise this loop would wait forever and block the calling task.
                         */
                        if (jobId == m_workerData.response.jobId)
                        {
                            m_workerData.response   = WorkerResponse();
                            m_workerData.jobToAbort = INVALID_HTTP_JOB_ID;
                            m_activeJobId           = INVALID_HTTP_JOB_ID;
                            isAborted               = true;
                            (void)m_workerData.mutex.give();
                            break;
                        }

                        /* No pending job anymore? */
                        if (INVALID_HTTP_JOB_ID == m_workerData.jobToAbort)
                        {
                            m_activeJobId = INVALID_HTTP_JOB_ID;
                            (void)m_workerData.mutex.give();
                            break;
                        }
                        /* Wait until worker aborted the job. */
                        else
                        {
                            (void)m_workerData.mutex.give();
                            delay(1U);
                        }
                    }
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
