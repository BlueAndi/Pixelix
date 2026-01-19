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
 * @file   PngImgLoader.h
 * @brief  PNG image loader
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup GFX
 *
 * @{
 */

#ifndef PNG_IMG_LOADER_H
#define PNG_IMG_LOADER_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <YAGfxBitmap.h>
#include <FS.h>
#include <PNGdec.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * PNG image loader, which supports various PNG formats (1-32 bpp).
 */
class PngImgLoader
{
public:

    /**
     * Construct a new PNG loader object.
     */
    PngImgLoader() :
        m_bitmap(nullptr)
    {
    }

    /**
     * Destroy the PNG loader object.
     */
    ~PngImgLoader()
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
     * Load PNG image (.png) from file system to bitmap buffer.
     *
     * @param[in] fs        File system
     * @param[in] fileName  Name of the file
     * @param[out] bitmap   Bitmap buffer
     *
     * @return If successful, it will return RET_OK. See Ret type for more informations.
     */
    Ret load(FS& fs, const String& fileName, YAGfxDynamicBitmap& bitmap);

private:

    YAGfxDynamicBitmap* m_bitmap; /**< Pointer to target bitmap */

    /**
     * PNG draw callback function.
     * This is called by the PNGdec library for each decoded line.
     *
     * @param[in] pDraw PNG draw structure containing decoded line data
     *
     * @return 1 to continue decoding, 0 to stop
     */
    static int drawCallback(PNGDRAW* pDraw);

    /**
     * PNG file open callback.
     *
     * @param[in] szFilename  Filename to open
     * @param[out] pFileSize  Pointer to receive file size
     *
     * @return File handle or nullptr on error
     */
    static void* openCallback(const char* szFilename, int32_t* pFileSize);

    /**
     * PNG file close callback.
     *
     * @param[in] pHandle File handle to close
     */
    static void closeCallback(void* pHandle);

    /**
     * PNG file read callback.
     *
     * @param[in] pFile  PNG file structure
     * @param[in] pBuf   Buffer to read into
     * @param[in] iLen   Number of bytes to read
     *
     * @return Number of bytes read
     */
    static int32_t readCallback(PNGFILE* pFile, uint8_t* pBuf, int32_t iLen);

    /**
     * PNG file seek callback.
     *
     * @param[in] pFile      PNG file structure
     * @param[in] iPosition  Position to seek to
     *
     * @return New position or -1 on error
     */
    static int32_t seekCallback(PNGFILE* pFile, int32_t iPosition);

    PngImgLoader(const PngImgLoader& loader);
    PngImgLoader& operator=(const PngImgLoader& loader);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* PNG_IMG_LOADER_H */

/** @} */
