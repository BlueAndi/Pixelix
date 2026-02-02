/* MIT License
 *
 * Copyright (c) 2019 - 2025 Andreas Merkle <web@blue-andi.de>
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
 * @file   ViewUpdateTimer.h
 * @brief  View update timer used to send view updates via MQTT periodically.
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup PLUGIN
 *
 * @{
 */

#ifndef VIEW_UPDATE_TIMER_H
#define VIEW_UPDATE_TIMER_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <SimpleTimer.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * View update timer class.
 * Used to send view updates via MQTT periodically.
 */
class ViewUpdateTimer
{
public:

    /**
     * Constructs the view update timer.
     */
    ViewUpdateTimer() :
        m_timer(),
        m_idx(0U),
        m_isLongTerm(false)
    {
    }

    /**
     * Destroys the view update timer.
     */
    ~ViewUpdateTimer()
    {
    }

    /**
     * Start the view update timer.
     * It will reset the timer and start it.
     */
    void start();

    /**
     * Start next view update period.
     * It will adjust the timer period to the next one.
     */
    void startNext();

    /**
     * Is timeout?
     * Note, if timer is not running, it will always return false.
     *
     * @return If timeout, it will return true otherwise false.
     */
    bool isTimeout();

public:

    SimpleTimer m_timer;      /**< Timer to send view updates via MQTT. */
    uint8_t     m_idx;        /**< Current index in view update sequence. */
    bool        m_isLongTerm; /**< Indicates whether long term updates are active or not. */

    /**
     * Get current period in milliseconds.
     *
     * @return Period in milliseconds.
     */
    uint32_t getPeriodMs() const;
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* VIEW_UPDATE_TIMER_H */

/** @} */
