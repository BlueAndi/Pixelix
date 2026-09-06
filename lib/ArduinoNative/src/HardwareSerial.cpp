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
 * @file   HardwareSerial.cpp
 * @brief  Serial interface for test purposes only
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "HardwareSerial.h"

#include <stdio.h>

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

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

HardwareSerial Serial;

/******************************************************************************
 * Public Methods
 *****************************************************************************/

HardwareSerial::HardwareSerial() :
    Stream()
{
}

HardwareSerial::~HardwareSerial()
{
}

void HardwareSerial::begin(unsigned long baudrate)
{
    /* There is no baudrate on the host. */
    (void)baudrate;

    /* A serial console shows every character immediately. The standard output
     * of the host is fully buffered as soon as it is redirected to a file or a
     * pipe, which would delay the log output. Therefore the buffering is
     * disabled.
     */
    (void)setvbuf(stdout, nullptr, _IONBF, 0U);
}

void HardwareSerial::end()
{
    flush();
}

void HardwareSerial::setTxTimeoutMs(uint32_t timeout)
{
    /* Not used on the host. */
    (void)timeout;
}

size_t HardwareSerial::write(uint8_t data)
{
    return write(&data, sizeof(data));
}

size_t HardwareSerial::write(const uint8_t* buffer, size_t size)
{
    size_t written = 0U;

    if (nullptr != buffer)
    {
        written = fwrite(buffer, 1U, size, stdout);
    }

    return written;
}

void HardwareSerial::flush()
{
    (void)fflush(stdout);
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
