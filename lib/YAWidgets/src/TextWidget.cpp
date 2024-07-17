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
const char*     TextWidget::WIDGET_TYPE     = "text";

/* Initialize default font */
const YAFont&   TextWidget::DEFAULT_FONT    =  Fonts::getFontByType(Fonts::FONT_TYPE_DEFAULT);

/* Initialize keyword list 1 */
const TextWidget::FormatKeywordRow  TextWidget::FORMAT_KEYWORD_TABLE_1[] =
{
    "{hl}",     &TextWidget::horizontalLeftAligned,
    "{hc}",     &TextWidget::horizontalCenterAligned,
    "{hr}",     &TextWidget::horizontalRightAligned,

    "{vt}",     &TextWidget::verticalTopAligned,
    "{vc}",     &TextWidget::verticalCenterAligned,
    "{vb}",     &TextWidget::verticalBottomAligned
};

/* Initialize keyword list 2 */
const TextWidget::FormatKeywordRow  TextWidget::FORMAT_KEYWORD_TABLE_2[] =
{
    "{#*}",     &TextWidget::handleColor,
    "{0x*}",    &TextWidget::handleCharCode,

    "{hm *}",   &TextWidget::horizontalMove,
    "{vm *}",   &TextWidget::verticalMove
};

/* Set default scroll pause in ms. */
uint32_t        TextWidget::m_scrollPause   = TextWidget::DEFAULT_SCROLL_PAUSE;

/******************************************************************************
 * Public Methods
 *****************************************************************************/

TextWidget::TextWidget(uint16_t width, uint16_t height, int16_t x, int16_t y) :
    Widget(WIDGET_TYPE, width, height, x, y),
    m_formatStr(),
    m_formatStrNew(),
    m_fadeState(FADE_STATE_IDLE),
    m_fadeBrightness(0U),
    m_isFadeEffectEnabled(true),
    m_scrollInfo(),
    m_scrollInfoNew(),
    m_prepareNewText(false),
    m_updateText(false),
    m_gfxText(DEFAULT_FONT, DEFAULT_TEXT_COLOR),
    m_scrollingCnt(0U),
    m_scrollOffset(0),
    m_scrollTimer(),
    m_hAlign(Alignment::Horizontal::HORIZONTAL_LEFT),
    m_vAlign(Alignment::Vertical::VERTICAL_TOP),
    m_hAlignPosX(0),
    m_vAlignPosY(0)
{
    /* Enable text wrap for multi-line text widget. */
    if (1U < getLineCount())
    {
        m_gfxText.setTextWrap(true);
    }
}

TextWidget::TextWidget(const String& str, const Color& color) :
    Widget(WIDGET_TYPE, 0U, 0U, 0, 0),
    m_formatStr(str),
    m_formatStrNew(str),
    m_fadeState(FADE_STATE_IDLE),
    m_fadeBrightness(0U),
    m_isFadeEffectEnabled(true),
    m_scrollInfo(),
    m_scrollInfoNew(),
    m_prepareNewText(false),
    m_updateText(false),
    m_gfxText(DEFAULT_FONT, DEFAULT_TEXT_COLOR),
    m_scrollingCnt(0U),
    m_scrollOffset(0),
    m_scrollTimer(),
    m_hAlign(Alignment::Horizontal::HORIZONTAL_LEFT),
    m_vAlign(Alignment::Vertical::VERTICAL_TOP),
    m_hAlignPosX(0),
    m_vAlignPosY(0)
{
    /* Enable text wrap for multi-line text widget. */
    if (1U < getLineCount())
    {
        m_gfxText.setTextWrap(true);
    }
}

TextWidget::TextWidget(const TextWidget& widget) :
    Widget(widget),
    m_formatStr(widget.m_formatStr),
    m_formatStrNew(widget.m_formatStrNew),
    m_fadeState(widget.m_fadeState),
    m_fadeBrightness(widget.m_fadeBrightness),
    m_isFadeEffectEnabled(widget.m_isFadeEffectEnabled),
    m_scrollInfo(widget.m_scrollInfo),
    m_scrollInfoNew(widget.m_scrollInfoNew),
    m_prepareNewText(widget.m_prepareNewText),
    m_updateText(widget.m_updateText),
    m_gfxText(widget.m_gfxText),
    m_scrollingCnt(widget.m_scrollingCnt),
    m_scrollOffset(widget.m_scrollOffset),
    m_scrollTimer(widget.m_scrollTimer),
    m_hAlign(widget.m_hAlign),
    m_vAlign(widget.m_vAlign),
    m_hAlignPosX(widget.m_hAlignPosX),
    m_vAlignPosY(widget.m_vAlignPosY)
{
}

