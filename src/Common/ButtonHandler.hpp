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
 * 
 * @addtogroup app
 *
 * @{
 */

#ifndef BUTTON_HANDLER_HPP
#define BUTTON_HANDLER_HPP

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "ButtonDrv.h"
#include "ButtonActions.h"

#include <Queue.hpp>
#include <SimpleTimer.hpp>

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
 * The button handler executes functions depended on the button state changes.
 * 
 * One short button pulse : Activate next slot
 * Two short button pulses: Activate next fade effect
 * Keep pressed           : Display brightness increases/decreases
 */
template < typename ButtonCtrl >
class ButtonHandler : public IButtonObserver, private ButtonCtrl, private ButtonActions
{
public:

    /**
     * Construct the button handler.
     */
    ButtonHandler() :
        IButtonObserver(),
        ButtonCtrl(),
        ButtonActions(),
        m_stateQueue(),
        m_lastButtonInfo(),
        m_triggerCnt{0U},
        m_timer()
    {
        (void)m_stateQueue.create(STATE_QUEUE_LENGTH);
    }

    /**
     * Destroy the button handler.
     * 
     */
    ~ButtonHandler()
    {
        m_stateQueue.destroy();
    }

    /**
     * The button handler shall be processed periodically.
     */
    void process()
    {
        ButtonInfo  info;
        uint8_t     buttonIdx   = 0U;

        /* A new button state change received? */
        if (true == m_stateQueue.receive(&info, 0U))
        {
            /* Button id shall be valid. */
            if (BUTTON_ID_CNT > info.buttonId)
            {
                bool skipButtonInfo = false;

                /* Skip not connected button. */
                if (BUTTON_STATE_NC != info.state)
                {
                    /* A button shall be released the very first time. Otherwise
                     * all received states are ignored.
                     */
                    if (BUTTON_STATE_NC == m_lastButtonInfo[info.buttonId].state)
                    {
                        if (BUTTON_STATE_RELEASED != info.state)
                        {
                            skipButtonInfo = true;
                        }
                    }
                    /* Button changed from pressed to release state? */
                    else if ((BUTTON_STATE_PRESSED == m_lastButtonInfo[info.buttonId].state) &&
                             (BUTTON_STATE_RELEASED == info.state))
                    {
                        uint32_t delta = info.timestamp - m_lastButtonInfo[info.buttonId].timestamp;

                        /* Short pulse detected? */
                        if (SHORT_PULSE_THRESHOLD > delta)
                        {
                            /* Count the short pulse */
                            ++m_triggerCnt[info.buttonId];
                        }

                        /* Reset the counted short pulses always after a long released state. */
                        if ((true == m_timer[info.buttonId].isTimerRunning()) &&
                            (true == m_timer[info.buttonId].isTimeout()))
                        {
                            m_triggerCnt[info.buttonId] = 0U;
                        }

                        m_timer[info.buttonId].start(SHORT_PULSE_THRESHOLD);
                    }
                    /* If button is pressed, the short pulse detection timer will be started. */
                    else if (BUTTON_STATE_PRESSED == info.state)
                    {
                        m_timer[info.buttonId].start(SHORT_PULSE_THRESHOLD);
                    }
                    else
                    {
                        ;
                    }
                }

                if (false == skipButtonInfo)
                {
                    m_lastButtonInfo[info.buttonId] = info;
                }
            }
        }
        
        while(BUTTON_ID_CNT > buttonIdx)
        {
            /* Skip not connected buttons. */
            if (BUTTON_STATE_NC != m_lastButtonInfo[buttonIdx].state)
            {
                /* After short pulse timer timeout, perform the action. */
                if ((true == m_timer[buttonIdx].isTimerRunning()) &&
                    (true == m_timer[buttonIdx].isTimeout()))
                {
                    if (BUTTON_STATE_RELEASED == m_lastButtonInfo[buttonIdx].state)
                    {
                        handleTriggers(static_cast<ButtonId>(buttonIdx), m_triggerCnt[buttonIdx]);
                        m_triggerCnt[buttonIdx] = 0U;
                    }
                    else if (BUTTON_STATE_PRESSED == m_lastButtonInfo[buttonIdx].state)
                    {
                        handlePressed(static_cast<ButtonId>(buttonIdx));
                    }
                    else
                    {
                        ;
                    }
                }
            }

            ++buttonIdx;
        }
    }

private:

    /** Length of the button info queue. */
    static const size_t     STATE_QUEUE_LENGTH      = 10U;

    /** Short pulse threshold in ms. */
    static const uint32_t   SHORT_PULSE_THRESHOLD   = 400U;

    /**
     * The button information combines the new button state with a
     * absolute timestamp about its reception.
     */
    struct ButtonInfo
    {
        ButtonId    buttonId;   /**< The id of the button. */
        ButtonState state;      /**< Button state. */
        uint32_t    timestamp;  /**< Timestamp about button state reception in ms. */

        ButtonInfo() :
            buttonId(BUTTON_ID_CNT),
            state(BUTTON_STATE_NC),
            timestamp(0U)
        {
        }
    };

    Queue<ButtonInfo>   m_stateQueue;                       /**< Button info queue */
    ButtonInfo          m_lastButtonInfo[BUTTON_ID_CNT];    /**< Last handled button info */
    uint8_t             m_triggerCnt[BUTTON_ID_CNT];        /**< Number of counted button triggers (pressed -> released) */
    SimpleTimer         m_timer[BUTTON_ID_CNT];             /**< Timer used to detect different pulse variants. */

    /**
     * The observed button will notify about changes.
     * 
     * @param[in] buttonId  The id of the button.
     * @param[in] state     New button state.
     */
    void notify(ButtonId buttonId, ButtonState state) override
    {
        ButtonInfo  info;

        info.buttonId   = buttonId;
        info.state      = state;
        info.timestamp  = millis();

        (void)m_stateQueue.sendToBack(info, portMAX_DELAY);
    }

    /**
     * Handle button triggers and executing the corresponding action.
     * 
     * @param[in] buttonId      The button id.
     * @param[in] triggerCnt    Number of button triggers.
     */
    void handleTriggers(ButtonId buttonId, uint32_t triggerCnt)
    {
        ButtonActionId action = ButtonCtrl::handleTriggers(buttonId, triggerCnt);
        executeAction(action);
    }

    /**
     * Handle button pressed and executing the corresponding action.
     * 
     * @param buttonId  The button id.
     */
    void handlePressed(ButtonId buttonId)
    {
        ButtonActionId action = ButtonCtrl::handlePressed(buttonId);
        executeAction(action);
    }
};

/******************************************************************************
 * Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* BUTTON_HANDLER_HPP */

/** @} */