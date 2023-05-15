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

#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "ButtonDrv.h"

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
class ButtonHandler : public IButtonObserver
{
public:

    /**
     * Construct the button handler.
     */
    ButtonHandler() :
        IButtonObserver(),
        m_stateQueue(),
        m_lastButtonInfo({BUTTON_STATE_UNKNOWN, 0UL}),
        m_triggerCnt(0U),
        m_timer(),
        m_incBrightness(true)
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
    void process();

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
        ButtonState state;      /**< Button state */
        uint32_t    timestamp;  /**< Timestamp about button state reception in ms */
    };

    Queue<ButtonInfo>   m_stateQueue;       /**< Button info queue */
    ButtonInfo          m_lastButtonInfo;   /**< Last handled button info */
    uint8_t             m_triggerCnt;       /**< Number of counted button triggers (pressed -> released) */
    SimpleTimer         m_timer;            /**< Timer used to detect different pulse variants. */
    bool                m_incBrightness;    /**< If true the brightness will increase otherwise decrease. */

    /**
     * The observed button will notify about changes.
     * 
     * @param[in] state New button state
     */
    void notify(ButtonState state) override;

};

/******************************************************************************
 * Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* BUTTON_HANDLER_H */

/** @} */