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
 * @brief  System state: Connected
 * @author Andreas Merkle <web@blue-andi.de>
 * 
 * @addtogroup sys_states
 * 
 * @{
 */

#ifndef CONNECTEDSTATE_H
#define CONNECTEDSTATE_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "AsyncHttpClient.h"

#include <stdint.h>
#include <StateMachine.hpp>
#include <WString.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * System state: Connected
 */
class ConnectedState : public AbstractState
{
public:

    /**
     * Get state instance.
     * 
     * @return State instance
     */
    static ConnectedState& getInstance()
    {
        static ConnectedState instance; /* singleton idiom to force initialization in the first usage. */

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

private:

    AsyncHttpClient m_client; /**< Asynchronous HTTP client. */

    /**
     * Constructs the state.
     */
    ConnectedState():
        m_client()
    {
        initHttpClient();
    }

    /**
     * Destroys the state.
     */
    ~ConnectedState()
    {
    }

    ConnectedState(const ConnectedState& state);
    ConnectedState& operator=(const ConnectedState& state);

    /**
     * Register callback function on response reception.
     */
    void initHttpClient(void);
    
    /**
     * Notify via URL that the system is online.
     * 
     * @param[in] pushUrl   Push URL
     */
    void pushUrl(const String& pushUrl);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* CONNECTEDSTATE_H */

/** @} */