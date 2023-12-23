/* MIT License
 *
 * Copyright (c) 2019 - 2023 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Defines the filesystem.
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup common
 *
 * @{
 */

#ifndef FILESYSTEM_H
#define FILESYSTEM_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/**
 * Defines LittleFS as filesystem format (must be 1).
 */
#define FILESYSTEM_USE_LITTLEFS (1)

#if FILESYSTEM_USE_LITTLEFS
#define FILESYSTEM              LittleFS
#define FILESYSTEM_FILENAME     "littlefs.bin"
#endif  /* FILESYSTEM_USE_LITTLEFS */

/**
 * Defines SPIFFS as filesystem format (must be 1).
 */
#define FILESYSTEM_USE_SPIFFS   (0)

#if FILESYSTEM_USE_SPIFFS
#define FILESYSTEM              SPIFFS
#define FILESYSTEM_FILENAME     "spiffs.bin"
#endif  /* FILESYSTEM_USE_SPIFFS */

/******************************************************************************
 * Includes
 *****************************************************************************/
#if FILESYSTEM_USE_LITTLEFS
#include <LittleFS.h>
#endif  /* FILESYSTEM_USE_LITTLEFS */

#if FILESYSTEM_USE_SPIFFS
#include <SPIFFS.h>
#endif  /* FILESYSTEM_USE_SPIFFS */

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* FILESYSTEM_H */

/** @} */