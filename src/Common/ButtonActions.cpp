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
 * @brief  Button actions
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "ButtonActions.h"
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

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

void ButtonActions::executeAction(ButtonActionId id)
{
    switch(id)
    {
    case BUTTON_ACTION_ID_NO_ACTION:
        /* Nothing to do. */
        break;
    
    case BUTTON_ACTION_ID_ACTIVATE_NEXT_SLOT:
        nextSlot();
        break;
    
    case BUTTON_ACTION_ID_ACTIVATE_PREV_SLOT:
        previousSlot();
        break;

    case BUTTON_ACTION_ID_NEXT_FADE_EFFECT:
        nextFadeEffect();
        break;

    case BUTTON_ACTION_ID_SHOW_IP_ADDRESS:
        showIpAddress();
        break;

    case BUTTON_ACTION_ID_TOGGLE_DISPLAY_OFF_ON:
        toggleDisplayOffOn();
        break;

    case BUTTON_ACTION_ID_SWEEP_BRIGHTNESS:
        sweepBrightness();
        break;

    case BUTTON_ACTION_ID_INC_BRIGHTNESS:
        increaseBrightness();
        break;

    case BUTTON_ACTION_ID_DEC_BRIGHTNESS:
        decreaseBrightness();
        break;

    default:
        break;
    }
}

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void ButtonActions::sweepBrightness()
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

void ButtonActions::increaseBrightness()
{
    uint8_t brightness = DisplayMgr::getInstance().getBrightness();

    if ((UINT8_MAX - BRIGHTNESS_DELTA) < brightness)
    {
        brightness = UINT8_MAX;

        m_incBrightness = false;
    }
    else
    {
        brightness += BRIGHTNESS_DELTA;
    }

    DisplayMgr::getInstance().setBrightness(brightness);
}

void ButtonActions::decreaseBrightness()
{
    uint8_t brightness = DisplayMgr::getInstance().getBrightness();

    if (BRIGHTNESS_DELTA > brightness)
    {
        brightness = 0U;

        m_incBrightness = true;
    }
    else
    {
        brightness -= BRIGHTNESS_DELTA;
    }

    DisplayMgr::getInstance().setBrightness(brightness);
}

void ButtonActions::nextSlot() const
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

void ButtonActions::previousSlot() const
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

void ButtonActions::nextFadeEffect() const
{
    DisplayMgr::FadeEffect  currentFadeEffect   = DisplayMgr::getInstance().getFadeEffect();
    uint8_t                 fadeEffectId        = static_cast<uint8_t>(currentFadeEffect);
    DisplayMgr::FadeEffect  nextFadeEffect      = static_cast<DisplayMgr::FadeEffect>(fadeEffectId + 1U);

    DisplayMgr::getInstance().activateNextFadeEffect(nextFadeEffect);
}

void ButtonActions::showIpAddress() const
{
    const uint32_t  DURATION_NON_SCROLLING  = 4000U; /* ms */
    const uint32_t  SCROLLING_REPEAT_NUM    = 2U;
    String          infoStr                 = "IP: ";

    infoStr += WiFi.localIP().toString();
    SysMsg::getInstance().show(infoStr, DURATION_NON_SCROLLING, SCROLLING_REPEAT_NUM);
}

void ButtonActions::toggleDisplayOffOn()
{
    DisplayMgr& displayMgr = DisplayMgr::getInstance();

    if (false == displayMgr.isDisplayOn())
    {
        displayMgr.displayOn();
    }
    else
    {
        displayMgr.displayOff();
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
