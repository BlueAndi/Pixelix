/* MIT License
 *
 * Copyright (c) 2019 Andreas Merkle <web@blue-andi.de>
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
@brief  Simple timer
@author Andreas Merkle <web@blue-andi.de>

@section desc Description
This module provides a simple timer, based on millis().

*******************************************************************************/
/** @defgroup simpletimer Simple timer
 * This module provides a simple timer, based on millis().
 *
 * @{
 */

#ifndef __SIMPLETIMER_HPP__
#define __SIMPLETIMER_HPP__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <Arduino.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Simple timer, based on millis().
 */
class SimpleTimer
{
public:

    /**
     * Constructs a simple timer.
     */
    SimpleTimer() :
        m_isRunning(false),
        m_isTimeout(false),
        m_duration(0u),
        m_start(0u)
    {
    }

    /**
     * Destroys a simple timer.
     */
    ~SimpleTimer()
    {
    }

    /**
     * Constructs a simple timer by assign one.
     * 
     * @param[in] sTimer    Simple timer, which to assign.
     */
    SimpleTimer(const SimpleTimer& sTimer) :
        m_isRunning(sTimer.m_isRunning),
        m_isTimeout(sTimer.m_isTimeout),
        m_duration(sTimer.m_duration),
        m_start(sTimer.m_start)
    {
    }

    /**
     * Copy a simple timer.
     * 
     * @param[in] sTimer    Simple timer, which to copy.
     */
    SimpleTimer& operator=(const SimpleTimer& sTimer)
    {
        m_isRunning = sTimer.m_isRunning;
        m_isTimeout = sTimer.m_isTimeout;
        m_duration  = sTimer.m_duration;
        m_start     = sTimer.m_start;

        return *this;
    }

    /**
     * Start timer with the given duration.
     * 
     * @param[in] duration  Duration in ms
     */
    void start(uint32_t duration)
    {
        m_isRunning = true;
        m_duration  = duration;
        m_start     = millis();

        return;
    }

    /**
     * Stop timer.
     */
    void stop(void)
    {
        m_isRunning = false;
        m_isTimeout = false;

        return;
    }

    /**
     * Restart timer with the previous specified duration.
     */
    void restart(void)
    {
        m_isRunning = true;
        m_isTimeout = false;
        m_start     = millis();

        return;
    }

    /**
     * Is timer running?
     * 
     * @return If timer is running, it will return true otherwise false.
     */
    bool isTimerRunning(void) const
    {
        return m_isRunning;
    }

    /**
     * Is timeout?
     * If timer is not running, it will always return false.
     * 
     * @return If timeout it will return true, otherwise false.
     */
    bool isTimeout(void)
    {
        bool isTimeout = false;

        if (true == m_isRunning)
        {            
            if (false == m_isTimeout)
            {
                uint32_t delta = millis() - m_start;

                if (m_duration <= delta)
                {
                    m_isTimeout = true;
                }
            }

            isTimeout = m_isTimeout;
        }

        return isTimeout;
    }

private:

    bool        m_isRunning;    /**< Timer is running or not. */
    bool        m_isTimeout;    /**< Timer timeout active or not. */
    uint32_t    m_duration;     /**< Duration in ms */
    uint32_t    m_start;        /**< Timestamp at start time */
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __SIMPLETIMER_HPP__ */

/** @} */