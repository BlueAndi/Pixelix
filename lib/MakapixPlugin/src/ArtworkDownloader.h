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
 * @file   ArtworkDownloader.h
 * @brief  Makapix artwork downloader
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup PLUGIN
 *
 * @{
 */

#ifndef ARTWORK_DOWNLOADER_H
#define ARTWORK_DOWNLOADER_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <WString.h>
#include <HttpServiceTypes.h>
#include <HttpFileResponseHandler.h>
#include "MakapixTypes.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Makapix artwork downloader class.
 */
class ArtworkDownloader
{
public:

    /**
     * Constructs the artwork downloader.
     */
    ArtworkDownloader() :
        m_httpFileResponseHandler(FILE_CACHE_FS),
        m_dstFilePath(),
        m_httpJobId(INVALID_HTTP_JOB_ID),
        m_isSuccessful(false)
    {
    }

    /**
     * Destroys the artwork downloader.
     */
    ~ArtworkDownloader()
    {
    }

    /**
     * Start artwork download from URL to destination file path.
     *
     * @param[in] url          Artwork URL
     * @param[in] dstFilePath  Destination file path
     *
     * @return If download started successfully, it will return true otherwise false.
     */
    bool download(const char* url, const char* dstFilePath);

    /**
     * Start artwork download from URL to destination file path.
     *
     * @param[in] url          Artwork URL
     * @param[in] dstFilePath  Destination file path
     *
     * @return If download started successfully, it will return true otherwise false.
     */
    bool download(const String& url, const String& dstFilePath);

    /**
     * Get destination file path of the currently downloading artwork.
     *
     * @return Destination file path
     */
    const String& getDestinationFilePath() const
    {
        return m_dstFilePath;
    }

    /**
     * Check if a download is pending.
     *
     * @return If download is pending, it will return true otherwise false.
     */
    bool isPending() const
    {
        return (INVALID_HTTP_JOB_ID != m_httpJobId);
    }

    /**
     * Get download status.
     *
     * @param[out] isSuccessful If download was successful it will be true otherwise false.
     *
     * @return If download is finished, it will return true otherwise false.
     */
    bool getStatus(bool& isSuccessful);

    /**
     * Abort ongoing download.
     */
    void abort();

private:

    /**
     * HTTP file response handler to download artwork files.
     */
    HttpFileResponseHandler m_httpFileResponseHandler;

    /**
     * Destination file path of the currently downloading artwork.
     */
    String m_dstFilePath;

    /**
     * HTTP job id of the current downloading file. If now no download is pending,
     * it will be INVALID_HTTP_JOB_ID.
     */
    HttpJobId m_httpJobId;

    /**
     * Download success flag.
     */
    bool m_isSuccessful;
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* ARTWORK_DOWNLOADER_H */

/** @} */
