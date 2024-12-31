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
 * @addtogroup GFX
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
#include "TWAbstractSyntaxTree.h"

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
     * Set widget height.
     * 
     * @param[in] height Height in pixel
     */
    void setHeight(uint16_t height) override
    {
        Widget::setHeight(height);
        alignTextVertical();
    }

    /**
     * Set the text string. It can contain format tags like:
     * - "#RRGGBB" Color information in RGB888 format
     * 
     * @param[in] formatStr String, which may contain format tags
     */
    void setFormatStr(const String& formatStr);

    /**
     * Clear immediately and don't show anything further.
     */
    void clear();

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
    String getStr() const;
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
        m_prepareNewText = true;
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
    bool getScrollInfo(bool& isScrollingEnabled, uint32_t& scrollingCnt);

    /**
     * Set the horizontal alignment.
     * 
     * @param[in] align The horizontal aligment.
     */
    void setHorizontalAlignment(Alignment::Horizontal align)
    {
        m_hAlign = align;
    }

    /**
     * Set the vertical alignment.
     * 
     * @param[in] align The vertical aligment.
     */
    void setVerticalAlignment(Alignment::Vertical align)
    {
        m_vAlign = align;
        alignTextVertical();
    }

    /**
     * Enable the fade effect.
     */
    void enableFadeEffect()
    {
        m_isFadeEffectEnabled = true;
    }

    /**
     * Disable the fade effect.
     */
    void disableFadeEffect()
    {
        m_isFadeEffectEnabled = false;
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
        uint16_t    textHeight;         /**< Text height in pixel */

        /**
         * Initializes scroll information.
         */
        ScrollInfo() :
            isEnabled(false),
            isScrollingToLeft(true),
            offsetDest(0),
            offset(0),
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
            textHeight          = 0U;
        }
    };

    String                  m_formatStr;            /**< Current shown string, which contains format tags. */
    String                  m_formatStrNew;         /**< New text string, which contains format tags. */
    FadeState               m_fadeState;            /**< The current fade state. Used to switch from old to new text. */
    uint8_t                 m_fadeBrightness;       /**< Brightness value used for fading. */
    bool                    m_isFadeEffectEnabled;  /**< Is fade effect enabled? */
    ScrollInfo              m_scrollInfo;           /**< Scroll information */
    ScrollInfo              m_scrollInfoNew;        /**< Scroll information for the new text. */
    bool                    m_prepareNewText;       /**< User set new text, which shall be prepared. */
    bool                    m_updateText;           /**< New text is prepared shall be updated. */
    TWAbstractSyntaxTree    m_ast;                  /**< AST for the current format string. */
    TWAbstractSyntaxTree    m_astNew;               /**< AST for the new format string. */
    YAGfxText               m_gfxText;              /**< GFX for current text. */
    YAGfxText               m_gfxNewText;           /**< GFX for new text. */
    uint32_t                m_scrollingCnt;         /**< Counts how often a text was complete scrolled. */
    int16_t                 m_scrollOffset;         /**< Pixel offset of cursor x position, used for scrolling. */
    SimpleTimer             m_scrollTimer;          /**< Timer, used for scrolling */
    
    /**
     * Horizontal alignment which is the default one.
     * During an display update it might be overwritten by a keyword, but will always
     * be restored back, after the update is finished.
     * 
     * Horizontal alignment is done line by line, divided by a line feed.
     */
    Alignment::Horizontal   m_hAlign;

    /**
     * Vertical alignment which is the default one.
     * During an display update it might be overwritten by a keyword, but will always
     * be restored back, after the update is finished.
     * 
     * Vertical alignment is done per text block.
     */
    Alignment::Vertical     m_vAlign;

    /**
     * y-coordinate calculated from vertical alignment.
     */
    int16_t                 m_vAlignPosY;

    /**
     * Pause in ms, between each scroll movement.
     * Its used by all text widget instances.
     */
    static uint32_t         m_scrollPause;

    /**
     * Align the current text horizontal by calculating the x-coordinate of the
     * current text box.
     * 
     * Horizontal alignment will only be done for
     * - Static text
     * - Text scrolling from bottom to top
     * 
     * Otherwise the x-coordinate will be set to 0.
     *
     * @param[in] gfx       Graphic functionality, used for bound box calculation.
     * @param[in] text      Text for which the alignment is calculated.
     * @param[in] hAlign    Horizontal alignment.
     * 
     * @return x-coordinate
     */
    int16_t alignTextHorizontal(YAGfx& gfx, const String& text, Alignment::Horizontal hAlign) const;

    /**
     * Align the current text vertical by calculating the y-coordinate of the
     * text box.
     * 
     * Vertical alignment will only be done for
     * - Static text
     * - Text scrolling from left to right
     * 
     * Otherwise the y-coordinate will be set to 0.
     */
    void alignTextVertical();

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
     * Handle the fade effect.
     */
    void handleFadeEffect();

    /**
     * Handle fading text out.
     * 
     * @param[in] gfx Graphic functionality
     */
    void handleFadeOut();

    /**
     * Handle fading text in.
     * 
     * @param[in] gfx Graphic functionality
     */
    void handleFadeIn();

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
     * Walks throught the AST and integrates the special character code
     * keywords. Thats means the token will be converted to a text token and
     * its string will be the character code.
     * 
     * @param[in, out] ast  The abstract syntax tree (AST)
     */
    void specialCharacterCodeKeywordToText(TWAbstractSyntaxTree& ast);

    /**
     * Get only the text from abstract syntax tree.
     * 
     * @param[out]  text    Contains only the text from AST after call.
     * @param[in]   ast     The abstract syntax tree.
     */
    void getText(String& text, const TWAbstractSyntaxTree& ast) const;

    /**
     * Get a single line from abstract syntax tree, starting at the given index.
     * 
     * @param[out]  singleLine  Contains the single line at the end.
     * @param[in]   ast         The abstract syntax tree.
     * @param[in]   startIdx    Start index in the AST.
     * 
     * @return Next index
     */
    uint32_t getSingleLine(String& singleLine, const TWAbstractSyntaxTree& ast, uint32_t startIdx);

    /**
     * Show formatted text.
     * Format tags:
     * - #RRGGBB Color in HTML form (RGB888)
     *
     * @param[in] gfx           Graphics, used to draw the characters.
     * @param[in] ast           The abstract syntax tree.
     * @param[in] isScrolling   Is text scrolling or not.
     */
    void show(YAGfx& gfx, const TWAbstractSyntaxTree& ast, bool isScrolling);

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