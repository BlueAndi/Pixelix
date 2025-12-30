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
 * @file   HttpFileResponseHandler.h
 * @brief  HTTP file response handler
 * @author Andreas Merkle (web@blue-andi.de)
 *
 * @addtogroup HTTP_SERVICE
 *
 * @{
 */

#ifndef HTTP_FILE_RESPONSE_HANDLER_H
#define HTTP_FILE_RESPONSE_HANDLER_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <FileSystem.h>

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
 * HTTP response handler that writes the payload into a file.
 */
class HttpFileResponseHandler : public IHttpResponseHandler
{
public:

    /**
     * Constructs the file response handler.
     */
    HttpFileResponseHandler() :
        m_filePath(nullptr),
        m_file(),
        m_isError(false)
    {
    }

    /**
     * Constructs the file response handler.
     *
     * @param[in] filePath  Path of the file where the payload will be written to.
     */
    HttpFileResponseHandler(const char* filePath) :
        m_filePath(filePath),
        m_file(),
        m_isError(false)
    {
    }

    /**
     * Destroys the file response handler.
     */
    virtual ~HttpFileResponseHandler()
    {
    }

    /**
     * Get file path.
     *
     * @return File path
     */
    const char* getFilePath() const
    {
        return m_filePath;
    }

    /**
     * Set file path.
     * 
     * @param[in] filePath  File path
     */
    void setFilePath(const char* filePath)
    {
        m_filePath = filePath;
    }

    /**
     * Check if an error occurred during file creation.
     *
     * @return If an error occurred, it will return true otherwise false.
     */
    bool isError() const
    {
        return m_isError;
    }

    /**
     * This method will be called when a HTTP response is available.
     *
     * @param[in] index     Index of the response chunk, starting from 0.
     * @param[in] isFinal   Indicates that this is the final chunk.
     * @param[in] payload   Payload of the HTTP response.
     * @param[in] size      Size of the payload in byte.
     */
    void onResponse(uint32_t index, bool isFinal, const uint8_t* payload, size_t size) final;

private:

    const char* m_filePath; /**< Path of the file where the payload will be written to. */
    File        m_file;     /**< File handle. */
    bool        m_isError;  /**< Indicates that an error occurred during file creation. */

    /**
     * Disable copy constructor.
     *
     * @param[in] handler   Handler to copy.
     */
    HttpFileResponseHandler(const HttpFileResponseHandler& handler)            = delete;

    /**
     * Disable assignment operator.
     *
     * @param[in] handler   Handler to assign.
     *
     * @return Assigned handler.
     */
    HttpFileResponseHandler& operator=(const HttpFileResponseHandler& handler) = delete;
};

/******************************************************************************
 * Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* HTTP_FILE_RESPONSE_HANDLER_H */

/** @} */