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
 *
 * @addtogroup Hal
 *
 * @{
 */

#ifndef CLOCKDRV_H
#define CLOCKDRV_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Arduino.h"
#include <IRtc.h>

#include <SimpleTimer.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Clock driver.
 */
class ClockDrv
{
public:

    /**
     *  Get the ClockDrv instance.
     *
     * @return ClockDrv instance.
     */
    static ClockDrv& getInstance()
    {
        static ClockDrv instance; /* singleton idiom to force initialization in the first usage. */

        return instance;
    }

    /**
     * Initialize the ClockDrv.
     * If no RTC is available, use nullptr for the rtc parameter.
     *
     * @param[in] rtc   Real time clock driver
     */
    void init(IRtc* rtc);

    /**
     * Get the local time by considering device timezone.
     *
     * @param[out] timeInfo Time information.
     *
     * @return If time is not synchronized, it will return false otherwise true.
     */
    bool getTime(struct tm& timeInfo);

    /**
     * Get the current time in UTC.
     *
     * @param[out] timeInfo Time information.
     *
     * @return If time is not synchronized, it will return false otherwise true.
     */
    bool getUtcTime(struct tm& timeInfo);

    /**
     * Get the local time by considering the timezone.
     * 
     * @param[in]   tz          Timzone string
     * @param[out]  timeInfo    Local time information
     * 
     * @return If time is not synchronized, it will return false otherwise true.
     */
    bool getTzTime(const char* tz, struct tm& timeInfo);

private:

    /**
     * The minimum timezone string size (incl. string termination).
     */
    static const size_t     TZ_MIN_SIZE             = 60U;

    /**
     * Use UTC timezone by default.
     */
    static const char*      TZ_UTC;

    /**
     * Period for time synchronization by NTP in ms.
     */
    static const uint32_t   SYNC_TIME_BY_NTP_PERIOD = SIMPLE_TIMER_HOURS(12U);

    /**
     * Period for time synchronization by RTC in ms.
     */
    static const int32_t    SYNC_TIME_BY_RTC_PERIOD = SIMPLE_TIMER_HOURS(1U);

    /**
     * Period for RTC synchronization by time in ms.
     */
    static const uint32_t   SYNC_RTC_BY_TIME_PERIOD = SIMPLE_TIMER_DAYS(2U);

    /** Flag indicating a initialized clock driver. */
    bool        m_isClockDrvInitialized;

    /** Device timezone */
    String      m_timeZone;

    /** newlib's internal timezone buffer. */
    char*       m_internalTimeZoneBuffer;

    /** NTP server address, used by sntp. Don't remove it! */
    String      m_ntpServerAddress;

    /** Real time clock */
    IRtc*       m_rtc;

    /** Timer used to synchronize the time by the RTC. */
    SimpleTimer m_syncTimeByRtcTimer;

    /** Timer used to synchronize the RTC by the time. */
    SimpleTimer m_syncRtcByNtpTimer;

    /**
     * Construct ClockDrv.
     */
    ClockDrv() :
        m_isClockDrvInitialized(false),
        m_timeZone(TZ_UTC),
        m_internalTimeZoneBuffer(nullptr),
        m_ntpServerAddress(),
        m_rtc(nullptr),
        m_syncTimeByRtcTimer(),
        m_syncRtcByNtpTimer()
    {
    }

    /**
     * Destroys ClockDrv.
     */
    ~ClockDrv()
    {
    }

    /* Prevent copying */
    ClockDrv(const ClockDrv&);
    ClockDrv&operator=(const ClockDrv&);

    /**
     * Fill string up with spaces.
     * 
     * @param[in, out]  str     String which to fill up.
     * @param[in]       size    String buffer size in byte (incl. termination)
     */
    void fillUpWithSpaces(char* str, size_t size);

    /**
     * Update the time by the RTC.
     * If no RTC is available, nothing will happen.
     */
    void setTimeByRtc();

    /**
     * Update the RTC by the time.
     * If no RTC is available, nothing will happen.
     */
    void setRtcByTime();

    /**
     * Synchronize periodically the time by the RTC.
     * If the synchronization time period is expired, it will synchronizse
     * otherwise not.
     * If no RTC is available, nothing will happen.
     */
    void syncTimeByRtc();

    /**
     * Synchronize periodically the RTC by the time.
     * If the synchronization time period is expired, it will synchronizse
     * otherwise not.
     * If no RTC is available, nothing will happen.
     */
    void syncRtcByTime();

    /* Allow the SNTP callback to synchronize the RTC by the NTP. */
    friend void sntpCallback(struct timeval *tv);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* CLOCKDRV_H */

/** @} */