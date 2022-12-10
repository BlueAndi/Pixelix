/* MIT License
 *
 * Copyright (c) 2019 - 2022 Andreas Merkle <web@blue-andi.de>
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
            const uint32_t ACTIVATE_NEXT_SLOT   = 1U;
            const uint32_t NEXT_FADE_EFFECT     = 2U;
            const uint32_t SHOW_IP_ADDR         = 5U;

            switch(m_triggerCnt)
            {
            case ACTIVATE_NEXT_SLOT:
                DisplayMgr::getInstance().activateNextSlot();
                break;

            case NEXT_FADE_EFFECT:
                {
                    DisplayMgr::FadeEffect  currentFadeEffect   = DisplayMgr::getInstance().getFadeEffect();
                    uint8_t                 fadeEffectId        = static_cast<uint8_t>(currentFadeEffect);
                    DisplayMgr::FadeEffect  nextFadeEffect      = static_cast<DisplayMgr::FadeEffect>(fadeEffectId + 1U);

                    DisplayMgr::getInstance().activateNextFadeEffect(nextFadeEffect);
                }
                break;

            case SHOW_IP_ADDR:
                {
                    const uint32_t  DURATION_NON_SCROLLING  = 4000U; /* ms */
                    const uint32_t  SCROLLING_REPEAT_NUM    = 2U;
                    String          infoStr                 = "IP: ";

                    infoStr += WiFi.localIP().toString();
                    SysMsg::getInstance().show(infoStr, DURATION_NON_SCROLLING, SCROLLING_REPEAT_NUM);
                }
                break;

            default:
                break;
            }

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

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
