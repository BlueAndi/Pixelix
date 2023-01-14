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
#include <Settings.h>

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
            m_timeFormat        = Settings::getInstance().getTimeFormat().getDefault();
            m_dateFormat        = Settings::getInstance().getDateFormat().getDefault();
        }
        else
        {
            timezone            = Settings::getInstance().getTimezone().getValue();
            ntpServerAddress    = Settings::getInstance().getNTPServerAddress().getValue();
            m_timeFormat        = Settings::getInstance().getTimeFormat().getValue();
            m_dateFormat        = Settings::getInstance().getDateFormat().getValue();
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
            LOG_INFO("Local time: %d-%d-%d %d:%d", 
                (timeInfo.tm_year + 1900),
                (timeInfo.tm_mon + 1),
                timeInfo.tm_mday,
                timeInfo.tm_hour,
                timeInfo.tm_min);
        }

        m_isClockDrvInitialized = true;
    }
}

bool ClockDrv::getTime(tm *currentTime)
{
    const uint32_t WAIT_TIME_MS = 0;

    return getLocalTime(currentTime, WAIT_TIME_MS);
}

bool ClockDrv::getTimeAsString(String& time, const String& format, const tm *currentTime)
{
    bool        isSuccessful    = false;
    tm          timeStruct;
    const tm*   timeStructPtr   = nullptr;

    if (nullptr == currentTime)
    {
        timeStructPtr = &timeStruct;

        if (false == getTime(&timeStruct))
        {
            timeStructPtr = nullptr;
        }
    }
    else
    {
        timeStructPtr = currentTime;
    }

    if (nullptr != timeStructPtr)
    {
        const uint32_t  MAX_TIME_BUFFER_SIZE    = format.length() + 20U;
        char            buffer[MAX_TIME_BUFFER_SIZE];

        if (0U != strftime(buffer, sizeof(buffer), format.c_str(), currentTime))
        {
            time = buffer;
            isSuccessful = true;
        }
    }

    return isSuccessful;
}

const String& ClockDrv::getTimeFormat()
{
    return m_timeFormat;
}

const String& ClockDrv::getDateFormat()
{
    return m_dateFormat;
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

