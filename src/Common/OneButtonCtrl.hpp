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
 * @brief  One button controller
 * @author Andreas Merkle <web@blue-andi.de>
 * 
 * @addtogroup app
 *
 * @{
 */

#ifndef ONE_BUTTON_CTRL_HPP
#define ONE_BUTTON_CTRL_HPP

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "ButtonActions.h"

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Used in case only one button is available to control the application.
 * 
 * @tparam tButtonOk    Button id of the single button.
 */
template < ButtonId tButtonOk >
class OneButtonCtrl
{
protected:

    /**
     * Handles short button triggers.
     * 
     * @param[in]   buttonId    The triggered button.
     * @param[in]   triggerCnt  The number of triggers.
     * 
     * @return Returns the action which to execute.
     */
    ButtonActionId handleTriggers(ButtonId buttonId, uint32_t triggerCnt)
    {
        ButtonActionId action = BUTTON_ACTION_ID_NO_ACTION;

        if (tButtonOk == buttonId)
        {
            action = handleButtonOkTriggers(triggerCnt);
        }

        return action;
    }

    /**
     * Handles a pressed button.
     * 
     * @param[in]   buttonId    The pressed button.
     * 
     * @return Returns the action which to execute.
     */
    ButtonActionId handlePressed(ButtonId buttonId)
    {
        ButtonActionId action = BUTTON_ACTION_ID_NO_ACTION;

        if (tButtonOk == buttonId)
        {
            action = BUTTON_ACTION_ID_SWEEP_BRIGHTNESS;
        }

        return action;
    }

    /**
     * Handles short button triggers.
     * 
     * @param[in]   triggerCnt  The number of triggers.
     * 
     * @return Returns the action which to execute.
     */
    ButtonActionId handleButtonOkTriggers(uint32_t triggerCnt)
    {
        ButtonActionId          action          = BUTTON_ACTION_ID_NO_ACTION;
        const ButtonActionId    ACTION_TABLE[]  =
        {
            /* 0 */ BUTTON_ACTION_ID_NO_ACTION,
            /* 1 */ BUTTON_ACTION_ID_ACTIVATE_NEXT_SLOT,
            /* 2 */ BUTTON_ACTION_ID_ACTIVATE_PREV_SLOT,
            /* 3 */ BUTTON_ACTION_ID_NEXT_FADE_EFFECT,
            /* 4 */ BUTTON_ACTION_ID_SHOW_IP_ADDRESS,
            /* 5 */ BUTTON_ACTION_ID_TOGGLE_DISPLAY_OFF_ON
        };
        const size_t            TABLE_NUM_ELEMENTS  = sizeof(ACTION_TABLE) / sizeof(ACTION_TABLE[0]);

        if (TABLE_NUM_ELEMENTS > triggerCnt)
        {
            action = ACTION_TABLE[triggerCnt];
        }

        return action;
    }
};

/******************************************************************************
 * Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* ONE_BUTTON_CTRL_HPP */

/** @} */