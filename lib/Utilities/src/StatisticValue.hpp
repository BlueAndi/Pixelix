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
 * @brief  Statistic value
 * @author Andreas Merkle <web@blue-andi.de>
 * 
 * @addtogroup utilities
 *
 * @{
 */

#ifndef STATISTIC_VALUE_HPP
#define STATISTIC_VALUE_HPP

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * This class can be used to get further statistic relevant informations about
 * a value.
 * 
 * @tparam T        Data type of the value
 * @tparam zero     The number zero, compliant to the data type of the value.
 * @tparam avgCnt   Number of values for the moving average calculation.
 */
template < typename T, T zero, uint32_t avgCnt >
class StatisticValue
{
public:

    /**
     * Create the statistic value in initial state.
     */
    StatisticValue() :
        m_isInit(true),
        m_min(zero),
        m_max(zero),
        m_current(zero),
        m_wrIdx(0U),
        m_cnt(0U),
        m_sum(zero),
        m_values()
    {
    }

    /**
     * Destroys the statistic value.
     */
    ~StatisticValue()
    {
    }

    /**
     * Update the value and derive further information.
     * 
     * @param[in] value The value which to observe.
     */
    void update(const T& value)
    {
        /* Store the "live" value */
        m_current = value;

        /* Especially for determining the minimum and maximum value, it can not be
         * just compared to the initial value. Because it may happen that the
         * initial value zero is already the minimum or maximum. Therefore they
         * must be initialized with the first given real value.
         */
        if (false == m_isInit)
        {
            if (m_min > value)
            {
                m_min = value;
            }

            if (m_max < value)
            {
                m_max = value;
            }
        }
        else
        /* Initialize with first given real value. */
        {
            m_min       = value;
            m_max       = value;
            m_isInit    = false;
        }

        /* Calculate the moving average. To avoid running over all values,
         * the sum of all values is considered during every update call.
         */
        if (avgCnt <= m_cnt)
        {
            m_sum -= m_values[m_wrIdx];
        }
        m_sum += value;

        /* Handle value FIFO for moving average calculation. */
        m_values[m_wrIdx] = value;
        ++m_wrIdx;

        if (avgCnt <= m_wrIdx)
        {
            m_wrIdx = zero;
        }

        if (avgCnt > m_cnt)
        {
            ++m_cnt;
        }
    }

    /**
     * Reset everything to get it back in initial state.
     */
    void reset()
    {
        m_isInit    = true;
        m_min       = zero;
        m_max       = zero;
        m_current   = zero;
        m_wrIdx     = 0U;
        m_cnt       = 0U;
        m_sum       = zero;
    }

    /**
     * Get the minimum value, determined during the last value updates.
     * 
     * @return Minimum value
     */
    T getMin() const
    {
        return m_min;
    }

    /**
     * Get the maximum value, determined during the last value updates.
     * 
     * @return Maximum value
     */
    T getMax() const
    {
        return m_max;
    }

    /**
     * Get the moving average value, determined during the last value updates.
     * 
     * @return Moving average value
     */
    T getAvg() const
    {
        T avg = zero;

        if (0U < m_cnt)
        {
            avg = m_sum / m_cnt;
        }

        return avg;
    }

    /**
     * Get the last updated value.
     * 
     * @return Current value
     */
    T getCurrent() const
    {
        return m_current;
    }

private:

    bool        m_isInit;           /**< Is min. and max. value initialized with the first given value? */
    T           m_min;              /**< Minimum value, determined during value updates. */
    T           m_max;              /**< Maximum value, determined during value updates. */
    T           m_current;          /**< Last updated value. */
    uint32_t    m_wrIdx;            /**< Write index for the value array, used for moving average calculation. */
    uint32_t    m_cnt;              /**< Number of values in the value array. */
    T           m_sum;              /**< Sum over all values in the value array. */
    T           m_values[avgCnt];   /**< Value array, used for moving average calculation. */

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* STATISTIC_VALUE_HPP */

/** @} */