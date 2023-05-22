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
 * @brief  System state: Connecting
 * @author Andreas Merkle <web@blue-andi.de>
 * 
 * @addtogroup sys_states
 * 
 * @{
 */

#ifndef CONNECTINGSTATE_H
#define CONNECTINGSTATE_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <StateMachine.hpp>
#include <WString.h>
#include <SimpleTimer.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * System state: Connecting
 */
class ConnectingState : public AbstractState
{
public:

    /**
     * Get state instance.
     * 
     * @return State instance
     */
    static ConnectingState& getInstance()
    {
        static ConnectingState instance; /* singleton idiom to force initialization in the first usage. */

        return instance;
    }

    /**
     * The entry is called once, a state is entered.
     * 
     * @param[in] sm    Responsible state machine
     */
    void entry(StateMachine& sm) final;

    /**
     * The process routine is called cyclic, as long as the state is active.
     * 
     * @param[in] sm    Responsible state machine
     */
    void process(StateMachine& sm) final;

    /**
     * The exit is called once, a state will be left.
     * 
     * @param[in] sm    Responsible state machine
     */
    void exit(StateMachine& sm) final;

    /** Retry delay after a failed connection attempt in ms. */
    static const uint32_t   RETRY_DELAY             = 30000U;

    /** Standard wait time for showing a system message in ms */
    static const uint32_t   SYS_MSG_WAIT_TIME_STD   = 2000U;

    /** Short wait time for showing a system message in ms */
    static const uint32_t   SYS_MSG_WAIT_TIME_SHORT = 250U;

private:

    /** Remote wifi SSID */
    String          m_wifiSSID;

    /** Remote wifi passphrase */
    String          m_wifiPassphrase;

    /** Timer, used for retry mechanism. */
    SimpleTimer     m_retryTimer;

    /** Is quiet mode active? Quiet mode no unnecessary system messages on the display. */
    bool            m_isQuiet;

    /**
     * Constructs the state.
     */
    ConnectingState() :
        m_wifiSSID(),
        m_wifiPassphrase(),
        m_retryTimer(),
        m_isQuiet(false)
    {
    }

    /**
     * Destroys the state.
     */
    ~ConnectingState()
    {
    }

    ConnectingState(const ConnectingState& state);
    ConnectingState& operator=(const ConnectingState& state);

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* CONNECTINGSTATE_H */

/** @} */