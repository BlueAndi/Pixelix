/* MIT License
 *
 * Copyright (c) 2019 - 2026 Andreas Merkle <web@blue-andi.de>
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
 * @file   ClockDrv.h
 * @brief  Clock driver for test purposes only
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * Counterpart of the clock driver in src/Service, which is only on the include
 * path of the target environments. It is required, because the Views library
 * compiles every layout, independent of the configured display size.
 *
 * It provides the local time of the host and none of the NTP resp. RTC
 * synchronization of the target.
 *
 * @addtogroup TEST
 *
 * @{
 */

#ifndef CLOCK_DRV_H
#define CLOCK_DRV_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <time.h>
#include <WString.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Clock driver for test purposes only.
 */
class ClockDrv
{
public:

    /**
     * Get the clock driver instance.
     *
     * @return Clock driver instance
     */
    static ClockDrv& getInstance()
    {
        static ClockDrv instance; /* singleton idiom to force initialization in the first usage. */

        return instance;
    }

    /**
     * Initialize the clock driver.
     * The host provides the local time, therefore the configuration is not used.
     *
     * @param[in] timeZone          Time zone string in POSIX format.
     * @param[in] ntpServerAddress  Address of the NTP server.
     */
    void init(const String& timeZone, const String& ntpServerAddress)
    {
        (void)timeZone;
        (void)ntpServerAddress;

        m_isInitialized = true;
    }

    /**
     * Get the local time.
     *
     * @param[out] timeInfo Local time
     *
     * @return If the time is available, it will return true otherwise false.
     */
    bool getTime(struct tm& timeInfo);

    /**
     * Get the UTC time.
     *
     * @param[out] timeInfo UTC time
     *
     * @return If the time is available, it will return true otherwise false.
     */
    bool getTimeUtc(struct tm& timeInfo);

    /**
     * Get the time of the given timezone.
     * The timezone is ignored on the host, therefore the local time is
     * returned.
     *
     * @param[in]  tz       Timezone string
     * @param[out] timeInfo Time of the given timezone
     *
     * @return If the time is available, it will return true otherwise false.
     */
    bool getTzTime(const char* tz, struct tm& timeInfo);

    /**
     * Get the current time zone offset in seconds.
     *
     * @return Current time zone offset in seconds.
     */
    long getCurrentTimeZoneOffset() const;

private:

    bool m_isInitialized; /**< Is the clock driver initialized? */

    /**
     * Constructs the clock driver.
     */
    ClockDrv() :
        m_isInitialized(false)
    {
    }

    /**
     * Destroys the clock driver.
     */
    ~ClockDrv()
    {
    }

    ClockDrv(const ClockDrv& drv);
    ClockDrv& operator=(const ClockDrv& drv);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* CLOCK_DRV_H */

/** @} */
