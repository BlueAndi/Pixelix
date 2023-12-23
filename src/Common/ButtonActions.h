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
 * 
 * @addtogroup app
 *
 * @{
 */

#ifndef BUTTON_ACTIONS_H
#define BUTTON_ACTIONS_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>

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
 * Defines the possible actions, triggered by a button.
 */
enum ButtonActionId
{
    BUTTON_ACTION_ID_NO_ACTION = 0,         /**< No action */
    BUTTON_ACTION_ID_ACTIVATE_NEXT_SLOT,    /**< Activate next slot */
    BUTTON_ACTION_ID_ACTIVATE_PREV_SLOT,    /**< Activate previous slot */
    BUTTON_ACTION_ID_NEXT_FADE_EFFECT,      /**< Select next fade effect */
    BUTTON_ACTION_ID_SHOW_IP_ADDRESS,       /**< Show IP address on display */
    BUTTON_ACTION_ID_TOGGLE_DISPLAY_OFF_ON, /**< Toggle the display off/on */
    BUTTON_ACTION_ID_SWEEP_BRIGHTNESS,      /**< Sweep brightness from dark to bright and reverse */
    BUTTON_ACTION_ID_INC_BRIGHTNESS,        /**< Increase display brightness till maximum. */
    BUTTON_ACTION_ID_DEC_BRIGHTNESS,        /**< Decrease display brightness till minimum. */
    
    BUTTON_ACTION_ID_MAX                    /**< Max. action id (always the last one) */
};

/**
 * This is a collection of all possible button actions.
 * It is independed of the number of buttons.
 */
class ButtonActions
{
protected:

    /**
     * Construct the button actions.
     */
    ButtonActions() :
        m_incBrightness(true),
        m_isSwitchOffRequested(false)
    {
    }

    /**
     * Destroys the button actions.
     * 
     */
    ~ButtonActions()
    {
    }

    /**
     * Execute action by button action id.
     * 
     * @param[in] id    Button action id
     */
    void executeAction(ButtonActionId id);

    /**
     * Is switch off requested?
     * 
     * @return If switch off is requested, it will return true otherwise false.
     */
    bool isSwitchOffRequested()
    {
        bool isSwitchOffRequested = m_isSwitchOffRequested;

        m_isSwitchOffRequested = false;

        return isSwitchOffRequested;
    }

private:

    static const uint8_t    BRIGHTNESS_DELTA    = 10U;

    bool    m_incBrightness;        /**< If true the brightness will increase otherwise decrease. */
    bool    m_isSwitchOffRequested; /**< Is switch off requested? */

    /**
     * Sweep brightness from dark to bright and then the other way around.
     */
    void sweepBrightness();

    /**
     * Increase brightness until maximum.
     * Every call is one step to brightness.
     */
    void increaseBrightness();

    /**
     * Decreaes brightness until minimum.
     * Every call is one step to darkness.
     */
    void decreaseBrightness();

    /**
     * Trigger action: Activate next slot.
     */
    void nextSlot() const;

    /**
     * Trigger action: Activate previous slot.
     */
    void previousSlot() const;

    /**
     * Trigger action: Select next fade effect.
     */
    void nextFadeEffect() const;

    /**
     * Trigger action: Show IP address on display.
     */
    void showIpAddress() const;

    /**
     * Trigger action: Toggle display on/off.
     */
    void toggleDisplayOffOn();
};

/******************************************************************************
 * Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* BUTTON_ACTIONS_H */

/** @} */