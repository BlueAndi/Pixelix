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
 * @file   FileUtil.cpp
 * @brief  File utilities
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "FileUtil.h"

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

/******************************************************************************
 * External Functions
 *****************************************************************************/

bool FileUtil::createDirectories(const String& path, FS& fs)
{
    bool    status = true;
    uint8_t idx    = 0U;
    String  currentPath;

    while ((true == status) && (path.length() > idx))
    {
        if ('/' == path[idx])
        {
            if (0U < currentPath.length())
            {
                if (false == fs.exists(currentPath))
                {
                    status = fs.mkdir(currentPath);
                }
            }
        }

        currentPath += path[idx];

        ++idx;
    }

    return status;
}

String FileUtil::getFileName(const String& filePath)
{
    String  fileName;
    int32_t lastSlashIdx = filePath.lastIndexOf('/');

    if (0 <= lastSlashIdx)
    {
        fileName = filePath.substring(lastSlashIdx + 1);
    }
    else
    {
        fileName = filePath;
    }

    return fileName;
}

String FileUtil::getFileExtension(const String& filePath)
{
    String  fileExtension;
    int32_t lastDotIdx = filePath.lastIndexOf('.');

    if (0 <= lastDotIdx)
    {
        fileExtension = filePath.substring(lastDotIdx + 1);
    }

    return fileExtension;
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/