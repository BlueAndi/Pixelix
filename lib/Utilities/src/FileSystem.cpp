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
 * @file   FileSystem.cpp
 * @brief  Defines the filesystem.
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "FileSystem.h"

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

#if CONFIG_FILESYSTEM_TYPE != FILESYSTEM_NATIVE

static bool mountFilesystem();

#endif /* CONFIG_FILESYSTEM_TYPE != FILESYSTEM_NATIVE */

/******************************************************************************
 * Local Variables
 *****************************************************************************/

#if CONFIG_FILESYSTEM_TYPE != FILESYSTEM_NATIVE

/**
 * The max. number of open files in the filesystem.
 */
static const uint8_t FILESYSTEM_MAX_OPEN_FILES = 10U;

#endif /* CONFIG_FILESYSTEM_TYPE != FILESYSTEM_NATIVE */

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

bool FileSystem::init()
{
    bool isSuccessful = false;

#if CONFIG_FILESYSTEM_TYPE == FILESYSTEM_NATIVE
    isSuccessful = true;
#else
    isSuccessful = mountFilesystem();

#if CONFIG_FILESYSTEM_PSRAMFS_ENABLE == 1

    if (true == isSuccessful)
    {
        /* Use half of the available PSRAM for the filesystem. */
        // const size_t PSRAMFS_SIZE = ESP.getFreePsram() / 2U;
        const size_t PSRAMFS_SIZE = 4194304U;

        if (false == PSRamFS.setPartitionSize(PSRAMFS_SIZE))
        {
            isSuccessful = false;
        }
        else if (false == PSRamFS.begin())
        {
            isSuccessful = false;
        }
        else
        {
            /* Successful mounted PSRAM filesystem. */
            ;
        }
    }

#endif /* CONFIG_FILESYSTEM_PSRAMFS_ENABLE == 1 */

#endif /* CONFIG_FILESYSTEM_TYPE == FILESYSTEM_NATIVE */

    return isSuccessful;
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/

#if CONFIG_FILESYSTEM_TYPE != FILESYSTEM_NATIVE

/**
 * Mount the filesystem.
 *
 * @return If successful mounted, it will return true otherwise false.
 */
static bool mountFilesystem()
{
    bool        isSuccessful                = false;
    bool        formatOnFail                = false;
    const char* BASE_PATH                   = "/littlefs";
    const char* PARTITION_LABEL_DEFAULT     = "spiffs"; /* Default for most of the partitions, defined by Platformio. */
    const char* PARTITION_LABEL_ALTERNATIVE = "ffat";   /* Sometimes its different, than the default in Platformio. */

    /* Mount filesytem with default partition label. If it fails, use alternative. */
    if (false == FILESYSTEM.begin(formatOnFail, BASE_PATH, FILESYSTEM_MAX_OPEN_FILES, PARTITION_LABEL_DEFAULT))
    {
        /* Try to mount with alternative partition label. */
        if (true == FILESYSTEM.begin(formatOnFail, BASE_PATH, FILESYSTEM_MAX_OPEN_FILES, PARTITION_LABEL_ALTERNATIVE))
        {
            /* Successful mounted with alternative partition label. */
            isSuccessful = true;
        }
    }
    /* Successful mounted with default partition label. */
    else
    {
        isSuccessful = true;
    }

    return isSuccessful;
}

#endif /* CONFIG_FILESYSTEM_TYPE != FILESYSTEM_NATIVE */
