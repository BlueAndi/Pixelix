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
 * @file   ViewUpdateTimer.cpp
 * @brief  View update timer used to send view updates via MQTT periodically.
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "ViewUpdateTimer.h"
#include <Util.h>

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and classes
 *****************************************************************************/

/******************************************************************************
 * Prototypes
 *****************************************************************************/

/**
 * Calculate view event update time based on count.
 *
 * @param[in] count   Count
 *
 * @return Calculated update time in seconds.
 */
static constexpr uint32_t calcUpdateTime(uint32_t count);

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/** Coefficient A for view update algorithm. */
static constexpr uint32_t COEFFICIENT_A             = (21575 / 18);

/** Coefficient B for view update algorithm. */
static constexpr uint32_t COEFFICIENT_B             = (63435 / 18);

/** Coefficient C for view update algorithm. */
static constexpr uint32_t COEFFICIENT_C             = (42048 / 18);

/**
 * View event update periods in seconds.
 * These are absolute times and require adjustment to get the relative timeouts.
 */
static const uint32_t VIEW_EVENT_UPDATE_PERIODS_S[] = {
    calcUpdateTime(1U), /*  0 h  0 min 10 s */
    calcUpdateTime(2U), /*  0 h  1 min 20 s */
    calcUpdateTime(3U), /*  0 h 42 min 26 s */
    calcUpdateTime(4U), /*  2 h  3 min 28 s */
    calcUpdateTime(5U), /*  4 h  4 min 26 s */
    calcUpdateTime(6U), /*  6 h 45 min 20 s */
    calcUpdateTime(7U), /* 10 h  6 min 10 s */
    calcUpdateTime(8U), /* 14 h  6 min 56 s */
    calcUpdateTime(9U), /* 18 h 47 min 38 s */
    calcUpdateTime(10U) /* 24 h  8 min 16 s */
};

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void ViewUpdateTimer::start()
{
    m_idx        = 0U;
    m_isLongTerm = false;
    m_timer.start(getPeriodMs());
}

void ViewUpdateTimer::startNext()
{
    /* Move to next period. */
    if (UTIL_ARRAY_NUM(VIEW_EVENT_UPDATE_PERIODS_S) - 1U > m_idx)
    {
        ++m_idx;
    }
    else
    {
        m_isLongTerm = true;
    }

    m_timer.start(getPeriodMs());
}

bool ViewUpdateTimer::isTimeout()
{
    return m_timer.isTimeout();
}

void ViewUpdateTimer::pause()
{
    m_timer.stop();
}

void ViewUpdateTimer::play()
{
    m_timer.resume();
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

uint32_t ViewUpdateTimer::getPeriodMs() const
{
    uint8_t  idx      = m_idx;
    uint32_t periodMs = 0U;

    if (true == m_isLongTerm)
    {
        idx      = UTIL_ARRAY_NUM(VIEW_EVENT_UPDATE_PERIODS_S) - 1U;
        periodMs = SIMPLE_TIMER_SECONDS(VIEW_EVENT_UPDATE_PERIODS_S[idx]);
    }
    else
    {
        if (UTIL_ARRAY_NUM(VIEW_EVENT_UPDATE_PERIODS_S) <= idx)
        {
            idx = UTIL_ARRAY_NUM(VIEW_EVENT_UPDATE_PERIODS_S) - 1U;
        }

        periodMs = SIMPLE_TIMER_SECONDS(VIEW_EVENT_UPDATE_PERIODS_S[idx]);

        /* Subtract already elapsed periods. */
        while (0U < idx)
        {
            periodMs -= SIMPLE_TIMER_SECONDS(VIEW_EVENT_UPDATE_PERIODS_S[idx - 1U]);
            --idx;
        }
    }

    return periodMs;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/**
 * Calculate view event update time based on count.
 *
 * @param[in] count   Count
 *
 * @return Calculated update time in seconds.
 */
static constexpr uint32_t calcUpdateTime(uint32_t count)
{
    return (COEFFICIENT_A * (count * count) - COEFFICIENT_B * count + COEFFICIENT_C);
}
