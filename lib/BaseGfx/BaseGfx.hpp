/* MIT License
 *
 * Copyright (c) 2019 - 2022 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Basic graphics interface
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup gfx
 *
 * @{
 */

#ifndef __BASE_GFX_HPP__
#define __BASE_GFX_HPP__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * This class provides the base graphic functions, which are
 * color format agnostic. This way it can be used for different
 * kind of color formats, e.g. RGB565 or RGB888 format.
 *
 */
template < typename TColor >
class BaseGfx
{
public:

    /**
     * Destroys the base graphics functionality object.
     */
    virtual ~BaseGfx()
    {
    }

    /**
     * Assign properties of another base graphics functionality.
     *
     * @param[in] gfx   Base gfx which to copy
     *
     * @return Base graphics functionality
     */
    BaseGfx& operator=(const BaseGfx& gfx)
    {
        if (&gfx != this)
        {
            m_width     = gfx.m_width;
            m_height    = gfx.m_height;
        }

        return *this;
    }

    /**
     * Get width in pixel.
     *
     * @return Canvas width in pixel
     */
    uint16_t getWidth() const
    {
        return m_width;
    };

    /**
     * Get height in pixel.
     *
     * @return Canvas height in pixel
     */
    uint16_t getHeight() const
    {
        return m_height;
    }

    /**
     * Get pixel color at given position.
     *
     * @param[in] x     x-coordinate
     * @param[in] y     y-coordinate
     *
     * @return Color
     */
    virtual TColor getColor(int16_t x, int16_t y) const = 0;

    /**
     * Draw a single pixel at given position.
     *
     * @param[in] x     x-coordinate
     * @param[in] y     y-coordinate
     * @param[in] color Color
     */
    virtual void drawPixel(int16_t x, int16_t y, const TColor& color) = 0;

    /**
     * Dim color to black.
     * A dim ratio of 255 means no change.
     *
     * Note, the base colors may be destroyed, depends on the color type.
     *
     * @param[in] x     x-coordinate
     * @param[in] y     y-coordinate
     * @param[in] ratio Dim ratio [0; 255]
     */
    virtual void dimPixel(int16_t x, int16_t y, uint8_t ratio) = 0;

    /**
     * Copy framebuffer content.
     *
     * @param[in] gfx   Graphics interface of framebuffer source
     */
    void copy(const BaseGfx<TColor>& gfx)
    {
        int16_t x   = 0;
        int16_t y   = 0;

        for(y = 0; y < m_height; ++y)
        {
            for(x = 0; x < m_width; ++x)
            {
                drawPixel(x, y, gfx.getColor(x, y));
            }
        }
    }

    /**
     * Draw vertical line.
     * Note, this is faster than using drawLine().
     *
     * @param[in] x         x-coordinate of start point
     * @param[in] y         y-coordinate of start point
     * @param[in] height    Vertical line height in pixel
     * @param[in] color     Color
     */
    void drawVLine(int16_t x, int16_t y, uint16_t height, const TColor& color)
    {
        uint16_t idx = 0U;

        for(idx = 0U; idx < height; ++idx)
        {
            drawPixel(x, y + idx, color);
        }
    }

    /**
     * Draw horizontal line.
     * Note, this is faster than using drawLine().
     *
     * @param[in] x     x-coordinate of start point
     * @param[in] y     y-coordinate of start point
     * @param[in] width Horizontal line width in pixel
     * @param[in] color Color
     */
    void drawHLine(int16_t x, int16_t y, uint16_t width, const TColor& color)
    {
        uint16_t idx = 0U;

        for(idx = 0U; idx < width; ++idx)
        {
            drawPixel(x + idx, y, color);
        }
    }

    /**
     * Draw a line.
     *
     * @param[in] xs    x-coordinate of start point
     * @param[in] ys    y-coordinate of start point
     * @param[in] xe    x-coordinate of end point
     * @param[in] ye    y-coordinate of end point
     * @param[in] color Color
     */
    void drawLine(int16_t xs, int16_t ys, int16_t xe, int16_t ye, const TColor& color)
    {
        int16_t dX      = abs(xe - xs);
        int16_t stepX   = (xs < xe) ? 1 : -1;
        int16_t dY      = - abs(ye - ys);
        int16_t stepY   = (ys < ye) ? 1 : -1;
        int16_t err     = dX + dY;  /* err_xy */
        int16_t err2    = 0;

        /* https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm */

        while(1)
        {
            drawPixel(xs, ys, color);

            if ((xs == xe) && (ys == ye))
            {
                break;
            }

            err2 = 2 * err;

            if (err2 >= dY)
            {
                err += dY; /* err_xy + err_x > 0 */
                xs  += stepX;
            }

            if (err2 <= dX)
            {
                err += dX; /* err_xy + err_y < 0 */
                ys  += stepY;
            }
        }
    }

