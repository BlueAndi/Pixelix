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
 * @brief  System message plugin
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "SysMsgPlugin.h"

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

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void SysMsgPlugin::start(uint16_t width, uint16_t height)
{
    PLUGIN_NOT_USED(width);

    /* Choose font. */
    m_textWidget.setFont(Fonts::getFontByType(m_fontType));

    /* The text widget is left aligned on x-axis and aligned to the center
     * of y-axis.
     */
    if (height > m_textWidget.getFont().getHeight())
    {
        uint16_t diffY = height - m_textWidget.getFont().getHeight();
        uint16_t offsY = diffY / 2U;

        m_textWidget.move(0, offsY);
    }
    
    return;
}

void SysMsgPlugin::stop()
{
    /* Nothing to do. */
    return;
}

void SysMsgPlugin::active(YAGfx& gfx)
{
    /* Nothing to do. */
    return;
}

void SysMsgPlugin::inactive()
{
    /* User selected next slot. Skip showing current information further. */
    disable();

    /* Clear information to avoid that old information is later shown again,
     * caused by scrolling feature of the underlying text widget.
     */
    m_textWidget.clear();

    return;
}

void SysMsgPlugin::update(YAGfx& gfx)
{
    bool        isScrollingEnabled  = false;
    uint32_t    scrollingCnt        = 0U;
    bool        status              = false;

    gfx.fillScreen(ColorDef::BLACK);
    m_textWidget.update(gfx);

    status = m_textWidget.getScrollInfo(isScrollingEnabled, scrollingCnt);

    /* In initialization phase? */
    if (true == m_isInit)
    {
        m_timer.stop();

        /* Is the scroll info ready? */
        if (true == status)
        {
            /* Start timer if text doesn't scroll and shall not be shown infinite. */
            if ((false == isScrollingEnabled) &&
                (0U < m_duration))
            {
                m_timer.start(m_duration);
            }

            m_isInit = false;
        }
    }
    /* Is timer running for non-scrolled text? */
    else if (true == m_timer.isTimerRunning())
    {
        /* Disable plugin after duration. */
        if (true == m_timer.isTimeout())
        {
            disable();
        }
    }
    /* Shall scrolling text be shown a specific number of times? */
    else if (0U < m_max)
    {
        /* Disable plugin after specific number of times, the text was shown. */
        if (m_max <= scrollingCnt)
        {
            disable();
        }
    }
    else
    {
        /* Show infinite */
        ;
    }

    return;
}

void SysMsgPlugin::show(const String& msg, uint32_t duration, uint32_t max)
{
    m_textWidget.setFormatStr(msg);
    m_duration  = duration;
    m_max       = max;
    m_isInit    = true;

    enable();

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
