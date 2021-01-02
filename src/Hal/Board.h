/* MIT License
 *
 * Copyright (c) 2019 - 2021 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Board Abstraction
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup hal
 *
 * @{
 */

#ifndef __BOARD_H__
#define __BOARD_H__

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Io.hpp"

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/** Electronic board abstraction */
namespace Board
{

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/******************************************************************************
 * Variables
 *****************************************************************************/

/** Pin number of all used pins. */
namespace Pin
{
    /** Pin number of onboard LED */
    static const uint8_t    onBoardLedPinNo         = 2U;

    /** Pin number of user button */
    static const uint8_t    userButtonPinNo         = 4U;

    /** Pin number of test pin */
    static const uint8_t    testPinNo               = 23U;

    /** Pin number of LED matrix data out */
    static const uint8_t    ledMatrixDataOutPinNo   = 27U;

    /** Pin number of LDR in */
    static const uint8_t    ldrInPinNo              = 34U;
};

/** Digital output pin: Onboard LED */
static const DOutPin<Pin::onBoardLedPinNo>              onBoardLedOut;

/** Digital input pin: User button (input with pull-up) */
static const DInPin<Pin::userButtonPinNo, INPUT_PULLUP> userButtonIn;

/** Digital output pin: Test pin (only for debug purposes) */
static const DOutPin<Pin::testPinNo>                    testPinOut;

/** Digital output pin: LED matrix data out */
static const DOutPin<Pin::ledMatrixDataOutPinNo>        ledMatrixDataOut;

/** Analog input pin: LDR in */
static const AnalogPin<Pin::ldrInPinNo>                 ldrIn;

/** ADC resolution in digits */
static const uint16_t   adcResolution   = 4096U;

/** ADC reference voltage in mV */
static const uint16_t   adcRefVoltage   = 3300U;

/** LED matrix specific values */
namespace LedMatrix
{

/** LED matrix width in pixels */
static const uint8_t    width               = 32U;

/** LED matrix height in pixels */
static const uint8_t    height              = 8U;

/** LED matrix supply voltage in volt */
static const uint8_t    supplyVoltage       = 5U;

/** LED matrix max. supply current in mA */
static const uint32_t   supplyCurrentMax    = 3500U;

/** Max. current in mA per LED */
static const uint32_t   maxCurrentPerLed    = 60U;

/** Time to load the data for one single pixel in us. */
static const uint32_t   pixelLoadTime       = 30U;

/** Time to load the data of the whole matrix in ms. */
static const uint32_t   matrixLoadTime      = ((width * height) * pixelLoadTime + 500U) / 1000U;

};

/******************************************************************************
 * Functions
 *****************************************************************************/

/**
 * Initialize all i/o pins.
 */
extern void init();

/**
 * Execute a hard reset!
 */
extern void reset();

};

#endif  /* __BOARD_H__ */

/** @} */