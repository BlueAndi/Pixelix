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
 * @brief  Base font
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup gfx
 *
 * @{
 */

#ifndef BASE_FONT_HPP
#define BASE_FONT_HPP

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include "BaseGfx.hpp"
#include "gfxfont.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * A graphical font, providing simple single character drawing functionality.
 */
template < typename TColor >
class BaseFont
{
public:

    /**
     * Constructs a font, without graphical representation.
     * Note, until no GFXfont is assigned, it can not draw any character.
     */
    BaseFont() :
        m_gfxFont(nullptr)
    {
    }

    /**
     * Constructs a font by copy.
     * 
     * @param[in] font  Font, which to copy.
     */
    BaseFont(const BaseFont& font) :
        m_gfxFont(font.m_gfxFont)
    {
    }

    /**
     * Constructs a font with the given GFXfont.
     * 
     * @param[in] gfxFont   GFXfont
     */
    BaseFont(const GFXfont* gfxFont) :
        m_gfxFont(gfxFont)
    {
    }

    /**
     * Destroys the font.
     */
    ~BaseFont()
    {
    }

    /**
     * Assign a font.
     * 
     * @param[in] font  Font which to assign
     * 
     * @return The font itself.
     */
    BaseFont& operator=(const BaseFont& font)
    {
        if (&font != this)
        {
            m_gfxFont = font.m_gfxFont;
        }

        return *this;
    }

    /**
     * Get GFXfont.
     *
     * @return GFXfont
     */
    const GFXfont* getGfxFont() const
    {
        return m_gfxFont;
    }

    /**
     * Set GFXfont.
     *
     * @param[in] gfxFont   GFXfont
     */
    void setGfxFont(const GFXfont* gfxFont)
    {
        m_gfxFont = gfxFont;
    }

    /**
     * Get font character height.
     * If no GFXfont is set, it will return 0.
     * 
     * @return Height in pixels
     */
    uint16_t getHeight() const
    {
        uint16_t height = 0U;

        if (nullptr != m_gfxFont)
        {
            height = m_gfxFont->yAdvance;
        }

        return height;
    }

    /**
     * Get a glyph object from the font for the choosen character.
     * 
     * @param[in] singleChar    Character for what the glyph is requested.
     * 
     * @return If glyph is found, it will be returned otherwise nullptr.
     */
    const GFXglyph* getGlyph(char singleChar) const
    {
        const GFXglyph* glyph   = nullptr;
        uint8_t uChar           = static_cast<uint8_t>(singleChar);

        if ((nullptr != m_gfxFont) &&
            (m_gfxFont->first <= uChar) &&
            (m_gfxFont->last >= uChar) &&
            ('\n' != singleChar) &&
            ('\r' != singleChar))
        {
            uint8_t glyphIndex  = uChar - m_gfxFont->first;

            glyph = &(m_gfxFont->glyph[glyphIndex]);
        }

        return glyph;
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
        bool            status  = false;
        const GFXglyph* glyph   = getGlyph(singleChar);

        if (nullptr != glyph)
        {
            width   = glyph->xAdvance;
            height  = m_gfxFont->yAdvance;
            status  = true;
        }

        return status;
    }

    /**
     * Draw single character at current cursor position. The cursor is
     * automatically moved to the new position.
     * 
     * A newline will place the cursor on the begin of the next line.
     * 
     * If text wrap around handling is necessary, this must be done in a
     * higher layer.
     *
     * @param[in]       gfx         Graphics interface
     * @param[in,out]   cursorX     The cursor position x-coordinate.
     * @param[in,out]   cursorY     The cursor position y-coordinate.
     * @param[in]       singleChar  Single character which to draw
     * @param[in]       color       Text color
     */
    void drawChar(BaseGfx<TColor>& gfx, int16_t& cursorX, int16_t& cursorY, char singleChar, const TColor& color)
    {
        uint8_t uChar = static_cast<uint8_t>(singleChar);

        if (nullptr == m_gfxFont)
        {
            return;
        }

        /* Set cursor to next line? */
        if ('\n' == singleChar)
        {
            /* Move cursor to begin and one row down. */
            cursorX = 0;
            cursorY += m_gfxFont->yAdvance;
        }
        else
        {
            const GFXglyph* glyph = getGlyph(singleChar);

            /* Is character available in the font? Note, carriage return is skipped. */
            if (nullptr != glyph)
            {
                /* Handle character only, if it is really drawn on the screen. */
                if (0 <= (cursorX + glyph->xAdvance))
                {
                    int16_t     x               = 0;
                    int16_t     y               = 0;
                    uint16_t    bitmapOffset    = glyph->bitmapOffset;
                    uint8_t     bitmapRowBits   = 0U;
                    uint8_t     bitCnt          = 0U;

                    for(y = 0U; y < glyph->height; ++y)
                    {
                        for(x = 0U; x < glyph->width; ++x)
                        {
                            /* Every 8 bit, the bitmap offset must be increased. */
                            if (0U == (bitCnt & 0x07))
                            {
                                bitmapRowBits = m_gfxFont->bitmap[bitmapOffset];
                                ++bitmapOffset;
                            }
                            ++bitCnt;

                            /* A 1b in the bitmap row bits must be drawn as single pixel. */
                            if (0U != (bitmapRowBits & 0x80U))
                            {
                                gfx.drawPixel(cursorX + x + glyph->xOffset, cursorY + y + glyph->yOffset, color);
                            }

                            bitmapRowBits <<= 1U;
                        }
                    }
                }

                cursorX += glyph->xAdvance;
            }
            else
            /* Skip character */
            {
                ;
            }
        }
    }

private:

    const GFXfont*  m_gfxFont;  /**< Current selected graphics font, based on Adafruit GFXfont format. */

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* BASE_FONT_HPP */

/** @} */