TextWidget& TextWidget::operator=(const TextWidget& widget)
{
    if (&widget != this)
    {
        Widget::operator=(widget);
        
        m_formatStr             = widget.m_formatStr;
        m_formatStrNew          = widget.m_formatStrNew;
        m_fadeState             = widget.m_fadeState;
        m_fadeBrightness        = widget.m_fadeBrightness;
        m_isFadeEffectEnabled   = widget.m_isFadeEffectEnabled;
        m_scrollInfo            = widget.m_scrollInfo;
        m_scrollInfoNew         = widget.m_scrollInfoNew;
        m_prepareNewText        = widget.m_prepareNewText;
        m_updateText            = widget.m_updateText;
        m_gfxText               = widget.m_gfxText;
        m_scrollingCnt          = widget.m_scrollingCnt;
        m_scrollOffset          = widget.m_scrollOffset;
        m_scrollTimer           = widget.m_scrollTimer;
        m_hAlign                = widget.m_hAlign;
        m_vAlign                = widget.m_vAlign;
        m_hAlignPosX            = widget.m_hAlignPosX;
        m_vAlignPosY            = widget.m_vAlignPosY;
    }

    return *this;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void TextWidget::alignText(Alignment::Horizontal hAlign, Alignment::Vertical vAlign)
{
    m_hAlignPosX    = alignTextHorizontal(hAlign);
    m_vAlignPosY    = alignTextVertical(vAlign);
}

int16_t TextWidget::alignTextHorizontal(Alignment::Horizontal hAlign) const
{
    int16_t xPos = 0;

    /* Horizontal alignment is supported for
     * - Static text
     * - Text scrolling from bottom to top
     */
    if ((false == m_scrollInfo.isEnabled) ||
        ((true == m_scrollInfo.isEnabled) && (false == m_scrollInfo.isScrollingToLeft)))
    {
        switch(hAlign)
        {
        case Alignment::Horizontal::HORIZONTAL_LEFT:
            xPos = 0;
            break;

        case Alignment::Horizontal::HORIZONTAL_CENTER:
            xPos = (m_canvas.getWidth() - m_scrollInfo.textWidth) / 2;
            break;

        case Alignment::Horizontal::HORIZONTAL_RIGHT:
            xPos = m_canvas.getWidth() - m_scrollInfo.textWidth;
            break;

        default:
            break;
        }
    }

    return xPos;
}

int16_t TextWidget::alignTextVertical(Alignment::Vertical vAlign) const
{
    int16_t yPos = 0;

    /* Vertical alignment is supported for
     * - Static text
     * - Text scrolling from left to right
     */
    if ((false == m_scrollInfo.isEnabled) ||
        ((true == m_scrollInfo.isEnabled) && (true == m_scrollInfo.isScrollingToLeft)))
    {
        switch(vAlign)
        {
        case Alignment::Vertical::VERTICAL_TOP:
            yPos = 0;
            break;

        case Alignment::Vertical::VERTICAL_CENTER:
            yPos = (m_canvas.getHeight() - m_scrollInfo.textHeight) / 2;
            break;

        case Alignment::Vertical::VERTICAL_BOTTOM:
            yPos = m_canvas.getHeight() - m_scrollInfo.textHeight;
            break;
        
        default:
            break;
        }
    }

    return yPos;
}

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

        /* Fade effect disabled? */
        if (false == m_isFadeEffectEnabled)
        {
            m_fadeState         = FADE_STATE_IDLE;
            m_fadeBrightness    = FADING_BRIGHTNESS_HIGH;
            m_updateText        = true;
        }
        /* No fading active? */
        else if (FADE_STATE_IDLE == m_fadeState)
        {
            /* If no text is shown, fade in immediately. */
            if (true == m_formatStr.isEmpty())
            {
                m_fadeState         = FADE_STATE_IN;
                m_fadeBrightness    = FADING_BRIGHTNESS_LOW;
                m_updateText        = true;
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
            /* Just continue to fade out. */
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
            m_scrollInfoNew.offset              = gfx.getHeight() + m_gfxText.getFont().getHeight() - 1; /* Set cursor to baseline */
        }
    }
}

