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
 * @brief  Text Widget
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "TextWidget.h"

#include <Fonts/TomThumb.h>
#include <Util.h>

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and classes
 *****************************************************************************/

/******************************************************************************
 * Prototypes
 *****************************************************************************/

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/* Initialize text widget type. */
const char*     TextWidget::WIDGET_TYPE     = "text";

/* Initialize default font */
const GFXfont*  TextWidget::DEFAULT_FONT    = &TomThumb;

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void TextWidget::update(Adafruit_GFX& gfx)
{
    const int16_t   CURSOR_X    = m_posX - m_scrollOffset;
    const int16_t   CURSOR_Y    = m_posY + m_font->yAdvance - 1; /* Set cursor to baseline */

    /* Set base parameters */
    gfx.setFont(m_font);
    gfx.setTextColor(m_textColor.to565());
    gfx.setCursor(CURSOR_X, CURSOR_Y);
    gfx.setTextWrap(false); /* If text is too long, don't wrap around. */

    /* Check for scrolling necessary? */
    if (true == m_checkScrollingNeed)
    {
        int16_t     boundaryX       = 0;
        int16_t     boundaryY       = 0;
        uint16_t    textHeight      = 0u;
        String      str             = removeFormatTags(m_formatStr);

        gfx.getTextBounds(str, CURSOR_X, CURSOR_Y, &boundaryX, &boundaryY, &m_textWidth, &textHeight);

        /* Text too long for the display? */
        if (gfx.width() < m_textWidth)
        {
            m_isScrollingEnabled    = true;
            m_scrollOffset          = ((-1) * gfx.width()) + 1; /* The user can see the first characters better, if starting nearly outside the canvas. */
            m_scrollTimer.start(0u);                            /* Ensure immediate update */
        }
        else
        {
            m_isScrollingEnabled    = false;
            m_scrollOffset          = 0;
            m_scrollTimer.stop();
        }

        m_checkScrollingNeed = false;
    }

    /* Show text */
    show(gfx, m_formatStr);

    /* Shall we scroll again? */
    if (true == m_scrollTimer.isTimeout())
    {
        /* Text scrolls completly out, until it starts from the beginning again. */
        ++m_scrollOffset;
        if (m_textWidth < m_scrollOffset)
        {
            m_scrollOffset = ((-1) * gfx.width()) + 1; /* The user can see the first characters better, if starting nearly outside the canvas. */
        }

        m_scrollTimer.start(DEFAULT_SCROLL_PAUSE);
    }

    return;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

String TextWidget::removeFormatTags(const String& formatStr) const
{
    uint32_t    index       = 0u;
    bool        escapeFound = false;
    bool        showChar    = false;
    String      str;
    uint32_t    length      = formatStr.length();

    while(length > index)
    {
        /* Escape found? */
        if ('\\' == formatStr[index])
        {
            /* Another escape found? */
            if (true == escapeFound)
            {
                escapeFound = false;
                showChar    = true;
            }
            else
            {
                escapeFound = true;
            }
        }
        /* Begin of a color tag? */
        else if ('#' == formatStr[index])
        {
            /* Escaped? */
            if (true == escapeFound)
            {
                escapeFound = false;
                showChar    = true;
            }
            else if (length <= (index + 6u))
            {
                showChar = true;
            }
            else
            {
                String      colorStr    = String("0x") + formatStr.substring(index + 1, index + 1 + 6);
                uint32_t    colorRGB888;
                bool        convStatus  = Util::strToUInt32(colorStr, colorRGB888);

                if (false == convStatus)
                {
                    showChar = true;
                }
                else
                {
                    /* Overstep only color information. The '#' will be overstepped at the end of the loop. */
                    index += 6u;
                }
            }
        }
        else
        {
            showChar = true;
        }

        if (true == showChar)
        {
            showChar = false;

            str += formatStr[index];
        }

        ++index;
    }

    return str;
}

void TextWidget::show(Adafruit_GFX& gfx, const String& formatStr) const
{
    uint32_t    index       = 0u;
    bool        escapeFound = false;
    bool        showChar    = false;
    uint32_t    length      = formatStr.length();

    while(length > index)
    {
        /* Escape found? */
        if ('\\' == formatStr[index])
        {
            /* Another escape found? */
            if (true == escapeFound)
            {
                escapeFound = false;
                showChar    = true;
            }
            else
            {
                escapeFound = true;
            }
        }
        /* Begin of a color tag? */
        else if ('#' == formatStr[index])
        {
            /* Escaped? */
            if (true == escapeFound)
            {
                escapeFound = false;
                showChar    = true;
            }
            else if (length <= (index + 6u))
            {
                showChar = true;
            }
            else
            {
                String      colorStr    = String("0x") + formatStr.substring(index + 1, index + 1 + 6);
                uint32_t    colorRGB888;
                bool        convStatus  = Util::strToUInt32(colorStr, colorRGB888);

                if (false == convStatus)
                {
                    showChar = true;
                }
                else
                {
                    Color textColor(colorRGB888);
                    gfx.setTextColor(textColor.to565());

                    /* Overstep only color information. The '#' will be overstepped at the end of the loop. */
                    index += 6u;
                }
            }
        }
        else
        {
            showChar = true;
        }

        if (true == showChar)
        {
            showChar = false;

            gfx.print(formatStr[index]);
        }

        ++index;
    }

    gfx.setTextColor(m_textColor.to565());

    return;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
