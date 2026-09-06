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
 * @file   HostCompat.h
 * @brief  C library functions which are missing on some hosts
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * The C library of the target provides functions, which are standardized by
 * POSIX, but not by the C standard itself. Windows doesn't provide them,
 * therefore they are implemented here. Linux and the target have them already.
 *
 * This header is force included in the native environment, see the -include
 * build flag in config/mcu.ini. That way the application and the libraries
 * don't need a host specific include.
 *
 * @addtogroup TEST
 *
 * @{
 */

#ifndef HOSTCOMPAT_H
#define HOSTCOMPAT_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <time.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

#ifdef _WIN32

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Convert a date/time string to a broken down time, according to the given
 * format.
 *
 * Counterpart of the POSIX strptime(). Only the conversion specifiers are
 * supported, which the application uses. A unsupported one stops the
 * conversion, like a mismatch does.
 *
 * @param[in]  str      Date/time string, which to convert.
 * @param[in]  format   Format of the date/time string.
 * @param[out] timeInfo Broken down time, only the converted parts are written.
 *
 * @return Pointer to the first character after the converted part or nullptr
 *         in case the string doesn't match the format.
 */
char* strptime(const char* str, const char* format, struct tm* timeInfo);

#ifdef __cplusplus
}
#endif

#endif /* _WIN32 */

#endif /* HOSTCOMPAT_H */

/** @} */
