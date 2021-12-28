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
     * @param img 
     */
    BmpImg(const BmpImg& img) :
        m_pixels(nullptr),
        m_width(img.m_width),
        m_height(img.m_height)
    {
        if (false == allocatePixels(img.m_width, img.m_height))
        {
            m_width     = 0U;
            m_height    = 0U;
        }
    }

    /**
     * Destroy the bitmap image object.
     */
    ~BmpImg()
    {
        /* Release memory if necessary. */
        if (nullptr != m_pixels)
        {
            delete[] m_pixels;
            m_pixels = nullptr;
        }
    }

    /**
     * Assign bitmap image.
     * 
     */
    BmpImg& operator=(const BmpImg& img)
    {
        if (this != (&img))
        {
            m_width     = img.m_width;
            m_height    = img.m_height;

            if (false == allocatePixels(img.m_width, img.m_height))
            {
                m_width     = 0U;
                m_height    = 0U;
            }
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
     * @param fs        File system
     * @param fileName  Name of the file
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
     * @param x x-coordinate
     * @param y y-coordinate
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
     */
    void copy(const Color* buffer, const uint16_t& width, const uint16_t& height)
    {
        if (nullptr != m_pixels)
        {
            delete[] m_pixels;
        }

        if (true == allocatePixels(width, height))
        {
            uint16_t    x = 0U;
            uint16_t    y = 0U;

            m_width     = width;
            m_height    = height;

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

private:

    Color*      m_pixels;   /**< Pixel buffer */
    uint16_t    m_width;    /**< Image width in pixels. */
    uint16_t    m_height;   /**< Image height in pixels. */

    bool loadBmpFileHeader(File& fd, BmpFileHeader& header);
    bool loadDibHeader(File& fd, BmpV5Header& header);
    bool allocatePixels(uint16_t width, uint16_t height);

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __BMP_IMG_H__ */

/** @} */