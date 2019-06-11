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

#ifndef __GPIODRV_H__
#define __GPIODRV_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <Arduino.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * The GPIO driver is responsible to initialize all kind of GPIOs
 * and provide an interface for external peripherals.
 */
class GPIODrv
{
public:

    /** GPIO levels */
    enum Level
    {
        LEVEL_LOW = 0,  /**< Low */
        LEVEL_HIGH      /**< High */
    };

    /**
     * Pin number of used GPIO pins.
     * The pin number corresponds to the arduino pin scheme.
     */
    enum PinNo
    {
        PINNO_ONBOARD_LED           = 2,    /**< Onboard LED pin number */
        PINNO_USER_BUTTON           = 4,    /**< User button pin number */
        PINNO_LED_MATRIX_DATA_OUT   = 27    /**< LED matrix pin number */
    };

    /**
     * Get GPIO driver instance.
     * 
     * @return GPIO driver instance.
     */
    static GPIODrv& getInstance()
    {
        return m_instance;
    }

    /**
     * Initialize the GPIOs.
     */
    void init();

    /**
     * Get user button state without debouncing.
     * 
     * @return User button level.
     */
    Level getUserButtonState();

    /**
     * Enable/Disable onboard LED.
     */
    void setOnboardLED(bool enable);

private:

    static GPIODrv  m_instance; /**< GPIO driver instance */

    /**
     * Initializes the complete GPIOs of the board.
     */
    GPIODrv()
    {
        /* Nothing to do */
    }

    /* Copy-constructor is not allowed. */
    GPIODrv(const GPIODrv& gpioDrv);

    /* Assignment operator is not allowed. */
    GPIODrv& operator=(const GPIODrv& gpioDrv);

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __GPIODRV_H__ */