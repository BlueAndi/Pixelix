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
 * @brief  Text Widget
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "TextWidget.h"

#include <Fonts.h>
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

uint16_t TextWidget::getLineCount() const
{
    uint16_t lineCount = getHeight() / m_gfxText.getFont().getHeight();

    if (0U == lineCount)
    {
        lineCount = 1U;
    }

    return lineCount;
}

bool TextWidget::isStaticText(YAGfx& gfx, uint16_t textBoxWidth, uint16_t textBoxHeight) const
{
    bool isStatic = false;

    /* Single line text widget? */
    if (1U >= getLineCount())
    {
        /* As long as the text box is smaller than the available width, 
         * the text can be static shown (no scrolling).
         */
        if (gfx.getWidth() >= textBoxWidth)
        {
            isStatic = true;
        }
    }
    /* Multi-line text widget */
    else
    {
        /* As long as the text box is smaller than the available height, 
         * the text can be static shown (no scrolling).
         */
        if (gfx.getHeight() >= textBoxHeight)
        {
            isStatic = true;
        }
    }

    return isStatic;
}

void TextWidget::prepareNewText(YAGfx& gfx)
{
    uint16_t    textBoxWidth    = 0U;
    uint16_t    textBoxHeight   = 0U;
    String      newStr          = removeFormatTags(m_formatStrNew);

    /* Get bounding box of the new text, without any format tags. */
    if (true == m_gfxText.getTextBoundingBox(gfx.getWidth(), newStr.c_str(), textBoxWidth, textBoxHeight))
    {
        m_scrollInfoNew.textWidth   = textBoxWidth;
        m_scrollInfoNew.textHeight  = textBoxHeight;

        /* Stop current scrolling. */
        m_scrollTimer.stop();
        m_scrollingCnt = 0U;

        /* Scenarios:
         *
         * - Independed of number of text widget lines:
         *     - Old text is faded out.
         *     - Static text is shown after faded in.
         * 
         * - Single line text widget:
         *     - Scrolling direction is from left to right.
         *     - Start scrolling outside of canvas.
         * 
         * - Multi-line text widget:
         *     - Scrolling direction is from bottom to top.
         *     - Start scrolling outside of canvas.
         */

        /* No fading active? */
        if (FADE_STATE_IDLE == m_fadeState)
        {
            /* If no text is shown, fade in immediately. */
            if (true == m_formatStr.isEmpty())
            {
                m_fadeState         = FADE_STATE_IN;
                m_fadeBrightness    = FADING_BRIGHTNESS_LOW;
            }
            else
            {
                m_fadeState         = FADE_STATE_OUT;
                m_fadeBrightness    = FADING_BRIGHTNESS_HIGH;
            }
        }
        /* Fading in active? */
        else if (FADE_STATE_IN == m_fadeState)
        {
            /* Take over fade brightness. */
            m_fadeState = FADE_STATE_OUT;
        }
        /* Fading out active. */
        else
        {
            /* Just continoue to fade out. */
        }

        /* Can new text be static (no scrolling) shown? */
        if (true == isStaticText(gfx, textBoxWidth, textBoxHeight))
        {
            /* New text is kept static. */
            m_scrollInfoNew.isEnabled   = false;
            m_scrollInfoNew.offsetDest  = 0;
            m_scrollInfoNew.offset      = 0;
        }
        /* If single line text widget, the current text will be scrolled
         * to the left and the new text scrolled in from right.
         */
        else if (1U == getLineCount())
        {
            /* The new text shall scroll in. */
            m_scrollInfoNew.isEnabled           = true;
            m_scrollInfoNew.isScrollingToLeft   = true;
            m_scrollInfoNew.offsetDest          = -m_scrollInfoNew.textWidth;
            m_scrollInfoNew.offset              = gfx.getWidth();
        }
        /* In multi-line text widget the current text will be scrolled
         * to the top and the new text scrolled in from bottom.
         */
        else
        {
            /* The new text shall scroll in. */
            m_scrollInfoNew.isEnabled           = true;
            m_scrollInfoNew.isScrollingToLeft   = false;
            m_scrollInfoNew.offsetDest          = -m_scrollInfoNew.textHeight;
            m_scrollInfoNew.offset              = gfx.getHeight();
        }
    }
}

void TextWidget::calculateCursorPos(int16_t& curX, int16_t& curY) const
{
    if (true == m_scrollInfo.isScrollingToLeft)
    {
        curX = m_scrollInfo.offset;
        curY = m_gfxText.getFont().getHeight() - 1; /* Set cursor to baseline */
    }
    else
    {
        curX = 0;
        curY = m_scrollInfo.offset;
    }
}

