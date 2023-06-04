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
 * @brief  Board Abstraction
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Board.h"

#include <Util.h>
#include <Esp.h>

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

using namespace Board;

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
const DOutPin<Pin::onBoardLedPinNo>                 Board::onBoardLedOut;

/** Digital input pin: Button "ok" (input with pull-up) */
const DInPin<Pin::buttonOkPinNo, INPUT_PULLUP>      Board::buttonOkIn;

/** Digital input pin: Button "left" (input with pull-up) */
const DInPin<Pin::buttonLeftPinNo, INPUT_PULLUP>    Board::buttonLeftIn;

/** Digital input pin: Button "right" (input with pull-up) */
const DInPin<Pin::buttonRightPinNo, INPUT_PULLUP>   Board::buttonRightIn;

/** Digital output pin: Test pin (only for debug purposes) */
const DOutPin<Pin::testPinNo>                       Board::testPinOut;

/** Digital output pin: LED matrix data out */
const DOutPin<Pin::ledMatrixDataOutPinNo>           Board::ledMatrixDataOut;

/** Analog input pin: LDR in */
const AnalogPin<Pin::ldrInPinNo>                    Board::ldrIn;

/** Digital input pin: DHT Sensor (input with pull-up) */
const DInPin<Pin::dhtInPinNo, INPUT_PULLUP>         Board::dhtIn;

/** Analog input pin: battery voltage in */
const AnalogPin<Pin::batteryInPinNo>                Board::batteryVoltageIn;

/** Digital output pin: Buzzer */
const DOutPin<Pin::buzzerOutPinNo>                  Board::buzzerOut;

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/** A list of all used i/o pins, used for initialization. */
static const IoPin* ioPinList[] =
{
    &onBoardLedOut,
    &buttonOkIn,
    &buttonLeftIn,
    &buttonRightIn,
    &testPinOut,
    &ledMatrixDataOut,
    &ldrIn,
    &dhtIn,
    &batteryVoltageIn,
    &buzzerOut
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

extern void Board::init()
{
    uint8_t index = 0U;

    /* Initialize all i/o pins */
    for(index = 0U; index < UTIL_ARRAY_NUM(ioPinList); ++index)
    {
        if (nullptr != ioPinList[index])
        {
            ioPinList[index]->init();
        }
    }

    /* Disable buzzer */
    buzzerOut.write(LOW);
}

extern void Board::reset()
{
    ESP.restart();

    /* Will never be reached. */
}

extern void Board::ledOn()
{
    /* High active */
    onBoardLedOut.write(HIGH);
}

extern void Board::ledOff()
{
    /* High active */
    onBoardLedOut.write(LOW);
}

extern bool Board::isLedOn()
{
    return (HIGH == onBoardLedOut.read()) ? true : false;
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/
