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
 * @file   HttpFileResponseHandler.cpp
 * @brief  HTTP file response handler
 * @author Andreas Merkle (web@blue-andi.de)
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "HttpFileResponseHandler.h"
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

HttpFileResponseHandler& HttpFileResponseHandler::operator=(const HttpFileResponseHandler& other)
{
    if (this != &other)
    {
        m_filePath = other.m_filePath;
        m_isError  = other.m_isError;
        m_file     = File(); /* Do not copy file handle. */
    }

    return *this;
}

void HttpFileResponseHandler::onResponse(uint32_t index, bool isFinal, const uint8_t* payload, size_t size)
{
    if (0U == index)
    {
        m_isError = false;
        m_file    = FILESYSTEM.open(m_filePath, FILE_WRITE);

        if (false == m_file)
        {
            LOG_ERROR("Unable to open file %s for writing HTTP response.", m_filePath.c_str());
            m_isError = true;
        }
        else
        {
            LOG_DEBUG("Writing HTTP response to file %s.", m_filePath.c_str());
        }
    }

    if (true == m_file)
    {
        if (size != m_file.write(payload, size))
        {
            LOG_ERROR("Unable to write HTTP response to file %s.", m_filePath.c_str());
            m_isError = true;
        }

        if (true == m_isError)
        {
            m_file.close();
        }
        else if (true == isFinal)
        {
            LOG_DEBUG("Finished writing HTTP response to file %s.", m_filePath.c_str());
            m_file.close();
        }
        else
        {
            /* Continue writing. */
            ;
        }
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
