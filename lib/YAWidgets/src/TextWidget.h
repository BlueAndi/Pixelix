/* MIT License
 *
 * Copyright (c) 2019 - 2026 Andreas Merkle <web@blue-andi.de>
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
 * @file   TextWidget.h
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
#include <YAGfxBrush.h>
#include "Alignment.h"
#include "TWAbstractSyntaxTree.h"
#include "ScrollController.h"

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
 * - "{#RRGGBB}": Set solid text color; RRGGBB in hex
 * - "{lgv #RRGGBB,#RRGGBB,OFFSET,LENGTH}": Set linear vertical gradient text color; RRGGBB in hex; OFFSET in pixels; LENGTH in pixels.
 * - "{lgh #RRGGBB,#RRGGBB,OFFSET,LENGTH}": Set linear horizontal gradient text color; RRGGBB in hex; OFFSET in pixels; LENGTH in pixels.
 * - "{stc}": Activate solid text color
 * - "{lgtc}": Activate linear gradient text color
 * - "{hl}" : Horizontal alignment left
 * - "{hc}" : Horizontal alignment center
 * - "{hr}" : Horizontal alignment right
 * - "{vt}" : Vertical alignment top
 * - "{vc}" : Vertical alignment center
 * - "{vb}" : Vertical alignment bottom
 * - "{0xCC}" : Special character hex code
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
     * @param[in] color Color of the string.
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
     *
     * @return Text widget
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
     * Set the text string. It can contain format tags like "#RRGGBB" color
     * information in RGB888 format.
     *
     * Encoding: UTF-8
     *
     * @param[in] formatStrUtf8 UTF-8 string, which may contain format tags.
     */
    void setFormatStr(const String& formatStrUtf8);

    /**
     * Clear immediately and don't show anything further.
     */
    void clear();

    /**
     * Get the text string, which may contain format tags.
     *
     * Encoding: UTF-8
     *
     * @return UTF-8 string, which may contain format tags.
     */
    String getFormatStr() const
    {
        return m_formatStrNewUtf8;
    }

    /**
     * Get the text string, without format tags.
     *
     * Encoding: Internal
     *
     * @return String
     */
    String getStr() const;

    /**
     * Get brush used to draw text.
     *
     * @return Brush used to draw text.
     */
    YAGfxBrush& getBrush()
    {
        return m_gfxText.getBrush();
    }

    /**
     * Set brush used to draw text. The brush must be kept alive as long as the text widget is used.
     *
     * @param[in] brush Brush used to draw text.
     */
    void setBrush(YAGfxBrush& brush)
    {
        m_gfxText.setBrush(brush);
    }

    /**
     * Use the internal solid brush with the already configured color.
     */
    void setSolidBrush()
    {
        m_gfxText.setBrush(m_solidBrush);
    }

    /**
     * Use the internal linear gradient brush with the already configured values.
     */
    void setLinearGradientBrush()
    {
        m_gfxText.setBrush(m_linearGradientBrush);
    }

    /**
     * Set internal solid brush.
     *
     * @param[in] color Color of the brush
     */
    void setSolidBrush(const Color& color)
    {
        m_solidBrush.setColor(color);
        m_gfxText.setBrush(m_solidBrush);
    }

    /**
     * Set internal linear gradient brush.
     *
     * @param[in] color1    Start color of the gradient.
     * @param[in] color2    End color of the gradient.
     * @param[in] offset    Offset in pixels of the gradient start color.
     * @param[in] length    Length of the gradient in pixels.
     * @param[in] vertical  Flag for vertical gradient.
     */
    void setLinearGradientBrush(const Color& color1, const Color& color2, uint32_t offset, uint32_t length, bool vertical)
    {
        m_linearGradientBrush.setStartColor(color1);
        m_linearGradientBrush.setEndColor(color2);
        m_linearGradientBrush.setOffset(offset);
        m_linearGradientBrush.setLength(length);
        m_linearGradientBrush.setDirection(vertical);
        m_gfxText.setBrush(m_linearGradientBrush);
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
        return ScrollController::setScrollPause(pause);
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
    static const uint32_t DEFAULT_TEXT_COLOR                  = ColorDef::WHITE;

    /** Default text color gradient color 1 */
    static const uint32_t DEFAULT_TEXT_COLOR_GRADIENT_COLOR_1 = ColorDef::RED;

    /** Default text color gradient color 2 */
    static const uint32_t DEFAULT_TEXT_COLOR_GRADIENT_COLOR_2 = ColorDef::BLUE;

    /** Default text color gradient offset in pixels. */
    static const int16_t DEFAULT_TEXT_COLOR_GRADIENT_OFFSET   = 0;

    /** Default text color gradient length in pixels. */
    static const uint16_t DEFAULT_TEXT_COLOR_GRADIENT_LENGTH  = 32U;

    /** Default text color gradient direction (true = vertical, false = horizontal). */
    static const bool DEFAULT_TEXT_COLOR_GRADIENT_VERTICAL    = true;

    /** Widget type string */
    static const char* WIDGET_TYPE;

    /** Default font */
    static const YAFont& DEFAULT_FONT;

private:

    /** Fading brightness delta value per cycle. */
    static const uint8_t FADING_BRIGHTNESS_DELTA = 10U;

    /** Fading brigthness low (darkest value). */
    static const uint8_t FADING_BRIGHTNESS_LOW   = 0U;

    /** Fading brigthness high (brigthest value). */
    static const uint8_t FADING_BRIGHTNESS_HIGH  = 255U;

    /** Keyword handler method. */
    typedef void (TextWidget::*KeywordHandler)(YAGfx& gfx, const String& keyword);

    /**
     * Format keyword row, which specifies how does the keyword look like and
     * its corresponding handler.
     */
    struct FormatKeywordRow
    {
        const char*    keyword; /**< Keyword */
        KeywordHandler handler; /**< Handler method */
    };

    /**
     * Table with keywords, which to apply before text is shown.
     */
    static const FormatKeywordRow FORMAT_KEYWORD_TABLE_1[];

    /**
     * Table with keywords, which to apply during text is shown.
     */
    static const FormatKeywordRow FORMAT_KEYWORD_TABLE_2[];

    /**
     * Fade state.
     */
    enum FadeState
    {
        FADE_STATE_IDLE = 0, /**< No fading. */
        FADE_STATE_OUT,      /**< Fading out. */
        FADE_STATE_IN        /**< Fading in. */
    };

    String                   m_formatStrUtf8;       /**< Current shown string, which contains format tags. Encoding: UTF-8 */
    String                   m_formatStrNewUtf8;    /**< New text string, which contains format tags. Encoding: UTF-8 */
    FadeState                m_fadeState;           /**< The current fade state. Used to switch from old to new text. */
    uint8_t                  m_fadeBrightness;      /**< Brightness value used for fading. */
    bool                     m_isFadeEffectEnabled; /**< Is fade effect enabled? */
    ScrollController         m_scrollCtrl;          /**< Scroll controller for current text */
    ScrollController         m_scrollCtrlNew;       /**< Scroll controller for new text */
    uint16_t                 m_textHeight;          /**< Text height in pixel */
    uint16_t                 m_textHeightNew;       /**< Text height in pixel for new text */
    bool                     m_prepareNewText;      /**< User set new text, which shall be prepared. */
    bool                     m_updateText;          /**< New text is prepared shall be updated. */
    TWAbstractSyntaxTree     m_ast;                 /**< AST for the current format string. Encoding: Internal */
    TWAbstractSyntaxTree     m_astNew;              /**< AST for the new format string. Encoding: Internal */
    YAGfxSolidBrush          m_solidBrush;          /**< Solid text color brush. */
    YAGfxLinearGradientBrush m_linearGradientBrush; /**< Linear gradient text color brush. */
    YAGfxText                m_gfxText;             /**< GFX for current text. */

    /**
     * Horizontal alignment which is the default one.
     * During an display update it might be overwritten by a keyword, but will always
     * be restored back, after the update is finished.
     *
     * Horizontal alignment is done line by line, divided by a line feed.
     */
    Alignment::Horizontal m_hAlign;

    /**
     * Vertical alignment which is the default one.
     * During an display update it might be overwritten by a keyword, but will always
     * be restored back, after the update is finished.
     *
     * Vertical alignment is done per text block.
     */
    Alignment::Vertical m_vAlign;

    /**
     * y-coordinate calculated from vertical alignment.
     */
    int16_t m_vAlignPosY;

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
     */
    void handleFadeOut();

    /**
     * Handle fading text in.
     */
    void handleFadeIn();

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
     * - "#RRGGBB" Color in HTML form (RGB888)
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
     * Handle solid text color keyword code.
     *
     * @param[in] gfx       Graphic functionality
     * @param[in] keyword   Keyword
     */
    void handleSolidColor(YAGfx& gfx, const String& keyword);

    /**
     * Handle linear gradient keyword code, but without changing the
     * direction.
     *
     * @param[in] gfx       Graphic functionality
     * @param[in] keyword   Keyword
     *
     * @return If successful, it will return true otherwise false.
     */
    bool handleLinearGradient(YAGfx& gfx, const String& keyword);

    /**
     * Handle vertical linear gradient keyword code.
     *
     * @param[in] gfx       Graphic functionality
     * @param[in] keyword   Keyword
     */
    void handleLinearGradientVertical(YAGfx& gfx, const String& keyword);

    /**
     * Handle horizontal linear gradient keyword code.
     *
     * @param[in] gfx       Graphic functionality
     * @param[in] keyword   Keyword
     */
    void handleLinearGradientHorizontal(YAGfx& gfx, const String& keyword);

    /**
     * Activate solid text color.
     *
     * @param[in] gfx       Graphic functionality
     * @param[in] keyword   Keyword
     */
    void solidTextColor(YAGfx& gfx, const String& keyword);

    /**
     * Activate linear gradient text color.
     *
     * @param[in] gfx       Graphic functionality
     * @param[in] keyword   Keyword
     */
    void linearGradientTextColor(YAGfx& gfx, const String& keyword);

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

#endif /* TEXTWIDGET_H */

/** @} */