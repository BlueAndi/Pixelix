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
 * @brief  Clock driver
 * @author Yann Le Glaz <yann_le@web.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "ClockDrv.h"
#include "time.h"

#include <sys/time.h>
#include <Logging.h>
#include <SettingsService.h>

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

/* Initialize static constants. */
const char* ClockDrv::TZ_UTC = "UTC+0";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void ClockDrv::init()
{
    if (false == m_isClockDrvInitialized)
    {
        String              ntpServerAddress;
        struct tm           timeInfo            = { 0 };
        SettingsService&    settings            = SettingsService::getInstance();
        char                tzBuffer[TZ_MIN_SIZE];

        /* Get the GMT offset, daylight saving enabled/disabled and NTP server address from persistent memory. */
        if (false == settings.open(true))
        {
            LOG_WARNING("Use default values for NTP request.");

            m_timeZone          = settings.getTimezone().getDefault();
            ntpServerAddress    = settings.getNTPServerAddress().getDefault();
        }
        else
        {
            m_timeZone          = settings.getTimezone().getValue();
            ntpServerAddress    = settings.getNTPServerAddress().getValue();
            settings.close();
        }

        /* Workaround part 1 to avoid memory leaks by calling setenv() of the newlib.
         * https://github.com/espressif/esp-idf/issues/3046
         */
        strcpy(tzBuffer, TZ_UTC);
        fillUpWithSpaces(tzBuffer, TZ_MIN_SIZE);

        /* Configure NTP:
         * This will periodically synchronize the time. The time synchronization
         * period is determined by CONFIG_LWIP_SNTP_UPDATE_DELAY (default value is one hour).
         * To modify the variable, set CONFIG_LWIP_SNTP_UPDATE_DELAY in project configuration.
         * https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/system/system_time.html
         * https://github.com/espressif/esp-idf/issues/4386
         */
        configTzTime(tzBuffer, ntpServerAddress.c_str());

        /* Workaround part 2 to avoid memory leaks by calling setenv() of the newlib.
         * https://github.com/espressif/esp-idf/issues/3046
         */
        m_internalTimeZoneBuffer = getenv("TZ");

        /* Wait for synchronization (default 5s) */
        if (false == getLocalTime(&timeInfo))
        {
            LOG_ERROR("Failed to synchronize time.");
        }
        else
        {
            LOG_INFO("UTC: %d-%d-%d %d:%d", 
                (timeInfo.tm_year + 1900),
                (timeInfo.tm_mon + 1),
                timeInfo.tm_mday,
                timeInfo.tm_hour,
                timeInfo.tm_min);
        }

        m_isClockDrvInitialized = true;
    }
}

bool ClockDrv::getTime(tm* timeInfo)
{
    return getTzTime(m_timeZone.c_str(), timeInfo);
}

bool ClockDrv::getUtcTime(tm* timeInfo)
{
    const uint32_t WAIT_TIME_MS = 0U;

    return getLocalTime(timeInfo, WAIT_TIME_MS);
}

bool ClockDrv::getTzTime(const char* tz, tm* timeInfo)
{
    const uint32_t  WAIT_TIME_MS    = 0U;
    bool            result          = false;

    if (nullptr != tz)
    {
        /* Configure timezone */
        if (nullptr != m_internalTimeZoneBuffer)
        {
            /* Not nice, just a workaround which replaces
             * setenv("TZ", tz, 1);
             * to avoid memory leaks.
             */
            strncpy(m_internalTimeZoneBuffer, tz, TZ_MIN_SIZE - 1U);
            m_internalTimeZoneBuffer[TZ_MIN_SIZE - 1U] = '\0';

            tzset();
        }
    }

    result = getLocalTime(timeInfo, WAIT_TIME_MS);

    if (nullptr != tz)
    {
        /* Reset timezone to UTC */
        if (nullptr != m_internalTimeZoneBuffer)
        {
            /* Not nice, just a workaround which replaces
             * setenv("TZ", TZ_UTC, 1);
             * to avoid memory leaks.
             */
            strncpy(m_internalTimeZoneBuffer, TZ_UTC, TZ_MIN_SIZE - 1U);
            m_internalTimeZoneBuffer[TZ_MIN_SIZE - 1U] = '\0';

            tzset();
        }
    }

    return result;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void ClockDrv::fillUpWithSpaces(char* str, size_t size)
{
    size_t idx          = strlen(str);
    size_t maxLength    = size - 1U;

    while(maxLength > idx)
    {
        str[idx] = ' ';
        ++idx;
    }

    str[size - 1U] = '\0';
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

