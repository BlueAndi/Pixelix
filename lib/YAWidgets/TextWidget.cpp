/* MIT License
 *
 * Copyright (c) 2019 - 2021 Andreas Merkle <web@blue-andi.de>
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

#include <TomThumb.h>
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
const char*                 TextWidget::WIDGET_TYPE         = "text";

/* Initialize default font */
const GFXfont*              TextWidget::DEFAULT_FONT        = &TomThumb;

/* Initialize keyword list */
TextWidget::KeywordHandler  TextWidget::m_keywordHandlers[] =
{
    &TextWidget::handleColor,
    &TextWidget::handleAlignment
};

/* Set default scroll pause in ms. */
uint32_t                    TextWidget::m_scrollPause       = TextWidget::DEFAULT_SCROLL_PAUSE;

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void TextWidget::update(YAGfx& gfx)
{
    int16_t         cursorX         = m_posX;
    int16_t         cursorY         = m_posY + m_gfxText.getFont().getHeight() - 1; /* Set cursor to baseline */
    const uint16_t  SCROLL_DISTANCE = gfx.getWidth() / 2U; /* Distance in pixel after a scrolling text starts to repeat. */

    /* Text changed, check whether scrolling is necessary? */
    if (true == m_checkScrollingNeed)
    {
        uint16_t    textHeight      = 0U;
        String      str             = removeFormatTags(m_formatStr);

        if (true == m_gfxText.getTextBoundingBox(gfx.getWidth(), gfx.getHeight(), str.c_str(), m_textWidth, textHeight))
        {
            /* Text too long for the display? */
            if (gfx.getWidth() < m_textWidth)
            {
                m_isScrollingEnabled    = true;
                m_scrollOffset          = ((-1) * gfx.getWidth()) + 1;  /* The user can see the first characters better, if starting nearly outside the canvas. */
                m_scrollTimer.start(0U);                                /* Ensure immediate update */
            }
            else
            {
                m_isScrollingEnabled    = false;
                m_scrollOffset          = 0;
                m_scrollTimer.stop();
            }
        }

        m_checkScrollingNeed    = false;
        m_scrollingCnt          = 0U;
    }

    /* Move cursor to left side of the display. It may be set outside the canvas
     * in case the text is scrolling.
     */
    cursorX -= m_scrollOffset;
    m_gfxText.setTextCursorPos(cursorX, cursorY);

    /* Show text */
    show(gfx, m_formatStr);

    /* If text is scrolling, repeat the text after a defined distance. */
    if (true == m_isScrollingEnabled)
    {
        if (static_cast<int32_t>(gfx.getWidth() - SCROLL_DISTANCE) > static_cast<int32_t>(m_textWidth - m_scrollOffset))
        {
            m_gfxText.setTextCursorPos(m_textWidth - m_scrollOffset + SCROLL_DISTANCE, cursorY);
            show(gfx, m_formatStr);
        }
    }

    /* Shall we scroll again? */
    if (true == m_scrollTimer.isTimeout())
    {
        /* Text scrolls completly out, until it starts from the beginning again. */
        ++m_scrollOffset;

        /* Only count one complete scrolling text cycle. */
        if (static_cast<int32_t>(m_textWidth) < static_cast<int32_t>(m_scrollOffset))
        {
            /* Here we know that the text was once complete scrolled through the display. */
            ++m_scrollingCnt;
        }

        /* Reset scrolling offset at the right place, so the user won't see any difference
         * on the display.
         */
        if (static_cast<int32_t>(m_textWidth + SCROLL_DISTANCE - 1U) < static_cast<int32_t>(m_scrollOffset))
        {
            m_scrollOffset = 0;
        }

        m_scrollTimer.start(m_scrollPause);
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
    uint32_t    index       = 0U;
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
            uint32_t keywordIndex = 0U;

            for(keywordIndex = 0U; keywordIndex < UTIL_ARRAY_NUM(m_keywordHandlers); ++keywordIndex)
            {
                KeywordHandler  handler     = m_keywordHandlers[keywordIndex];
                uint8_t         overstep    = 0U;
                bool            status      = (this->*handler)(nullptr, nullptr, false, formatStr.substring(index), overstep);

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

void TextWidget::show(YAGfx& gfx, const String& formatStr)
{
    uint32_t    index           = 0U;
    bool        escapeFound     = false;
    bool        useChar         = false;
    uint32_t    length          = formatStr.length();
    Color       textColorBackup = m_gfxText.getTextColor();

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
            uint32_t keywordIndex = 0U;

            for(keywordIndex = 0U; keywordIndex < UTIL_ARRAY_NUM(m_keywordHandlers); ++keywordIndex)
            {
                KeywordHandler  handler     = m_keywordHandlers[keywordIndex];
                uint8_t         overstep    = 0U;
                bool            status      = (this->*handler)(&gfx, &m_gfxText, false, formatStr.substring(index), overstep);

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

            m_gfxText.drawChar(gfx, formatStr[index]);
            ++index;
        }
    }

    /* Text color might be changed, restore original. */
    m_gfxText.setTextColor(textColorBackup);

    return;
}

bool TextWidget::handleColor(YAGfx* gfx, YAGfxText* gfxText, bool noAction, const String& formatStr, uint8_t& overstep) const
{
    bool status = false;

    if ('#' == formatStr[0])
    {
        const uint8_t   RGB_HEX_LEN = 6U;
        String          colorStr    = String("0x") + formatStr.substring(1, 1 + RGB_HEX_LEN);
        uint32_t        colorRGB888 = 0U;
        bool            convStatus  = Util::strToUInt32(colorStr, colorRGB888);

        if (true == convStatus)
        {
            if ((false == noAction) &&
                (nullptr != gfx) &&
                (nullptr != gfxText))
            {
                gfxText->setTextColor(colorRGB888);
            }

            overstep    = 1U + RGB_HEX_LEN;
            status      = true;
        }
    }

    return status;
}

bool TextWidget::handleAlignment(YAGfx* gfx, YAGfxText* gfxText, bool noAction, const String& formatStr, uint8_t& overstep) const
{
    bool status                 = false;
    const uint8_t   KEYWORD_LEN = 6U;

    /* Alignment left? */
    if (true == formatStr.startsWith("lalign"))
    {
        overstep    = KEYWORD_LEN;
        status      = true;
    }
    /* Alignment right? */
    else if (true == formatStr.startsWith("ralign"))
    {
        if ((false == noAction) &&
            (nullptr != gfx) &&
            (nullptr != gfxText) &&
            (false == m_isScrollingEnabled))
        {
            String      text        = removeFormatTags(formatStr.substring(KEYWORD_LEN));
            uint16_t    textWidth   = 0U;
            uint16_t    textHeight  = 0U;

            if (true == gfxText->getTextBoundingBox(gfx->getWidth(), gfx->getHeight(), text.c_str(), textWidth, textHeight))
            {
                gfxText->setTextCursorPos(gfx->getWidth() - textWidth, gfxText->getTextCursorPosY());
            }
        }

        overstep    = KEYWORD_LEN;
        status      = true;
    }
    /* Alignment center? */
    else if (true == formatStr.startsWith("calign"))
    {
        if ((false == noAction) &&
            (nullptr != gfx) &&
            (nullptr != gfxText) &&
            (false == m_isScrollingEnabled))
        {
            String      text        = removeFormatTags(formatStr.substring(KEYWORD_LEN));
            uint16_t    textWidth   = 0U;
            uint16_t    textHeight  = 0U;

            if (true == gfxText->getTextBoundingBox(gfx->getWidth(), gfx->getHeight(), text.c_str(), textWidth, textHeight))
            {
                gfxText->setTextCursorPos(gfxText->getTextCursorPosX() + (gfx->getWidth() - gfxText->getTextCursorPosX() - textWidth) / 2, gfxText->getTextCursorPosY());
            }
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
