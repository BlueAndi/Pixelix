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
 * @file   ArtworkDownloader.cpp
 * @brief  Makapix artwork downloader
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "ArtworkDownloader.h"
#include <HttpService.h>
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

bool ArtworkDownloader::download(const char* url, const char* dstFilePath)
{
    bool isSuccessful = false;

    if ((nullptr != url) &&
        (nullptr != dstFilePath) &&
        (INVALID_HTTP_JOB_ID == m_httpJobId)) /* No pending download! */
    {
        HttpService& httpService = HttpService::getInstance();

        m_httpFileResponseHandler.setFilePath(dstFilePath);
        m_httpJobId = httpService.get(url, &m_httpFileResponseHandler);

        if (INVALID_HTTP_JOB_ID == m_httpJobId)
        {
            LOG_ERROR("Failed to start artwork download from URL: %s", url);
        }
        else
        {
            m_dstFilePath = dstFilePath;
            isSuccessful  = true;
        }
    }

    return isSuccessful;
}

bool ArtworkDownloader::download(const String& url, const String& dstFilePath)
{
    return download(url.c_str(), dstFilePath.c_str());
}

bool ArtworkDownloader::getStatus(bool& isSuccessful)
{
    bool isFinished = false;

    /* No artwork download is pending? */
    if (INVALID_HTTP_JOB_ID == m_httpJobId)
    {
        isFinished   = true;
        isSuccessful = m_isSuccessful;
    }
    else
    {
        HttpService& httpService = HttpService::getInstance();
        HttpRsp      httpResponse;

        /* Receive HTTP response. */
        if (true == httpService.getResponse(m_httpJobId, httpResponse))
        {
            /* Artwork download failed? */
            if (HTTP_CODE_OK != httpResponse.statusCode)
            {
                LOG_WARNING("Artwork download failed with HTTP status: %d", httpResponse.statusCode);

                m_isSuccessful = false;
            }
            /* Artwork download had file write error? */
            else if (true == m_httpFileResponseHandler.isError())
            {
                LOG_WARNING("Artwork download failed during file write.");

                m_isSuccessful = false;
            }
            /* Artwork successful downloaded. */
            else
            {
                m_isSuccessful = true;
            }

            m_httpJobId = INVALID_HTTP_JOB_ID;
        }
    }

    return isFinished;
}

void ArtworkDownloader::abort()
{
    /* Any artwork download is pending? */
    if (INVALID_HTTP_JOB_ID != m_httpJobId)
    {
        HttpService& httpService = HttpService::getInstance();

        httpService.abortJob(m_httpJobId);

        m_httpJobId = INVALID_HTTP_JOB_ID;
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
