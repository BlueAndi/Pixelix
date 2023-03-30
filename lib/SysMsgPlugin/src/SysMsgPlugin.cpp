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
}

void SysMsgPlugin::stop()
{
    /* Nothing to do. */
}

void SysMsgPlugin::active(YAGfx& gfx)
{
    /* Nothing to do. */
}

void SysMsgPlugin::inactive()
{
    /* User selected next slot. Skip showing current information further. */
    disable();

    /* Clear information to avoid that old information is later shown again,
     * caused by scrolling feature of the underlying text widget or by the
     * queued system messages.
     */
    m_textWidget.clear();
    m_rdIndex = m_wrIndex;
}

void SysMsgPlugin::update(YAGfx& gfx)
{
    bool        isScrollingEnabled  = false;
    uint32_t    scrollingCnt        = 0U;
    bool        status              = false;

    gfx.fillScreen(ColorDef::BLACK);
    m_textWidget.update(gfx);

    if (true == m_isSignalEnabled)
    {
        int16_t xMax = static_cast<int16_t>(gfx.getWidth()) - 1;
        int16_t yMax = static_cast<int16_t>(gfx.getHeight()) - 1;

        gfx.drawPixel(0, 0, ColorDef::YELLOW);
        gfx.drawPixel(0, yMax, ColorDef::YELLOW);
        gfx.drawPixel(xMax, 0, ColorDef::YELLOW);
        gfx.drawPixel(xMax, yMax, ColorDef::YELLOW);
    }

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
        if (true == m_timer.isTimeout())
        {
            /* If no message is available anymore, the plugin will be disabled. */
            if (false == nextMessage())
            {
                disable();
            }
        }
    }
    /* Shall scrolling text be shown a specific number of times? */
    else if (0U < m_max)
    {
        /* Show next message after specific number of times, the text was shown. */
        if (m_max < scrollingCnt)
        {
            /* If no message is available anymore, the plugin will be disabled. */
            if (false == nextMessage())
            {
                disable();
            }
        }
    }
    else
    {
        /* Show infinite until next message arrives. */
        (void)nextMessage();
    }
}

void SysMsgPlugin::show(const String& msg, uint32_t duration, uint32_t max)
{
    size_t nextWrIndex = m_wrIndex + 1U;
    nextWrIndex %= MAX_SYS_MSG;

    /* Queue full? */
    if (nextWrIndex == m_rdIndex)
    {
        LOG_WARNING("System message queue full.");
    }
    else
    {
        m_messages[m_wrIndex].msg       = msg;
        m_messages[m_wrIndex].duration  = duration;
        m_messages[m_wrIndex].max       = max;

        m_wrIndex = nextWrIndex;

        /* If plugin is disabled, it will be enabled and next is shown. */
        if (false == isEnabled())
        {
            nextMessage();
            enable();
        }
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

bool SysMsgPlugin::nextMessage()
{
    bool isMsgAvailable = false;

    /* Not empty? */
    if (m_rdIndex != m_wrIndex)
    {
        SysMsg& sysMsg = m_messages[m_rdIndex];

        m_textWidget.setFormatStr(sysMsg.msg);
        m_duration  = sysMsg.duration;
        m_max       = sysMsg.max;
        m_isInit    = true;

        ++m_rdIndex;
        m_rdIndex %= MAX_SYS_MSG;

        isMsgAvailable = true;
    }

    return isMsgAvailable;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
