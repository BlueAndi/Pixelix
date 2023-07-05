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
 * @brief  RTC driver without functionality
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup Hal
 *
 * @{
 */

#ifndef RTCNONE_DRV_H
#define RTCNONE_DRV_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Arduino.h"
#include <IRtc.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * The RTC driver provides the abstract RTC interface, but has no functionality.
 */
class RtcNoneDrv : public IRtc
{
public:

    /**
     * Constructs the driver.
     */
    RtcNoneDrv() :
        IRtc()
    {
    }

    /**
     * Destroys the driver.
     */
    ~RtcNoneDrv()
    {
    }

    /**
     * Checks for the RTC and if available, it will be initialized
     * and started.
     * 
     * @return If no RTC is available, it will return false.
     */
    bool begin() final
    {
        return false;
    }

    /**
     * Get the time from the RTC.
     * 
     * @param[out] timeInfo Time destination
     * 
     * @return If time info is updated, it will return true otherwise false.
     */
    bool getTime(struct tm& timeInfo) final
    {
        (void)timeInfo;

        return false;
    }

    /**
     * Set the RTC by time.
     * 
     * @param[in] timeInfo Time source
     */
    void  setTime(const struct tm& timeInfo) final
    {
        (void)timeInfo;
    }

private:

    RtcNoneDrv(const RtcNoneDrv& drv);
    RtcNoneDrv& operator=(const RtcNoneDrv& drv);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* RTCNONE_DRV_H */

/** @} */