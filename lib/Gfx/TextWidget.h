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
 * The text has a given color, which can be changed.
 *
 * Different keywords in the string are supported, e.g. for coloring or alignment.
 * Each keyword starts with a '\\', otherwise its treated as just text.
 * Example: "\\#FF0000H#FFFFFFello!" contains a red "H" and a white "ello!".
 *
 * Keywords:
 * - "\\#RRGGBB": Change text color; RRGGBB in hex
 * - "\\lalign" : Alignment left
 * - "\\ralign" : Alignment right
 * - "\\calign" : Alignment center
 */
class TextWidget : public Widget
{
public:

    /**
     * Constructs a text widget with a empty string in default color.
     */
    TextWidget() :
        Widget(WIDGET_TYPE),
        m_formatStr(),
        m_textColor(DEFAULT_TEXT_COLOR),
        m_font(DEFAULT_FONT),
        m_checkScrollingNeed(false),
        m_isScrollingEnabled(false),
        m_scrollingCnt(0U),
        m_textWidth(0U),
        m_scrollOffset(0),
        m_scrollTimer()
    {
    }

    /**
     * Constructs a text widget with the given string and its color.
     * If there is no color given, it will be default color.
     *
     * @param[in] str   String, which may contain format tags.
     * @param[in] color Color of the string
     */
    TextWidget(const String& str, const Color& color = DEFAULT_TEXT_COLOR) :
        Widget(WIDGET_TYPE),
        m_formatStr(str),
        m_textColor(color),
        m_font(DEFAULT_FONT),
        m_checkScrollingNeed(false),
        m_isScrollingEnabled(false),
        m_scrollingCnt(0U),
        m_textWidth(0U),
        m_scrollOffset(0),
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
        m_formatStr(widget.m_formatStr),
        m_textColor(widget.m_textColor),
        m_font(widget.m_font),
        m_checkScrollingNeed(widget.m_checkScrollingNeed),
        m_isScrollingEnabled(widget.m_isScrollingEnabled),
        m_scrollingCnt(widget.m_scrollingCnt),
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
        if (&widget != this)
        {
            m_formatStr             = widget.m_formatStr;
            m_textColor             = widget.m_textColor;
            m_font                  = widget.m_font;
            m_checkScrollingNeed    = widget.m_checkScrollingNeed;
            m_isScrollingEnabled    = widget.m_isScrollingEnabled;
            m_scrollingCnt          = widget.m_scrollingCnt;
            m_textWidth             = widget.m_textWidth;
            m_scrollOffset          = widget.m_scrollOffset;
            m_scrollTimer           = widget.m_scrollTimer;
        }

        return *this;
    }

    /**
     * Update/Draw the text widget.
     *
     * @param[in] gfx Graphics interface
     */
    void update(IGfx& gfx) override;

    /**
     * Set the text string. It can contain format tags like:
     * - "#RRGGBB" Color information in RGB888 format
     *
     * @param[in] formatStr String, which may contain format tags
     */
    void setFormatStr(const String& formatStr)
    {
        /* Avoid upate if not necessary. */
        if (m_formatStr != formatStr)
        {
            m_formatStr             = formatStr;
            m_checkScrollingNeed    = true;
        }

        return;
    }

    /**
     * Get the text string, which may contain format tags.
     *
     * @return String, which may contain format tags.
     */
    String getFormatStr() const
    {
        return m_formatStr;
    }

    /**
     * Get the text string, without format tags.
     *
     * @return String
     */
    String getStr() const
    {
        return removeFormatTags(m_formatStr);
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
    const Color& getTextColor() const
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
     * @return If a font is set, it will be returned otherwise nullptr.
     */
    const GFXfont* getFont() const
    {
        return m_font;
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
     * Get scrolling informations.
     *
     * @param[out] isScrollingEnabled   Is scrolling enabled or not?
     * @param[out] scrollingCnt         How often was the text complete scrolled over the display?
     *
     * @return If scroll information is ready, it will return true otherwise false.
     */
    bool getScrollInfo(bool& isScrollingEnabled, uint32_t& scrollingCnt)
    {
        bool status = (true == m_checkScrollingNeed) ? false : true;

        if (true == status)
        {
            isScrollingEnabled  = m_isScrollingEnabled;
            scrollingCnt        = m_scrollingCnt;
        }

        return status;
    }

    /** Default text color */
    static const uint32_t   DEFAULT_TEXT_COLOR      = ColorDef::WHITE;

    /** Widget type string */
    static const char*      WIDGET_TYPE;

    /** Default font */
    static const GFXfont*   DEFAULT_FONT;

    /** Default pause between character scrolling in ms */
    static const uint32_t   DEFAULT_SCROLL_PAUSE    = 80U;

    /** Minimal scroll pause in ms */
    static const uint32_t   MIN_SCROLL_PAUSE        = 20U;

    /** Maximal scroll pause in ms */
    static const uint32_t   MAX_SCROLL_PAUSE        = 500U;

private:

    /** Keyword handler method. */
    typedef bool (TextWidget::*KeywordHandler)(IGfx* gfx, bool noAction, const String& formatStr, uint8_t& overstep) const;

    String          m_formatStr;            /**< String, which contains format tags. */
    Color           m_textColor;            /**< Text color of the string */
    const GFXfont*  m_font;                 /**< Current font */
    bool            m_checkScrollingNeed;   /**< Check for scrolling need or not */
    bool            m_isScrollingEnabled;   /**< Is scrolling enabled or disabled */
    uint32_t        m_scrollingCnt;         /**< Counts how often a text was complete scrolled. */
    uint16_t        m_textWidth;            /**< Text width in pixel */
    int16_t         m_scrollOffset;         /**< Pixel offset of cursor x position, used for scrolling. */
    SimpleTimer     m_scrollTimer;          /**< Timer, used for scrolling */

    static KeywordHandler   m_keywordHandlers[];    /**< List of all supported keyword handlers. */
    static uint32_t         m_scrollPause;          /**< Pause in ms, between each scroll movement. */

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
     * @param[in] gfx       Graphics, used to draw the characters
     * @param[in] formatStr String which contains format tags
     */
    void show(IGfx& gfx, const String& formatStr) const;

    /**
     * Handles the keyword for color changes.
     *
     * @param[in] gfx       Graphics interface, only necessary if actions shall take place.
     * @param[in] noAction  The handler shall take no action. This is only used to get rid of the keywords in the text.
     * @param[in] formatStr String which may contain keywords.
     * @param[out] overstep Number of characters, which must be overstepped before the next normal character comes.
     *
     * @return If keyword is handled successful, it returns true otherwise false.
     */
    bool handleColor(IGfx* gfx, bool noAction, const String& formatStr, uint8_t& overstep) const;

    /**
     * Handles the keyword for alignment changes.
     *
     * @param[in] gfx       Graphics interface, only necessary if actions shall take place.
     * @param[in] noAction  The handler shall take no action. This is only used to get rid of the keywords in the text.
     * @param[in] formatStr String which may contain keywords.
     * @param[out] overstep Number of characters, which must be overstepped before the next normal character comes.
     *
     * @return If keyword is handled successful, it returns true otherwise false.
     */
    bool handleAlignment(IGfx* gfx, bool noAction, const String& formatStr, uint8_t& overstep) const;
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __TEXTWIDGET_H__ */

/** @} */