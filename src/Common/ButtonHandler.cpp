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
 * @brief  Button handler
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "ButtonHandler.h"
#include "DisplayMgr.h"
#include "SysMsg.h"

#include <WiFi.h>

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

void ButtonHandler::process()
{
    ButtonInfo info;

    /* A new button state change received? */
    if (true == m_stateQueue.receive(&info, 0U))
    {
        uint32_t delta = info.timestamp - m_lastButtonInfo.timestamp;

        /* Button changed from pressed to release state? */
        if ((BUTTON_STATE_PRESSED == m_lastButtonInfo.state) &&
            (BUTTON_STATE_RELEASED == info.state))
        {
            /* Short pulse detected? */
            if (SHORT_PULSE_THRESHOLD > delta)
            {
                /* Count the short pulse */
                ++m_triggerCnt;
            }

            /* Reset the counted short pulses always after a long released state. */
            if ((true == m_timer.isTimerRunning()) &&
                (true == m_timer.isTimeout()))
            {
                m_triggerCnt = 0U;
            }

            m_timer.start(SHORT_PULSE_THRESHOLD);
        }
        /* If button is pressed, the short pulse detection timer will be started. */
        else if (BUTTON_STATE_PRESSED == info.state)
        {
            m_timer.start(SHORT_PULSE_THRESHOLD);
        }
        else
        {
            ;
        }

        m_lastButtonInfo = info;
    }

    /* After short pulse timer timeout, perform the action. */
    if ((true == m_timer.isTimerRunning()) &&
        (true == m_timer.isTimeout()))
    {
        if (BUTTON_STATE_RELEASED == m_lastButtonInfo.state)
        {
            handleTriggers(m_triggerCnt);
            m_triggerCnt = 0U;
        }
        else if (BUTTON_STATE_PRESSED == m_lastButtonInfo.state)
        {
            const uint8_t BRIGHTNESS_DELTA = 10U;

            if (0U == m_triggerCnt)
            {
                uint8_t brightness = DisplayMgr::getInstance().getBrightness();

                if (false == m_incBrightness)
                {
                    if (BRIGHTNESS_DELTA > brightness)
                    {
                        brightness = 0U;

                        m_incBrightness = true;
                    }
                    else
                    {
                        brightness -= BRIGHTNESS_DELTA;
                    }
                }
                else
                {
                    if ((UINT8_MAX - BRIGHTNESS_DELTA) < brightness)
                    {
                        brightness = UINT8_MAX;

                        m_incBrightness = false;
                    }
                    else
                    {
                        brightness += BRIGHTNESS_DELTA;
                    }
                }

                DisplayMgr::getInstance().setBrightness(brightness);
            }
        }
        else
        {
            ;
        }
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void ButtonHandler::notify(ButtonState state)
{
    ButtonInfo  info;

    info.state      = state;
    info.timestamp  = millis();

    (void)m_stateQueue.sendToBack(info, portMAX_DELAY);
}

void ButtonHandler::handleTriggers(uint32_t triggerCnt)
{
    switch(triggerCnt)
    {
    case TRIGGER_ACTION_ACTIVATE_NEXT_SLOT:
        nextSlot();
        break;
    
    case TRIGGER_ACTION_ACTIVATE_PREV_SLOT:
        previousSlot();
        break;

    case TRIGGER_ACTION_NEXT_FADE_EFFECT:
        nextFadeEffect();
        break;

    case TRIGGER_ACTION_SHOW_IP_ADDRESS:
        showIpAddress();
        break;

    case TRIGGER_ACTION_SWITCH_OFF:
        switchOff();
        break;

    default:
        break;
    }
}

void ButtonHandler::nextSlot() const
{
    /* If a system message is active shown, the next one shall be shown.
     * Otherwise activate the next slot.
     */
    if (true == SysMsg::getInstance().isActive())
    {
        SysMsg::getInstance().next();
    }
    else
    {
        DisplayMgr::getInstance().activateNextSlot();
    }
}

void ButtonHandler::previousSlot() const
{
    /* If a system message is active shown, the next one shall be shown.
     * Otherwise activate the previous slot.
     */
    if (true == SysMsg::getInstance().isActive())
    {
        SysMsg::getInstance().next();
    }
    else
    {
        DisplayMgr::getInstance().activatePreviousSlot();
    }
}

void ButtonHandler::nextFadeEffect() const
{
    DisplayMgr::FadeEffect  currentFadeEffect   = DisplayMgr::getInstance().getFadeEffect();
    uint8_t                 fadeEffectId        = static_cast<uint8_t>(currentFadeEffect);
    DisplayMgr::FadeEffect  nextFadeEffect      = static_cast<DisplayMgr::FadeEffect>(fadeEffectId + 1U);

    DisplayMgr::getInstance().activateNextFadeEffect(nextFadeEffect);
}

void ButtonHandler::showIpAddress() const
{
    const uint32_t  DURATION_NON_SCROLLING  = 4000U; /* ms */
    const uint32_t  SCROLLING_REPEAT_NUM    = 2U;
    String          infoStr                 = "IP: ";

    infoStr += WiFi.localIP().toString();
    SysMsg::getInstance().show(infoStr, DURATION_NON_SCROLLING, SCROLLING_REPEAT_NUM);
}

void ButtonHandler::switchOff()
{
    m_isSwitchOffRequested = true;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
