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

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "GPIODrv.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/** Get number of array elements. */
#define ARRAY_NUM(__arr)    (sizeof(__arr) / sizeof((__arr)[0]))

/******************************************************************************
 * Types and classes
 *****************************************************************************/

/** This type defines a pin configuration. */
typedef struct
{
    GPIODrv::PinNo  pinNo;  /**< Arduino pin number */
    uint8_t         mode;   /**< Pin mode */

} PinConfig;

/******************************************************************************
 * Prototypes
 *****************************************************************************/

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/* Initialize GPIO driver instance. */
GPIODrv GPIODrv::m_instance;

/** Pin configuration */
static const PinConfig  pinConfiguration[] =
{
    { GPIODrv::PINNO_ONBOARD_LED,           OUTPUT       },
    { GPIODrv::PINNO_USER_BUTTON,           INPUT_PULLUP },
    { GPIODrv::PINNO_LED_MATRIX_DATA_OUT,   OUTPUT       }
};

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void GPIODrv::init()
{
    uint8_t index = 0u;

    /* Initialize pin configuration */
    for(index = 0u; index < ARRAY_NUM(pinConfiguration); ++index)
    {
        pinMode(pinConfiguration[index].pinNo, pinConfiguration[index].mode);
    }

    return;
}

GPIODrv::Level GPIODrv::getUserButtonState()
{
    Level   level = LEVEL_LOW;

    if (LOW != digitalRead(PINNO_USER_BUTTON))
    {
        level = LEVEL_HIGH;
    }

    return level;
}

void GPIODrv::setOnboardLED(bool enable)
{
    if (false == enable)
    {
        digitalWrite(PINNO_ONBOARD_LED, HIGH);
    }
    else
    {
        digitalWrite(PINNO_ONBOARD_LED, LOW);
    }

    return;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
