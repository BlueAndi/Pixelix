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
 * @brief  Bitmap image loader
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup gfx
 *
 * @{
 */

#ifndef BMP_IMG_LOADER_H
#define BMP_IMG_LOADER_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <YAGfxBitmap.h>
#include <FS.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/* Forward declarations */
typedef struct _BmpFileHeader BmpFileHeader;
typedef struct _BmpV5Header BmpV5Header;

/**
 * Bitmap image loader, which supports images that have
 * - 24/32 bit per pixel
 * - No compression
 * - No palette colors
 * - Resolution of max. 65535 x 65535 pixels
 */
class BmpImgLoader
{
public:

    /**
     * Construct a new bitmap loader object.
     */
    BmpImgLoader()
    {
    }

    /**
     * Destroy the bitmap loader object.
     */
    ~BmpImgLoader()
    {
    }

    /**
     * Possible return values with more information.
     */
    enum Ret
    {
        RET_OK = 0,                     /**< Successful */
        RET_FILE_NOT_FOUND,             /**< File not found. */
        RET_FILE_FORMAT_INVALID,        /**< Invalid file format. */
        RET_FILE_FORMAT_UNSUPPORTED,    /**< File format is not supported. */
        RET_IMG_TOO_BIG                 /**< Image size is too big. */
    };

    /**
     * Load bitmap image (.bmp) from file system to bitmap buffer.
     * 
     * @param[in] fs        File system
     * @param[in] fileName  Name of the file
     * @param[out] bitmap   Bitmap buffer
     * 
     * @return If successful, it will return RET_OK. See Ret type for more informations.
     */
    Ret load(FS& fs, const String& fileName, YAGfxDynamicBitmap& bitmap);

private:

    /**
     * Load bitmap file header from file system.
     * 
     * @param[in] fd        File descriptor
     * @param[in] header    Bitmap file header
     *
     * @return If successful, it will return true otherwise false.
     */
    bool loadBmpFileHeader(File& fd, BmpFileHeader& header);

    /**
     * Load device independent header (DIB header) from file system.
     * 
     * @param[in] fd        File descriptor
     * @param[in] header    DIB header
     *
     * @return If successful, it will return true otherwise false.
     */
    bool loadDibHeader(File& fd, BmpV5Header& header);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* BMP_IMG_LOADER_H */

/** @} */