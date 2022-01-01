/* MIT License
 *
 * Copyright (c) 2019 - 2022 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Clock driver
 * @author Yann Le Glaz <yann_le@web.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "ClockDrv.h"
#include "Settings.h"
#include "time.h"

#include <sys/time.h>
#include <Logging.h>

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

void ClockDrv::init()
{
    if (false == m_isClockDrvInitialized)
    {
        String      timezone;
        String      ntpServerAddress;
        struct tm   timeInfo            = { 0 };

        /* Get the GMT offset, daylight saving enabled/disabled and NTP server address from persistent memory. */
        if (false == Settings::getInstance().open(true))
        {
            LOG_WARNING("Use default values for NTP request.");

            timezone            = Settings::getInstance().getTimezone().getDefault();
            ntpServerAddress    = Settings::getInstance().getNTPServerAddress().getDefault();
            m_is24HourFormat    = Settings::getInstance().getTimeFormatAdjustment().getDefault();
            m_isDayMonthYear    = Settings::getInstance().getDateFormatAdjustment().getDefault();
        }
        else
        {
            timezone            = Settings::getInstance().getTimezone().getValue();
            ntpServerAddress    = Settings::getInstance().getNTPServerAddress().getValue();
            m_is24HourFormat    = Settings::getInstance().getTimeFormatAdjustment().getValue();
            m_isDayMonthYear    = Settings::getInstance().getDateFormatAdjustment().getValue();
            Settings::getInstance().close();
        }

        /* Configure NTP:
         * This will periodically synchronize the time. The time synchronization
         * period is determined by CONFIG_LWIP_SNTP_UPDATE_DELAY (default value is one hour).
         * To modify the variable, set CONFIG_LWIP_SNTP_UPDATE_DELAY in project configuration.
         * https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/system/system_time.html
         * https://github.com/espressif/esp-idf/issues/4386
         */
        configTzTime(timezone.c_str(), ntpServerAddress.c_str());

        /* Wait for synchronization (default 5s) */
        if (false == getLocalTime(&timeInfo))
        {
            LOG_ERROR("Failed to synchronize time.");
        }
        else
        {
            LOG_INFO("Time successfully synchronized: %d:%d", timeInfo.tm_hour, timeInfo.tm_min);
        }

        m_isClockDrvInitialized = true;
    }
}

bool ClockDrv::getTime(tm *currentTime)
{
    const uint32_t WAIT_TIME_MS = 0;

    return getLocalTime(currentTime, WAIT_TIME_MS);
}

bool ClockDrv::getTimeFormat()
{
    return m_is24HourFormat;
}

bool ClockDrv::getDateFormat()
{
    return m_isDayMonthYear;
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

