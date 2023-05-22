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
 * @brief  System state: Off
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "OffState.h"
#include "RestartState.h"
#include "DisplayMgr.h"
#include "Display.h"

#include <Util.h>
#include <Logging.h>
#include <Board.h>
#include <esp_sleep.h>

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

void OffState::entry(StateMachine& sm)
{
    UTIL_NOT_USED(sm);

    LOG_INFO("Going in off state.");

    /* Prepare wakeup sources */
    gpio_wakeup_enable(static_cast<gpio_num_t>(Board::Pin::userButtonPinNo), GPIO_INTR_LOW_LEVEL);
    esp_sleep_enable_gpio_wakeup();
}

void OffState::process(StateMachine& sm)
{
    UTIL_NOT_USED(sm);

    /* Stop display manager and clear the display to minimize power consumption. */
    DisplayMgr::getInstance().end();
    Display::getInstance().clear();
    Display::getInstance().show();

    /* Wait until the LED matrix is updated to avoid artifacts on the
     * display.
     */
    while(false == Display::getInstance().isReady())
    {
        /* Just wait and give other tasks a chance. */
        delay(1U);
    }

    /* Enter sleep mode. The function will return by wakeup. */
    esp_light_sleep_start();

    /* Restart the device. */
    sm.setState(RestartState::getInstance());
}

void OffState::exit(StateMachine& sm)
{
    UTIL_NOT_USED(sm);

    /* Nothing to do. */
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
