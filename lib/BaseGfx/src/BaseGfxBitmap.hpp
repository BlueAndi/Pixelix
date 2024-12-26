/* MIT License
 *
 * Copyright (c) 2019 - 2024 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Basic graphics bitmap
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup gfx
 *
 * @{
 */

#ifndef BASE_GFX_BITMAP_HPP
#define BASE_GFX_BITMAP_HPP

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <BaseGfx.hpp>
#include <new>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * This interface provides the base bitmap functions, which are
 * color format agnostic. This way it can be used for different
 * kind of color formats, e.g. RGB565 or RGB888 format.
 * 
 * Inside a base bitmap it can be drawn with the standard base
 * graphic functionality.
 *
 * @tparam TColor The color representation.
 */
template < typename TColor >
class BaseGfxBitmap : public BaseGfx<TColor>
{
public:

    /**
     * Destroys the bitmap.
     */
    virtual ~BaseGfxBitmap()
    {
    }

protected:

    /**
     * Constructs a bitmap.
     */
    BaseGfxBitmap()
    {
    }

private:

};

/**
 * This class provides a static allocated bitmap.
 * 
 * @tparam TColor   The color representation.
 * @tparam width    Pixel bitmap width in pixels
 * @tparam height   Pixel bitmap height in pixels
 */
template < typename TColor, uint16_t width, uint16_t height >
class BaseGfxStaticBitmap : public BaseGfxBitmap<TColor>
{
public:

    /**
     * Constructs the bitmap.
     */
    BaseGfxStaticBitmap() :
        BaseGfxBitmap<TColor>()
    {
    }

    /**
     * Constructs the bitmap by copy.
     * 
     * @param[in] bitmap    Source bitmap, which to copy.
     */
    BaseGfxStaticBitmap(const BaseGfxStaticBitmap& bitmap) :
        BaseGfxBitmap<TColor>(bitmap)
    {
        *this = bitmap;
    }

    /**
     * Destroys the bitmap.
     */
    virtual ~BaseGfxStaticBitmap()
    {
    }

    /**
     * Assigns a bitmap.
     * 
     * @param[in] bitmap    Source bitmap
     * 
     * @return Bitmap
     */
    BaseGfxStaticBitmap& operator=(const BaseGfxStaticBitmap& bitmap)
    {
        if (&bitmap != this)
        {
            const size_t PIXEL_BUFFER_SIZE = width * height;

            BaseGfxBitmap<TColor>::operator=(bitmap);

            if (0U < PIXEL_BUFFER_SIZE)
            {
                size_t idx = 0U;

                while(PIXEL_BUFFER_SIZE > idx)
                {
                    m_pixels[idx] = bitmap.m_pixels[idx];

                    ++idx;
                }
            }
        }

        return *this;
    }

    /**
     * Get the width of the bitmap in pixels.
     * 
     * @return Width in pixels
     */
    uint16_t getWidth() const override
    {
        return width;
    }

    /**
     * Get the height of the bitmap in pixels.
     * 
     * @return Height in pixels
     */
    uint16_t getHeight() const override
    {
        return height;
    }

    /**
     * Get pixel color at given position.
     * This is used for color manipulation in higher layers.
     *
     * @param[in] x x-coordinate
     * @param[in] y y-coordinate
     *
     * @return Color
     */
    TColor& getColor(int16_t x, int16_t y) override
    {
        static TColor   trash;
        TColor*         pixel   = &trash;

        if ((0 <= x) &&
            (0 <= y) &&
            (width > x) &&
            (height > y))
        {
            pixel = &m_pixels[pixelMap(x, y)];
        }

        return *pixel;
    }

    /**
     * Get pixel color at given position.
     *
     * @param[in] x x-coordinate
     * @param[in] y y-coordinate
     *
     * @return Color
     */
    const TColor& getColor(int16_t x, int16_t y) const override
    {
        static TColor   trash;
        const TColor*   pixel   = &trash;

        if ((0 <= x) &&
            (0 <= y) &&
            (width > x) &&
            (height > y))
        {
            pixel = &m_pixels[pixelMap(x, y)];
        }

        return *pixel;
    }

