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
#include "TWTokenizer.h"

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
const char*     TextWidget::WIDGET_TYPE     = "text";

/* Initialize default font */
const YAFont&   TextWidget::DEFAULT_FONT    =  Fonts::getFontByType(Fonts::FONT_TYPE_DEFAULT);

/* Initialize keyword list 1: These keywords are only handled once at the begin, before printing the text. */
const TextWidget::FormatKeywordRow  TextWidget::FORMAT_KEYWORD_TABLE_1[] =
{
    {   "{vt}",     &TextWidget::verticalTopAligned         },
    {   "{vc}",     &TextWidget::verticalCenterAligned      },
    {   "{vb}",     &TextWidget::verticalBottomAligned      }
};

/* Initialize keyword list 2: These keywords are handled during printing the text. */
const TextWidget::FormatKeywordRow  TextWidget::FORMAT_KEYWORD_TABLE_2[] =
{
    {   "{hl}",     &TextWidget::horizontalLeftAligned      },
    {   "{hc}",     &TextWidget::horizontalCenterAligned    },
    {   "{hr}",     &TextWidget::horizontalRightAligned     },

    {   "{#*}",     &TextWidget::handleColor                },

    {   "{hm *}",   &TextWidget::horizontalMove             },
    {   "{vm *}",   &TextWidget::verticalMove               }
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
    m_ast(),
    m_astNew(),
    m_gfxText(DEFAULT_FONT, DEFAULT_TEXT_COLOR),
    m_scrollingCnt(0U),
    m_scrollOffset(0),
    m_scrollTimer(),
    m_hAlign(Alignment::Horizontal::HORIZONTAL_LEFT),
    m_vAlign(Alignment::Vertical::VERTICAL_TOP),
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
    m_formatStr(),
    m_formatStrNew(str),
    m_fadeState(FADE_STATE_IDLE),
    m_fadeBrightness(0U),
    m_isFadeEffectEnabled(true),
    m_scrollInfo(),
    m_scrollInfoNew(),
    m_prepareNewText(true),
    m_updateText(false),
    m_ast(),
    m_astNew(),
    m_gfxText(DEFAULT_FONT, DEFAULT_TEXT_COLOR),
    m_scrollingCnt(0U),
    m_scrollOffset(0),
    m_scrollTimer(),
    m_hAlign(Alignment::Horizontal::HORIZONTAL_LEFT),
    m_vAlign(Alignment::Vertical::VERTICAL_TOP),
    m_vAlignPosY(0)
{
    TWTokenizer tokenizer;

    /* Build AST for the new format string. */
    if (false == tokenizer.parse(m_astNew, m_formatStrNew))
    {
        m_formatStrNew.clear();
        m_prepareNewText = false;
    }

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
    m_ast(widget.m_ast),
    m_astNew(widget.m_astNew),
    m_gfxText(widget.m_gfxText),
    m_scrollingCnt(widget.m_scrollingCnt),
    m_scrollOffset(widget.m_scrollOffset),
    m_scrollTimer(widget.m_scrollTimer),
    m_hAlign(widget.m_hAlign),
    m_vAlign(widget.m_vAlign),
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
        m_ast                   = widget.m_ast;
        m_astNew                = widget.m_astNew;
        m_gfxText               = widget.m_gfxText;
        m_scrollingCnt          = widget.m_scrollingCnt;
        m_scrollOffset          = widget.m_scrollOffset;
        m_scrollTimer           = widget.m_scrollTimer;
        m_hAlign                = widget.m_hAlign;
        m_vAlign                = widget.m_vAlign;
        m_vAlignPosY            = widget.m_vAlignPosY;
    }

    return *this;
}

