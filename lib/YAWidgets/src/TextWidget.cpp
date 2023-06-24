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
 * @brief  Text Widget
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "TextWidget.h"

#include <Fonts.h>
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
const YAFont&               TextWidget::DEFAULT_FONT        = Fonts::getFontByType(Fonts::FONT_TYPE_DEFAULT);

/* Initialize keyword list */
TextWidget::KeywordHandler  TextWidget::m_keywordHandlers[] =
{
    &TextWidget::handleColor,
    &TextWidget::handleAlignment,
    &TextWidget::handleCharCode
};

/* Set default scroll pause in ms. */
uint32_t                    TextWidget::m_scrollPause       = TextWidget::DEFAULT_SCROLL_PAUSE;

/******************************************************************************
 * Public Methods
 *****************************************************************************/

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void TextWidget::prepareNewText(YAGfx& gfx)
{
    const uint16_t  SCROLL_DISTANCE = gfx.getWidth() / 2U; /* Distance in pixel after a scrolling text starts to repeat. */
    uint16_t        textWidth       = 0U;
    uint16_t        textHeight      = 0U;
    String          str             = removeFormatTags(m_formatStrNew);

    /* Get bounding box of the text, without any format tags. */
    if (true == m_gfxText.getTextBoundingBox(gfx.getWidth(), str.c_str(), textWidth, textHeight))
    {
        m_scrollInfoNew.textWidth   = textWidth;
        m_handleNewText             = true;

        /* Can new text be static shown or must it be scrolled? */
        if (gfx.getWidth() >= m_scrollInfoNew.textWidth)
        {
            /* Static */
            m_scrollInfoNew.isEnabled = false;
        }
        else
        {
            /* Scrolling */
            m_scrollInfoNew.isEnabled = true;
        }

        /* Handle the following scenarios:
         * +==============+==============+
         * | Current text | New text     |
         * +==============+==============+
         * | Static       | Static       | --> Show new text immediately, no scrolling.
         * +--------------+--------------+
         * | Static       | Scrolling    | --> Scroll current text out and new one in.
         * +--------------+--------------+
         * | Scrolling    | Static       | --> Scroll current text out, new one in and stop scrolling at the end.
         * +--------------+--------------+
         * | Scrolling    | Scrolling    | --> Continue scrolling and just scroll new one in.
         * +--------------+--------------+
         */

        /* Is current text static shown? */
        if (false == m_scrollInfo.isEnabled)
        {
            /* Can new text be static shown?
             * If yes, it will jump immediately in.
             */
            if (false == m_scrollInfoNew.isEnabled)
            {
                /* New text is kept static. */
                m_scrollInfoNew.stopAtDest  = false;
                m_scrollInfoNew.offsetDest  = 0;
                m_scrollInfoNew.offset      = 0;

                /* Immediate take over. */
                m_formatStr     = m_formatStrNew;
                m_scrollInfo    = m_scrollInfoNew;
                m_handleNewText = false;
            }
            else
            /* New text will be scrolling, starting outside the display. */
            {
                /* The current text shall scroll out. */
                m_scrollInfo.isEnabled  = true;
                m_scrollInfo.offsetDest = -m_scrollInfo.textWidth;
                m_scrollInfo.offset     = 0;

                /* The next text shall scroll in. */
                m_scrollInfoNew.stopAtDest  = false;
                m_scrollInfoNew.offsetDest  = 0;
                m_scrollInfoNew.offset      = m_scrollInfo.offset + m_scrollInfo.textWidth + SCROLL_DISTANCE;

                /* Avoid that the new text is jumping in, instead of scrolling in. */
                if (gfx.getWidth() > m_scrollInfoNew.offset)
                {
                    m_scrollInfoNew.offset = gfx.getWidth();
                }

                /* Because the scroll timer is stopped, it must be enabled again. */
                m_scrollTimer.start(0U);
            }
        }
        /* Current text is scrolling. */
        else
        {
            /* Can new text be static shown? */
            if (false == m_scrollInfoNew.isEnabled)
            {
                /* New text will be scrolling in and then static shown.
                 * If the current text is near the end, the new text will start outside the display.
                 */
                m_scrollInfoNew.isEnabled   = true;
                m_scrollInfoNew.stopAtDest  = true;
                m_scrollInfoNew.offsetDest  = 0;
                m_scrollInfoNew.offset      = m_scrollInfo.offset + m_scrollInfo.textWidth + SCROLL_DISTANCE;

                /* Avoid that the new text is jumping in, instead of scrolling in. */
                if (gfx.getWidth() > m_scrollInfoNew.offset)
                {
                    m_scrollInfoNew.offset = gfx.getWidth();
                }
            }
            else
            /* New text will be scrolling, starting right after current scrolling text.
             * If the current text is near the end, the new text will start outside the display.
             */
            {
                m_scrollInfoNew.stopAtDest  = false;
                m_scrollInfoNew.offsetDest  = 0;
                m_scrollInfoNew.offset      = m_scrollInfo.offset + m_scrollInfo.textWidth + SCROLL_DISTANCE;

                /* Avoid that the new text is jumping in, instead of scrolling in. */
                if (gfx.getWidth() > m_scrollInfoNew.offset)
                {
                    m_scrollInfoNew.offset = gfx.getWidth();
                }
            }
        }
    }
}

