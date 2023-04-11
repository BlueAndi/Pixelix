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
     * Get the local time by considering device timezone.
     *
     * @param[out] timeInfo Time information.
     *
     * @return If time is not synchronized, it will return false otherwise true.
     */
    bool getTime(tm* timeInfo);

    /**
     * Get the current time in UTC.
     *
     * @param[out] timeInfo Time information.
     *
     * @return If time is not synchronized, it will return false otherwise true.
     */
    bool getUtcTime(tm* timeInfo);

    /**
     * Get the local time by considering the timezone.
     * 
     * @param[in]   tz          Timzone string
     * @param[out]  timeInfo    Local time information
     * 
     * @return If time is not synchronized, it will return false otherwise true.
     */
    bool getTzTime(const char* tz, tm* timeInfo);

private:

    /**
     * The minimum timezone string size (incl. string termination).
     */
    static const size_t TZ_MIN_SIZE = 60U;

    /** Use UTC timezone by default. */
    static const char*  TZ_UTC;

    /** Flag indicating a initialized clock driver. */
    bool    m_isClockDrvInitialized;

    /** Device timezone */
    String  m_timeZone;

    /** newlib's internal timezone buffer. */
    char*   m_internalTimeZoneBuffer;

    /**
     * Construct ClockDrv.
     */
    ClockDrv() :
        m_isClockDrvInitialized(false),
        m_timeZone(TZ_UTC),
        m_internalTimeZoneBuffer(nullptr)
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
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* CLOCKDRV_H */

/** @} */