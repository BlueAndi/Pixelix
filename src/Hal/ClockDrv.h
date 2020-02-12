/* MIT License
 *
 * Copyright (c) 2019 - 2020 Andreas Merkle <web@blue-andi.de>
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
 * @brief  RelatimeClock driver
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
#include "SimpleTimer.hpp"
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
        
        return m_instance;
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
     * @return true if successful, otherwise false.
    */
    bool getTime(tm *currentTime);

     /**
     * Process the ClockDrv, if the ntp sync timer expires a ntp update is triggered..
     */
    void process(void);

    /**
     * Get the current syncState.
     * 
     * @return true if synchronized, otherwise false.
    */
    bool isSynchronized(void);

private:

    /** ClockDrv instance. */
    static ClockDrv m_instance;
    
    /** Flag indicating a initialized ClockDrv. */
    bool m_isClockDrvInitialized;

    /** Flag indicating a synchronized time. */
    bool m_isSynchronized;

    /** Timer, used for cyclic ntp synchronization. */
    SimpleTimer m_ntpSyncTimer;
   
   /** The GMT offset as read from settings.*/
   int32_t m_gmtOffset;

    /** Daylight saving value.*/
   int16_t m_daylightSavingValue;

    /** Period for cyclic NTP synchronization in ms */
    static const uint32_t   NTP_SYNC_PERIOD          = 3600000U;

     /**
     * Construct ClockDrv.
     */
    ClockDrv() :
        m_isClockDrvInitialized(false),
        m_isSynchronized(false),
        m_ntpSyncTimer(),
        m_gmtOffset(0),
        m_daylightSavingValue(0)
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