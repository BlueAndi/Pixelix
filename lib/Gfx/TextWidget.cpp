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
 * @brief  Text Widget
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "TextWidget.h"

#include <Fonts/TomThumb.h>
#include <Util.h>

#include <Logging.h>

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
const char*                 TextWidget::WIDGET_TYPE     = "text";

/* Initialize default font */
const GFXfont*              TextWidget::DEFAULT_FONT    = &TomThumb;

/* Initialize keyword list */
TextWidget::KeywordHandler  TextWidget::m_keywordHandlers[]    =
{
    &TextWidget::handleColor,
    &TextWidget::handleAlignment
};

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void TextWidget::update(IGfx& gfx)
{
    int16_t cursorX = m_posX;
    int16_t cursorY = m_posY + m_font->yAdvance - 1; /* Set cursor to baseline */

    /* Set base parameters */
    gfx.setFont(m_font);
    gfx.setTextColor(m_textColor.to565());
    gfx.setTextWrap(false); /* If text is too long, don't wrap around. */

    /* Text changed, check whether scrolling is necessary? */
    if (true == m_checkScrollingNeed)
    {
        int16_t     boundaryX       = 0;
        int16_t     boundaryY       = 0;
        uint16_t    textHeight      = 0u;
        String      str             = removeFormatTags(m_formatStr);

        gfx.getTextBounds(str, 0, 0, &boundaryX, &boundaryY, &m_textWidth, &textHeight);

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

    /* Move cursor to right position */
    cursorX -= m_scrollOffset;
    gfx.setCursor(cursorX, cursorY);

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
    bool        useChar     = false;
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
                useChar     = true;
            }
            else
            {
                escapeFound = true;
                ++index;
            }
        }
        else if (true == escapeFound)
        {
            uint8_t keywordIndex = 0u;

            for(keywordIndex = 0u; keywordIndex < UTIL_ARRAY_NUM(m_keywordHandlers); ++keywordIndex)
            {
                KeywordHandler  handler     = m_keywordHandlers[keywordIndex];
                uint8_t         overstep    = 0u;
                bool            status      = (this->*handler)(NULL, false, formatStr.substring(index), overstep);

                if (true == status)
                {
                    index += overstep;
                    break;
                }
            }

            if (UTIL_ARRAY_NUM(m_keywordHandlers) <= keywordIndex)
            {
                useChar = true;
            }

            escapeFound = false;
        }
        else
        {
            useChar = true;
        }

        if (true == useChar)
        {
            useChar = false;
            str += formatStr[index];
            ++index;
        }
    }

    return str;
}

void TextWidget::show(IGfx& gfx, const String& formatStr) const
{
    uint32_t    index       = 0u;
    bool        escapeFound = false;
    bool        useChar     = false;
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
                useChar     = true;
            }
            else
            {
                escapeFound = true;
                ++index;
            }
        }
        else if (true == escapeFound)
        {
            uint8_t keywordIndex = 0u;

            for(keywordIndex = 0u; keywordIndex < UTIL_ARRAY_NUM(m_keywordHandlers); ++keywordIndex)
            {
                KeywordHandler  handler     = m_keywordHandlers[keywordIndex];
                uint8_t         overstep    = 0u;
                bool            status      = (this->*handler)(&gfx, false, formatStr.substring(index), overstep);

                if (true == status)
                {
                    index += overstep;
                    break;
                }
            }

            if (UTIL_ARRAY_NUM(m_keywordHandlers) <= keywordIndex)
            {
                useChar = true;
            }

            escapeFound = false;
        }
        else
        {
            useChar = true;
        }

        if (true == useChar)
        {
            useChar = false;

            gfx.print(formatStr[index]);
            ++index;
        }
    }

    /* Text color might be changed, restore original. */
    gfx.setTextColor(m_textColor.to565());

    return;
}

bool TextWidget::handleColor(IGfx* gfx, bool noAction, const String& formatStr, uint8_t& overstep) const
{
    bool status = false;

    if ('#' == formatStr[0])
    {
        const uint8_t   RGB_HEX_LEN = 6u;
        String          colorStr    = String("0x") + formatStr.substring(1, 1 + RGB_HEX_LEN);
        uint32_t        colorRGB888;
        bool            convStatus  = Util::strToUInt32(colorStr, colorRGB888);

        if (true == convStatus)
        {
            if ((false == noAction) &&
                (NULL != gfx))
            {
                Color textColor(colorRGB888);
                gfx->setTextColor(textColor.to565());
            }

            overstep    = 1u + RGB_HEX_LEN;
            status      = true;
        }
    }

    return status;
}

bool TextWidget::handleAlignment(IGfx* gfx, bool noAction, const String& formatStr, uint8_t& overstep) const
{
    bool status                 = false;
    const uint8_t   KEYWORD_LEN = 6u;

    /* Alignment left? */
    if (true == formatStr.startsWith("lalign"))
    {
        overstep    = KEYWORD_LEN;
        status      = true;
    }
    /* Alignment right? */
    else if (true == formatStr.startsWith("ralign"))
    {
        String      text        = removeFormatTags(formatStr.substring(KEYWORD_LEN));
        int16_t     boundaryX   = 0;
        int16_t     boundaryY   = 0;
        uint16_t    textWidth   = 0u;
        uint16_t    textHeight  = 0u;

        if ((false == noAction) &&
            (NULL != gfx))
        {
            gfx->getTextBounds(text, 0, 0, &boundaryX, &boundaryY, &textWidth, &textHeight);
            gfx->setCursor(gfx->width() - textWidth, gfx->getCursorY());
        }

        overstep    = KEYWORD_LEN;
        status      = true;
    }
    /* Alignment center? */
    else if (true == formatStr.startsWith("calign"))
    {
        String      text        = removeFormatTags(formatStr.substring(KEYWORD_LEN));
        int16_t     boundaryX   = 0;
        int16_t     boundaryY   = 0;
        uint16_t    textWidth   = 0u;
        uint16_t    textHeight  = 0u;

        if ((false == noAction) &&
            (NULL != gfx))
        {
            gfx->getTextBounds(text, 0, 0, &boundaryX, &boundaryY, &textWidth, &textHeight);
            gfx->setCursor(gfx->getCursorX() + (gfx->width() - gfx->getCursorX() - textWidth) / 2, gfx->getCursorY());
        }

        overstep    = KEYWORD_LEN;
        status      = true;
    }
    else
    {
        ;
    }

    return status;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
