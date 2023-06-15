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
#include "ButtonDrv.h"

#include <Util.h>
#include <Logging.h>
#include <Board.h>
#include <esp_sleep.h>
#include <esp_wifi.h>

#if 0
#include <esp_bt.h>
#include <esp_bt_main.h>
#endif

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

    /* Before entering light sleep mode, WiFi and BT must be disabled by using
     * appropriate calls (esp_bluedroid_disable(), esp_bt_controller_disable(), esp_wifi_stop()).
     * WiFi and BT connections will not be maintained in deep sleep or light sleep,
     * even if these functions are not called.
     */
    (void)esp_wifi_stop();
#if 0
    (void)esp_bluedroid_disable();
    (void)esp_bt_controller_disable();
#endif

    /* Stop display manager and clear the display to minimize power consumption.
     * Additional clearing will show the user that he can stop pressing the "off"
     * button.
     */
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
}

void OffState::process(StateMachine& sm)
{
    UTIL_NOT_USED(sm);

    /* Prepare wakeup sources.
     * Use all available buttons as wakeup sources.
     */
    if (true == ButtonDrv::getInstance().enableWakeUpSources())
    {
        esp_sleep_wakeup_cause_t wakeupCause = ESP_SLEEP_WAKEUP_UNDEFINED;

        while(ESP_SLEEP_WAKEUP_GPIO != wakeupCause)
        {
            /* Enter sleep mode. The function will return by wakeup. */
            if (ESP_OK != esp_light_sleep_start())
            {
                LOG_ERROR("Enter light sleep mode not possible.");
            }
            else
            {
                wakeupCause = esp_sleep_get_wakeup_cause();
            }
        }

        /* Restart the device. */
        sm.setState(RestartState::getInstance());
    }
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
