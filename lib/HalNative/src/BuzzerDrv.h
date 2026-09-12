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
 * @file   BuzzerDrv.h
 * @brief  Buzzer driver for the native environment
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * There is no buzzer on the host, therefore it stays silent.
 *
 * @addtogroup TEST
 *
 * @{
 */

#ifndef BUZZER_DRV_H
#define BUZZER_DRV_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <IBuzzerDrv.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Buzzer driver for the native environment.
 */
class BuzzerDrv : public IBuzzerDrv
{
public:

    /**
     * Constructs the buzzer driver.
     */
    BuzzerDrv() :
        IBuzzerDrv()
    {
    }

    /**
     * Destroys the buzzer driver.
     */
    virtual ~BuzzerDrv()
    {
    }

    /**
     * Initialize the buzzer driver.
     *
     * @param[in] buzzerOut Pin of the buzzer.
     */
    void init(const DOutPin& buzzerOut) final
    {
        (void)buzzerOut;
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
     * @param[in] freq  Frequency in Hz.
     */
    void play(uint32_t freq) final
    {
        (void)freq;
    }

    /**
     * Play a tone with the given frequency and duty cycle.
     *
     * @param[in] freq  Frequency in Hz.
     * @param[in] dc    Duty cycle.
     */
    void play(uint32_t freq, uint16_t dc) final
    {
        (void)freq;
        (void)dc;
    }

    /**
     * Change the duty cycle.
     *
     * @param[in] dc    Duty cycle.
     */
    void changeDutyCycle(uint16_t dc) final
    {
        (void)dc;
    }

private:

    BuzzerDrv(const BuzzerDrv& drv);
    BuzzerDrv& operator=(const BuzzerDrv& drv);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* BUZZER_DRV_H */

/** @} */