    /**
     * Draw a single pixel at given position.
     *
     * @param[in] x     x-coordinate
     * @param[in] y     y-coordinate
     * @param[in] color Color
     */
    void drawPixel(int16_t x, int16_t y, const TColor& color) override
    {
        if ((0 <= x) &&
            (0 <= y) &&
            (width > x) &&
            (height > y))
        {
            m_pixels[pixelMap(x, y)] = color;
        }
    }

    /**
     * Get the address inside the framebuffer at certain coordinates.
     * If the requested length is not available, it will return nullptr.
     * 
     * To address pixel by pixel on the x-axis, the returned offset shall be considered.
     * Otherwise its not guaranteed to address out of bounds!
     * 
     * @param[in] x         x-coordinate
     * @param[in] y         y-coordinate
     * @param[in] length    Requested number of colors on x-axis.
     * @param[out] offset   Address offset in pixel which to use to calculate address of next pixel.
     * 
     * @return Address in the framebuffer or nullptr.
     */
    TColor* getFrameBufferXAddr(int16_t x, int16_t y, uint16_t length, uint16_t& offset) override
    {
        TColor* addr = nullptr;

        if ((0 <= x) &&
            (0 <= y) &&
            (width >= (x + length)) &&
            (height > y))
        {
            addr    = &m_pixels[pixelMap(x, y)];
            offset  = 1U;
        }

        return addr;
    }

    /**
     * Get the address inside the framebuffer at certain coordinates.
     * If the requested length is not available, it will return nullptr.
     * 
     * To address pixel by pixel on the x-axis, the returned offset shall be considered.
     * Otherwise its not guaranteed to address out of bounds!
     * 
     * @param[in] x         x-coordinate
     * @param[in] y         y-coordinate
     * @param[in] length    Requested number of colors on x-axis.
     * @param[out] offset   Address offset in pixel which to use to calculate address of next pixel.
     * 
     * @return Address in the framebuffer or nullptr.
     */
    const TColor* getFrameBufferXAddr(int16_t x, int16_t y, uint16_t length, uint16_t& offset) const override
    {
        const TColor* addr = nullptr;

        if ((0 <= x) &&
            (0 <= y) &&
            (width >= (x + length)) &&
            (height > y))
        {
            addr    = &m_pixels[pixelMap(x, y)];
            offset  = 1U;
        }

        return addr;
    }

    /**
     * Get the address inside the framebuffer at certain coordinates.
     * If the requested length is not available, it will return nullptr.
     * 
     * To address pixel by pixel on the y-axis, the returned offset shall be considered.
     * Otherwise its not guaranteed to address out of bounds!
     * 
     * @param[in] x         x-coordinate
     * @param[in] y         y-coordinate
     * @param[in] length    Requested number of colors on y-axis.
     * @param[out] offset   Address offset in pixel which to use to calculate address of next pixel.
     * 
     * @return Address in the framebuffer or nullptr.
     */
    TColor* getFrameBufferYAddr(int16_t x, int16_t y, uint16_t length, uint16_t& offset) override
    {
        TColor* addr = nullptr;

        if ((0 <= x) &&
            (0 <= y) &&
            (width > x) &&
            (height >= (y + length)))
        {
            addr    = &m_pixels[pixelMap(x, y)];
            offset  = width;
        }

        return addr;
    }

    /**
     * Get the address inside the framebuffer at certain coordinates.
     * If the requested length is not available, it will return nullptr.
     * 
     * To address pixel by pixel on the y-axis, the returned offset shall be considered.
     * Otherwise its not guaranteed to address out of bounds!
     * 
     * @param[in] x         x-coordinate
     * @param[in] y         y-coordinate
     * @param[in] length    Requested number of colors on y-axis.
     * @param[out] offset   Address offset in pixel which to use to calculate address of next pixel.
     * 
     * @return Address in the framebuffer or nullptr.
     */
    const TColor* getFrameBufferYAddr(int16_t x, int16_t y, uint16_t length, uint16_t& offset) const override
    {
        const TColor* addr = nullptr;

        if ((0 <= x) &&
            (0 <= y) &&
            (width > x) &&
            (height >= (y + length)))
        {
            addr    = &m_pixels[pixelMap(x, y)];
            offset  = width;
        }

        return addr;
    }

private:

