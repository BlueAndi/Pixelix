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
 * @file   SysMsgPlugin.cpp
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
    m_view.init(width, height);
}

void SysMsgPlugin::stop()
{
    /* Nothing to do. */
}

void SysMsgPlugin::active(YAGfx& gfx)
{
    /* Nothing to do. */
    UTIL_NOT_USED(gfx);
}

void SysMsgPlugin::inactive()
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    /* User selected next slot. Skip showing current information further. */
    disable();

    /* Clear information to avoid that old information is later shown again,
     * caused by scrolling feature of the underlying text widget or by the
     * queued system messages.
     */
    m_view.clear();
    m_timer.stop();
    m_rdIndex = m_wrIndex;
}

void SysMsgPlugin::update(YAGfx& gfx)
{
    MutexGuard<MutexRecursive> guard(m_mutex);
    bool                       isScrollingEnabled = false;
    uint32_t                   scrollingCnt       = 0U;
    bool                       status             = false;

    m_view.update(gfx);

    status = m_view.getScrollInfo(isScrollingEnabled, scrollingCnt);

    /* In initialization phase? */
    if (true == m_isInit)
    {
        /* A hard duration ends the message independent of the scrolling. It is
         * started immediately and not only once the scroll info is available,
         * otherwise a view which never reports it could block the display.
         */
        if (true == m_isDurationHard)
        {
            if ((0U < m_duration) &&
                (false == m_timer.isTimerRunning()))
            {
                m_timer.start(m_duration);
            }
        }
        else
        {
            m_timer.stop();
        }

        /* Is the scroll info ready? */
        if (true == status)
        {
            /* Start timer if text doesn't scroll and shall not be shown infinite. */
            if ((false == m_isDurationHard) &&
                (false == isScrollingEnabled) &&
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
        /* Is the scroll info ready? */
        if (true == status)
        {
            /* Show next message after specific number of times, the text was shown. */
            if (m_max <= scrollingCnt)
            {
                /* If no message is available anymore, the plugin will be disabled. */
                if (false == nextMessage())
                {
                    disable();
                }
            }
        }
    }
    else
    {
        /* Show infinite until next message arrives. */
        (void)nextMessage();
    }
}

bool SysMsgPlugin::show(const String& msg, uint32_t duration, uint32_t max)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    return enqueue(msg, duration, max, String(), String(), 0U, false);
}

bool SysMsgPlugin::showNotification(const String& msg, uint32_t duration, const String& iconFileName, const String& melody, uint8_t volume)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    return enqueue(msg, duration, 0U, iconFileName, melody, volume, true);
}

void SysMsgPlugin::registerMelodyHandler(MelodyHandler handler)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_melodyHandler = handler;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

bool SysMsgPlugin::enqueue(const String& msg, uint32_t duration, uint32_t max, const String& iconFileName, const String& melody, uint8_t volume, bool isDurationHard)
{
    bool   isQueued     = false;
    size_t nextWrIndex  = m_wrIndex + 1U;
    nextWrIndex        %= MAX_SYS_MSG;

    /* Queue full? */
    if (nextWrIndex == m_rdIndex)
    {
        LOG_WARNING("System message queue full.");
    }
    else
    {
        m_messages[m_wrIndex].msg            = msg;
        m_messages[m_wrIndex].duration       = duration;
        m_messages[m_wrIndex].max            = max;
        m_messages[m_wrIndex].iconFileName   = iconFileName;
        m_messages[m_wrIndex].melody         = melody;
        m_messages[m_wrIndex].volume         = volume;
        m_messages[m_wrIndex].isDurationHard = isDurationHard;

        m_wrIndex                            = nextWrIndex;
        isQueued                             = true;

        /* If plugin is disabled, it will be enabled and next is shown. */
        if (false == isEnabled())
        {
            nextMessage();
            enable();
        }
    }

    return isQueued;
}

bool SysMsgPlugin::nextMessage()
{
    bool isMsgAvailable = false;

    /* Not empty? */
    if (m_rdIndex != m_wrIndex)
    {
        SysMsg& sysMsg = m_messages[m_rdIndex];

        m_view.setFormatText(sysMsg.msg);

        if (true == sysMsg.iconFileName.isEmpty())
        {
            m_view.clearIcon();
        }
        else if (false == m_view.loadIcon(sysMsg.iconFileName))
        {
            LOG_ERROR("Icon not found: %s", sysMsg.iconFileName.c_str());
        }
        else
        {
            /* Icon successful loaded. */
            ;
        }

        m_duration       = sysMsg.duration;
        m_max            = sysMsg.max;
        m_isDurationHard = sysMsg.isDurationHard;
        m_isInit         = true;

        /* An expired timer of the previous message must not end this one. */
        m_timer.stop();

        /* The melody belongs to this message and starts now, together with it.
         * Starting it already when the message is queued would play it to the
         * previous message or to a message which is never shown at all.
         */
        if ((false == sysMsg.melody.isEmpty()) &&
            (nullptr != m_melodyHandler))
        {
            m_melodyHandler(sysMsg.melody, sysMsg.volume);
        }

        /* Release the message, it is handed over to the view now. */
        sysMsg.msg.clear();
        sysMsg.iconFileName.clear();
        sysMsg.melody.clear();

        ++m_rdIndex;
        m_rdIndex      %= MAX_SYS_MSG;

        isMsgAvailable  = true;
    }

    return isMsgAvailable;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
