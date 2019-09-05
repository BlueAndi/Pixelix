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
@see TextWidget.h

*******************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "TextWidget.h"

#include <Fonts/TomThumb.h>

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

/* Initialize default text color (white) */
const Color     TextWidget::DEFAULT_TEXT_COLOR  = 0xffffu;

/* Initialize text widget type. */
const char*     TextWidget::WIDGET_TYPE         = "text";

/* Initialize default font */
const GFXfont*  TextWidget::DEFAULT_FONT        = &TomThumb;

/* Initialize default scrolling pause between two characters. */
const uint32_t  TextWidget::DEFAULT_SCOLL_PAUSE = 200u;

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void TextWidget::update(Adafruit_GFX& gfx)
{
    const int16_t   CURSOR_X    = 0;
    const int16_t   CURSOR_Y    = 0;

    /* Set base parameters */
    gfx.setFont(m_font);
    gfx.setTextColor(m_textColor.get565());
    gfx.setCursor(CURSOR_X, CURSOR_Y);
    gfx.setTextWrap(false); /* If text is too long, don't wrap around. */

    /* Check for scrolling necessary? */
    if (true == m_checkScrollingNeed)
    {
        int16_t     boundaryX       = 0;
        int16_t     boundaryY       = 0;
        uint16_t    textWidth       = 0u;
        uint16_t    textHeight      = 0u;

        gfx.getTextBounds(m_str, CURSOR_X, CURSOR_Y, &boundaryX, &boundaryY, &textWidth, &textHeight);

        /* Text too long for the display? */
        if (gfx.width() < textWidth)
        {
            m_isScrollingEnabled    = true;
            m_scrollIndex           = 0u;
            m_scrollTimer.start(0u);    /* Ensure immediate update */
        }
        else
        {
            m_isScrollingEnabled    = false;
            m_scrollIndex           = 0u;
            m_scrollTimer.stop();
        }
    }

    /* If text width is lower or equal than the display width, no scrolling is necessary. */
    if (false == m_isScrollingEnabled)
    {
        gfx.print(m_str);
    }
    /* Scrolling is necessary.
     * Between two characters shall be a short delay, to give the user a chance to read it. ;-)
     */
    else if (true == m_scrollTimer.isTimeout())
    {
        gfx.print(&m_str.c_str()[m_scrollIndex]);
        
        /* Text scrolls completly out, until it starts from the beginning again. */
        ++m_scrollIndex;
        if (m_str.length() <= m_scrollIndex)
        {
            m_scrollIndex = 0;
        }

        m_scrollTimer.start(DEFAULT_SCOLL_PAUSE);
    }
    else
    {
        /* Nothing to do. */
        ;
    }
    
    return;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
