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
 * The serial interface of the host is the standard output.
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
     * @param[in] baudrate  Baudrate in bps.
     */
    void begin(unsigned long baudrate);

    /**
     * Stop the serial interface.
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
     * Get the number of bytes which are available to read.
     * The serial input is not used on the host.
     *
     * @return Always 0.
     */
    int available() final
    {
        return 0;
    }

    /**
     * Read a single byte.
     * The serial input is not used on the host.
     *
     * @return Always -1.
     */
    int read() final
    {
        return -1;
    }

    /**
     * Get the next byte without removing it.
     * The serial input is not used on the host.
     *
     * @return Always -1.
     */
    int peek() final
    {
        return -1;
    }

private:

    HardwareSerial(const HardwareSerial& serial);
    HardwareSerial& operator=(const HardwareSerial& serial);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

/** The serial interface, like the Arduino provides it. */
extern HardwareSerial Serial;

#endif /* HARDWARE_SERIAL_H */

/** @} */