void TextWidget::calculateCursorPos(int16_t& curX, int16_t& curY) const
{
    /* No scrolling? */
    if (false == m_scrollInfo.isEnabled)
    {
        curX = 0;
        curY = m_gfxText.getFont().getHeight() - 1; /* Set cursor to baseline */
    }
    /* Scrolling from left to right? */
    else if (true == m_scrollInfo.isScrollingToLeft)
    {
        curX = m_scrollInfo.offset;
        curY = m_gfxText.getFont().getHeight() - 1; /* Set cursor to baseline */
    }
    /* Scrolling from bottom to top. */
    else
    {
        curX = 0;
        curY = m_scrollInfo.offset;
    }

    /* Consider alignment */
    curX += m_hAlignPosX;
    curY += m_vAlignPosY;
}

void TextWidget::handleFadeEffect()
{
    switch(m_fadeState)
    {
    case FADE_STATE_IDLE:
        /* Nothing to do. */
        break;

    case FADE_STATE_OUT:
        handleFadeOut();

        /* If text is faded out, replace it with the new text and start fading in. */
        if (FADING_BRIGHTNESS_LOW == m_fadeBrightness)
        {
            m_fadeState     = FADE_STATE_IN;
            m_updateText    = true;
        }
        break;

    case FADE_STATE_IN:
        handleFadeIn();

        /* If text is faded in, the fading effect is finished. */
        if (FADING_BRIGHTNESS_HIGH == m_fadeBrightness)
        {
            m_fadeState = FADE_STATE_IDLE;
        }
        break;

    default:
        break;
    }
}

void TextWidget::handleFadeOut()
{
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
    }
}

void TextWidget::handleFadeIn()
{
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
    }
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
    Color   textColor   = m_gfxText.getTextColor();
    int16_t cursorX     = 0;
    int16_t cursorY     = 0;
    
    /* If there is a new text available, it shall be determined how to show it on the display. */
    if (true == m_prepareNewText)
    {
        prepareNewText(gfx);
        m_prepareNewText = false;
    }

    /* Update of current text requested? */
    if (true == m_updateText)
    {
        m_formatStr     = m_formatStrNew;
        m_scrollInfo    = m_scrollInfoNew;
        m_updateText    = false;

        alignText(m_hAlign, m_vAlign);

        if (true == m_scrollInfo.isEnabled)
        {
            m_scrollTimer.start(m_scrollPause);
        }
    }

    /* Update text brightness, even if fade effect is disabled. */
    textColor.setIntensity(m_fadeBrightness);
    m_gfxText.setTextColor(textColor);

    /* Update the cursor position, it may have changed by scrolling. */
    calculateCursorPos(cursorX, cursorY);
    m_gfxText.setTextCursorPos(cursorX, cursorY);

    /* Show the text. */
    show(gfx, m_formatStr, m_scrollInfo.isEnabled);

    /* Handle fade effect. */
    handleFadeEffect();

    /* Is it time to scroll the text(s) again? */
    if (true == m_scrollTimer.isTimeout())
    {
        scrollText(gfx);
        m_scrollTimer.restart();
    }
}

String TextWidget::removeFormatTags(const String& formatStr) const
{
    String      str;
    uint32_t    length      = formatStr.length();
    size_t      beginIdx    = 0U;

    while(length > beginIdx)
    {
        String  resultStr;
        size_t  remainingIdx    = 0U;
        Result  result = parseFormattedText(formatStr, beginIdx, resultStr, remainingIdx);

        if (RESULT_TEXT == result)
        {
            str += resultStr;
        }
        else if (RESULT_KEYWORD == result)
        {
            /* Character code keyword is a exception, the character itself must
             * be added to the string.
             */
            if (true == isKeywordEqual("{0x*}", resultStr.c_str()))
            {
                size_t  length      = resultStr.length();
                String  charCodeStr = resultStr.substring(1U, length - 1U); /* {0x*} */
                uint8_t charCode    = 0U;
                bool    convStatus  = Util::strToUInt8(charCodeStr, charCode);

                if (true == convStatus)
                {
                    str += static_cast<char>(charCode);
                }
            }
        }
        else
        {
            ;
        }

        beginIdx = remainingIdx;
    }

    return str;
}

