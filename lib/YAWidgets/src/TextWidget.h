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
 *
 * @addtogroup gfx
 *
 * @{
 */

#ifndef TEXTWIDGET_H
#define TEXTWIDGET_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <WString.h>
#include <Widget.hpp>
#include <YAColor.h>
#include <YAFont.h>
#include <YAGfxText.h>
#include <SimpleTimer.hpp>
#include "Alignment.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * A text widget, showing a colored string.
 * The text has a given color, which can be changed.
 *
 * Different keywords in the string are supported, e.g. for coloring or alignment.
 * Each keyword is inside curly braces, otherwise its treated as just text.
 * Example: "{#FF0000}H{#FFFFFF}ello!" contains a red "H" and a white "ello!".
 *
 * Keywords:
 * - "{#RRGGBB}": Change text color; RRGGBB in hex
 * - "{hl}" : Horizontal alignment left
 * - "{hc}" : Horizontal alignment center
 * - "{hr}" : Horizontal alignment right
 * - "{vt}" : Vertical alignment top
 * - "{vc}" : Vertical alignment center
 * - "{vb}" : Vertical alignment bottom
 * - "{xAA}" : Special character hex code
 */
class TextWidget : public Widget
{
public:

    /**
     * Constructs a text widget with a empty string in default color.
     * 
     * @param[in] width     Widget width in pixel.
     * @param[in] height    Widget height in pixel.
     * @param[in] x         Upper left corner (x-coordinate) of the widget in a canvas.
     * @param[in] y         Upper left corner (y-coordinate) of the widget in a canvas.
     */
    TextWidget(uint16_t width = 0U, uint16_t height = 0U, int16_t x = 0, int16_t y = 0);

    /**
     * Constructs a text widget with the given string and its color.
     * If there is no color given, it will be default color.
     * 
     * The width and height is set to the parent canvas.
     *
     * @param[in] str   String, which may contain format tags.
     * @param[in] color Color of the string
     */
    TextWidget(const String& str, const Color& color = DEFAULT_TEXT_COLOR);

    /**
     * Constructs a text widget by copying another one.
     *
     * @param[in] widget Widget, which to copy
     */
    TextWidget(const TextWidget& widget);

    /**
     * Destroys the text widget.
     */
    ~TextWidget()
    {
    }

    /**
     * Assign the content of a text widget.
     *
     * @param[in] widget Widget, which to assign
     */
    TextWidget& operator=(const TextWidget& widget);

    /**
     * Set widget width.
     * 
     * @param[in] width Width in pixel
     */
    void setWidth(uint16_t width) override
    {
        Widget::setWidth(width);
        alignText(m_hAlign, m_vAlign);
    }

    /**
     * Set widget height.
     * 
     * @param[in] height Height in pixel
     */
    void setHeight(uint16_t height) override
    {
        Widget::setHeight(height);
        alignText(m_hAlign, m_vAlign);
    }

    /**
     * Set the text string. It can contain format tags like:
     * - "#RRGGBB" Color information in RGB888 format
     * 
     * @param[in] formatStr String, which may contain format tags
     */
    void setFormatStr(const String& formatStr)
    {
        /* Avoid update if not necessary. */
        if (((m_formatStr != formatStr) && (false == m_isNewTextAvailable)) ||
            ((m_formatStrNew != formatStr) && (true == m_isNewTextAvailable)))
        {
            m_formatStrNew          = formatStr;
            m_isNewTextAvailable    = true;
        }
    }

    /**
     * Clear immediately and don't show anything further.
     */
    void clear()
    {
        m_formatStr.clear();
        m_scrollInfo.clear();

        m_formatStrNew.clear();
        m_scrollInfoNew.clear();

        m_isNewTextAvailable = false;

        /* Reset calculated alignment coordinates. */
        m_hAlignPosX    = 0U;
        m_vAlignPosY    = 0U;
    }

    /**
     * Get the text string, which may contain format tags.
     *
     * @return String, which may contain format tags.
     */
    String getFormatStr() const
    {
        return m_formatStrNew;
    }

    /**
     * Get the text string, without format tags.
     *
     * @return String
     */
    String getStr() const
    {
        return removeFormatTags(m_formatStrNew);
    }

    /**
     * Set the text color of the string.
     *
     * @param[in] color Text color
     */
    void setTextColor(const Color& color)
    {
        m_gfxText.setTextColor(color);
    }

    /**
     * Get the text color of the string.
     *
     * @return Text color
     */
    Color getTextColor() const
    {
        return m_gfxText.getTextColor();
    }

    /**
     * Set font.
     *
     * @param[in] font  New font to set
     */
    void setFont(const YAFont& font)
    {
        m_gfxText.setFont(font);
        m_isNewTextAvailable = true;
    }

    /**
     * Get font.
     *
     * @return Font
     */
    YAFont& getFont()
    {
        return m_gfxText.getFont();
    }

    /**
     * Change scroll speed of all text widgets by changing the pause between each movement.
     *
     * @param[in] pause Scroll pause in ms
     *
     * @return If successful set, it will return true otherwise false.
     */
    static bool setScrollPause(uint32_t pause)
    {
        bool status = false;

        if ((MIN_SCROLL_PAUSE <= pause) &&
            (MAX_SCROLL_PAUSE >= pause))
        {
            m_scrollPause = pause;
            status = true;
        }

        return status;
    }

    /**
     * Get scrolling informations of latest set text.
     * If a text is just set, it needs one cycle to have the scroll information ready.
     *
     * @param[out] isScrollingEnabled   Is scrolling enabled or not?
     * @param[out] scrollingCnt         How often was the text complete scrolled over the display?
     *
     * @return If scroll information is ready, it will return true otherwise false.
     */
    bool getScrollInfo(bool& isScrollingEnabled, uint32_t& scrollingCnt)
    {
        bool status = false;
        
        if (false == m_isNewTextAvailable)
        {
            isScrollingEnabled  = m_scrollInfoNew.isEnabled;
            scrollingCnt        = m_scrollingCnt;
            status              = true;
        }

        return status;
    }

    /**
     * Set the horizontal alignment.
     * 
     * @param[in] align The horizontal aligment.
     */
    void setHorizontalAlignment(Alignment::Horizontal align)
    {
        m_hAlign        = align;
        m_hAlignPosX    = alignTextHorizontal(m_hAlign);
    }

    /**
     * Set the vertical alignment.
     * 
     * @param[in] align The vertical aligment.
     */
    void setVerticalAlignment(Alignment::Vertical align)
    {
        m_vAlign        = align;
        m_vAlignPosY    = alignTextVertical(m_vAlign);
    }

    /** Default text color */
    static const uint32_t   DEFAULT_TEXT_COLOR      = ColorDef::WHITE;

    /** Widget type string */
    static const char*      WIDGET_TYPE;

    /** Default font */
    static const YAFont&    DEFAULT_FONT;

    /** Default pause between character scrolling in ms */
    static const uint32_t   DEFAULT_SCROLL_PAUSE    = 80U;

    /** Minimal scroll pause in ms */
    static const uint32_t   MIN_SCROLL_PAUSE        = 20U;

    /** Maximal scroll pause in ms */
    static const uint32_t   MAX_SCROLL_PAUSE        = 500U;

private:

    /** Fading brightness delta value per cycle. */
    static const uint8_t    FADING_BRIGHTNESS_DELTA = 5U;

    /** Fading brigthness low (darkest value). */
    static const uint8_t    FADING_BRIGHTNESS_LOW   = 0U;

    /** Fading brigthness high (brigthest value). */
    static const uint8_t    FADING_BRIGHTNESS_HIGH  = 255U;

    /** Keyword handler method. */
    typedef void (TextWidget::*KeywordHandler)(YAGfx& gfx, const String& keyword);
 
    /**
     * Format keyword row, which specifies how does the keyword look like and
     * its corresponding handler.
     */
    struct FormatKeywordRow
    {
        const char*     keyword;    /**< Keyword */
        KeywordHandler  handler;    /**< Handler method */
    };

    /**
     * Table with keywords, which to apply before text is shown.
     */
    static const FormatKeywordRow   FORMAT_KEYWORD_TABLE_1[];

    /**
     * Table with keywords, which to apply during text is shown.
     */
    static const FormatKeywordRow   FORMAT_KEYWORD_TABLE_2[];

    /**
     * Fade state.
     */
    enum FadeState
    {
        FADE_STATE_IDLE = 0,    /**< No fading. */
        FADE_STATE_OUT,         /**< Fading out. */
        FADE_STATE_IN           /**< Fading in. */

    };

    /**
     * Scroll information, used per text.
     */
    struct ScrollInfo
    {
        bool        isEnabled;          /**< Is scrolling enabled? */
        bool        isScrollingToLeft;  /**< Is text scrolling to left? Otherwise scrolling to top. */
        int16_t     offsetDest;         /**< Offset destination in pixel */
        int16_t     offset;             /**< Current offset in pixel */
        uint16_t    textWidth;          /**< Text width in pixel */
        uint16_t    textHeight;          /**< Text height in pixel */

        /**
         * Initializes scroll information.
         */
        ScrollInfo() :
            isEnabled(false),
            isScrollingToLeft(true),
            offsetDest(0),
            offset(0),
            textWidth(0U),
            textHeight(0U)
        {
        }

        /**
         * Clear scroll information.
         */
        void clear()
        {
            isEnabled           = false;
            isScrollingToLeft   = true;
            offsetDest          = 0;
            offset              = 0;
            textWidth           = 0U;
            textHeight          = 0U;
        }
    };

    /**
     * Parser results
     */
    enum Result
    {
        RESULT_TEXT = 0,    /**< Text found */
        RESULT_KEYWORD,     /**< Format keyword found */
        RESULT_EMPTY        /**< Empty */
    };

    String                  m_formatStr;            /**< Current shown string, which contains format tags. */
    String                  m_formatStrNew;         /**< New text string, which contains format tags. */
    FadeState               m_fadeState;            /**< The current fade state. Used to switch from old to new text. */
    uint8_t                 m_fadeBrightness;       /**< Brightness value used for fading. */
    ScrollInfo              m_scrollInfo;           /**< Scroll information */
    ScrollInfo              m_scrollInfoNew;        /**< Scroll information for the new text. */
    bool                    m_isNewTextAvailable;   /**< Is new updated text available? */
    YAGfxText               m_gfxText;              /**< GFX for current text. */
    YAGfxText               m_gfxNewText;           /**< GFX for new text. */
    uint32_t                m_scrollingCnt;         /**< Counts how often a text was complete scrolled. */
    int16_t                 m_scrollOffset;         /**< Pixel offset of cursor x position, used for scrolling. */
    SimpleTimer             m_scrollTimer;          /**< Timer, used for scrolling */
    Alignment::Horizontal   m_hAlign;               /**< Horizontal alignment. */
    Alignment::Vertical     m_vAlign;               /**< Vertical alignment. */
    int16_t                 m_hAlignPosX;           /**< x-coordinate derived from horizontal alignment. */
    int16_t                 m_vAlignPosY;           /**< y-coordinate derived from vertical alignment. */

    static uint32_t         m_scrollPause;          /**< Pause in ms, between each scroll movement. */

    /**
     * Align the text horizontal and vertical.
     * It will adapt the m_hAlignPosX and m_vAlignPosY.
     */
    void alignText(Alignment::Horizontal hAlign, Alignment::Vertical vAlign);

    /**
     * Align the text horizontal by calculating the x-coordinate of the text box.
     *
     * @param[in] hAlign    Horizontal alignment
     * 
     * @return x-coordinate
     */
    int16_t alignTextHorizontal(Alignment::Horizontal hAlign) const;

    /**
     * Align the text vertical by calculating the y-coordinate of the text box.
     *
     * @param[in] vAlign    Vertical alignment
     * 
     * @return y-coordinate
     */
    int16_t alignTextVertical(Alignment::Vertical vAlign) const;

    /**
     * Get the number of lines, which can be used by the text widget.
     * 
     * @return Number of lines
     */
    uint16_t getLineCount() const;

    /**
     * Can text be shown static or is scrolling required?
     * 
     * @param[in] gfx           Graphics interface
     * @param[in] textBoxWidth  Text box width in pixel
     * @param[in] textBoxHeight Text box height in pixel
     * 
     * @return If text can be shown static, it will return true otherwise false.
     */
    bool isStaticText(YAGfx& gfx, uint16_t textBoxWidth, uint16_t textBoxHeight) const;

    /**
     * Checks new text and prepares the scroll information.
     * 
     * @param[in] gfx   The graphics functionality, necessary to determine text width and etc.
     */
    void prepareNewText(YAGfx& gfx);

    /**
     * Calculate the cursor start position depended on the scrolling direction.
     * 
     * @param[out] curX Cursor x-coordindate
     * @param[out] curY Cursor y-coordinate
     */
    void calculateCursorPos(int16_t& curX, int16_t& curY) const;

    /**
     * Handle text without fading.
     * 
     * @param[in] gfx Graphic functionality
     */
    void handleFadeIdle(YAGfx& gfx);

    /**
     * Handle fading text out.
     * 
     * @param[in] gfx Graphic functionality
     */
    void handleFadeOut(YAGfx& gfx);

    /**
     * Handle fading text in.
     * 
     * @param[in] gfx Graphic functionality
     */
    void handleFadeIn(YAGfx& gfx);

    /**
     * Scroll the text depended on the scrolling direction.
     * It will only update the scrolling offset.
     * 
     * @param[in] gfx Graphic functionality
     */
    void scrollText(YAGfx& gfx);

    /**
     * Paint the widget with the given graphics interface.
     * 
     * @param[in] gfx   Graphics interface
     */
    void paint(YAGfx& gfx) override;

    /**
     * Remove format tags from string.
     *
     * @param[in] formatStr String which contains format tags
     *
     * @return String without format tags
     */
    String removeFormatTags(const String& formatStr) const;

    /**
     * Show formatted text.
     * Format tags:
     * - #RRGGBB Color in HTML form (RGB888)
     *
     * @param[in] gfx           Graphics, used to draw the characters
     * @param[in] formatStr     String which contains format tags
     * @param[in] isScrolling   Is text scrolling or not.
     */
    void show(YAGfx& gfx, const String& formatStr, bool isScrolling);

    /**
     * Parse formatted text. Everytime a text or a format keyword is found, it will
     * immediately return. Call this function continously for parsing the whole
     * string, until its finished.
     * 
     * A format keyword is enclosed with {}.
     * If a '{' is part of the text itself, it must be escaped with a backslash.
     * 
     * @param[in]   str             String with formatted text.
     * @param[in]   beginIdx        Index in the string where to start parsing.
     * @param[out]  resultStr       Result string may contain text, a format keyword or is empty.
     * @param[out]  remainingIdx    Remaining index in the string
     * 
     * @return Parser result, consider it to know whats in the resultStr and whether to continoue parsing.
     */
    Result parseFormattedText(const String& str, size_t& beginIdx, String& resultStr, size_t& remainingIdx) const;

    /**
     * Compares two keywords.
     * 
     * @param[in] keyword   Keyword 1, consider wildcards.
     * @param[in] other     Keyword 2, the concrete one.
     * 
     * @return If equal, it will return true otherwise false. 
     */
    bool isKeywordEqual(const char* keyword, const char* other) const;

    /**
     * Handle concrete keyword.
     * 
     * @param[in] gfx       Graphic functionality
     * @param[in] table     Keyword table with the handlers
     * @param[in] tableSize Number of elements in the keyword table
     * @param[in] keyword   The keyword which to handle
     * 
     * @return If successful, it will return true otherwise false.
     */
    bool handleKeyword(YAGfx& gfx, const FormatKeywordRow* table, size_t tableSize, const String& keyword);

    /**
     * Align text horizontal left.
     * 
     * @param[in] gfx       Graphic functionality
     * @param[in] keyword   Keyword
     */
    void horizontalLeftAligned(YAGfx& gfx, const String& keyword);

    /**
     * Align text horizontal center.
     * 
     * @param[in] gfx       Graphic functionality
     * @param[in] keyword   Keyword
     */
    void horizontalCenterAligned(YAGfx& gfx, const String& keyword);

    /**
     * Align text horizontal right.
     * 
     * @param[in] gfx       Graphic functionality
     * @param[in] keyword   Keyword
     */
    void horizontalRightAligned(YAGfx& gfx, const String& keyword);

    /**
     * Align text vertical top.
     * 
     * @param[in] gfx       Graphic functionality
     * @param[in] keyword   Keyword
     */
    void verticalTopAligned(YAGfx& gfx, const String& keyword);

    /**
     * Align text vertical center.
     * 
     * @param[in] gfx       Graphic functionality
     * @param[in] keyword   Keyword
     */
    void verticalCenterAligned(YAGfx& gfx, const String& keyword);

    /**
     * Align text vertical bottom.
     * 
     * @param[in] gfx       Graphic functionality
     * @param[in] keyword   Keyword
     */
    void verticalBottomAligned(YAGfx& gfx, const String& keyword);

    /**
     * Handle text color keyword code.
     * 
     * @param[in] gfx       Graphic functionality
     * @param[in] keyword   Keyword
     */
    void handleColor(YAGfx& gfx, const String& keyword);

    /**
     * Handle character keyword code.
     * 
     * @param[in] gfx       Graphic functionality
     * @param[in] keyword   Keyword
     */
    void handleCharCode(YAGfx& gfx, const String& keyword);

    /**
     * Move text cursor horizontal.
     * 
     * @param[in] gfx       Graphic functionality
     * @param[in] keyword   Keyword
     */
    void horizontalMove(YAGfx& gfx, const String& keyword);

    /**
     * Move text cursor vertical.
     * 
     * @param[in] gfx       Graphic functionality
     * @param[in] keyword   Keyword
     */
    void verticalMove(YAGfx& gfx, const String& keyword);

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* TEXTWIDGET_H */

/** @} */