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
 * @file   BmpImgLoader.h
 * @brief  Bitmap image loader
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup GFX
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

/**
 * To store general information about the bitmap image file.
 * Not needed after the file is loaded in memory.
 */
typedef struct _BmpFileHeader BmpFileHeader;

/**
 * Device independent header (DIB): The bitmap v5 header (size: 124 bytes).
 */
typedef struct _BmpV5Header BmpV5Header;

/**
 * Bitmap image loader, which supports images that have
 * - 1/4/8/24/32 bit per pixel
 * - No compression (or bitfields for 32-bit)
 * - Palette colors (for 1/4/8 bpp)
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
        RET_OK = 0,                  /**< Successful */
        RET_FILE_NOT_FOUND,          /**< File not found. */
        RET_FILE_FORMAT_INVALID,     /**< Invalid file format. */
        RET_FILE_FORMAT_UNSUPPORTED, /**< File format is not supported. */
        RET_IMG_TOO_BIG              /**< Image size is too big. */
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

    /**
     * Load color palette from file system.
     *
     * @param[in] fd            File descriptor
     * @param[in] numColors     Number of colors in palette
     * @param[out] palette      Palette array (must be pre-allocated)
     *
     * @return If successful, it will return true otherwise false.
     */
    bool loadPalette(File& fd, uint32_t numColors, Color* palette);

    /**
     * Load pixel data from file system to bitmap buffer.
     *
     * @param[in] fd            File descriptor
     * @param[in] bmpFileHeader Bitmap file header
     * @param[in] dibHeader     DIB header
     * @param[out] bitmap       Bitmap buffer
     *
     * @return If successful, it will return RET_OK. See Ret type for more informations.
     */
    Ret loadPixelData(File& fd, const BmpFileHeader& bmpFileHeader, const BmpV5Header& dibHeader, YAGfxDynamicBitmap& bitmap);

    /**
     * Read a pixel color from a palettized image.
     *
     * @param[in] fd            File descriptor
     * @param[in] offset        Offset to pixel data in file
     * @param[in] pos           Position within pixel data
     * @param[in] bpp           Bits per pixel
     * @param[in] x             X coordinate of pixel (for bit shift calculation)
     * @param[in] palette       Color palette
     * @param[in] numColors     Number of colors in palette
     * @param[out] color        Resulting color
     *
     * @return If successful, it will return true otherwise false.
     */
    bool readPalettizedPixel(File& fd, uint32_t offset, uint32_t pos, uint16_t bpp, int16_t x, const Color* palette, uint32_t numColors, Color& color);

    /**
     * Read a pixel color from a direct RGB(A) image.
     *
     * @param[in] fd            File descriptor
     * @param[in] offset        Offset to pixel data in file
     * @param[in] pos           Position within pixel data
     * @param[in] bpp           Bits per pixel
     * @param[out] color        Resulting color
     *
     * @return If successful, it will return true otherwise false.
     */
    bool readDirectPixel(File& fd, uint32_t offset, uint32_t pos, uint16_t bpp, Color& color);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* BMP_IMG_LOADER_H */

/** @} */