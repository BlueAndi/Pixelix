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
 * @file   IImageLoader.h
 * @brief  Interface of image loader
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup GFX
 *
 * @{
 */

#ifndef IIMAGELOADER_H
#define IIMAGELOADER_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <WString.h>
#include <FS.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Interface for image file loader.
 */
class IImageLoader
{
public:

    /**
     * Destroy the interface.
     */
    virtual ~IImageLoader()
    {
    }

    /**
     * Open an image file.
     *
     * @param[in] fs        Filesystem to use
     * @param[in] fileName  Name of the image file.
     *
     * @return If successful, it will return true otherwise false.
     */
    virtual bool open(FS& fs, const String& fileName) = 0;

    /**
     * Close the image file.
     */
    virtual void close()                              = 0;

    /**
     * Read data from image.
     *
     * @param[in] buffer    Buffer to fill.
     * @param[in] size      Buffer size in bytes.
     *
     * @return If successful read, it will return true otherwise false.
     */
    virtual bool read(void* buffer, size_t size)      = 0;

    /**
     * Get file position.
     *
     * @return File position
     */
    virtual size_t position()                         = 0;

    /**
     * Set file position.
     *
     * @param[in] position  File position to set
     * @param[in] mode      The seek mode.
     *
     * @return If successful, it will return true otherwise false.
     */
    virtual bool seek(size_t position, SeekMode mode) = 0;

    /**
     * If file is opened, it will return true otherwise false.
     *
     * @return File status
     */
    virtual operator bool() const                     = 0;

protected:

    /**
     * Construct the interface.
     */
    IImageLoader()
    {
    }
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* IIMAGELOADER_H */

/** @} */