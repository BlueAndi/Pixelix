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
#include <algorithm>

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
     * Get canvas width in pixel.
     *
     * @return Canvas width in pixel.
     */
    virtual uint16_t getWidth() const = 0;

    /**
     * Get canvas height in pixel.
     *
     * @return Canvas height in pixel.
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
    virtual TColor* getFrameBufferXAddr(int16_t x, int16_t y, uint16_t length, uint16_t& offset) = 0;

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
    virtual const TColor* getFrameBufferXAddr(int16_t x, int16_t y, uint16_t length, uint16_t& offset) const = 0;

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
    virtual TColor* getFrameBufferYAddr(int16_t x, int16_t y, uint16_t length, uint16_t& offset) = 0;

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
    virtual const TColor* getFrameBufferYAddr(int16_t x, int16_t y, uint16_t length, uint16_t& offset) const = 0;

    /**
     * Copy from source, starting at upper left corner (0, 0).
     * 
     * If the source size is lower or equal than the destination, the
     * source will be copied complete.
     * 
     * If the source size is greater than the destination, the source
     * will be copied partly.
     *
     * @param[in] gfx   Graphics interface of source
     */
    void copy(const BaseGfx<TColor>& gfx)
    {
        uint16_t    minWidth    = std::min(getWidth(), gfx.getWidth());
        uint16_t    minHeight   = std::min(getHeight(), gfx.getHeight());

        /* For better performance choose larger side for the internal
         * copy operation.
         */
        if (minWidth >= minHeight)
        {
            int16_t y;

            for(y = 0; y < minHeight; ++y)
            {
                internalCopyX(0, y, minWidth, gfx, 0, y);
            }
        }
        else
        {
            int16_t x;

            for(x = 0; x < minWidth; ++x)
            {
                internalCopyY(x, 0, minHeight, gfx, x, 0);
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
        adaptCoordAndLength(y, height, getHeight());

        /* Anything to draw? */
        if (0U < height)
        {
            uint16_t    dstOffset   = 0U;
            TColor*     dstAddress  = getFrameBufferYAddr(x, y, height, dstOffset);

            if (nullptr != dstAddress)
            {
                uint16_t idx = 0U;

                while(height > idx)
                {
                    dstAddress[idx * dstOffset] = color;
                    ++idx;
                }
            }
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
        adaptCoordAndLength(x, width, getWidth());

        /* Anything to draw? */
        if (0U < width)
        {
            uint16_t    dstOffset   = 0U;
            TColor*     dstAddress  = getFrameBufferXAddr(x, y, width, dstOffset);

            if (nullptr != dstAddress)
            {
                uint16_t idx = 0U;

                while(width > idx)
                {
                    dstAddress[idx * dstOffset] = color;
                    ++idx;
                }
            }
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
        drawVLine(x1, y1 + 1, height - 2U, color);
        drawVLine(x1 + width - 1, y1 + 1, height - 2U, color);
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
        adaptCoordAndLength(x, width, getWidth());
        adaptCoordAndLength(y, height, getHeight());

        /* Anything to draw? */
        if ((0U < width) &&
            (0U < height))
        {
            /* For better performance choose larger side for the draw
             * operation.
             */
            if (width >= height)
            {
                int16_t yIndex;

                for(yIndex = 0; yIndex < height; ++yIndex)
                {
                    drawHLine(x, y + yIndex, width, color);
                }
            }
            else
            {
                int16_t xIndex;

                for(xIndex = 0; xIndex < height; ++xIndex)
                {
                    drawVLine(x + xIndex, y, height, color);
                }
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
     * Draw bitmap at specified location by given upper left point.
     * 
     * If the bitmap size is lower or equal than the destination, the
     * bitmap will be copied complete.
     * 
     * If the bitmap size is greater than the destination, the bitmap
     * will be copied partly.
     *
     * @param[in] x         x-coordinate of upper left point
     * @param[in] y         y-coordinate of upper left point
     * @param[in] bitmap    Bitmap pixel buffer
     */
    void drawBitmap(int16_t x, int16_t y, const BaseGfxBitmap<TColor>& bitmap)
    {
        uint16_t    minWidth    = std::min(bitmap.getWidth(), getWidth());
        uint16_t    minHeight   = std::min(bitmap.getHeight(), getHeight());

        adaptCoordAndLength(x, minWidth, getWidth());
        adaptCoordAndLength(y, minHeight, getHeight());

        /* Anything to draw? */
        if ((0U < minWidth) &&
            (0U < minHeight))
        {
            /* For better performance choose larger side for the internal
             * copy operation.
             */
            if (minWidth >= minHeight)
            {
                int16_t yIndex;

                for(yIndex = 0; yIndex < minHeight; ++yIndex)
                {
                    internalCopyX(x, y + yIndex, minWidth, bitmap, 0, yIndex);
                }
            }
            else
            {
                int16_t xIndex;

                for(xIndex = 0; xIndex < minWidth; ++xIndex)
                {
                    internalCopyY(x + xIndex, y, minHeight, bitmap, xIndex, 0);
                }
            }
        }
    }

protected:

    /* Constructs the graphic functionality. */
    BaseGfx()
    {
    }

private:

    /**
     * Adapts the coordinate and the length for a axis to ensure that
     * the coordinate is inside the valid canvas border and
     * the length, calculated from the coord on, will be inside the valid canvas too.
     * 
     * Example for x-axis:
     *      Line shall be drawn from -10 with a length of 80.
     *      The canvas width is 32.
     *      After adaption, the x-coordinate will be 0 and the length will be 32.
     * 
     * @param[in,out] coord     x- or y-coordinate, depends on the choosen axis.
     * @param[in,out] length    Width or height which is requested, starting from coord.
     * @param[in] maxLength     Max. length of the choosen axis.
     */
    void adaptCoordAndLength(int16_t& coord, uint16_t& length, uint16_t maxLength)
    {
        /* Positive? */
        if (0 <= coord)
        {
            /* Out of bounds? */
            if (maxLength < coord)
            {
                length  = 0U;
                coord   = 0;
            }
            else
            {
                maxLength -= static_cast<uint16_t>(coord);
                length = std::min(length, maxLength);
            }
        }
        /* Negative */
        else
        {
            length = std::min(length, maxLength);

            /* Out of bounds? */
            if (static_cast<uint16_t>(-coord) > length)
            {
                length  = 0U;
                coord   = 0;
            }
            else
            {
                length  -= static_cast<uint16_t>(-coord);
                coord   = 0;
            }
        }
    }

    /**
     * Copies pixels along the x-axis from a source at given coordinates to the
     * destination at given coordinates.
     * 
     * @param[in] x         Destination x-coordinate.
     * @param[in] y         Destination y-coordinate.
     * @param[in] width     Number of pixels which to copy.
     * @param[in] src       Source to copy from.
     * @param[in] srcX      Source x-coordinate.
     * @param[in] srcY      Source y-coordinate.
     */
    void internalCopyX(int16_t x, int16_t y, uint16_t width, const BaseGfx<TColor>& src, int16_t srcX, int16_t srcY)
    {
        uint16_t        dstOffset   = 0U;
        uint16_t        srcOffset   = 0U;
        TColor*         dstAddress  = getFrameBufferXAddr(x, y, width, dstOffset);
        const TColor*   srcAddress  = src.getFrameBufferXAddr(srcX, srcY, width, srcOffset);

        if ((nullptr != dstAddress) &&
            (nullptr != srcAddress))
        {
            uint16_t idx = 0U;

            while(width > idx)
            {
                dstAddress[idx * dstOffset] = srcAddress[idx * srcOffset];
                ++idx;
            }
        }
    }

    /**
     * Copies pixels along the y-axis from a source at given coordinates to the
     * destination at given coordinates.
     * 
     * @param[in] x         Destination x-coordinate.
     * @param[in] y         Destination y-coordinate.
     * @param[in] height    Number of pixels which to copy.
     * @param[in] src       Source to copy from.
     * @param[in] srcX      Source x-coordinate.
     * @param[in] srcY      Source y-coordinate.
     */
    void internalCopyY(int16_t x, int16_t y, uint16_t height, const BaseGfx<TColor>& src, int16_t srcX, int16_t srcY)
    {
        uint16_t        dstOffset   = 0U;
        uint16_t        srcOffset   = 0U;
        TColor*         dstAddress  = getFrameBufferYAddr(x, y, height, dstOffset);
        const TColor*   srcAddress  = src.getFrameBufferYAddr(srcX, srcY, height, srcOffset);

        if ((nullptr != dstAddress) &&
            (nullptr != srcAddress))
        {
            uint16_t idx = 0U;

            while(height > idx)
            {
                dstAddress[idx * dstOffset] = srcAddress[idx * srcOffset];
                ++idx;
            }
        }
    }
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* BASE_GFX_HPP */

/** @} */