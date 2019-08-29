/* MIT License
 *
 * Copyright (c) 2019 Andreas Merkle <web@blue-andi.de>
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
@brief  Abstract graphics interface for test
@author Andreas Merkle <web@blue-andi.de>

@section desc Description
This module provides the general graphics interface for testing purposes.

*******************************************************************************/
/** @defgroup adafruitgfx Abstract graphics interface for test
 * This module provides the abstract graphics interface for testing purposes.
 *
 * @{
 */

#ifndef __ADAFRUIT_GFX_H__
#define __ADAFRUIT_GFX_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <WString.h>
#include <gfxfont.h>
#include <Arduino.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Abstract graphics interface for testing purposes.
 */
class Adafruit_GFX
{
public:

    /**
     * Constructs the abstract graphics interface, which can be used to
     * draw inside a defined area.
     * 
     * @param[in] width     Area width in pixel
     * @param[in] height    Area height in pixel
     */
    Adafruit_GFX(int16_t width, int16_t height) :
        m_width(width),
        m_height(height),
        m_cursorX(0),
        m_cursorY(0),
        m_textColor(0u),
        m_textBgColor(0u),
        m_font(NULL)
    {
    }

    /**
     * Destroys the abstract graphics interface.
     */
    virtual ~Adafruit_GFX()
    {
    }

    /**
     * Get width in pixel.
     * 
     * @return Width in pixel
     */
    int16_t width(void) const
    {
        return m_width;
    }

    /**
     * Get height in pixel.
     * 
     * @return Height in pixel
     */
    int16_t height(void) const
    {
        return m_height;
    }

    /**
     * Draw a single pixel in the matrix and ensure that the drawing borders
     * are not violated.
     * Note, method declaration is from Adafruit_GFX.
     * 
     * @param[in] x     x-coordinate
     * @param[in] y     y-coordinate
     * @param[in] color Pixel color
     */
    virtual void drawPixel(int16_t x, int16_t y, uint16_t color) = 0;

    /**
     * Fill a rectangle.
     * Note, method declaration is from Adafruit_GFX.
     * 
     * @param[in] x         Upper left corner x-coordinate
     * @param[in] y         Upper left corner y-coordinate
     * @param[in] width     Rectangle width in pixel
     * @param[in] height    Rectangle height in pixel
     * @param[in] color     Color
     */
    void fillRect(int16_t x, int16_t y, int16_t width, int16_t height, uint16_t color)
    {
        int16_t relX = 0;
        int16_t relY = 0;

        for(relY = 0; relY < height; ++relY)
        {
            for(relX = 0; relX < width; ++relX)
            {
                drawPixel(x + relX, y + relY, color);
            }
        }

        return;
    }

    /**
     * Draw a PROGMEM-resident 16-bit image (RGB 5/6/5) at the specified (x,y) position.
     * For 16-bit display devices; no color reduction performed.
     * Note, method declaration is from Adafruit_GFX.
     * 
     * @param[in] x         Top left corner x-coordinate
     * @param[in] y         Top left corner y-coordinate
     * @param[in] bitmap    Byte array with 16-bit color bitmap
     * @param[in] width     Width of bitmap in pixels
     * @param[in] height    Height of bitmap in pixels
     */
    void drawRGBBitmap(int16_t x, int16_t y, const uint16_t bitmap[], int16_t width, int16_t height)
    {
        int16_t relX = 0;
        int16_t relY = 0;

        for(relY = 0; relY < height; ++relY)
        {
            for(relX = 0; relX < width; ++relX)
            {
                drawPixel(x + relX, y + relY, bitmap[relX + relY * width]);
            }
        }

        return;
    }

    /**
     * Set text cursor location.
     * Note, method declaration is from Adafruit_GFX.
     * 
     * @param[in] x x-coordinate in pixels.
     * @param[in] y y-coordinate in pixels.
     */
    void setCursor(int16_t x, int16_t y)
    {
        m_cursorX = x;
        m_cursorY = y;

        return;
    }

    /**
     * Set text font color with transparant background.
     * Note, method declaration is from Adafruit_GFX.
     * 
     * @param color 16-bit 5-6-5 Color to draw text with
     */
    void setTextColor(uint16_t color)
    {
        m_textColor     = color;
        m_textBgColor   = color;

        return;
    }

    /**
     * Print a string.
     * Note, method declaration is from Print class.
     */
    size_t print(const String &s)
    {
        /* Not supported yet. */
        return 0;
    }

    /**
     * Set font.
     * Note, method declaration is from Adafruit_GFX.
     * 
     * @param[in] font Font to set
     */
    void setFont(const GFXfont* font)
    {
        m_font = font;
        return;
    }

    /**
     * Get font.
     * 
     * @return Current selected font.
     */
    const GFXfont* getFont(void) const
    {
        return m_font;
    }

    /**
     * Set whether text that is too long for the screen width should
     * automatically wrap around to the next line (else clip right).
     * Note, method declaration is from Adafruit_GFX.
     * 
     * @param[in] wrapIt    true for wrapping, false for clipping.
     */
    void setTextWrap(boolean wrapIt)
    {
        /* Not supported yet. */
        return;
    }

    /**
     * Helper to determine size of a string with current font/size. Pass string and a cursor position, returns UL corner and W,H.
     * Note, method declaration is from Adafruit_GFX.
     * 
     * @param[in] str   The ascii string to measure (as an arduino String() class).
     * @param[in] x     The current cursor X.
     * @param[in] y     The current cursor Y.
     * @param[out] x1   The boundary X coordinate, set by function.
     * @param[out] y1   The boundary Y coordinate, set by function.
     * @param[out] w    The boundary width, set by function.
     * @param[out] h    The boundary height, set by function.
     */
    void getTextBounds(const String &str, int16_t x, int16_t y, int16_t *x1, int16_t *y1, uint16_t *w, uint16_t *h)
    {
        /* Not supported yet. */
        return;
    }

private:

    int16_t         m_width;        /**< Width in pixel */
    int16_t         m_height;       /**< Height in pixel */
    int16_t         m_cursorX;      /**< Cursor x-coordinate */
    int16_t         m_cursorY;      /**< Cursor y-coordinate */
    uint16_t        m_textColor;    /**< Text color */
    uint16_t        m_textBgColor;  /**< Text background color */
    const GFXfont*  m_font;         /**< Current selected font */

    Adafruit_GFX(const Adafruit_GFX& gfx);
    Adafruit_GFX& operator=(const Adafruit_GFX& gfx);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __ADAFRUIT_GFX_H__ */

/** @} */