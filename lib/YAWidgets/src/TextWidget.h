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
        m_formatStrNew(),
        m_formatStrTmp(),
        m_scrollInfo(),
        m_scrollInfoNew(),
        m_isNewTextAvailable(false),
        m_handleNewText(false),
        m_gfxText(DEFAULT_FONT, DEFAULT_TEXT_COLOR),
        m_scrollingCnt(0U),
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
        m_formatStrNew(str),
        m_formatStrTmp(),
        m_scrollInfo(),
        m_scrollInfoNew(),
        m_isNewTextAvailable(false),
        m_handleNewText(false),
        m_gfxText(DEFAULT_FONT, DEFAULT_TEXT_COLOR),
        m_scrollingCnt(0U),
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
        m_formatStrNew(widget.m_formatStrNew),
        m_formatStrTmp(widget.m_formatStrTmp),
        m_scrollInfo(widget.m_scrollInfo),
        m_scrollInfoNew(widget.m_scrollInfoNew),
        m_isNewTextAvailable(widget.m_isNewTextAvailable),
        m_handleNewText(widget.m_handleNewText),
        m_gfxText(widget.m_gfxText),
        m_scrollingCnt(widget.m_scrollingCnt),
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
            Widget::operator=(widget);
            
            m_formatStr             = widget.m_formatStr;
            m_formatStrNew          = widget.m_formatStrNew;
            m_formatStrTmp          = widget.m_formatStrTmp;
            m_scrollInfo            = widget.m_scrollInfo;
            m_scrollInfoNew         = widget.m_scrollInfoNew;
            m_isNewTextAvailable    = widget.m_isNewTextAvailable;
            m_handleNewText         = widget.m_handleNewText;
            m_gfxText               = widget.m_gfxText;
            m_scrollingCnt          = widget.m_scrollingCnt;
            m_scrollOffset          = widget.m_scrollOffset;
            m_scrollTimer           = widget.m_scrollTimer;
        }

        return *this;
    }

    /**
     * Set the text string. It can contain format tags like:
     * - "#RRGGBB" Color information in RGB888 format
     * 
     * Note: New text is always scrolled in and not immediate shown.
     *       If you want to show it immediately, you will need to clear() it first.
     * 
     * @param[in] formatStr String, which may contain format tags
     */
    void setFormatStr(const String& formatStr)
    {
        /* Avoid update if not necessary. */
        if ((m_formatStr != formatStr) &&
            (m_formatStrNew != formatStr))
        {
            /* If there is already a new text, which is not shown yet,
             * skip this new text.
             */
            if (false == m_handleNewText)
            {
                m_formatStrNew          = formatStr;
                m_isNewTextAvailable    = true;

                m_formatStrTmp.clear();
            }
            else
            {
                m_formatStrTmp = formatStr;
            }
        }

        m_scrollingCnt = 0U;
    }

    /**
     * Clear immediately and don't show anything further.
     */
    void clear()
    {
        m_formatStr.clear();
        m_formatStrNew.clear();
        m_formatStrTmp.clear();

        m_isNewTextAvailable = false;
        m_handleNewText = false;

        m_scrollInfo.isEnabled  = false;
        m_scrollInfo.offset     = 0;
        m_scrollInfo.offsetDest = 0;
        m_scrollInfo.stopAtDest = false;
        m_scrollInfo.textWidth  = 0U;
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
        return;
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
     * Get scrolling informations.
     *
     * @param[out] isScrollingEnabled   Is scrolling enabled or not?
     * @param[out] scrollingCnt         How often was the text complete scrolled over the display?
     *
     * @return If scroll information is ready, it will return true otherwise false.
     */
    bool getScrollInfo(bool& isScrollingEnabled, uint32_t& scrollingCnt)
    {
        bool status = false;
        
        if ((false == m_isNewTextAvailable) &&
            (false == m_handleNewText))
        {
            status              = true;
            isScrollingEnabled  = m_scrollInfoNew.isEnabled;
            scrollingCnt        = m_scrollingCnt;
        }

        return status;
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

    /** Keyword handler method. */
    typedef bool (TextWidget::*KeywordHandler)(YAGfx* gfx, YAGfxText* gfxText, bool noAction, const String& formatStr, bool isScrolling, uint8_t& overstep) const;

    /**
     * Scroll information, used per text.
     */
    struct ScrollInfo
    {
        bool        isEnabled;  /**< Is scrolling enabled? */
        bool        stopAtDest; /**< Shall scrolling be stopped if offset destination reached? */
        int16_t     offsetDest; /**< Offset destination in pixel */
        int16_t     offset;     /**< Current offset in pixel */
        uint16_t    textWidth;  /**< Text width in pixel */

        /**
         * Initializes scroll information.
         */
        ScrollInfo() :
            isEnabled(false),
            stopAtDest(false),
            offsetDest(0),
            offset(0),
            textWidth(0U)
        {
        }
    };

    String          m_formatStr;            /**< Current shown string, which contains format tags. */
    String          m_formatStrNew;         /**< New text string, which contains format tags. */
    String          m_formatStrTmp;         /**< Temporary formatted string. Used only as storage until a new text is completely taken over. */
    ScrollInfo      m_scrollInfo;           /**< Scroll information */
    ScrollInfo      m_scrollInfoNew;        /**< Scroll information for the new text. */
    bool            m_isNewTextAvailable;   /**< Is new updated text available? */
    bool            m_handleNewText;        /**< New text scroll information is determined, now it shall be handled. */
    YAGfxText       m_gfxText;              /**< Current gfx for text */
    uint32_t        m_scrollingCnt;         /**< Counts how often a text was complete scrolled. */
    int16_t         m_scrollOffset;         /**< Pixel offset of cursor x position, used for scrolling. */
    SimpleTimer     m_scrollTimer;          /**< Timer, used for scrolling */

    static KeywordHandler   m_keywordHandlers[];    /**< List of all supported keyword handlers. */
    static uint32_t         m_scrollPause;          /**< Pause in ms, between each scroll movement. */

    /**
     * Checks new text and prepares the scroll information.
     * 
     * @param[in] gfx   The graphics functionality, necessary to determine text width and etc.
     */
    void prepareNewText(YAGfx& gfx);

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
     * Handles the keyword for color changes.
     *
     * @param[in] gfx           Graphics interface, only necessary if actions shall take place.
     * @param[in] gfxText       The text to handle with the keyword.
     * @param[in] noAction      The handler shall take no action. This is only used to get rid of the keywords in the text.
     * @param[in] formatStr     String which may contain keywords.
     * @param[in] isScrolling   Is scrolling active?
     * @param[out] overstep     Number of characters, which must be overstepped before the next normal character comes.
     *
     * @return If keyword is handled successful, it returns true otherwise false.
     */
    bool handleColor(YAGfx* gfx, YAGfxText* gfxText, bool noAction, const String& formatStr, bool isScrolling, uint8_t& overstep) const;

    /**
     * Handles the keyword for alignment changes.
     *
     * @param[in] gfx           Graphics interface, only necessary if actions shall take place.
     * @param[in] gfxText       The text to handle with the keyword.
     * @param[in] noAction      The handler shall take no action. This is only used to get rid of the keywords in the text.
     * @param[in] formatStr     String which may contain keywords.
     * @param[in] isScrolling   Is scrolling active?
     * @param[out] overstep     Number of characters, which must be overstepped before the next normal character comes.
     *
     * @return If keyword is handled successful, it returns true otherwise false.
     */
    bool handleAlignment(YAGfx* gfx, YAGfxText* gfxText, bool noAction, const String& formatStr, bool isScrolling, uint8_t& overstep) const;

    /**
     * Handles the keyword for character code.
     *
     * @param[in] gfx           Graphics interface, only necessary if actions shall take place.
     * @param[in] gfxText       The text to handle with the keyword.
     * @param[in] noAction      The handler shall take no action. This is only used to get rid of the keywords in the text.
     * @param[in] formatStr     String which may contain keywords.
     * @param[in] isScrolling   Is scrolling active?
     * @param[out] overstep     Number of characters, which must be overstepped before the next normal character comes.
     *
     * @return If keyword is handled successful, it returns true otherwise false.
     */
    bool handleCharCode(YAGfx* gfx, YAGfxText* gfxText, bool noAction, const String& formatStr, bool isScrolling, uint8_t& overstep) const;
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* TEXTWIDGET_H */

/** @} */