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
 * @file   Pin.cpp
 * @brief  Pin definitions
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Pin.h"

#include <Util.h>

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

using namespace PinNo;

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and classes
 *****************************************************************************/

/******************************************************************************
 * Prototypes
 *****************************************************************************/

/******************************************************************************
 * Global Variables
 *****************************************************************************/

/** Digital output pin: Onboard LED */
const DOutPinT<onBoardLedPinNo> Pin::onBoardLedOut;

/** Digital input pin: Button "ok" (input with pull-up) */
const DInPinT<buttonOkPinNo, INPUT_PULLUP> Pin::buttonOkIn;

/** Digital input pin: Button "left" (input with pull-up) */
const DInPinT<buttonLeftPinNo, INPUT_PULLUP> Pin::buttonLeftIn;

/** Digital input pin: Button "right" (input with pull-up) */
const DInPinT<buttonRightPinNo, INPUT_PULLUP> Pin::buttonRightIn;

/** Digital input pin: Button "reset" (input with pull-up) */
const DInPinT<buttonResetPinNo, INPUT_PULLUP> Pin::buttonResetIn;

/** Digital output pin: Test pin (only for debug purposes) */
const DOutPinT<testPinNo> Pin::testPinOut;

/** Digital output pin: LED matrix data out */
const DOutPinT<ledMatrixDataOutPinNo> Pin::ledMatrixDataOut;

/** Analog input pin: LDR in */
const AnalogPinT<ldrInPinNo> Pin::ldrIn;

/** Digital input pin: DHT Sensor (input with pull-up) */
const DInPinT<dhtInPinNo, INPUT_PULLUP> Pin::dhtIn;

/** Analog input pin: battery voltage in */
const AnalogPinT<batteryInPinNo> Pin::batteryVoltageIn;

/** Digital output pin: Buzzer */
const DOutPinT<buzzerOutPinNo> Pin::buzzerOut;

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/** A list of all used i/o pins, used for initialization. */
static const IoPin* ioPinList[] = {
    &Pin::onBoardLedOut,
    &Pin::buttonOkIn,
    &Pin::buttonLeftIn,
    &Pin::buttonRightIn,
    &Pin::buttonResetIn,
    &Pin::testPinOut,
    &Pin::ledMatrixDataOut,
    &Pin::ldrIn,
    &Pin::dhtIn,
    &Pin::batteryVoltageIn,
    &Pin::buzzerOut
};

/******************************************************************************
 * Public Methods
 *****************************************************************************/

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

void Pin::init()
{
    uint8_t index = 0U;

    /* Initialize all i/o pins */
    for (index = 0U; index < UTIL_ARRAY_NUM(ioPinList); ++index)
    {
        if (nullptr != ioPinList[index])
        {
            ioPinList[index]->init();
        }
    }

    /* Disable buzzer */
    buzzerOut.write(LOW);
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/