    /** Number of pixels in the pixel buffer. */
    static const uint16_t   PIXEL_BUFFER_SIZE   = width * height;

    TColor  m_pixels[PIXEL_BUFFER_SIZE];    /**< Pixel buffer */

    /**
     * Map the x- and y-coordinates to the pixel buffer index.
     * No out of bounds check!
     * 
     * @param[in] x x-coordinate
     * @param[in] y y-coordinate
     * 
     * @return Pixel buffer position
     */
    size_t pixelMap(uint16_t x, uint16_t y) const
    {
        return x + y * width;
    }
};

/**
 * This class provides a dynamic allocated bitmap.
 * 
 * @tparam TColor   The color representation.
 */
template < typename TColor >
class BaseGfxDynamicBitmap : public BaseGfxBitmap<TColor>
{
public:

    /**
     * Constructs the bitmap, but without internal buffer.
     */
    BaseGfxDynamicBitmap() :
        BaseGfxBitmap<TColor>(),
        m_pixels(nullptr),
        m_width(0U),
        m_height(0U)
    {
    }

    /**
     * Constructs the bitmap.
     * 
     * @param[in] width     Pixel bitmap width in pixels
     * @param[in] height    Pixel bitmap height in pixels
     */
    BaseGfxDynamicBitmap(uint16_t width, uint16_t height) :
        BaseGfxBitmap<TColor>(),
        m_pixels(allocatePixels(width, height)),
        m_width(width),
        m_height(height)
    {
        if (nullptr == m_pixels)
        {
            m_width     = 0U;
            m_height    = 0U;
        }
    }

    /**
     * Constructs the bitmap by copy.
     * 
     * @param[in] bitmap    Source bitmap
     */
    BaseGfxDynamicBitmap(const BaseGfxDynamicBitmap& bitmap) :
        BaseGfxBitmap<TColor>(bitmap),
        m_pixels(allocatePixels(bitmap.m_width, bitmap.m_height)),
        m_width(bitmap.m_width),
        m_height(bitmap.m_height)
    {
        if (nullptr == m_pixels)
        {
            m_width     = 0U;
            m_height    = 0U;
        }
    }

    /**
     * Destroys the bitmap.
     */
    virtual ~BaseGfxDynamicBitmap()
    {
        releasePixels(m_pixels);
    }

    /**
     * Assigns a bitmap.
     * 
     * @param[in] bitmap    Source bitmap
     * 
     * @return Bitmap
     */
    BaseGfxDynamicBitmap& operator=(const BaseGfxDynamicBitmap& bitmap)
    {
        if (&bitmap != this)
        {
            BaseGfxBitmap<TColor>::operator=(bitmap);

            if ((nullptr != bitmap.m_pixels) &&
                (0U < bitmap.m_width) &&
                (0U < bitmap.m_height))
            {
                /* If image sizes are different, release the pixel buffer. */
                if ((m_width != bitmap.m_width) ||
                    (m_height != bitmap.m_height))
                {
                    releasePixels(m_pixels);
                    m_width     = 0U;
                    m_height    = 0U;
                }

                /* If pixel buffer is released, allocate with the appropriate size. */
                if (nullptr == m_pixels)
                {
                    m_pixels = allocatePixels(bitmap.m_width, bitmap.m_height);

                    if (nullptr != m_pixels)
                    {
                        m_width     = bitmap.m_width;
                        m_height    = bitmap.m_height;
                    }
                }

                /* If pixel buffer is available, copy the values from the source bitmap. */
                if (nullptr != m_pixels)
                {
                    const size_t    PIXEL_BUFFER_SIZE   = m_width * m_height;
                    size_t          idx                 = 0U;

                    while(PIXEL_BUFFER_SIZE > idx)
                    {
                        m_pixels[idx] = bitmap.m_pixels[idx];

                        ++idx;
                    }
                }
            }
        }

        return *this;
    }

