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
 * @file   FileUtil.h
 * @brief  File utilities
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup UTILITIES
 *
 * @{
 */

#ifndef FILE_UTIL_H
#define FILE_UTIL_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <WString.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** File utilities */
namespace FileUtil
{

/**
 * Create directories recursively by parsing the given path.
 *
 * @param[in] path  Path to create.
 *
 * @return If successful created, it will return true otherwise false.
 */
extern bool createDirectories(const String& path);

/**
 * Get file name from file path.
 *
 * @param[in] filePath  File path
 *
 * @return File name
 */
extern String getFileName(const String& filePath);

/**
 * Get file extension from file path.
 *
 * @param[in] filePath  File path
 *
 * @return File extension
 */
extern String getFileExtension(const String& filePath);

}; /* namespace FileUtil */

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* FILE_UTIL_H */

/** @} */