void TextWidget::show(YAGfx& gfx, const String& formatStr, bool isScrolling)
{
    uint32_t                length          = formatStr.length();
    Color                   textColorBackup = m_gfxText.getTextColor();
    Alignment::Horizontal   hAlignBackup    = m_hAlign;
    Alignment::Vertical     vAlignBackup    = m_vAlign;
    size_t                  beginIdx;

    /* First run handles only format tags, which influence the whole text. */
    beginIdx = 0U;
    while(length > beginIdx)
    {
        String  resultStr;
        size_t  remainingIdx    = 0U;
        Result  result = parseFormattedText(formatStr, beginIdx, resultStr, remainingIdx);

        if (RESULT_KEYWORD == result)
        {
            /* Handle only format tags, which influence the whole text. */
            (void)handleKeyword(gfx, FORMAT_KEYWORD_TABLE_1, UTIL_ARRAY_NUM(FORMAT_KEYWORD_TABLE_1), resultStr.c_str());
        }

        beginIdx = remainingIdx;
    }

    /* Second run, now showing text too. */
    beginIdx = 0U;
    while(length > beginIdx)
    {
        String  resultStr;
        size_t  remainingIdx    = 0U;
        Result  result = parseFormattedText(formatStr, beginIdx, resultStr, remainingIdx);

        switch(result)
        {
        case RESULT_EMPTY:
            break;
        
        case RESULT_TEXT:
            m_gfxText.drawText(gfx, resultStr.c_str());
            break;

        case RESULT_KEYWORD:
            (void)handleKeyword(gfx, FORMAT_KEYWORD_TABLE_2, UTIL_ARRAY_NUM(FORMAT_KEYWORD_TABLE_2), resultStr.c_str());
            break;

        default:
            break;
        }

        beginIdx = remainingIdx;
    }

    /* Restore original in case it was changed by format keywords. */
    m_gfxText.setTextColor(textColorBackup);
    alignText(hAlignBackup, vAlignBackup);
}

TextWidget::Result TextWidget::parseFormattedText(const String& str, size_t& beginIdx, String& resultStr, size_t& remainingIdx) const
{
    Result  result          = RESULT_EMPTY;
    size_t  strLength       = str.length();
    size_t  idx             = beginIdx;
    size_t  lastEscapeIdx   = beginIdx;
    bool    isTagBeginFound = false;
    bool    isEscapeActive  = false;

    /* As soon as we find a keyword or just text, we will inform the caller. */
    while((strLength > idx) && (RESULT_EMPTY == result))
    {
        /* Begin of keyword found?
         * It can only happen at the begin of the string, because otherwise
         * there is text in front of the keyword.
         */
        if (('{' == str[idx]) &&
            (false == isEscapeActive))
        {
            isTagBeginFound = true;

            if (beginIdx < idx)
            {
                /* Inform user about text, which is in front of the keyword. */
                result = RESULT_TEXT;

                /* Begin of keyword shall be parsed next time. */
                --idx;
            }

        }
        /* End of keyword found?
         * If the keyword begin is missing, it will be ignored.
         */
        else if (('}' == str[idx]) &&
                 (false == isEscapeActive) &&
                 (true == isTagBeginFound))
        {
            result = RESULT_KEYWORD;
        }
        /* Escape found? */
        else if ('\\' == str[idx])
        {
            if (false == isEscapeActive)
            {
                isEscapeActive = true;
            }
            else
            {
                isEscapeActive = false;
            }
        }
        /* If a escape is active, the next character will disable it. */
        else if (true == isEscapeActive)
        {
            isEscapeActive = false;

            /* The escape character shall not be inside the result string. */
            resultStr += str.substring(lastEscapeIdx, idx);
            lastEscapeIdx = idx;
        }
        else
        {
            /* Nothing to do. */
        }

        ++idx;
    }

    remainingIdx = idx;

    /* Really empty? */
    if (RESULT_EMPTY == result)
    {
        /* Any text found? */
        if (beginIdx != remainingIdx)
        {
            result = RESULT_TEXT;
        }
    }

    if (RESULT_EMPTY != result)
    {
        resultStr = str.substring(lastEscapeIdx, remainingIdx);
    }

    return result;

}

bool TextWidget::isKeywordEqual(const char* keyword, const char* other) const
{
    bool    isEqual     = true;
    bool    isWildcard  = false;
    size_t  tagIdx      = 0U;
    size_t  otherIdx    = 0U;

    while(('\0' != keyword[tagIdx]) && ('\0' != other[otherIdx]) && (true == isEqual) && (false == isWildcard))
    {
        /* Not equal? */
        if (keyword[tagIdx] != other[otherIdx])
        {
            /* If its a wildcard, its fine. */
            if ('*' == keyword[tagIdx])
            {
                isWildcard = true;
            }
            else
            {
                isEqual = false;
            }
        }
        else
        {
            ++tagIdx;
            ++otherIdx;
        }
    }

    return isEqual;
}

bool TextWidget::handleKeyword(YAGfx& gfx, const FormatKeywordRow* table, size_t tableSize, const String& keyword)
{
    bool    isFound = false;
    size_t  idx     = 0U;

    while((tableSize > idx) && (false == isFound))
    {
        const FormatKeywordRow* row = &table[idx];

        if (true == isKeywordEqual(row->keyword, keyword.c_str()))
        {
            KeywordHandler handler = row->handler;

            (this->*handler)(gfx, keyword);
            isFound = true;
        }

        ++idx;
    };

    return isFound;
}

