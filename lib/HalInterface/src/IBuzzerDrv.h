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
 * @file   IBuzzerDrv.h
 * @brief  Abstract buzzer driver interface
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup HAL
 *
 * @{
 */

#ifndef IBUZZERDRV_H
#define IBUZZERDRV_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Io.hpp"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Abstract buzzer driver interface.
 */
class IBuzzerDrv
{
public:

    /**
     * Destroys BuzzerDrv.
     */
    virtual ~IBuzzerDrv()
    {
    }

    /**
     * Initialize the driver.
     *
     * @param[in] buzzerOut  Digital output for buzzer.
     */
    virtual void init(const DOutPin& buzzerOut)   = 0;

    /**
     * Stop playing.
     */
    virtual void stop()                           = 0;

    /**
     * Play a tone by frequency. Last duty cycle is used.
     *
     * @param[in] freq  Frequency in Hz
     */
    virtual void play(uint32_t freq)              = 0;

    /**
     * Play a tone by frequency and duty cycle.
     *
     * @param[in] freq  Frequency in Hz
     * @param[in] dc    Duty cycle in digits [0; 1023]
     */
    virtual void play(uint32_t freq, uint16_t dc) = 0;

    /**
     * Change duty cycle.
     *
     * @param[in] dc    Duty cycle in digits [0; 1023]
     */
    virtual void changeDutyCycle(uint16_t dc)     = 0;

protected:

    /**
     * Construct BuzzerDrv.
     */
    IBuzzerDrv()
    {
    }
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* IBUZZERDRV_H */

/** @} */