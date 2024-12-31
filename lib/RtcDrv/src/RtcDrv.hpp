/* MIT License
 *
 * Copyright (c) 2019 - 2024 Andreas Merkle <web@blue-andi.de>
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
 * @brief  RTC driver selector
 * @author Andreas Merkle <web@blue-andi.de>
 * 
 * @addtogroup RTC
 *
 * @{
 */

#ifndef RTC_DRV_H
#define RTC_DRV_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "RtcNoneDrv.h"
#include "Rtc1307Drv.h"
#include "Rtc3231Drv.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Supported RTC drivers.
 */
typedef enum
{
    RTC_TYPE_NONE = 0,  /**< No RTC available */
    RTC_TYPE_1307,      /**< RTC 1307, 5V */
    RTC_TYPE_3231       /**< RTC 3231, 3.3V */

} RTC_TYPE;

template < RTC_TYPE tRtcType >
class RtcDrvSelector : public IRtc
{
public:
    virtual ~RtcDrvSelector() = default;
};

template <>
class RtcDrvSelector< RTC_TYPE_NONE > : public RtcNoneDrv
{
public:
    virtual ~RtcDrvSelector() = default;
};

template <>
class RtcDrvSelector< RTC_TYPE_1307 > : public Rtc1307Drv
{
public:
    virtual ~RtcDrvSelector() = default;
};

template <>
class RtcDrvSelector< RTC_TYPE_3231 > : public Rtc3231Drv
{
public:
    virtual ~RtcDrvSelector() = default;
};

using RtcDrv = RtcDrvSelector<CONFIG_RTC_DRV>;

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* RTC_DRV_H */

/** @} */
