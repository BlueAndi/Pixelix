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
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Board.h"

#include <Util.h>
#include <esp_int_wdt.h>
#include <esp_task_wdt.h>

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
 * Local Variables
 *****************************************************************************/

/** A list of all used i/o pins, used for intializaton. */
static const IoPin* ioPinList[] =
{
    &onBoardLedOut,
    &userButtonIn,
    &testPinOut,
    &ledMatrixDataOut,
    &ldrIn
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
    esp_task_wdt_init(1, true);
    esp_task_wdt_add(nullptr);
    
    for(;;)
    {
        /* Wait for reset. */
        ;
    }

    return;
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/
