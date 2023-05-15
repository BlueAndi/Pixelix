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
 * @brief  System state: AP
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup sys_states
 *
 * @{
 */

#ifndef APSTATE_H
#define APSTATE_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <StateMachine.hpp>
#include <IPAddress.h>
#include <DNSServer.h>

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
    static APState& getInstance()
    {
        static APState instance; /* singleton idiom to force initialization in the first usage. */

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

    /**
     * Minimum length of the passphrase. Don't change it, because
     * it depends on the lower layer.
     */
    static const uint8_t    WIFI_AP_PASSPHRASE_MIN_LEN;

    /** Access point local address */
    static const IPAddress  LOCAL_IP;

    /** Access point gateway address */
    static const IPAddress  GATEWAY;

    /** Access point subnet mask */
    static const IPAddress  SUBNET;

    /** DNS port */
    static const uint16_t   DNS_PORT;

private:

    DNSServer   m_dnsServer;    /**< DNS server, used for captive portal. */

    /**
     * Constructs the state.
     */
    APState() :
        m_dnsServer()
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

#endif  /* APSTATE_H */

/** @} */