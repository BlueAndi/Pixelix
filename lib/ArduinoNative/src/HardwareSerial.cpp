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
#include "ConsoleCompat.h"

#include <stdio.h>

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/** Serve the terminal input, which switches the terminal to the raw mode.
 * Disable it in case the raw mode disturbs e.g. a debugger or the terminal of
 * an IDE.
 */
#ifndef CONFIG_NATIVE_SERIAL_INPUT
#define CONFIG_NATIVE_SERIAL_INPUT (1)
#endif /* CONFIG_NATIVE_SERIAL_INPUT */

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
    Stream(),
    m_isInputEnabled(false),
    m_inputBuffer(),
    m_readIndex(0U),
    m_writeIndex(0U),
    m_inputCount(0U)
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

#if (0 != CONFIG_NATIVE_SERIAL_INPUT)

    /* The mini terminal echoes every character and handles the backspace on
     * its own, because the serial interface of the target does neither. The
     * terminal of the host is switched to the raw mode, so it behaves the same.
     */
    if ((false == m_isInputEnabled) &&
        (true == ConsoleCompat::isInteractive()))
    {
        m_isInputEnabled = ConsoleCompat::enterRawMode();
    }

#endif /* (0 != CONFIG_NATIVE_SERIAL_INPUT) */
}

void HardwareSerial::end()
{
    flush();

    if (true == m_isInputEnabled)
    {
        m_isInputEnabled = false;
        ConsoleCompat::restoreMode();
    }
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

int HardwareSerial::available()
{
    drainInput();

    return static_cast<int>(m_inputCount);
}

int HardwareSerial::read()
{
    int data = -1;

    drainInput();

    if (0U < m_inputCount)
    {
        data        = static_cast<int>(m_inputBuffer[m_readIndex]);
        m_readIndex = (m_readIndex + 1U) % INPUT_BUFFER_SIZE;
        --m_inputCount;
    }

    return data;
}

int HardwareSerial::peek()
{
    int data = -1;

    drainInput();

    if (0U < m_inputCount)
    {
        data = static_cast<int>(m_inputBuffer[m_readIndex]);
    }

    return data;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void HardwareSerial::drainInput()
{
    if (true == m_isInputEnabled)
    {
        while (INPUT_BUFFER_SIZE > m_inputCount)
        {
            int data = ConsoleCompat::readByte();

            if (0 > data)
            {
                break;
            }

            m_inputBuffer[m_writeIndex] = static_cast<uint8_t>(data);
            m_writeIndex                = (m_writeIndex + 1U) % INPUT_BUFFER_SIZE;
            ++m_inputCount;
        }
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
