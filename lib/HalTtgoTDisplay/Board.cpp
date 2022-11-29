/* MIT License
 *
 * Copyright (c) 2019 - 2022 Andreas Merkle <web@blue-andi.de>
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
const DOutPin<Pin::onBoardLedPinNo>              Board::onBoardLedOut;

/** Digital input pin: User button (input with pull-up) */
const DInPin<Pin::userButtonPinNo, INPUT_PULLUP> Board::userButtonIn;

/** Analog input pin: LDR in */
const AnalogPin<Pin::ldrInPinNo>                 Board::ldrIn;

/** Digital input pin: DHT Sensor (input with pull-up) */
const DInPin<Pin::dhtInPinNo, INPUT_PULLUP>      Board::dhtIn;

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/** A list of all used i/o pins, used for initialization. */
static const IoPin* ioPinList[] =
{
    &onBoardLedOut,
    &userButtonIn,
    &ldrIn,
    &dhtIn
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

    return;
}

extern void Board::reset()
{
    ESP.restart();

    /* Will never be reached. */
    return;
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/
