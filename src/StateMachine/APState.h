/* MIT License
 *
 * Copyright (c) 2019 Andreas Merkle <web@blue-andi.de>
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
@brief  System state: AccessPoint
@author Andreas Merkle <web@blue-andi.de>

@section desc Description
The AccessPoint system state.

*******************************************************************************/
/** @defgroup apstate System state: AccessPoint
 * The AccessPoint system state.
 *
 * @{
 */

#ifndef __APSTATE_H__
#define __APSTATE_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <StateMachine.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * System state: AccessPoint
 */
class APState : public AbstractState
{
public:

    /**
     * Get state instance.
     * 
     * @return State instance
     */
    static APState& getInstance(void)
    {
        return m_instance;
    }

    /**
     * The entry is called once, a state is entered.
     * 
     * @param[in] sm    Responsible state machine
     */
    void entry(StateMachine& sm);

    /**
     * The process routine is called cyclic, as long as the state is active.
     * 
     * @param[in] sm    Responsible state machine
     */
    void process(StateMachine& sm);

    /**
     * The exit is called once, a state will be left.
     * 
     * @param[in] sm    Responsible state machine
     */
    void exit(StateMachine& sm);

    /** Short wait time for showing a system message in ms */
    static const uint32_t   SYS_MSG_WAIT_TIME_SHORT;

    /** Access point SSID */
    static const char*      WIFI_AP_SSID;

    /** Access point passphrase (min. 8 characters) */
    static const char*      WIFI_AP_PASSPHRASE;

private:

    /** Access point state instance */
    static APState    m_instance;

    /**
     * Constructs the state.
     */
    APState()
    {
    }

    /**
     * Destroys the state.
     */
    ~APState()
    {
    }
    
    APState(const APState& state);
    APState& operator=(const APState& state);

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __APSTATE_H__ */

/** @} */