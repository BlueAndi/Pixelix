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
@brief  Text Widget
@author Andreas Merkle <web@blue-andi.de>

@section desc Description
This module provides the a text widget, showing a colored string.

*******************************************************************************/
/** @defgroup textwidget Text Widget
 * This module provides the a text widget, showing a colored string.
 *
 * @{
 */

#ifndef __TEXTWIDGET_HPP__
#define __TEXTWIDGET_HPP__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>

#include "IGfx.hpp"
#include "Widget.hpp"

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
     * Constructs a text widget with a empty white string.
     */
    TextWidget() :
        m_str(),
        m_textColor(0xFFFF)
    {
    }

    /**
     * Constructs a text widget with the given string and its color.
     * 
     * @param[in] str   String
     * @param[in] color Color of the string
     */
    TextWidget(const String& str, uint16_t color = 0xFFFF) :
        m_str(str),
        m_textColor(color)
    {
    }

    /**
     * Constructs a text widget by copying another one.
     * 
     * @param[in] widget Widget, which to copy
     */
    TextWidget(const TextWidget& widget) :
        m_str(widget.m_str),
        m_textColor(widget.m_textColor)
    {
    }

    /**
     * Assign the content of a text widget.
     * 
     * @param[in] widget Widget, which to assign
     */
    TextWidget& operator=(const TextWidget& widget)
    {
        m_str       = widget.m_str;
        m_textColor = widget.m_textColor;

        return *this;
    }

    /**
     * Update/Draw the text widget.
     * 
     * @param[in] gfx Graphics interface
     */
    void update(IGfx& gfx)
    {
        gfx.setCursor(m_posX, m_posY);
        gfx.setTextColor(m_textColor);
        gfx.print(m_str);

        return;
    }

    /**
     * Set the text string.
     * 
     * @param[in] str String
     */
    void setStr(const String& str)
    {
        m_str = str;
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
    void setTextColor(uint16_t color)
    {
        m_textColor = color;
        return;
    }

    /**
     * Get the text color of the string.
     * 
     * @return Text color
     */
    uint16_t getTextColor(void) const
    {
        return m_textColor;
    }

private:

    String      m_str;          /**< String */
    uint16_t    m_textColor;    /**< Text color of the string */

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __TEXTWIDGET_HPP__ */

/** @} */