    /**
     * Create internal pixel buffer.
     * If a pixel buffer already exists, it will fail.
     * 
     * @param[in] width     Pixel bitmap width in pixels
     * @param[in] height    Pixel bitmap height in pixels
     *
     * @return If successful, it will return true otherwise false.
     */
    bool create(uint16_t width, uint16_t height)
    {
        bool isSuccessful = false;

        if (nullptr == m_pixels)
        {
            m_pixels = allocatePixels(width, height);

            if (nullptr != m_pixels)
            {
                m_width     = width;
                m_height    = height;

                isSuccessful = true;
            }
        }

        return isSuccessful;
    }

    /**
     * Release the internal pixel buffer.
     */
    void release()
    {
        releasePixels(m_pixels);
        m_width     = 0U;
        m_height    = 0U;
    }

    /**
     * Get the width of the bitmap in pixels.
     * 
     * @return Width in pixels
     */
    uint16_t getWidth() const override
    {
        return m_width;
    }

    /**
     * Get the height of the bitmap in pixels.
     * 
     * @return Height in pixels
     */
    uint16_t getHeight() const override
    {
        return m_height;
    }

    /**
     * Get pixel color at given position.
     * This is used for color manipulation in higher layers.
     *
     * @param[in] x x-coordinate
     * @param[in] y y-coordinate
     *
     * @return Color
     */
    TColor& getColor(int16_t x, int16_t y) override
    {
        static TColor   trash;
        TColor*         pixel   = &trash;

        if ((nullptr != m_pixels) &&
            (0 <= x) &&
            (0 <= y) &&
            (m_width > x) &&
            (m_height > y))
        {
            pixel = &m_pixels[pixelMap(x, y)];
        }

        return *pixel;
    }

    /**
     * Get pixel color at given position.
     *
     * @param[in] x x-coordinate
     * @param[in] y y-coordinate
     *
     * @return Color
     */
    const TColor& getColor(int16_t x, int16_t y) const override
    {
        static TColor   trash;
        const TColor*   pixel   = &trash;

        if ((nullptr != m_pixels) &&
            (0 <= x) &&
            (0 <= y) &&
            (m_width > x) &&
            (m_height > y))
        {
            pixel = &m_pixels[pixelMap(x, y)];
        }

        return *pixel;
    }

    /**
     * Draw a single pixel at given position.
     *
     * @param[in] x     x-coordinate
     * @param[in] y     y-coordinate
     * @param[in] color Color
     */
    void drawPixel(int16_t x, int16_t y, const TColor& color) override
    {
        if ((nullptr != m_pixels) &&
            (0 <= x) &&
            (0 <= y) &&
            (m_width > x) &&
            (m_height > y))
        {
            m_pixels[pixelMap(x, y)] = color;
        }
    }

    /**
     * Get the address inside the framebuffer at certain coordinates.
     * If the requested length is not available, it will return nullptr.
     * 
     * To address pixel by pixel on the x-axis, the returned offset shall be considered.
     * Otherwise its not guaranteed to address out of bounds!
     * 
     * @param[in] x         x-coordinate
     * @param[in] y         y-coordinate
     * @param[in] length    Requested number of colors on x-axis.
     * @param[out] offset   Address offset in pixel which to use to calculate address of next pixel.
     * 
     * @return Address in the framebuffer or nullptr.
     */
    TColor* getFrameBufferXAddr(int16_t x, int16_t y, uint16_t length, uint16_t& offset) override
    {
        TColor* addr = nullptr;

        if ((nullptr != m_pixels) &&
            (0 <= x) &&
            (0 <= y) &&
            (m_width >= (x + length)) &&
            (m_height > y))
        {
            addr    = &m_pixels[pixelMap(x, y)];
            offset  = 1U;
        }

        return addr;
    }

    /**
     * Get the address inside the framebuffer at certain coordinates.
     * If the requested length is not available, it will return nullptr.
     * 
     * To address pixel by pixel on the x-axis, the returned offset shall be considered.
     * Otherwise its not guaranteed to address out of bounds!
     * 
     * @param[in] x         x-coordinate
     * @param[in] y         y-coordinate
     * @param[in] length    Requested number of colors on x-axis.
     * @param[out] offset   Address offset in pixel which to use to calculate address of next pixel.
     * 
     * @return Address in the framebuffer or nullptr.
     */
    const TColor* getFrameBufferXAddr(int16_t x, int16_t y, uint16_t length, uint16_t& offset) const override
    {
        const TColor* addr = nullptr;

        if ((nullptr != m_pixels) &&
            (0 <= x) &&
            (0 <= y) &&
            (m_width >= (x + length)) &&
            (m_height > y))
        {
            addr    = &m_pixels[pixelMap(x, y)];
            offset  = 1U;
        }

        return addr;
    }

