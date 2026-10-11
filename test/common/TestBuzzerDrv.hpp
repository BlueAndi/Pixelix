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
 * @file   TestBuzzerDrv.hpp
 * @brief  Buzzer driver stub for tests
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup TEST
 *
 * @{
 */

#ifndef TEST_BUZZER_DRV_HPP
#define TEST_BUZZER_DRV_HPP

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <IBuzzerDrv.h>
#include <Util.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Buzzer driver stub, which records what the player requested.
 */
class TestBuzzerDrv : public IBuzzerDrv
{
public:

    /**
     * Constructs the buzzer driver stub.
     */
    TestBuzzerDrv() :
        IBuzzerDrv(),
        m_playCount(0U),
        m_lastFreq(0U),
        m_lastDutyCycle(0U),
        m_maxDutyCycle(0U)
    {
    }

    /**
     * Destroys the buzzer driver stub.
     */
    virtual ~TestBuzzerDrv()
    {
    }

    /**
     * Initialize the buzzer driver.
     *
     * @param[in] buzzerOut Pin of the buzzer.
     */
    void init(const DOutPin& buzzerOut) final
    {
        UTIL_NOT_USED(buzzerOut);
    }

    /**
     * Stop the buzzer.
     */
    void stop() final
    {
    }

    /**
     * Play a tone with the given frequency.
     *
     * @param[in] freq Frequency in Hz.
     */
    void play(uint32_t freq) final
    {
        play(freq, 0U);
    }

    /**
     * Play a tone with the given frequency and duty cycle.
     *
     * @param[in] freq  Frequency in Hz.
     * @param[in] dc    Duty cycle in digits.
     */
    void play(uint32_t freq, uint16_t dc) final
    {
        ++m_playCount;
        m_lastFreq      = freq;
        m_lastDutyCycle = dc;

        if (m_maxDutyCycle < dc)
        {
            m_maxDutyCycle = dc;
        }
    }

    /**
     * Change the duty cycle of the currently played tone.
     *
     * @param[in] dc    Duty cycle in digits.
     */
    void changeDutyCycle(uint16_t dc) final
    {
        m_lastDutyCycle = dc;

        if (m_maxDutyCycle < dc)
        {
            m_maxDutyCycle = dc;
        }
    }

    /**
     * Reset the records.
     */
    void reset()
    {
        m_playCount     = 0U;
        m_lastFreq      = 0U;
        m_lastDutyCycle = 0U;
        m_maxDutyCycle  = 0U;
    }

    uint32_t m_playCount;     /**< Number of played tones. */
    uint32_t m_lastFreq;      /**< Frequency of the last played tone. */
    uint16_t m_lastDutyCycle; /**< Duty cycle of the last played tone. */
    uint16_t m_maxDutyCycle;  /**< Highest duty cycle which was requested. */
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* TEST_BUZZER_DRV_HPP */

/** @} */
