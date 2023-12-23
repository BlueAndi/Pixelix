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
 * @brief  System state: Init
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup sys_states
 *
 * @{
 */

#ifndef INITSTATE_H
#define INITSTATE_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <StateMachine.hpp>
#include <IPluginMaintenance.hpp>
#include <SimpleTimer.hpp>

#if CONFIG_RTC == 1
#include "Rtc1307Drv.h"
#else /* CONFIG_RTC == 1 */
#include "RtcNoneDrv.h"
#endif /* CONFIG_RTC == 1 */


/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Initialization state:
 * - Initializes the board.
 * - Check for user button press during start up.
 */
class InitState : public AbstractState
{
public:

    /**
     * Get state instance.
     *
     * @return State instance
     */
    static InitState& getInstance()
    {
        static InitState instance; /* singleton idiom to force initialization in the first usage. */

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

    /**
     * How long shall the logo be shown in ms.
     * As long as it is shown, stay in this state!
     */
    const uint32_t SHOW_LOGO_DURATION   = 2000U;

    bool        m_isQuiet;              /**< Is quite mode active? */
    bool        m_isApModeRequested;    /**< Is wifi AP mode requested? */
    SimpleTimer m_timer;                /**< Timer used to stay for a min. time in this state. */

#if CONFIG_RTC == 1
    Rtc1307Drv  m_rtcDrv;               /**< RTC driver */
#else /* CONFIG_RTC == 1 */
    RtcNoneDrv  m_rtcDrv;               /**< RTC driver without functionality. */
#endif /* CONFIG_RTC == 1 */

    /**
     * Constructs the state.
     */
    InitState() :
        m_isQuiet(false),
        m_isApModeRequested(false),
        m_timer(),
        m_rtcDrv()
    {
    }

    /**
     * Destroys the state.
     */
    ~InitState()
    {
    }

    InitState(const InitState& state);
    InitState& operator=(const InitState& state);

    /**
     * Show startup information on the serial interface.
     */
    void showStartupInfoOnSerial(void);

    /**
     * Show startup information on the display.
     */
    void showStartupInfoOnDisplay(bool isQuietEnabled);

    /**
     * Welcome the user on the very first start.
     * 
     * @param[in] plugin    The welcome plugin. If nullptr is given, the welcome
     *                      plugin will be created and installed.
     */
    void welcome(IPluginMaintenance* plugin);

    /**
     * Checks whether the filesystem content is compatible to the Pixelix version.
     * 
     * @return If filesystem content is compatible, it will return true otherwise false.
     */
    bool isFsCompatible();
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* INITSTATE_H */

/** @} */