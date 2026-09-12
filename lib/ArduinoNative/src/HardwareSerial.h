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
 * @file   HardwareSerial.h
 * @brief  Serial interface for test purposes only
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * The serial interface of the host is the standard output and the standard
 * input. The input is only served if the standard input is connected to a
 * terminal, which is used interactively. It is switched to the raw mode, so
 * the characters arrive one by one and without echo, like the serial interface
 * of the target delivers them. See ConsoleCompat.h for the details.
 *
 * @addtogroup TEST
 *
 * @{
 */

#ifndef HARDWARE_SERIAL_H
#define HARDWARE_SERIAL_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <stddef.h>

#include "Stream.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Serial interface for test purposes only.
 */
class HardwareSerial : public Stream
{
public:

    /**
     * Constructs the serial interface.
     */
    HardwareSerial();

    /**
     * Destroys the serial interface.
     */
    ~HardwareSerial();

    /**
     * Start the serial interface.
     * The baudrate is not used on the host.
     *
     * If the standard input is connected to a terminal, which can be used
     * interactively, it is switched to the raw mode. Otherwise the serial
     * input stays empty for the whole runtime.
     *
     * @param[in] baudrate  Baudrate in bps.
     */
    void begin(unsigned long baudrate);

    /**
     * Stop the serial interface.
     * The terminal mode is restored, if it was changed.
     */
    void end();

    /**
     * Set the timeout for a transmission.
     * Not used on the host, but part of the interface of the ESP32 USB CDC.
     *
     * @param[in] timeout   Timeout in ms.
     */
    void setTxTimeoutMs(uint32_t timeout);

    /**
     * Write a single byte to the standard output.
     *
     * @param[in] data  Data byte
     *
     * @return Number of written data bytes.
     */
    size_t write(uint8_t data) final;

    /**
     * Write several data bytes to the standard output.
     *
     * @param[in] buffer    Data buffer
     * @param[in] size      Data buffer size in byte
     *
     * @return Number of written data bytes.
     */
    size_t write(const uint8_t* buffer, size_t size) final;

    /**
     * Wait until everything is written out.
     */
    void flush();

    /**
     * Get the number of bytes which are available to read from the terminal.
     * It is always 0 if the terminal input is not served.
     *
     * @return Number of available bytes.
     */
    int available() final;

    /**
     * Read a single byte from the terminal.
     * It never blocks and it is always -1 if the terminal input is not served.
     *
     * @return Data byte or -1 if nothing is available.
     */
    int read() final;

    /**
     * Get the next byte from the terminal without removing it.
     * It never blocks and it is always -1 if the terminal input is not served.
     *
     * @return Data byte or -1 if nothing is available.
     */
    int peek() final;

private:

    /** Size of the terminal input ring buffer in byte. */
    static const size_t INPUT_BUFFER_SIZE = 64U;

    bool                m_isInputEnabled;                 /**< Is the terminal input served?                   */
    uint8_t             m_inputBuffer[INPUT_BUFFER_SIZE]; /**< Ring buffer with the read terminal input.       */
    size_t              m_readIndex;                      /**< Read index of the input ring buffer.            */
    size_t              m_writeIndex;                     /**< Write index of the input ring buffer.           */
    size_t              m_inputCount;                     /**< Number of bytes inside the input ring buffer.   */

    HardwareSerial(const HardwareSerial& serial);
    HardwareSerial& operator=(const HardwareSerial& serial);

    /**
     * Move every available byte from the terminal to the input ring buffer.
     * It stops as soon as the terminal has nothing anymore or the ring buffer
     * is full. It does nothing if the terminal input is not served.
     */
    void drainInput();
};

/******************************************************************************
 * Functions
 *****************************************************************************/

/** The serial interface, like the Arduino provides it. */
extern HardwareSerial Serial;

#endif /* HARDWARE_SERIAL_H */

/** @} */
