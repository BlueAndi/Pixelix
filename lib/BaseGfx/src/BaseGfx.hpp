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
 * @brief  Basic graphics interface
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup gfx
 *
 * @{
 */

#ifndef BASE_GFX_HPP
#define BASE_GFX_HPP

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

template < typename TColor >
class BaseGfxBitmap;

/**
 * This class provides the base graphic functions, which are
 * color format agnostic. This way it can be used for different
 * kind of color formats, e.g. RGB565 or RGB888 format.
 *
 * @tparam TColor The color representation.
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
     * Get width in pixel.
     *
     * @return Canvas width in pixel
     */
    virtual uint16_t getWidth() const = 0;

    /**
     * Get height in pixel.
     *
     * @return Canvas height in pixel
     */
    virtual uint16_t getHeight() const = 0;

    /**
     * Get pixel color at given position.
     * This is used for color manipulation in higher layers.
     *
     * @param[in] x x-coordinate
     * @param[in] y y-coordinate
     *
     * @return Color
     */
    virtual TColor& getColor(int16_t x, int16_t y) = 0;

    /**
     * Get pixel color at given position.
     *
     * @param[in] x x-coordinate
     * @param[in] y y-coordinate
     *
     * @return Color
     */
    virtual const TColor& getColor(int16_t x, int16_t y) const = 0;

    /**
     * Draw a single pixel at given position.
     *
     * @param[in] x     x-coordinate
     * @param[in] y     y-coordinate
     * @param[in] color Color
     */
    virtual void drawPixel(int16_t x, int16_t y, const TColor& color) = 0;

    /**
     * Copy framebuffer content.
     *
     * @param[in] gfx   Graphics interface of framebuffer source
     */
    void copy(const BaseGfx<TColor>& gfx)
    {
        uint16_t    canvasWidth     = getWidth();
        uint16_t    canvasHeight    = getHeight();
        int16_t     x               = 0;
        int16_t     y               = 0;

        for(y = 0; y < canvasHeight; ++y)
        {
            for(x = 0; x < canvasWidth; ++x)
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
     * Draw a circle with a specific color.
     * https://en.wikipedia.org/wiki/Midpoint_circle_algorithm
     * 
     * @param[in] mx        x-coordinate of middle point
     * @param[in] my        y-coordinate of middle point
     * @param[in] radius    Radius in pixel
     * @param[in] color     Color
     */
    void drawCircle(int16_t mx, int16_t my, uint16_t radius, const TColor& color)
    {
        int16_t f       = 1 - radius;
        int16_t ddF_x   = 0;
        int16_t ddF_y   = -2 * radius;
        int16_t x       = 0;
        int16_t y       = radius;

        drawPixel(mx, my + radius, color);
        drawPixel(mx, my - radius, color);
        drawPixel(mx + radius, my, color);
        drawPixel(mx - radius, my, color);

        while(x < y)
        {
            if (0 <=f)
            {
                --y;
                ddF_y += 2;
                f += ddF_y;
            }
            ++x;
            ddF_x += 2;
            f += ddF_x + 1;

            drawPixel(mx + x, my + y, color);
            drawPixel(mx - x, my + y, color);

            drawPixel(mx + x, my - y, color);
            drawPixel(mx - x, my - y, color);

            drawPixel(mx + y, my + x, color);
            drawPixel(mx - y, my + x, color);

            drawPixel(mx + y, my - x, color);
            drawPixel(mx - y, my - x, color);
        }
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
        fillRect(0, 0, getWidth(), getHeight(), color);
    }

    /**
     * Fill a circle with a specific color.
     * https://en.wikipedia.org/wiki/Midpoint_circle_algorithm
     * 
     * @param[in] mx        x-coordinate of middle point
     * @param[in] my        y-coordinate of middle point
     * @param[in] radius    Radius in pixel
     * @param[in] color     Color
     */
    void fillCircle(int16_t mx, int16_t my, uint16_t radius, const TColor& color)
    {
        int16_t f       = 1 - radius;
        int16_t ddF_x   = 0;
        int16_t ddF_y   = -2 * radius;
        int16_t x       = 0;
        int16_t y       = radius;

        drawHLine(mx - radius, my, 2 * radius, color);

        while(x < y)
        {
            if (0 <=f)
            {
                --y;
                ddF_y += 2;
                f += ddF_y;
            }
            ++x;
            ddF_x += 2;
            f += ddF_x + 1;

            drawHLine(mx - x, my + y, 2 * x, color);
            drawHLine(mx - x, my - y, 2 * x, color);
            
            drawHLine(mx - y, my + x, 2 * y, color);
            drawHLine(mx - y, my - x, 2 * y, color);
        }
    }

    /**
     * Draw bitmap at specified location (upper left point).
     *
     * @param[in] x         x-coordinate of upper left point
     * @param[in] y         y-coordinate of upper left point
     * @param[in] bitmap    Bitmap pixel buffer
     */
    void drawBitmap(int16_t x, int16_t y, const BaseGfxBitmap<TColor>& bitmap)
    {
        uint16_t    canvasWidth     = bitmap.getWidth();
        uint16_t    canvasHeight    = bitmap.getHeight();
        int16_t     xIndex          = 0;
        int16_t     yIndex          = 0;

        for(yIndex = 0; yIndex < canvasHeight; ++yIndex)
        {
            for(xIndex = 0; xIndex < canvasWidth; ++xIndex)
            {
                drawPixel(x + xIndex, y + yIndex, bitmap.getColor(xIndex, yIndex));
            }
        }
    }

protected:

    /* Constructs the graphic functionality. */
    BaseGfx()
    {
    }

private:

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* BASE_GFX_HPP */

/** @} */