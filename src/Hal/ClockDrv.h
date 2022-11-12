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
     * Get the current time as formatted string.
     * The format is equal to strftime(), please have a look there.
     * 
     * Use getTimeFormat() or getDateFormat() for the user configured format.
     * 
     * @param[out]  time            The formatted time string.
     * @param[in]   format          The format according to strftime().
     * @param[in]   currentTime     The current time (optional).
     * 
     * @return If successful, it will return true otherwise false.
     */
    bool getTimeAsString(String& time, const String& format, const tm *currentTime = nullptr);
    
    /**
     * Get the time format from configuration.
     *
     * @return Time format.
     */
    const String& getTimeFormat();

    /**
     * Get the date format from configuration.
     *
     * @return Date format.
     */
    const String& getDateFormat();

private:

    /** Flag indicating a initialized clock driver. */
    bool    m_isClockDrvInitialized;

    /** Time format */
    String  m_timeFormat;

    /** Date format */
    String  m_dateFormat;

    /**
     * Construct ClockDrv.
     */
    ClockDrv() :
        m_isClockDrvInitialized(false),
        m_timeFormat(),
        m_dateFormat()
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