void TextWidget::setFormatStr(const String& formatStr)
{
    /* Avoid update if not necessary. */
    if (((m_formatStr != formatStr) && (false == m_prepareNewText)) ||
        ((m_formatStrNew != formatStr) && (true == m_prepareNewText)))
    {
        TWTokenizer             tokenizer;
        TWAbstractSyntaxTree    ast;

        if (false == tokenizer.parse(ast, formatStr))
        {
            LOG_WARNING("Text format is invalid at pos %u", tokenizer.getErrorIndex());
        }
        else
        {
            m_formatStrNew      = formatStr;
            m_prepareNewText    = true;
            m_astNew            = std::move(ast);

            /* Convert special character codes here to avoid that they need
             * later always special handling.
             */
            specialCharacterCodeKeywordToText(m_astNew);
        }
    }
}

void TextWidget::clear()
{
    m_formatStr.clear();
    m_scrollInfo.clear();

    m_formatStrNew.clear();
    m_scrollInfoNew.clear();

    m_prepareNewText = false;

    m_ast.clear();
    
    m_vAlignPosY = 0U;
}

String TextWidget::getStr() const
{
    String textOnly;

    if ((true == m_prepareNewText) ||
        (true == m_updateText))
    {
        getText(textOnly, m_astNew);
    }
    else
    {
        getText(textOnly, m_ast);
    }

    return textOnly;
}