void TextWidget::handleFadeIdle(YAGfx& gfx)
{
    int16_t cursorX = 0;
    int16_t cursorY = 0;
    
    calculateCursorPos(cursorX, cursorY);

    /* Show current text. */
    m_gfxText.setTextCursorPos(cursorX, cursorY);
    show(gfx, m_formatStr, m_scrollInfo.isEnabled);
}

void TextWidget::handleFadeOut(YAGfx& gfx)
{
    Color   textColor   = m_gfxText.getTextColor();
    int16_t cursorX     = 0;
    int16_t cursorY     = 0;
    
    calculateCursorPos(cursorX, cursorY);

    if (FADING_BRIGHTNESS_LOW < m_fadeBrightness)
    {
        if (FADING_BRIGHTNESS_DELTA <= m_fadeBrightness)
        {
            m_fadeBrightness -= FADING_BRIGHTNESS_DELTA;
        }
        else
        {
            m_fadeBrightness = FADING_BRIGHTNESS_LOW;
        }

        textColor.setIntensity(m_fadeBrightness);
        m_gfxText.setTextColor(textColor);
    }
    
    /* Show current text. */
    m_gfxText.setTextCursorPos(cursorX, cursorY);
    show(gfx, m_formatStr, m_scrollInfo.isEnabled);
}

void TextWidget::handleFadeIn(YAGfx& gfx)
{
    Color   textColor   = m_gfxText.getTextColor();
    int16_t cursorX     = 0;
    int16_t cursorY     = 0;
    
    calculateCursorPos(cursorX, cursorY);

    if (FADING_BRIGHTNESS_HIGH > m_fadeBrightness)
    {
        if ((FADING_BRIGHTNESS_HIGH - FADING_BRIGHTNESS_DELTA) >= m_fadeBrightness)
        {
            m_fadeBrightness += FADING_BRIGHTNESS_DELTA;
        }
        else
        {
            m_fadeBrightness = FADING_BRIGHTNESS_HIGH;
        }

        textColor.setIntensity(m_fadeBrightness);
        m_gfxText.setTextColor(textColor);
    }
    
    /* Show current text. */
    m_gfxText.setTextCursorPos(cursorX, cursorY);
    show(gfx, m_formatStr, m_scrollInfo.isEnabled);
}

void TextWidget::scrollText(YAGfx& gfx)
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
    else
    {
        /* Left to right scrolling. */
        if (true == m_scrollInfo.isScrollingToLeft)
        {
            m_scrollInfo.offset = gfx.getWidth();
        }
        /* Bottom to top scrolling? */
        else
        {
            m_scrollInfo.offset = gfx.getHeight();
        }

        /* Count number of times the text was scrolled complete. */
        if (UINT32_MAX > m_scrollingCnt)
        {
            ++m_scrollingCnt;
        }
    }
}

void TextWidget::paint(YAGfx& gfx)
{
    int16_t cursorY = m_gfxText.getFont().getHeight() - 1; /* Set cursor to baseline */
    
    /* If there is an updated text available, it shall be determined how to show it on the display. */
    if (true == m_isNewTextAvailable)
    {
        prepareNewText(gfx);
        m_isNewTextAvailable = false;
    }

    switch(m_fadeState)
    {
    case FADE_STATE_IDLE:
        handleFadeIdle(gfx);
        break;

    case FADE_STATE_OUT:
        handleFadeOut(gfx);

        if (FADING_BRIGHTNESS_LOW == m_fadeBrightness)
        {
            m_fadeState = FADE_STATE_IN;
        }
        break;

    case FADE_STATE_IN:
        /* Take new string over? */
        if (FADING_BRIGHTNESS_LOW == m_fadeBrightness)
        {
            m_formatStr     = m_formatStrNew;
            m_scrollInfo    = m_scrollInfoNew;

            if (true == m_scrollInfo.isEnabled)
            {
                m_scrollTimer.start(m_scrollPause);
            }
        }

        handleFadeIn(gfx);

        if (FADING_BRIGHTNESS_HIGH == m_fadeBrightness)
        {
            m_fadeState = FADE_STATE_IDLE;
        }
        break;

    default:
        break;
    }

    /* Is it time to scroll the text(s) again? */
    if (true == m_scrollTimer.isTimeout())
    {
        scrollText(gfx);
        m_scrollTimer.restart();
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
                Color textColor = colorRGB888;

                textColor.setIntensity(m_fadeBrightness);
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
