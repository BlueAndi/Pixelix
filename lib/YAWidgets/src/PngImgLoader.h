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

/* The PNG support is only available if the build switch is enabled. Note, an
 * undefined switch evaluates to 0 and therefore disables everything below,
 * including the PNGdec include.
 */
#if (0 != CONFIG_IMAGE_FORMAT_PNG_ENABLE)

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <stddef.h>
#include <FS.h>
#include <YAGfxBitmap.h>
#include <TypedAllocator.hpp>
#include <PsAllocator.hpp>
#include <PNGdec.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * PNG image loader, which supports the PNG pixel types truecolor, indexed and
 * grayscale, with and without alpha channel.
 *
 * Uses the PNGdec library for decoding. The file is read into memory and
 * decoded line by line straight into the destination bitmap, therefore no
 * intermediate image buffer is required.
 *
 * Note: The LED matrix has no alpha channel. Every pixel is therefore
 *       composited against a black background.
 *
 * Note: Bit depths of 1, 2, 4 and 8 bit per sample are decoded. 16 bit per
 *       sample and interlaced images are not supported by PNGdec and are
 *       reported as RET_FILE_FORMAT_UNSUPPORTED.
 *
 * Note: Animated PNG (APNG) is not supported. A PNG is always a single still
 *       image.
 */
class PngImgLoader
{
public:

    /**
     * Construct a new PNG image loader object.
     */
    PngImgLoader()
    {
    }

    /**
     * Destroy the PNG image loader object.
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
        RET_IMG_TOO_BIG,             /**< Image is too big: its size can not be represented in the bitmap dimensions, its line exceeds the decoder line buffer, or its bitmap can not be allocated. */
        RET_OUT_OF_MEMORY            /**< Out of memory for an allocation which is not driven by the image size, e.g. the file buffer or the decoder object. */
    };

    /**
     * Load a PNG image (.png) from the given filesystem into the bitmap.
     *
     * If it fails, the bitmap is released and therefore left unallocated.
     *
     * It blocks until the image is completely decoded.
     *
     * Note: This method is reentrant. It keeps no state at all, neither in
     *       the loader instance nor in a file scope variable, and it installs
     *       no file callbacks. Two tasks may therefore decode two images at
     *       the same time, even through one and the same loader object,
     *       without serializing.
     *
     * Note: The complete file is held in memory while it is decoded, on top of
     *       the decoder object of around 40 kByte and the destination bitmap.
     *       The file buffer and the decoder object are taken from PSRAM where
     *       there is PSRAM, and released again before load() returns.
     *
     * @param[in] fs        Filesystem to use.
     * @param[in] fileName  Name of the PNG file.
     * @param[out] bitmap   Bitmap buffer, which will contain the image.
     *
     * @return If successful, it will return RET_OK. See Ret type for more informations.
     */
    Ret load(FS& fs, const String& fileName, YAGfxDynamicBitmap& bitmap);

private:

    /**
     * File data allocator type definition.
     */
    typedef TypedAllocator<uint8_t, PsAllocator> DataAllocator;

    /**
     * Read the complete file into memory.
     *
     * On success the buffer is valid and has to be released by the caller with
     * DataAllocator::deallocateArray(), otherwise no buffer is allocated at
     * all.
     *
     * @param[in] fs        Filesystem to use.
     * @param[in] fileName  Name of the PNG file.
     * @param[out] data     Buffer with the file content.
     * @param[out] size     Size of the file content in bytes.
     *
     * @return If successful, it will return RET_OK. See Ret type for more informations.
     */
    Ret readFileIntoMemory(FS& fs, const String& fileName, uint8_t*& data, size_t& size);

    /**
     * Open, verify and decode the PNG image with the given decoder.
     *
     * The decoder is always closed again, no matter whether the image could be
     * decoded or not.
     *
     * @param[in] png       PNG decoder to use.
     * @param[in] data      Buffer with the file content. Must not be nullptr.
     * @param[in] size      Size of the file content in bytes.
     * @param[in] fileName  Name of the PNG file, used for logging only.
     * @param[out] bitmap   Bitmap buffer, which will contain the image.
     *
     * @return If successful, it will return RET_OK. See Ret type for more informations.
     */
    Ret decodeImage(PNG& png, uint8_t* data, size_t size, const String& fileName, YAGfxDynamicBitmap& bitmap);

    /**
     * Draw one decoded line of the PNG image into the bitmap.
     *
     * @param[out] bitmap   Destination bitmap.
     * @param[in] draw      PNG draw information of the decoded line.
     */
    static void drawLine(YAGfxDynamicBitmap& bitmap, const PNGDRAW& draw);

    /**
     * Draw one decoded truecolor line, with or without alpha channel.
     *
     * @param[out] bitmap   Destination bitmap.
     * @param[in] draw      PNG draw information of the decoded line.
     */
    static void drawTruecolorLine(YAGfxDynamicBitmap& bitmap, const PNGDRAW& draw);

    /**
     * Draw one decoded indexed line, by looking the colors up in the palette.
     *
     * @param[out] bitmap   Destination bitmap.
     * @param[in] draw      PNG draw information of the decoded line.
     */
    static void drawIndexedLine(YAGfxDynamicBitmap& bitmap, const PNGDRAW& draw);

    /**
     * Draw one decoded grayscale line, with or without alpha channel.
     *
     * @param[out] bitmap   Destination bitmap.
     * @param[in] draw      PNG draw information of the decoded line.
     */
    static void drawGrayscaleLine(YAGfxDynamicBitmap& bitmap, const PNGDRAW& draw);

    /**
     * PNG draw callback, called by the PNGdec library for every decoded line.
     *
     * The destination bitmap is handed over through the user pointer of the
     * draw information.
     *
     * @param[in] pDraw PNG draw information of the decoded line.
     *
     * @return 1 to continue decoding, 0 to stop.
     */
    static int drawCallback(PNGDRAW* pDraw);

    PngImgLoader(const PngImgLoader& loader);
    PngImgLoader& operator=(const PngImgLoader& loader);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* (0 != CONFIG_IMAGE_FORMAT_PNG_ENABLE) */

#endif /* PNG_IMG_LOADER_H */

/** @} */