bool TextWidget::getScrollInfo(bool& isScrollingEnabled, uint32_t& scrollingCnt)
{
    bool status = false;
    
    if (false == m_prepareNewText)
    {
        isScrollingEnabled  = m_scrollInfoNew.isEnabled;
        scrollingCnt        = m_scrollingCnt;
        status              = true;
    }

    return status;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

int16_t TextWidget::alignTextHorizontal(YAGfx& gfx, const String& text, Alignment::Horizontal hAlign) const
{
    int16_t xPos = 0;

    /* Horizontal alignment is supported for
     * - Static text
     * - Text scrolling from bottom to top
     */
    if ((false == m_scrollInfo.isEnabled) ||
        ((true == m_scrollInfo.isEnabled) && (false == m_scrollInfo.isScrollingToLeft)))
    {
        uint16_t textBoxWidth   = 0U;
        uint16_t textBoxHeight  = 0U;

        if (true == m_gfxText.getTextBoundingBox(gfx.getWidth(), text.c_str(), textBoxWidth, textBoxHeight))
        {
            UTIL_NOT_USED(textBoxHeight);

            switch(hAlign)
            {
            case Alignment::Horizontal::HORIZONTAL_LEFT:
                xPos = 0;
                break;

            case Alignment::Horizontal::HORIZONTAL_CENTER:
                xPos = (m_canvas.getWidth() - textBoxWidth) / 2;
                break;

            case Alignment::Horizontal::HORIZONTAL_RIGHT:
                xPos = m_canvas.getWidth() - textBoxWidth;
                break;

            default:
                break;
            }
        }
    }

    return xPos;
}

void TextWidget::alignTextVertical()
{
    /* Vertical alignment is supported for
     * - Static text
     * - Text scrolling from left to right
     */
    if ((false == m_scrollInfo.isEnabled) ||
        ((true == m_scrollInfo.isEnabled) && (true == m_scrollInfo.isScrollingToLeft)))
    {
        switch(m_vAlign)
        {
        case Alignment::Vertical::VERTICAL_TOP:
            m_vAlignPosY = 0;
            break;

        case Alignment::Vertical::VERTICAL_CENTER:
            m_vAlignPosY = (m_canvas.getHeight() - m_scrollInfo.textHeight) / 2;
            break;

        case Alignment::Vertical::VERTICAL_BOTTOM:
            m_vAlignPosY = m_canvas.getHeight() - m_scrollInfo.textHeight;
            break;
        
        default:
            m_vAlignPosY = 0;
            break;
        }
    }
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
    String      newStr;
    
    getText(newStr, m_astNew);

    /* Get bounding box of the new text, without any format tags. */
    if (true == m_gfxText.getTextBoundingBox(gfx.getWidth(), newStr.c_str(), textBoxWidth, textBoxHeight))
    {
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
            m_scrollInfoNew.offsetDest          = -textBoxWidth;
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
            m_scrollInfoNew.offsetDest          = -textBoxHeight;
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

    /* Consider vertical alignment only.
     * Horizontal alignment takes place line by line.
     */
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
        m_updateText    = false;
        m_formatStr     = m_formatStrNew;
        m_scrollInfo    = m_scrollInfoNew;
        m_ast           = std::move(m_astNew);

        alignTextVertical();

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
    show(gfx, m_ast, m_scrollInfo.isEnabled);

    /* Handle fade effect. */
    handleFadeEffect();

    /* Is it time to scroll the text(s) again? */
    if (true == m_scrollTimer.isTimeout())
    {
        scrollText(gfx);
        m_scrollTimer.restart();
    }
}

void TextWidget::specialCharacterCodeKeywordToText(TWAbstractSyntaxTree& ast)
{
    uint32_t    astLength   = ast.length();
    uint32_t    idx;

    for(idx = 0U; idx < astLength; ++idx)
    {
        TWToken& token = ast[idx];

        if (TWToken::TYPE_KEYWORD == token.getType())
        {
            if (true == isKeywordEqual("{0x*}", token.getStr().c_str()))
            {
                size_t  length      = token.getStr().length();
                String  charCodeStr = token.getStr().substring(1U, length - 1U); /* {0x*} */
                uint8_t charCode    = 0U;
                bool    convStatus  = Util::strToUInt8(charCodeStr, charCode);

                if (true == convStatus)
                {
                    token.setType(TWToken::TYPE_TEXT);
                    token.setStr(String(static_cast<char>(charCode)));
                }
            }
        }
    }
}

void TextWidget::getText(String& text, const TWAbstractSyntaxTree& ast) const
{
    uint32_t    length = ast.length();
    uint32_t    idx;

    text.clear();

    for(idx = 0U; idx < length; ++idx)
    {
        const TWToken& token = ast[idx];

        switch(token.getType())
        {
        case TWToken::TYPE_KEYWORD:
            /* Skip keyword token. */
            break;
        
        case TWToken::TYPE_TEXT:
            /* fallthrough */
        case TWToken::TYPE_LINE_FEED:
            text += token.getStr();
            break;

        default:
            break;
        }
    }
}

uint32_t TextWidget::getSingleLine(String& singleLine, const TWAbstractSyntaxTree& ast, uint32_t startIdx)
{
    uint32_t    length      = ast.length();
    uint32_t    idx         = startIdx;
    bool        isFinished  = false;

    singleLine.clear();

    while((length > idx) && (false == isFinished))
    {
        const TWToken& token = ast[idx];

        switch(token.getType())
        {
        case TWToken::TYPE_KEYWORD:
            /* Skip keyword token. */
            break;
        
        case TWToken::TYPE_TEXT:
            singleLine += token.getStr();
            break;

        case TWToken::TYPE_LINE_FEED:
            isFinished = true;
            break;

        default:
            break;
        }

        ++idx;
    }

    return idx;
}

void TextWidget::show(YAGfx& gfx, const TWAbstractSyntaxTree& ast, bool isScrolling)
{
    uint32_t                astLength       = ast.length();
    Color                   textColorBackup = m_gfxText.getTextColor(); /* Backup text color */
    Alignment::Horizontal   hAlignBackup    = m_hAlign;                 /* Backup alignment */
    Alignment::Vertical     vAlignBackup    = m_vAlign;                 /* Backup alignment */
    Alignment::Horizontal   hAlign          = m_hAlign;                 /* Used to detect horizontal alignment change */
    int16_t                 hAlignPosX      = 0;
    uint32_t                idx;
    String                  singleLine;

    /* First run handles only format tags, which influence the whole text. */
    for(idx = 0U; idx < astLength; ++idx)
    {
        const TWToken& token = ast[idx];

        if (TWToken::TYPE_KEYWORD == token.getType())
        {
            /* Handle only format tags, which influence the whole text. */
            (void)handleKeyword(gfx, FORMAT_KEYWORD_TABLE_1, UTIL_ARRAY_NUM(FORMAT_KEYWORD_TABLE_1), token.getStr().c_str());
        }
    }

    /* Calculate text cursor position and consider horizontal alignment. */
    (void)getSingleLine(singleLine, ast, 0U);
    hAlignPosX = alignTextHorizontal(gfx, singleLine, m_hAlign);
    m_gfxText.setTextCursorPosX(m_gfxText.getTextCursorPosX() + hAlignPosX);

    /* Second run, now showing text too. */
    for(idx = 0U; idx < astLength; ++idx)
    {
        const TWToken& token = ast[idx];

        switch(token.getType())
        {
        case TWToken::TYPE_KEYWORD:
            (void)handleKeyword(gfx, FORMAT_KEYWORD_TABLE_2, UTIL_ARRAY_NUM(FORMAT_KEYWORD_TABLE_2), token.getStr().c_str());
            break;
        
        case TWToken::TYPE_TEXT:
            /* Horizontal alignment might be changed by keywords. */
            if (hAlign != m_hAlign)
            {
                hAlignPosX = alignTextHorizontal(gfx, singleLine, m_hAlign);
                m_gfxText.setTextCursorPosX(m_gfxText.getTextCursorPosX() + hAlignPosX);

                hAlign = m_hAlign;
            }

            m_gfxText.drawText(gfx, token.getStr().c_str());
            break;

        case TWToken::TYPE_LINE_FEED:
            /* Set text cursor to next line. */
            m_gfxText.drawText(gfx, "\n");

            /* Calculate next text cursor x position and consider horizontal alignment. */
            (void)getSingleLine(singleLine, ast, idx + 1U);
            hAlignPosX = alignTextHorizontal(gfx, singleLine, m_hAlign);
            m_gfxText.setTextCursorPosX(m_gfxText.getTextCursorPosX() + hAlignPosX);
            break;

        default:
            break;
        }
    }

    /* Restore original in case it was changed by format keywords. */
    m_gfxText.setTextColor(textColorBackup);
    m_hAlign = hAlignBackup;
    m_vAlign = vAlignBackup;
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

    setHorizontalAlignment(Alignment::Horizontal::HORIZONTAL_LEFT);
}

void TextWidget::horizontalCenterAligned(YAGfx& gfx, const String& keyword)
{
    UTIL_NOT_USED(gfx);
    UTIL_NOT_USED(keyword);

    setHorizontalAlignment(Alignment::Horizontal::HORIZONTAL_CENTER);
}

void TextWidget::horizontalRightAligned(YAGfx& gfx, const String& keyword)
{
    UTIL_NOT_USED(gfx);
    UTIL_NOT_USED(keyword);

    setHorizontalAlignment(Alignment::Horizontal::HORIZONTAL_RIGHT);
}

void TextWidget::verticalTopAligned(YAGfx& gfx, const String& keyword)
{
    UTIL_NOT_USED(gfx);
    UTIL_NOT_USED(keyword);

    setVerticalAlignment(Alignment::Vertical::VERTICAL_TOP);
}

void TextWidget::verticalCenterAligned(YAGfx& gfx, const String& keyword)
{
    UTIL_NOT_USED(gfx);
    UTIL_NOT_USED(keyword);

    setVerticalAlignment(Alignment::Vertical::VERTICAL_CENTER);
}

void TextWidget::verticalBottomAligned(YAGfx& gfx, const String& keyword)
{
    UTIL_NOT_USED(gfx);
    UTIL_NOT_USED(keyword);

    setVerticalAlignment(Alignment::Vertical::VERTICAL_BOTTOM);
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