    /**
     * Get the address inside the framebuffer at certain coordinates.
     * If the requested length is not available, it will return nullptr.
     * 
     * To address pixel by pixel on the y-axis, the returned offset shall be considered.
     * Otherwise its not guaranteed to address out of bounds!
     * 
     * @param[in] x         x-coordinate
     * @param[in] y         y-coordinate
     * @param[in] length    Requested number of colors on y-axis.
     * @param[out] offset   Address offset in pixel which to use to calculate address of next pixel.
     * 
     * @return Address in the framebuffer or nullptr.
     */
    TColor* getFrameBufferYAddr(int16_t x, int16_t y, uint16_t length, uint16_t& offset) override
    {
        TColor* addr = nullptr;

        if ((nullptr != m_pixels) &&
            (0 <= x) &&
            (0 <= y) &&
            (m_width > x) &&
            (m_height >= (y + length)))
        {
            addr    = &m_pixels[pixelMap(x, y)];
            offset  = m_width;
        }

        return addr;
    }

    /**
     * Get the address inside the framebuffer at certain coordinates.
     * If the requested length is not available, it will return nullptr.
     * 
     * To address pixel by pixel on the y-axis, the returned offset shall be considered.
     * Otherwise its not guaranteed to address out of bounds!
     * 
     * @param[in] x         x-coordinate
     * @param[in] y         y-coordinate
     * @param[in] length    Requested number of colors on y-axis.
     * @param[out] offset   Address offset in pixel which to use to calculate address of next pixel.
     * 
     * @return Address in the framebuffer or nullptr.
     */
    const TColor* getFrameBufferYAddr(int16_t x, int16_t y, uint16_t length, uint16_t& offset) const override
    {
        const TColor* addr = nullptr;

        if ((nullptr != m_pixels) &&
            (0 <= x) &&
            (0 <= y) &&
            (m_width > x) &&
            (m_height >= (y + length)))
        {
            addr    = &m_pixels[pixelMap(x, y)];
            offset  = m_width;
        }

        return addr;
    }

    /**
     * Use this function to determine whether a internal bitmap buffer is allocated or not.
     * 
     * @return If no bitmap buffer is allocated, it will return false otherwise true.
     */
    bool isAllocated() const
    {
        return (nullptr != m_pixels);
    }

private:

    TColor*     m_pixels;   /**< Pixel buffer */
    uint16_t    m_width;    /**< Bitmap width in pixels */
    uint16_t    m_height;   /**< Bitmap height in pixels */

    /**
     * Map the x- and y-coordinates to the pixel buffer index.
     * No out of bounds check!
     * 
     * @param[in] x x-coordinate
     * @param[in] y y-coordinate
     * 
     * @return Pixel buffer position
     */
    size_t pixelMap(uint16_t x, uint16_t y) const
    {
        return x + y * m_width;
    }

    /**
     * Release pixel buffer if allocated.
     * 
     * @param[inout] pixels     Pixel buffer which to release.
     */
    void releasePixels(TColor*& pixels)
    {
        if (nullptr != pixels)
        {
            delete[] pixels;
            pixels = nullptr;
        }
    }

    /**
     * Allocate pixel buffer.
     * 
     * @param[in] width     Pixel bitmap with in pixels
     * @param[in] height    Pixel bitmap height in pixels
     * 
     * @return If successful, it will return the pixel buffer otherwise nullptr.
     */
    TColor* allocatePixels(uint16_t width, uint16_t height)
    {
        TColor* buffer = nullptr;

        if ((0U < width) &&
            (0U < height))
        {
            buffer = new(std::nothrow) TColor[width * height];
        }

        return buffer;
    }
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* BASE_GFX_BITMAP_HPP */

/** @} */