    /**
     * Draw a rectangle with a specific color.
     *
     * @param[in] x1        x-coordinate of upper left point
     * @param[in] y1        y-coordinate of upper left point
     * @param[in] width     Rectangle width in pixel
     * @param[in] height    Rectangle height in pixel
     * @param[in] color     Color
     */
    void drawRectangle(int16_t x1, int16_t y1, uint16_t width, uint16_t height, const TColor& color)
    {
        drawHLine(x1, y1, width, color);
        drawHLine(x1, y1 + height - 1, width, color);
        drawVLine(x1, y1 + 1, height - 2, color);
        drawVLine(x1 + width - 1, y1 + 1, height -2, color);
    }

    /**
     * Fill a rectangle with a specific color.
     *
     * @param[in] x         x-coordinate of upper left point
     * @param[in] y         y-coordinate of upper left point
     * @param[in] width     Rectangle width in pixel
     * @param[in] height    Rectangle height in pixel
     * @param[in] color     Color
     */
    void fillRect(int16_t x, int16_t y, uint16_t width, uint16_t height, const TColor& color)
    {
        int16_t xIndex = 0;
        int16_t yIndex = 0;

        for(yIndex = 0; yIndex < height; ++yIndex)
        {
            for(xIndex = 0; xIndex < width; ++xIndex)
            {
                drawPixel(x + xIndex, y + yIndex, color);
            }
        }
    }

    /**
     * Fill screen with a specific color.
     *
     * @param[in] color Color
     */
    void fillScreen(const TColor& color)
    {
        fillRect(0, 0, m_width, m_height, color);
    }

    /**
     * Dim a rectangle with a given ratio.
     * A ratio of 255 means no change.
     *
     * @param[in] x         x-coordinate of upper left point
     * @param[in] y         y-coordinate of upper left point
     * @param[in] width     Rectangle width in pixel
     * @param[in] height    Rectangle height in pixel
     * @param[in] ratio     Dim ratio [0; 255]
     */
    void dimRect(int16_t x, int16_t y, uint16_t width, uint16_t height, uint8_t ratio)
    {
        int16_t xIndex = 0;
        int16_t yIndex = 0;

        for(yIndex = 0; yIndex < height; ++yIndex)
        {
            for(xIndex = 0; xIndex < width; ++xIndex)
            {
                dimPixel(x + xIndex, y + yIndex, ratio);
            }
        }
    }

    /**
     * Dim screen to black with a given ratio.
     * A dim ratio of 255 means no change.
     *
     * @param[in] ratio Dim ratio [0; 255]
     */
    void dimScreen(uint8_t ratio)
    {
        dimRect(0, 0, m_width, m_height, ratio);
    }

    /**
     * Draw bitmap buffer. It assumes that the pixel position in the buffer
     * follows this algorithm: position in buffer = x + y * width
     *
     * @param[in] x         x-coordinate of upper left point
     * @param[in] y         y-coordinate of upper left point
     * @param[in] bitmap    Bitmap pixel buffer
     * @param[in] width     Bitmap width in pixel
     * @param[in] height    Bitmap height in pixel
     */
    void drawBitmap(int16_t x, int16_t y, const TColor* bitmap, uint16_t width, uint16_t height)
    {
        if (nullptr != bitmap)
        {
            int16_t xIndex = 0;
            int16_t yIndex = 0;

            for(yIndex = 0; yIndex < height; ++yIndex)
            {
                for(xIndex = 0; xIndex < width; ++xIndex)
                {
                    drawPixel(x + xIndex, y + yIndex, bitmap[xIndex + width * yIndex]);
                }
            }
        }
    }

protected:

    uint16_t    m_width;    /**< Canvas width in pixel */
    uint16_t    m_height;   /**< Canvas height in pixel */

    /**
     * Constructs the base graphics functionality.
     *
     * @param[in] width     Canvas width in pixel
     * @param[in] height    Canvas height in pixel
     */
    BaseGfx(uint16_t width, uint16_t height) :
        m_width(width),
        m_height(height)
    {
    }

    /**
     * Constructs the base graphics functionality by copying one.
     *
     * @param[in] gfx   Base gfx which to copy
     */
    BaseGfx(const BaseGfx& gfx) :
        m_width(gfx.m_width),
        m_height(gfx.m_height)
    {
    }

private:

    /* Default constructor not allowed. */
    BaseGfx();

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __BASE_GFX_HPP__ */

/** @} */