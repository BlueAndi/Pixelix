/* MIT License
 *
 * Copyright (c) 2019 - 2021 Andreas Merkle <web@blue-andi.de>
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

#ifndef __CLOCKDRV_H__
#define __CLOCKDRV_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Arduino.h"
#include "time.h"

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
     *
     */
    void init();

    /**
     * Get the current time.
     *
     * @param[out] currentTime Pointer to the currentTime.
     *
     * @return If time is not synchronized, it will return false otherwise true.
     */
    bool getTime(tm *currentTime);

    /**
     * Get the time format.
     *
     * @return true if 24h format is set otherwise false.
     */
    bool getTimeFormat();

    /**
     * Get the date format.
     *
     * @return true if DayMonthYear format is set otherwise false.
     */
    bool getDateFormat();

    /** Daylight saving time offset in s */
    static const int16_t NTP_DAYLIGHT_OFFSET_SEC    = 3600;

private:

    /** Flag indicating a initialized clock driver. */
    bool m_isClockDrvInitialized;

    /** Flag holding the time format. */
    bool m_is24HourFormat;

    /** Flag holding the date format. */
    bool m_isDayMonthYear;

    /**
     * Construct ClockDrv.
     */
    ClockDrv() :
        m_isClockDrvInitialized(false),
        m_is24HourFormat(false),
        m_isDayMonthYear(false)
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
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __CLOCKDRV_H__ */

/** @} */