/* MIT License
 *
 * Copyright (c) 2019 - 2021 Andreas Merkle Merkle <web@blue-andi.de>
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

#ifndef __BMP_IMG_H__
#define __BMP_IMG_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <Widget.hpp>
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
class BmpImg
{
public:

    /**
     * Construct a new bitmap image object.
     */
    BmpImg() :
        m_pixels(nullptr),
        m_width(0U),
        m_height(0U)
    {
    }

    /**
     * Construct a bitmap image by copy.
     * 
     * @param[in] img Bitmap image, which to copy.
     */
    BmpImg(const BmpImg& img) :
        m_pixels(nullptr),
        m_width(0U),
        m_height(0U)
    {
        copy(img.m_pixels, img.m_width, img.m_height);
    }

    /**
     * Destroy the bitmap image object.
     */
    ~BmpImg()
    {
        releasePixels();
    }

    /**
     * Assign bitmap image.
     * 
     * @param[in] img   Bitmap image, which to assign.
     */
    BmpImg& operator=(const BmpImg& img)
    {
        if (this != (&img))
        {
            copy(img.m_pixels, img.m_width, img.m_height);
        }

        return *this;
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
     * Load bitmap image from file system.
     * 
     * @param[in] fs        File system
     * @param[in] fileName  Name of the file
     * 
     * @return If successful, it will return RET_OK. See Ret type for more informations.
     */
    Ret load(FS& fs, const String& fileName);

    /**
     * Get image width in pixels.
     * 
     * @return Image width in pixels.
     */
    uint16_t getWidth() const
    {
        return m_width;
    }

    /**
     * Get image height in pixels.
     * 
     * @return Image height in pixels.
     */
    uint16_t getHeight() const
    {
        return m_height;
    }

    /**
     * Get single pixel at the given location.
     * If the location is invalid or no bitmap image is loaded,
     * it will return a "trash" pixel.
     * 
     * @param[in] x x-coordinate
     * @param[in] y y-coordinate
     * 
     * @return Pixel color at given location.
     */
    Color& get(uint16_t x, uint16_t y)
    {
        static Color    trash;
        Color*          pixel = &trash;

        if ((nullptr != m_pixels) &&
            (m_width > x) &&
            (m_height > y))
        {
            pixel = &m_pixels[x + m_width * y];
        }

        return *pixel;
    }

    /**
     * Get access to the internal pixel buffer.
     * 
     * @return Internal pixel buffer.
     */
    const Color* get() const
    {
        return m_pixels;
    }

    /**
     * Copy ext. bitmap buffer.
     * 
     * @param[in] buffer    Ext. bitmap buffer
     * @param[in] width     Bitmap width in pixels
     * @param[in] height    Bitmap height in pixels
     */
    void copy(const Color* buffer, const uint16_t& width, const uint16_t& height)
    {
        if ((nullptr != buffer) &&
            (0U < width) &&
            (0U < height))
        {
            if (true == allocatePixels(width, height))
            {
                uint16_t    x = 0U;
                uint16_t    y = 0U;

                while(m_height > y)
                {
                    x = 0U;
                    while(m_width > x)
                    {
                        uint32_t pos = x + y * m_width;

                        m_pixels[pos] = buffer[pos];

                        ++x;
                    }

                    ++y;
                }
            }
        }
    }

private:

    Color*      m_pixels;   /**< Pixel buffer */
    uint16_t    m_width;    /**< Image width in pixels. */
    uint16_t    m_height;   /**< Image height in pixels. */

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
     * Allocate pixel memory and set width and height correspondingly.
     * If memory already allocated, it will be released.
     * 
     * @param[in] width     Width in pixels
     * @param[in] height    Height in pixels
     * 
     * @return If successful, it will return true otherwise false.
     */
    bool allocatePixels(uint16_t width, uint16_t height);

    /**
     * Release pixel memory and reset width and height to 0.
     */
    void releasePixels();

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __BMP_IMG_H__ */

/** @} */