void TextWidget::paint(YAGfx& gfx)
{
    int16_t cursorY = m_posY + m_gfxText.getFont().getHeight() - 1; /* Set cursor to baseline */
    
    /* If there is an updated text available, it shall be determined how to show it on the display. */
    if (true == m_isNewTextAvailable)
    {
        prepareNewText(gfx);
        m_isNewTextAvailable = false;
    }

    /* Show current text. */
    m_gfxText.setTextCursorPos(m_posX + m_scrollInfo.offset, cursorY);
    show(gfx, m_formatStr, m_scrollInfo.isEnabled);

    /* Show new text. */
    if (true == m_handleNewText)
    {
        m_gfxText.setTextCursorPos(m_posX + m_scrollInfoNew.offset, cursorY);
        show(gfx, m_formatStrNew, m_scrollInfoNew.isEnabled);
    }

    /* Is it time to scroll the text(s) again? */
    if (true == m_scrollTimer.isTimeout())
    {
        /* Handle scrolling text. */
        if (m_scrollInfo.offsetDest < m_scrollInfo.offset)
        {
            --m_scrollInfo.offset;
        }
        else if (m_scrollInfo.offsetDest > m_scrollInfo.offset)
        {
            ++m_scrollInfo.offset;
        }
        else if (false == m_handleNewText)
        {
            m_scrollInfo.offset = gfx.getWidth();
            
            ++m_scrollingCnt;
        }
        else
        {
            /* Wait till new text is at destination position. */
            ;
        }

        /* Handle scrolling new text. */
        if (true == m_handleNewText)
        {
            if (m_scrollInfoNew.offsetDest < m_scrollInfoNew.offset)
            {
                --m_scrollInfoNew.offset;
            }
            else if (m_scrollInfoNew.offsetDest > m_scrollInfoNew.offset)
            {
                ++m_scrollInfoNew.offset;
            }
            else
            {
                m_handleNewText = false;
                m_formatStr     = m_formatStrNew;
                m_scrollingCnt  = 0U;

                /* Any additional new format string available? */
                if (false == m_formatStrTmp.isEmpty())
                {
                    m_formatStrNew          = m_formatStrTmp;
                    m_isNewTextAvailable    = true;

                    m_formatStrTmp.clear();
                }

                /* If the new text can be shown static, it must be stopped scrolling  now. */
                if (true == m_scrollInfoNew.stopAtDest)
                {
                    m_scrollInfoNew.isEnabled   = false;
                    m_scrollInfoNew.stopAtDest  = false;
                }

                /* Show new text static? */
                if (false == m_scrollInfoNew.isEnabled)
                {
                    m_scrollInfo.isEnabled  = false;
                    m_scrollInfo.stopAtDest = false;
                    m_scrollInfo.offsetDest = 0;
                    m_scrollInfo.offset     = 0;
                    m_scrollInfo.textWidth  = m_scrollInfoNew.textWidth;
                }
                else
                /* Continue scrolling with new text. */
                {
                    m_scrollInfo.isEnabled  = true;
                    m_scrollInfo.stopAtDest = false;
                    m_scrollInfo.offsetDest = -m_scrollInfoNew.textWidth;
                    m_scrollInfo.offset     = m_scrollInfoNew.offset - 1;   /* Because new text is already at most left position, decrease one pixel to avoid a short stumble. */
                    m_scrollInfo.textWidth  = m_scrollInfoNew.textWidth;
                }
            }
        }

        if (false == m_scrollInfo.isEnabled)
        {
            m_scrollTimer.stop();
        }
        else
        {
            m_scrollTimer.start(m_scrollPause);
        }
    }
}

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
                bool            status      = (this->*handler)(nullptr, nullptr, false, formatStr.substring(index), false, overstep);

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

void TextWidget::show(YAGfx& gfx, const String& formatStr, bool isScrolling)
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
                bool            status      = (this->*handler)(&gfx, &m_gfxText, false, formatStr.substring(index), isScrolling, overstep);

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
}

bool TextWidget::handleColor(YAGfx* gfx, YAGfxText* gfxText, bool noAction, const String& formatStr, bool isScrolling, uint8_t& overstep) const
{
    bool status = false;

    UTIL_NOT_USED(isScrolling);

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

bool TextWidget::handleAlignment(YAGfx* gfx, YAGfxText* gfxText, bool noAction, const String& formatStr, bool isScrolling, uint8_t& overstep) const
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
            (false == isScrolling))
        {
            String      text        = removeFormatTags(formatStr.substring(KEYWORD_LEN));
            uint16_t    textWidth   = 0U;
            uint16_t    textHeight  = 0U;

            if (true == gfxText->getTextBoundingBox(gfx->getWidth(), text.c_str(), textWidth, textHeight))
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
            (false == isScrolling))
        {
            String      text        = removeFormatTags(formatStr.substring(KEYWORD_LEN));
            uint16_t    textWidth   = 0U;
            uint16_t    textHeight  = 0U;

            if (true == gfxText->getTextBoundingBox(gfx->getWidth(), text.c_str(), textWidth, textHeight))
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

bool TextWidget::handleCharCode(YAGfx* gfx, YAGfxText* gfxText, bool noAction, const String& formatStr, bool isScrolling, uint8_t& overstep) const
{
    bool status = false;

    UTIL_NOT_USED(isScrolling);

    if (('x' == formatStr[0]) ||
        ('X' == formatStr[0]))
    {
        const uint8_t   CHAR_CODE_LEN   = 2U;
        String          charCodeStr     = String("0x") + formatStr.substring(1, 1 + CHAR_CODE_LEN);
        uint8_t         charCode        = 0U;
        bool            convStatus      = Util::strToUInt8(charCodeStr, charCode);

        if (true == convStatus)
        {
            if ((false == noAction) &&
                (nullptr != gfx) &&
                (nullptr != gfxText))
            {
                gfxText->drawChar(*gfx, static_cast<char>(charCode));
            }

            overstep    = 1U + CHAR_CODE_LEN;
            status      = true;
        }
    }

    return status;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
