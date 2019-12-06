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
 * 
 * @addtogroup gfx
 *
 * @{
 */

#ifndef __TEXTWIDGET_H__
#define __TEXTWIDGET_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <WString.h>
#include <Widget.hpp>
#include <Color.h>
#include <SimpleTimer.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * A text widget, showing a colored string.
 */
class TextWidget : public Widget
{
public:

    /**
     * Constructs a text widget with a empty string in default color.
     */
    TextWidget() :
        Widget(WIDGET_TYPE),
        m_str(),
        m_textColor(DEFAULT_TEXT_COLOR),
        m_font(DEFAULT_FONT),
        m_checkScrollingNeed(false),
        m_isScrollingEnabled(false),
        m_textWidth(0u),
        m_scrollOffset(0u),
        m_scrollTimer()
    {
    }

    /**
     * Constructs a text widget with the given string and its color.
     * If there is no color given, it will be default color.
     * 
     * @param[in] str   String
     * @param[in] color Color of the string
     */
    TextWidget(const String& str, const Color& color = DEFAULT_TEXT_COLOR) :
        Widget(WIDGET_TYPE),
        m_str(str),
        m_textColor(color),
        m_font(DEFAULT_FONT),
        m_checkScrollingNeed(false),
        m_isScrollingEnabled(false),
        m_textWidth(0u),
        m_scrollOffset(0u),
        m_scrollTimer()
    {
    }

    /**
     * Constructs a text widget by copying another one.
     * 
     * @param[in] widget Widget, which to copy
     */
    TextWidget(const TextWidget& widget) :
        Widget(WIDGET_TYPE),
        m_str(widget.m_str),
        m_textColor(widget.m_textColor),
        m_font(widget.m_font),
        m_checkScrollingNeed(widget.m_checkScrollingNeed),
        m_isScrollingEnabled(widget.m_isScrollingEnabled),
        m_textWidth(widget.m_textWidth),
        m_scrollOffset(widget.m_scrollOffset),
        m_scrollTimer(widget.m_scrollTimer)
    {
    }

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
    TextWidget& operator=(const TextWidget& widget)
    {
        m_str                   = widget.m_str;
        m_textColor             = widget.m_textColor;
        m_font                  = widget.m_font;
        m_checkScrollingNeed    = widget.m_checkScrollingNeed;
        m_isScrollingEnabled    = widget.m_isScrollingEnabled;
        m_textWidth             = widget.m_textWidth;
        m_scrollOffset          = widget.m_scrollOffset;
        m_scrollTimer           = widget.m_scrollTimer;

        return *this;
    }

    /**
     * Update/Draw the text widget.
     * 
     * @param[in] gfx Graphics interface
     */
    void update(Adafruit_GFX& gfx);

    /**
     * Set the text string.
     * 
     * @param[in] str String
     */
    void setStr(const String& str)
    {
        m_str                   = str;
        m_checkScrollingNeed    = true;
        return;
    }

    /**
     * Get the text string.
     * 
     * @return String
     */
    String getStr(void) const
    {
        return m_str;
    }

    /**
     * Set the text color of the string.
     * 
     * @param[in] color Text color
     */
    void setTextColor(const Color& color)
    {
        m_textColor = color;
        return;
    }

    /**
     * Get the text color of the string.
     * 
     * @return Text color
     */
    const Color& getTextColor(void) const
    {
        return m_textColor;
    }

    /**
     * Set font.
     * 
     * @param[in] font  New font to set
     */
    void setFont(const GFXfont* font)
    {
        m_font                  = font;
        m_checkScrollingNeed    = true;
        return;
    }

    /**
     * Get font.
     * 
     * @return If a font is set, it will be returned otherwise NULL.
     */
    const GFXfont* getFont(void) const
    {
        return m_font;
    }

    /** Default text color */
    static const uint32_t   DEFAULT_TEXT_COLOR      = ColorDef::WHITE;
    
    /** Widget type string */
    static const char*      WIDGET_TYPE;

    /** Default font */
    static const GFXfont*   DEFAULT_FONT;

    /** Default pause between character scrolling in ms */
    static const uint32_t   DEFAULT_SCROLL_PAUSE    = 200u;

private:

    String          m_str;                  /**< String */
    Color           m_textColor;            /**< Text color of the string */
    const GFXfont*  m_font;                 /**< Current font */
    bool            m_checkScrollingNeed;   /**< Check for scrolling need or not */
    bool            m_isScrollingEnabled;   /**< Is scrolling enabled or disabled */
    uint16_t        m_textWidth;            /**< Text width in pixel */
    uint16_t        m_scrollOffset;         /**< Pixel offset of cursor x position, used for scrolling. */
    SimpleTimer     m_scrollTimer;          /**< Timer, used for scrolling */

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __TEXTWIDGET_H__ */

/** @} */