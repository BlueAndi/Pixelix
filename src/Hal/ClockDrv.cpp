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
#include <esp_sntp.h>

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

extern void sntpCallback(struct timeval *tv);

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/* Initialize static constants. */
const char* ClockDrv::TZ_UTC = "UTC+0";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void ClockDrv::init(IRtc* rtc)
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

    /* Initialize RTC use its time. */
    m_rtc = rtc;

    if (nullptr != m_rtc)
    {
        /* Check whether RTC is available and initialize it. */
        if (false == m_rtc->begin())
        {
            LOG_INFO("No RTC is available.");
        }
        else
        {
            LOG_INFO("RTC is available.");
            syncTimeByRtc();
            sntp_set_time_sync_notification_cb(sntpCallback);
        }
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
}

bool ClockDrv::getTime(struct tm& timeInfo)
{
    return getTzTime(m_timeZone.c_str(), timeInfo);
}

bool ClockDrv::getUtcTime(struct tm& timeInfo)
{
    const uint32_t WAIT_TIME_MS = 0U;

    syncTimeByRtc();

    return getLocalTime(&timeInfo, WAIT_TIME_MS);
}

bool ClockDrv::getTzTime(const char* tz, struct tm& timeInfo)
{
    const uint32_t  WAIT_TIME_MS    = 0U;
    bool            result          = false;

    syncTimeByRtc();

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

    result = getLocalTime(&timeInfo, WAIT_TIME_MS);

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

void ClockDrv::setTimeByRtc()
{
    if (nullptr != m_rtc)
    {
        struct tm timeInfo;

        if (false == m_rtc->getTime(timeInfo))
        {
            time_t          timeSinceEpoch  = mktime(&timeInfo);
            struct timeval  tv              = { timeSinceEpoch, 0 };

            settimeofday(&tv, nullptr);
        }
    }
}

void ClockDrv::setRtcByTime()
{
    if (nullptr != m_rtc)
    {
        struct tm timeInfo;

        if (true == getUtcTime(timeInfo))
        {
            m_rtc->setTime(timeInfo);
        }
    }
}

void ClockDrv::syncTimeByRtc()
{
    bool sync = false;

    if (false == m_syncTimeByRtcTimer.isTimerRunning())
    {
        m_syncTimeByRtcTimer.start(SYNC_TIME_BY_RTC_PERIOD);
        sync = true;
    }
    else if (true == m_syncTimeByRtcTimer.isTimeout())
    {
        sync = true;
    }

    if (true == sync)
    {
        LOG_INFO("Sync time by RTC.");

        setTimeByRtc();
        m_syncTimeByRtcTimer.restart();
    }
}

void ClockDrv::syncRtcByTime()
{
    bool sync = false;

    if (false == m_syncRtcByNtpTimer.isTimerRunning())
    {
        m_syncRtcByNtpTimer.start(SYNC_RTC_BY_TIME_PERIOD);
        sync = true;
    }
    else if (true == m_syncRtcByNtpTimer.isTimeout())
    {
        sync = true;
    }

    if (true == sync)
    {
        LOG_INFO("Sync RTC by time.");

        setRtcByTime();
        m_syncRtcByNtpTimer.restart();
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/**
 * This function is called by the SNTP for every received time information
 * from the NTP.
 * 
 * @param[in] tv    Time information
 */
extern void sntpCallback(struct timeval *tv)
{
    ClockDrv& clockDrv = ClockDrv::getInstance();
    
    (void)tv;

    /* As long as updates from NTP are received, no synchronization from the RTC
     * to the local timer shall be done.
     */
    clockDrv.m_syncTimeByRtcTimer.restart();
    
    /* Synchronize RTC by time. */
    clockDrv.syncRtcByTime();
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/
