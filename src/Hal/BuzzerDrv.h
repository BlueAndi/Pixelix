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
 * @brief  Buzzer driver
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup Hal
 *
 * @{
 */

#ifndef BUZZERDRV_H
#define BUZZERDRV_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Arduino.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Buzzer driver.
 */
class BuzzerDrv
{
public:

    /**
     *  Get the BuzzerDrv instance.
     *
     * @return BuzzerDrv instance.
     */
    static BuzzerDrv& getInstance()
    {
        static BuzzerDrv instance; /* singleton idiom to force initialization in the first usage. */

        return instance;
    }

    /**
     * Stop playing.
     */
    void stop();

    /**
     * Play a tone by frequency. Last duty cycle is used.
     * 
     * @param[in] freq  Frequency in Hz
     */
    void play(uint32_t freq);

    /**
     * Play a tone by frequency and duty cycle.
     * 
     * @param[in] freq  Frequency in Hz
     * @param[in] dc    Duty cycle in percent
     */
    void play(uint32_t freq, uint8_t dc);

    /**
     * Change duty cycle.
     * 
     * @param[in] dc    Duty cycle in percent
     */
    void changeDutyCycle(uint8_t dc);

private:

    /**
     * The PWM channel which to use for the tone generation.
     */
    static const uint8_t    TONE_PWM_CHANNEL            = 0U;

    /**
     * Frequency in Hz used to initialize the PWM. Has no influence on a tone.
     * Its just required for initialization.
     */
    static const uint32_t   INIT_FREQUENCY              = 1000U;

    /**
     * Number of bits used for the duty cycle.
     */
    static const uint8_t    DUTY_CYCLE_RESOLUTION_BITS  = 10U;

    /**
     * Default duty cycle in digits [0; 1023] set to 50 %.
     */
    static const uint32_t   DEFAULT_DUTY_CYCLE          = 0x01FF;

    uint8_t     m_isInit;       /**< Is initialized or not? */
    uint32_t    m_dutyCycle;    /**< Duty cycle in digits [0; 1023] */

    /**
     * Construct BuzzerDrv.
     */
    BuzzerDrv() :
        m_isInit(false),
        m_dutyCycle(DEFAULT_DUTY_CYCLE)
    {
    }

    /**
     * Destroys BuzzerDrv.
     */
    ~BuzzerDrv()
    {
    }

    /* Prevent copying */
    BuzzerDrv(const BuzzerDrv&);
    BuzzerDrv&operator=(const BuzzerDrv&);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* BUZZERDRV_H */

/** @} */