void TextWidget::horizontalLeftAligned(YAGfx& gfx, const String& keyword)
{
    UTIL_NOT_USED(gfx);
    UTIL_NOT_USED(keyword);

    m_hAlign        = Alignment::Horizontal::HORIZONTAL_LEFT;
    m_hAlignPosX    = alignTextHorizontal(m_hAlign);
}

void TextWidget::horizontalCenterAligned(YAGfx& gfx, const String& keyword)
{
    UTIL_NOT_USED(gfx);
    UTIL_NOT_USED(keyword);

    m_hAlign        = Alignment::Horizontal::HORIZONTAL_CENTER;
    m_hAlignPosX    = alignTextHorizontal(m_hAlign);
}

void TextWidget::horizontalRightAligned(YAGfx& gfx, const String& keyword)
{
    UTIL_NOT_USED(gfx);
    UTIL_NOT_USED(keyword);

    m_hAlign        = Alignment::Horizontal::HORIZONTAL_RIGHT;
    m_hAlignPosX    = alignTextHorizontal(m_hAlign);
}

void TextWidget::verticalTopAligned(YAGfx& gfx, const String& keyword)
{
    UTIL_NOT_USED(gfx);
    UTIL_NOT_USED(keyword);

    m_vAlign        = Alignment::Vertical::VERTICAL_TOP;
    m_vAlignPosY    = alignTextVertical(m_vAlign);
}

void TextWidget::verticalCenterAligned(YAGfx& gfx, const String& keyword)
{
    UTIL_NOT_USED(gfx);
    UTIL_NOT_USED(keyword);

    m_vAlign        = Alignment::Vertical::VERTICAL_CENTER;
    m_vAlignPosY    = alignTextVertical(m_vAlign);

}

void TextWidget::verticalBottomAligned(YAGfx& gfx, const String& keyword)
{
    UTIL_NOT_USED(gfx);
    UTIL_NOT_USED(keyword);

    m_vAlign        = Alignment::Vertical::VERTICAL_BOTTOM;
    m_vAlignPosY    = alignTextVertical(m_vAlign);

}

void TextWidget::handleColor(YAGfx& gfx, const String& keyword)
{
    size_t          length      = keyword.length();
    String          colorStr    = String("0x") + keyword.substring(2U, length - 1U); /* {#*} */
    uint32_t        colorRGB888 = 0U;
    bool            convStatus  = Util::strToUInt32(colorStr, colorRGB888);

    UTIL_NOT_USED(gfx);

    if (true == convStatus)
    {
        Color textColor = colorRGB888;

        textColor.setIntensity(m_fadeBrightness);
        m_gfxText.setTextColor(colorRGB888);
    }
}

void TextWidget::handleCharCode(YAGfx& gfx, const String& keyword)
{
    size_t          length      = keyword.length();
    String          charCodeStr = keyword.substring(1U, length - 1U); /* {0x*} */
    uint8_t         charCode    = 0U;
    bool            convStatus  = Util::strToUInt8(charCodeStr, charCode);

    if (true == convStatus)
    {
        m_gfxText.drawChar(gfx, static_cast<char>(charCode));
    }
}

void TextWidget::horizontalMove(YAGfx& gfx, const String& keyword)
{
    size_t  length      = keyword.length();
    String  strOffset   = keyword.substring(4U, length - 1U); /* {hm *} */
    int32_t offset      = 0;
    bool    convStatus  = Util::strToInt32(strOffset, offset);

    if ((true == convStatus) &&
        (INT16_MAX >= offset) &&
        (INT16_MIN <= offset))
    {
        int16_t x = m_gfxText.getTextCursorPosX() + offset;
        int16_t y = m_gfxText.getTextCursorPosY();

        m_gfxText.setTextCursorPos(x, y);
    }
}

void TextWidget::verticalMove(YAGfx& gfx, const String& keyword)
{
    size_t  length      = keyword.length();
    String  strOffset   = keyword.substring(4U, length - 1U); /* {vm *} */
    int32_t offset      = 0;
    bool    convStatus  = Util::strToInt32(strOffset, offset);

    if ((true == convStatus) &&
        (INT16_MAX >= offset) &&
        (INT16_MIN <= offset))
    {
        int16_t x = m_gfxText.getTextCursorPosX();
        int16_t y = m_gfxText.getTextCursorPosY() + offset;

        m_gfxText.setTextCursorPos(x, y);
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
