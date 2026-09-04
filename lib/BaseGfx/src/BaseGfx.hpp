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
 * @file   BaseGfx.hpp
 * @brief  Basic graphics interface
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup GFX
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
    virtual uint16_t getWidth() const  = 0;

    /**
     * Get canvas height in pixel.
     *
     * @return Canvas height in pixel.
     */
    virtual uint16_t getHeight() const = 0;

    /**
     * Get the visible area in canvas coordinates. Every graphic operation
     * outside of the visible area has no effect.
     *
     * By default the whole canvas is visible. A canvas, which is part of a
     * parent canvas, may be visible only partly.
     *
     * @param[out] x        x-coordinate of the upper left corner of the visible area.
     * @param[out] y        y-coordinate of the upper left corner of the visible area.
     * @param[out] width    Width of the visible area in pixel.
     * @param[out] height   Height of the visible area in pixel.
     */
    virtual void getVisibleArea(int16_t& x, int16_t& y, uint16_t& width, uint16_t& height) const
    {
        x      = 0;
        y      = 0;
        width  = getWidth();
        height = getHeight();
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
    virtual TColor& getColor(int16_t x, int16_t y)                                                           = 0;

    /**
     * Get pixel color at given position.
     *
     * @param[in] x x-coordinate
     * @param[in] y y-coordinate
     *
     * @return Color
     */
    virtual const TColor& getColor(int16_t x, int16_t y) const                                               = 0;

    /**
     * Draw a single pixel at given position.
     *
     * @param[in] x     x-coordinate
     * @param[in] y     y-coordinate
     * @param[in] color Color
     */
    virtual void drawPixel(int16_t x, int16_t y, const TColor& color)                                        = 0;

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
    virtual TColor* getFrameBufferXAddr(int16_t x, int16_t y, uint16_t length, uint16_t& offset)             = 0;

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
    virtual TColor* getFrameBufferYAddr(int16_t x, int16_t y, uint16_t length, uint16_t& offset)             = 0;

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
        uint16_t minWidth  = std::min(getWidth(), gfx.getWidth());
        uint16_t minHeight = std::min(getHeight(), gfx.getHeight());

        /* For better performance choose larger side for the internal
         * copy operation.
         */
        if (minWidth >= minHeight)
        {
            int16_t y;

            for (y = 0; y < minHeight; ++y)
            {
                internalCopyX(0, y, minWidth, gfx, 0, y);
            }
        }
        else
        {
            int16_t x;

            for (x = 0; x < minWidth; ++x)
            {
                internalCopyY(x, 0, minHeight, gfx, x, 0);
            }
        }
    }

    /**
     * Copy from source, starting at upper left corner (0, 0), while dimming
     * every pixel by the given brightness level.
     *
     * The source is not modified, so this is a non-destructive dim used e.g.
     * for fade in/out effects. It replaces the former approach of storing a
     * per-pixel intensity in the color and dimming the source in place.
     *
     * @param[in] gfx       Graphics interface of source
     * @param[in] intensity Brightness level [0; 255] - 0: black / 255: no change.
     */
    void copyDimmed(const BaseGfx<TColor>& gfx, uint8_t intensity)
    {
        uint16_t minWidth  = std::min(getWidth(), gfx.getWidth());
        uint16_t minHeight = std::min(getHeight(), gfx.getHeight());
        int16_t  x;
        int16_t  y;

        for (y = 0; y < minHeight; ++y)
        {
            for (x = 0; x < minWidth; ++x)
            {
                TColor color = gfx.getColor(x, y);

                color.dim(intensity);
                drawPixel(x, y, color);
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
        (void)clipRunY(x, y, height);

        /* Anything to draw? */
        if (0U < height)
        {
            uint16_t dstOffset  = 0U;
            TColor*  dstAddress = getFrameBufferYAddr(x, y, height, dstOffset);
            uint16_t idx        = 0U;

            if (nullptr != dstAddress)
            {
                while (height > idx)
                {
                    dstAddress[idx * dstOffset] = color;
                    ++idx;
                }
            }
            /* No framebuffer access available, draw pixel by pixel. */
            else
            {
                while (height > idx)
                {
                    drawPixel(x, static_cast<int16_t>(y + idx), color);
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
        (void)clipRunX(x, y, width);

        /* Anything to draw? */
        if (0U < width)
        {
            uint16_t dstOffset  = 0U;
            TColor*  dstAddress = getFrameBufferXAddr(x, y, width, dstOffset);
            uint16_t idx        = 0U;

            if (nullptr != dstAddress)
            {
                while (width > idx)
                {
                    dstAddress[idx * dstOffset] = color;
                    ++idx;
                }
            }
            /* No framebuffer access available, draw pixel by pixel. */
            else
            {
                while (width > idx)
                {
                    drawPixel(static_cast<int16_t>(x + idx), y, color);
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
        int16_t dX    = abs(xe - xs);
        int16_t stepX = (xs < xe) ? 1 : -1;
        int16_t dY    = -abs(ye - ys);
        int16_t stepY = (ys < ye) ? 1 : -1;
        int16_t err   = dX + dY; /* err_xy */
        int16_t err2  = 0;

        /* https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm */

        while (1)
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
        int16_t f     = 1 - radius;
        int16_t ddF_x = 0;
        int16_t ddF_y = -2 * radius;
        int16_t x     = 0;
        int16_t y     = radius;

        drawPixel(mx, my + radius, color);
        drawPixel(mx, my - radius, color);
        drawPixel(mx + radius, my, color);
        drawPixel(mx - radius, my, color);

        while (x < y)
        {
            if (0 <= f)
            {
                --y;
                ddF_y += 2;
                f     += ddF_y;
            }
            ++x;
            ddF_x += 2;
            f     += ddF_x + 1;

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

                for (yIndex = 0; yIndex < height; ++yIndex)
                {
                    drawHLine(x, y + yIndex, width, color);
                }
            }
            else
            {
                int16_t xIndex;

                for (xIndex = 0; xIndex < width; ++xIndex)
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
        int16_t f     = 1 - radius;
        int16_t ddF_x = 0;
        int16_t ddF_y = -2 * radius;
        int16_t x     = 0;
        int16_t y     = radius;

        drawHLine(mx - radius, my, 2 * radius, color);

        while (x < y)
        {
            if (0 <= f)
            {
                --y;
                ddF_y += 2;
                f     += ddF_y;
            }
            ++x;
            ddF_x += 2;
            f     += ddF_x + 1;

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
        const uint16_t bitmapWidth  = bitmap.getWidth();
        const uint16_t bitmapHeight = bitmap.getHeight();

        /* Anything to draw? The clipping to the visible area takes place during
         * the copy operation, which considers the source coordinates as well.
         * This way a bitmap, which is larger than the destination or partly
         * outside of it, will be cropped and not moved.
         */
        if ((0U < bitmapWidth) &&
            (0U < bitmapHeight))
        {
            /* For better performance choose larger side for the internal
             * copy operation.
             */
            if (bitmapWidth >= bitmapHeight)
            {
                uint16_t yIndex;

                for (yIndex = 0U; yIndex < bitmapHeight; ++yIndex)
                {
                    internalCopyX(x, static_cast<int16_t>(y + yIndex), bitmapWidth, bitmap, 0, static_cast<int16_t>(yIndex));
                }
            }
            else
            {
                uint16_t xIndex;

                for (xIndex = 0U; xIndex < bitmapWidth; ++xIndex)
                {
                    internalCopyY(static_cast<int16_t>(x + xIndex), y, bitmapHeight, bitmap, static_cast<int16_t>(xIndex), 0);
                }
            }
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
     * @param[in] x                 x-coordinate of upper left point
     * @param[in] y                 y-coordinate of upper left point
     * @param[in] bitmap            Bitmap pixel buffer
     * @param[in] transparentColor  Color which shall be treated as transparent.
     */
    void drawBitmap(int16_t x, int16_t y, const BaseGfxBitmap<TColor>& bitmap, const TColor& transparentColor)
    {
        const uint16_t bitmapWidth  = bitmap.getWidth();
        const uint16_t bitmapHeight = bitmap.getHeight();

        /* See drawBitmap() without transparent color. */
        if ((0U < bitmapWidth) &&
            (0U < bitmapHeight))
        {
            /* For better performance choose larger side for the internal
             * copy operation.
             */
            if (bitmapWidth >= bitmapHeight)
            {
                uint16_t yIndex;

                for (yIndex = 0U; yIndex < bitmapHeight; ++yIndex)
                {
                    internalCopyX(x, static_cast<int16_t>(y + yIndex), bitmapWidth, bitmap, 0, static_cast<int16_t>(yIndex), transparentColor);
                }
            }
            else
            {
                uint16_t xIndex;

                for (xIndex = 0U; xIndex < bitmapWidth; ++xIndex)
                {
                    internalCopyY(static_cast<int16_t>(x + xIndex), y, bitmapHeight, bitmap, static_cast<int16_t>(xIndex), 0, transparentColor);
                }
            }
        }
    }

protected:

    /* Constructs the graphic functionality. */
    BaseGfx()
    {
    }

    /**
     * Intersects two ranges on the same axis. The result is the part which
     * both ranges have in common. If they have nothing in common, the
     * resulting length will be 0.
     *
     * @param[in,out] coord         Start coordinate of the first range, will contain the start of the intersection.
     * @param[in,out] length        Length of the first range, will contain the length of the intersection.
     * @param[in] otherCoord        Start coordinate of the second range.
     * @param[in] otherLength       Length of the second range.
     */
    static void intersectRange(int16_t& coord, uint16_t& length, int16_t otherCoord, uint16_t otherLength)
    {
        const int32_t end      = static_cast<int32_t>(coord) + static_cast<int32_t>(length);
        const int32_t otherEnd = static_cast<int32_t>(otherCoord) + static_cast<int32_t>(otherLength);
        const int32_t start    = (coord > otherCoord) ? coord : otherCoord;
        const int32_t stop     = (end < otherEnd) ? end : otherEnd;

        if (stop <= start)
        {
            coord  = 0;
            length = 0U;
        }
        else
        {
            coord  = static_cast<int16_t>(start);
            length = static_cast<uint16_t>(stop - start);
        }
    }

private:

    /**
     * Clips a horizontal run of pixels to the visible area. This way the run
     * can be drawn via the framebuffer, even if it is only partly visible.
     *
     * @param[in,out] x         Start x-coordinate, will be moved to the first visible pixel.
     * @param[in] y             y-coordinate of the run.
     * @param[in,out] length    Number of pixels, will be reduced to the visible ones.
     *
     * @return Number of pixels which are clipped at the begin of the run.
     */
    uint16_t clipRunX(int16_t& x, int16_t y, uint16_t& length) const
    {
        int16_t  visibleX      = 0;
        int16_t  visibleY      = 0;
        uint16_t visibleWidth  = 0U;
        uint16_t visibleHeight = 0U;
        int16_t  startX        = x;
        uint16_t clippedAtHead = 0U;

        getVisibleArea(visibleX, visibleY, visibleWidth, visibleHeight);

        /* Is the whole run outside of the visible area? */
        if ((y < visibleY) ||
            ((visibleY + visibleHeight) <= y))
        {
            length = 0U;
        }
        else
        {
            intersectRange(x, length, visibleX, visibleWidth);

            if (0U < length)
            {
                clippedAtHead = static_cast<uint16_t>(x - startX);
            }
        }

        return clippedAtHead;
    }

    /**
     * Clips a vertical run of pixels to the visible area. This way the run
     * can be drawn via the framebuffer, even if it is only partly visible.
     *
     * @param[in] x             x-coordinate of the run.
     * @param[in,out] y         Start y-coordinate, will be moved to the first visible pixel.
     * @param[in,out] length    Number of pixels, will be reduced to the visible ones.
     *
     * @return Number of pixels which are clipped at the begin of the run.
     */
    uint16_t clipRunY(int16_t x, int16_t& y, uint16_t& length) const
    {
        int16_t  visibleX      = 0;
        int16_t  visibleY      = 0;
        uint16_t visibleWidth  = 0U;
        uint16_t visibleHeight = 0U;
        int16_t  startY        = y;
        uint16_t clippedAtHead = 0U;

        getVisibleArea(visibleX, visibleY, visibleWidth, visibleHeight);

        /* Is the whole run outside of the visible area? */
        if ((x < visibleX) ||
            ((visibleX + visibleWidth) <= x))
        {
            length = 0U;
        }
        else
        {
            intersectRange(y, length, visibleY, visibleHeight);

            if (0U < length)
            {
                clippedAtHead = static_cast<uint16_t>(y - startY);
            }
        }

        return clippedAtHead;
    }

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
                length = 0U;
                coord  = 0;
            }
            else
            {
                maxLength -= static_cast<uint16_t>(coord);
                length     = std::min(length, maxLength);
            }
        }
        /* Negative */
        else
        {
            length = std::min(length, maxLength);

            /* Out of bounds? */
            if (static_cast<uint16_t>(-coord) > length)
            {
                length = 0U;
                coord  = 0;
            }
            else
            {
                length -= static_cast<uint16_t>(-coord);
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
        /* Clip the run to the visible area and keep the source in sync. */
        const uint16_t clippedAtHead = clipRunX(x, y, width);
        const int16_t  srcStartX     = static_cast<int16_t>(srcX + clippedAtHead);

        /* Anything to copy? */
        if (0U < width)
        {
            uint16_t      dstOffset  = 0U;
            uint16_t      srcOffset  = 0U;
            TColor*       dstAddress = getFrameBufferXAddr(x, y, width, dstOffset);
            const TColor* srcAddress = src.getFrameBufferXAddr(srcStartX, srcY, width, srcOffset);
            uint16_t      idx        = 0U;

            if ((nullptr != dstAddress) &&
                (nullptr != srcAddress))
            {
                while (width > idx)
                {
                    dstAddress[idx * dstOffset] = srcAddress[idx * srcOffset];
                    ++idx;
                }
            }
            /* No framebuffer access available, copy pixel by pixel. */
            else
            {
                while (width > idx)
                {
                    drawPixel(static_cast<int16_t>(x + idx), y, src.getColor(static_cast<int16_t>(srcStartX + idx), srcY));
                    ++idx;
                }
            }
        }
    }

    /**
     * Copies pixels along the x-axis from a source at given coordinates to the
     * destination at given coordinates. If the source pixel color matches the
     * transparent color, it will not be copied.
     *
     * @param[in] x                 Destination x-coordinate.
     * @param[in] y                 Destination y-coordinate.
     * @param[in] width             Number of pixels which to copy.
     * @param[in] src               Source to copy from.
     * @param[in] srcX              Source x-coordinate.
     * @param[in] srcY              Source y-coordinate.
     * @param[in] transparentColor  Color which shall be treated as transparent.
     */
    void internalCopyX(int16_t x, int16_t y, uint16_t width, const BaseGfx<TColor>& src, int16_t srcX, int16_t srcY, const TColor& transparentColor)
    {
        /* Clip the run to the visible area and keep the source in sync. */
        const uint16_t clippedAtHead = clipRunX(x, y, width);
        const int16_t  srcStartX     = static_cast<int16_t>(srcX + clippedAtHead);

        /* Anything to copy? */
        if (0U < width)
        {
            uint16_t      dstOffset  = 0U;
            uint16_t      srcOffset  = 0U;
            TColor*       dstAddress = getFrameBufferXAddr(x, y, width, dstOffset);
            const TColor* srcAddress = src.getFrameBufferXAddr(srcStartX, srcY, width, srcOffset);
            uint16_t      idx        = 0U;

            if ((nullptr != dstAddress) &&
                (nullptr != srcAddress))
            {
                while (width > idx)
                {
                    const TColor* color = &srcAddress[idx * srcOffset];

                    if (transparentColor != (*color))
                    {
                        dstAddress[idx * dstOffset] = srcAddress[idx * srcOffset];
                    }

                    ++idx;
                }
            }
            /* No framebuffer access available, copy pixel by pixel. */
            else
            {
                while (width > idx)
                {
                    const TColor& color = src.getColor(static_cast<int16_t>(srcStartX + idx), srcY);

                    if (transparentColor != color)
                    {
                        drawPixel(static_cast<int16_t>(x + idx), y, color);
                    }

                    ++idx;
                }
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
        /* Clip the run to the visible area and keep the source in sync. */
        const uint16_t clippedAtHead = clipRunY(x, y, height);
        const int16_t  srcStartY     = static_cast<int16_t>(srcY + clippedAtHead);

        /* Anything to copy? */
        if (0U < height)
        {
            uint16_t      dstOffset  = 0U;
            uint16_t      srcOffset  = 0U;
            TColor*       dstAddress = getFrameBufferYAddr(x, y, height, dstOffset);
            const TColor* srcAddress = src.getFrameBufferYAddr(srcX, srcStartY, height, srcOffset);
            uint16_t      idx        = 0U;

            if ((nullptr != dstAddress) &&
                (nullptr != srcAddress))
            {
                while (height > idx)
                {
                    dstAddress[idx * dstOffset] = srcAddress[idx * srcOffset];
                    ++idx;
                }
            }
            /* No framebuffer access available, copy pixel by pixel. */
            else
            {
                while (height > idx)
                {
                    drawPixel(x, static_cast<int16_t>(y + idx), src.getColor(srcX, static_cast<int16_t>(srcStartY + idx)));
                    ++idx;
                }
            }
        }
    }

    /**
     * Copies pixels along the y-axis from a source at given coordinates to the
     * destination at given coordinates. If the source pixel color matches the
     * transparent color, it will not be copied.
     *
     * @param[in] x                 Destination x-coordinate.
     * @param[in] y                 Destination y-coordinate.
     * @param[in] height            Number of pixels which to copy.
     * @param[in] src               Source to copy from.
     * @param[in] srcX              Source x-coordinate.
     * @param[in] srcY              Source y-coordinate.
     * @param[in] transparentColor  Color which shall be treated as transparent.
     */
    void internalCopyY(int16_t x, int16_t y, uint16_t height, const BaseGfx<TColor>& src, int16_t srcX, int16_t srcY, const TColor& transparentColor)
    {
        /* Clip the run to the visible area and keep the source in sync. */
        const uint16_t clippedAtHead = clipRunY(x, y, height);
        const int16_t  srcStartY     = static_cast<int16_t>(srcY + clippedAtHead);

        /* Anything to copy? */
        if (0U < height)
        {
            uint16_t      dstOffset  = 0U;
            uint16_t      srcOffset  = 0U;
            TColor*       dstAddress = getFrameBufferYAddr(x, y, height, dstOffset);
            const TColor* srcAddress = src.getFrameBufferYAddr(srcX, srcStartY, height, srcOffset);
            uint16_t      idx        = 0U;

            if ((nullptr != dstAddress) &&
                (nullptr != srcAddress))
            {
                while (height > idx)
                {
                    const TColor* color = &srcAddress[idx * srcOffset];

                    if (transparentColor != (*color))
                    {
                        dstAddress[idx * dstOffset] = srcAddress[idx * srcOffset];
                    }

                    ++idx;
                }
            }
            /* No framebuffer access available, copy pixel by pixel. */
            else
            {
                while (height > idx)
                {
                    const TColor& color = src.getColor(srcX, static_cast<int16_t>(srcStartY + idx));

                    if (transparentColor != color)
                    {
                        drawPixel(x, static_cast<int16_t>(y + idx), color);
                    }

                    ++idx;
                }
            }
        }
    }
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* BASE_GFX_HPP */

/** @} */