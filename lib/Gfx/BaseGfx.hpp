/* MIT License
 *
 * Copyright (c) 2019 - 2020 Andreas Merkle <web@blue-andi.de>
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
#include <gfxfont.h>

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
     * Constructs the base graphics functionality.
     *
     * @param[in] width     Canvas width in pixel
     * @param[in] height    Canvas height in pixel
     */
    BaseGfx(uint16_t width, uint16_t height) :
        m_width(width),
        m_height(height),
        m_cursorX(0),
        m_cursorY(0),
        m_textColor(0U),
        m_isTextWrapEnabled(false),
        m_font(nullptr)
    {
    }

    /**
     * Destroys the base graphics functionality object.
     */
    ~BaseGfx()
    {
    }

    /**
     * Constructs the base graphics functionality by copying one.
     *
     * @param[in] gfx   Base gfx which to copy
     */
    BaseGfx(const BaseGfx& gfx) :
        m_width(gfx.m_width),
        m_height(gfx.m_height),
        m_cursorX(gfx.m_cursorX),
        m_cursorY(gfx.m_cursorY),
        m_isTextWrapEnabled(gfx.m_isTextWrapEnabled),
        m_font(gfx.m_font)
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
            m_width             = gfx.m_width;
            m_height            = gfx.m_height;
            m_cursorX           = gfx.m_cursorX;
            m_cursorY           = gfx.m_cursorY;
            m_isTextWrapEnabled = gfx.m_isTextWrapEnabled;
            m_font              = gfx.m_font;
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
        uint16_t index = 0U;

        for(index = 0U; index < height; ++index)
        {
            drawPixel(x, y + index, color);
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
        uint16_t index = 0U;

        for(index = 0U; index < width; ++index)
        {
            drawPixel(x + index, y, color);
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
     * Fill a rectangle with a specific color.
     *
     * @param[in] color     Color
     */
    void fillScreen(const TColor& color)
    {
        fillRect(0, 0, m_width, m_height, color);
    }

    /**
     * Draw bitmap buffer.
     *
     * @param[in] x         x-coordinate of upper left point
     * @param[in] y         y-coordinate of upper left point
     * @param[in] bitmap    Bitmap pixel buffer
     * @param[in] width     Bitmap width in pixel
     * @param[in] height    Bitmap height in pixel
     */
    void drawRGBBitmap(int16_t x, int16_t y, const TColor* bitmap, uint16_t width, uint16_t height)
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

    /**
     * Get cursor position.
     *
     * @param[out] x x-coordinate
     * @param[out] y y-coordinate
     */
    void getTextCursorPos(int16_t& x, int16_t& y) const
    {
        x = m_cursorX;
        y = m_cursorY;
    }

    /**
     * Get cursor x-position.
     *
     * @return Cursor x-position
     */
    int16_t getTextCursorPosX() const
    {
        return m_cursorX;
    }

    /**
     * Get cursor y-position.
     *
     * @return Cursor y-position
     */
    int16_t getTextCursorPosY() const
    {
        return m_cursorY;
    }

    /**
     * Move text cursor to position.
     *
     * @param[in] x x-coordinate
     * @param[in] y y-coordinate
     */
    void setTextCursorPos(int16_t x, int16_t y)
    {
        m_cursorX = x;
        m_cursorY = y;
    }

    /**
     * Get text color.
     *
     * @return Text color
     */
    TColor getTextColor() const
    {
        return m_textColor;
    }

    /**
     * Set text color.
     *
     * @param[in] color Text color
     */
    void setTextColor(const TColor& color)
    {
        m_textColor = color;
    }

    /**
     * Get font.
     *
     * @return Font
     */
    const GFXfont* getFont() const
    {
        return m_font;
    }

    /**
     * Set font.
     *
     * @param[in] font  Font
     */
    void setFont(const GFXfont* font)
    {
        m_font = font;
    }

    /**
     * Draw single character at current cursor position.
     *
     * @param[in] singleChar    Single character which to draw
     */
    void drawChar(char singleChar)
    {
        uint8_t uChar = static_cast<uint8_t>(singleChar);

        if (nullptr == m_font)
        {
            return;
        }

        /* Set cursor to next line? */
        if ('\n' == singleChar)
        {
            /* Move cursor to begin and one row down. */
            m_cursorX = 0;
            m_cursorY += m_font->yAdvance;
        }
        /* Is character available in the font? Note, carriage return is skipped. */
        else if (('\r' != singleChar) &&
                 (m_font->first <= uChar) &&
                 (m_font->last >= uChar))
        {
            uint8_t         glyphIndex      = uChar - m_font->first;
            const GFXglyph* glyph           = &(m_font->glyph[glyphIndex]);
            int16_t         x               = 0;
            int16_t         y               = 0;
            uint16_t        bitmapOffset    = glyph->bitmapOffset;
            uint8_t         bitmapRowBits   = 0U;
            uint8_t         bitCnt          = 0U;

            /* If text wrap around is enabled and the character is clipping,
             * jump to the next line.
             */
            if (true == m_isTextWrapEnabled)
            {
                if (m_width < (m_cursorX + glyph->xOffset + glyph->width))
                {
                    m_cursorX = 0;
                    m_cursorY += m_font->yAdvance;
                }
            }

            for(y = 0U; y < glyph->height; ++y)
            {
                for(x = 0U; x < glyph->width; ++x)
                {
                    /* Every 8 bit, the bitmap offset must be increased. */
                    if (0U == (bitCnt & 0x07))
                    {
                        bitmapRowBits = m_font->bitmap[bitmapOffset];
                        ++bitmapOffset;
                    }
                    ++bitCnt;

                    /* A 1b in the bitmap row bits must be drawn as single pixel. */
                    if (0U != (bitmapRowBits & 0x80U))
                    {
                        drawPixel(m_cursorX + x + glyph->xOffset, m_cursorY + y + glyph->yOffset, m_textColor);
                    }

                    bitmapRowBits <<= 1U;
                }
            }

            m_cursorX += glyph->xAdvance;
        }
        /* Skip character */
        else
        {
            ;
        }
    }

    /**
     * Draw a text at given cursor position.
     *
     * @param[in] text  Text which to draw
     */
    void drawText(const char* text)
    {
        size_t index = 0U;

        if (nullptr == m_font)
        {
            return;
        }

        while('\0' != text[index])
        {
            drawChar(text[index]);

            ++index;
        }
    }

    /**
     * Is text wrap around enabled?
     *
     * @return If text wrap around is enabled, it will return true otherwise false.
     */
    bool isTextWrapEnabled() const
    {
        return m_isTextWrapEnabled;
    }

    /**
     * Set text wrap around behaviour.
     *
     * @param[in] isEnabled Enable (true) wrap around or disable (false) it
     */
    void setTextWrap(bool isEnabled)
    {
        m_isTextWrapEnabled = isEnabled;
    }

    /**
     * Get bounding box of single character.
     *
     * @param[in]   singleChar  Single character
     * @param[out]  width       Width in pixel
     * @param[out]  height      Height in pixel
     *
     * @return If character is valid, it will return true otherwise false.
     */
    bool getCharBoundingBox(char singleChar, uint16_t& width, uint16_t& height) const
    {
        bool    status  = false;
        uint8_t uChar   = static_cast<uint8_t>(singleChar);

        if ((nullptr != m_font) &&
            (m_font->first <= uChar) &&
            (m_font->last >= uChar) &&
            ('\n' != singleChar) &&
            ('\r' != singleChar))
        {
            uint8_t         glyphIndex  = uChar - m_font->first;
            const GFXglyph* glyph       = &(m_font->glyph[glyphIndex]);

            width   = glyph->xAdvance;
            height  = m_font->yAdvance;
            status  = true;
        }

        return status;
    }

    /**
     * Get bounding box of text.
     *
     * @param[in]   text    Text
     * @param[out]  width   Width in pixel
     * @param[out]  height  Height in pixel
     */
    bool getTextBoundingBox(const char* text, uint16_t& width, uint16_t& height) const
    {
        bool status = false;

        if ((nullptr != text) &&
            (nullptr != m_font))
        {
            size_t      index       = 0U;
            uint16_t    lineWidth   = 0U;

            width   = 0U;
            height  = 0U;

            while('\0' != text[index])
            {
                uint16_t charWidth  = 0U;
                uint16_t charHeight = 0U;

                if ('\n' == text[index])
                {
                    if (width < lineWidth)
                    {
                        width = lineWidth;
                    }

                    lineWidth = 0U;
                    height += m_font->yAdvance;;
                }
                else if (true == getCharBoundingBox(text[index], charWidth, charHeight))
                {
                    if (0U == index)
                    {
                        height += m_font->yAdvance;
                    }

                    /* If text wrap around is enabled and the character is clipping,
                     * jump to the next line.
                     */
                    if (true == m_isTextWrapEnabled)
                    {
                        if (m_width < (lineWidth + charWidth))
                        {
                            if (width < lineWidth)
                            {
                                width = lineWidth;
                            }

                            lineWidth = 0U;
                            height += m_font->yAdvance;
                        }
                    }

                    lineWidth += charWidth;
                }
                else
                {
                    ;
                }

                ++index;
            }

            if (width < lineWidth)
            {
                width = lineWidth;
            }

            status = true;
        }

        return status;
    }

protected:

    uint16_t        m_width;                /**< Canvas width in pixel */
    uint16_t        m_height;               /**< Canvas height in pixel */
    int16_t         m_cursorX;              /**< Cursor x-coordinate */
    int16_t         m_cursorY;              /**< Cursor y-coordinate */
    TColor          m_textColor;            /**< Text color */
    bool            m_isTextWrapEnabled;    /**< Is text wrap around enabled or not? */
    const GFXfont*  m_font;                 /**< Current selected font */

    /* Default constructor not allowed. */
    BaseGfx();

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __BASE_GFX_HPP__ */

/** @} */