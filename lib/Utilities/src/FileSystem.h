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
 * @file   FileSystem.h
 * @brief  Defines the filesystem.
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup UTILITIES
 *
 * @{
 */

#ifndef FILESYSTEM_H
#define FILESYSTEM_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/**
 * The filesystem Native FS.
 */
#define FILESYSTEM_NATIVE (0)

/**
 * The filesytem SPIFFS.
 */
#define FILESYSTEM_SPIFFS (1)

/**
 * The filesystem LittleFS.
 */
#define FILESYSTEM_LITTLEFS (2)

#ifndef CONFIG_FILESYSTEM_TYPE
/** Select the filesystem type here. */
#define CONFIG_FILESYSTEM_TYPE FILESYSTEM_NATIVE
#endif /* CONFIG_FILESYSTEM_TYPE */

#if CONFIG_FILESYSTEM_TYPE == FILESYSTEM_NATIVE

/** The used filesystem type. */
#define FILESYSTEM NativeFS

/** The filename of the filesystem image. */
#define FILESYSTEM_FILENAME "fs.bin"

#endif /* CONFIG_FILESYSTEM_TYPE == FILESYSTEM_NATIVE */

#if CONFIG_FILESYSTEM_TYPE == FILESYSTEM_LITTLEFS

/** The used filesystem type. */
#define FILESYSTEM LittleFS

/** The filename of the filesystem image. */
#define FILESYSTEM_FILENAME "littlefs.bin"

#endif /* CONFIG_FILESYSTEM_TYPE == FILESYSTEM_LITTLEFS */

#if CONFIG_FILESYSTEM_TYPE == FILESYSTEM_SPIFFS

/** The used filesystem type. */
#define FILESYSTEM SPIFFS

/** The filename of the filesystem image. */
#define FILESYSTEM_FILENAME "spiffs.bin"

#endif /* CONFIG_FILESYSTEM_TYPE == FILESYSTEM_SPIFFS */

/******************************************************************************
 * Includes
 *****************************************************************************/
#if CONFIG_FILESYSTEM_TYPE == FILESYSTEM_NATIVE
#include <FS.h>
#endif /* CONFIG_FILESYSTEM_TYPE == FILESYSTEM_NATIVE */

#if CONFIG_FILESYSTEM_TYPE == FILESYSTEM_LITTLEFS
#include <LittleFS.h>
#endif /* CONFIG_FILESYSTEM_TYPE == FILESYSTEM_LITTLEFS */

#if CONFIG_FILESYSTEM_TYPE == FILESYSTEM_SPIFFS
#include <SPIFFS.h>
#endif /* CONFIG_FILESYSTEM_TYPE == FILESYSTEM_SPIFFS */

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* FILESYSTEM_H */

/** @} */