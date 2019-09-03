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
@brief  Board Abstraction
@author Andreas Merkle <web@blue-andi.de>

@section desc Description
This module provides the hardware board abstraction.

*******************************************************************************/
/** @defgroup board Board Abstraction
 * The board abstraction is unique for the electronic hardware board and
 * provides its functionality on a lower level interface.
 *
 * @{
 */

#ifndef __BOARD_H__
#define __BOARD_H__

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Io.h"

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

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
    static const uint8_t    onBoardLedPinNo         = 2u;
    /** Pin number of user button */
    static const uint8_t    userButtonPinNo         = 4u;
    /** Pin number of LDR in */
    static const uint8_t    ldrInPinNo              = 26u;
    /** Pin number of LED matrix data out */
    static const uint8_t    ledMatrixDataOutPinNo   = 27u;
};

/** Digital output pin: Onboard LED */
static const DOutPin<Pin::onBoardLedPinNo>              onBoardLedOut;

/** Digital input pin: User button (input with pull-up) */
static const DInPin<Pin::userButtonPinNo, INPUT_PULLUP> userButtonIn;

/** Analog input pin: LDR in */
static const AnalogPin<Pin::ldrInPinNo>                 ldrIn;

/** Digital output pin: LED matrix data out */
static const DOutPin<Pin::ledMatrixDataOutPinNo>        ledMatrixDataOut;

/** LED matrix specific values */
namespace LedMatrix
{

/** LED matrix width in pixels */
static const uint8_t    width               = 32u;

/** LED matrix height in pixels */
static const uint8_t    height              = 8u;

/** LED matrix supply voltage in volt */
static const uint8_t    supplyVoltage       = 5u;

/** LED matrix max. supply current in mA */
static const uint32_t   supplyCurrentMax    = 3500u;

};

/******************************************************************************
 * Functions
 *****************************************************************************/

/**
 * Initialize all i/o pins.
 */
extern void init();

};

#endif  /* __BOARD_H__ */

/** @} */