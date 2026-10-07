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
 * @file   Wire.h
 * @brief  Stub for the two-wire (I2C) interface
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * There is no two-wire interface on the host. Every device on the bus, like the
 * RTC or a sensor, is unavailable.
 *
 * @addtogroup TEST
 *
 * @{
 */

#ifndef WIRE_H
#define WIRE_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <stddef.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Two-wire (I2C) interface stub.
 */
class TwoWire
{
public:

    /**
     * Constructs the two-wire interface.
     */
    TwoWire()
    {
    }

    /**
     * Destroys the two-wire interface.
     */
    ~TwoWire()
    {
    }

    /**
     * Set the pins of the two-wire interface.
     *
     * @param[in] sdaPinNo  Pin number of the data line.
     * @param[in] sclPinNo  Pin number of the clock line.
     *
     * @return Always true, there is no two-wire interface on the host.
     */
    bool setPins(int sdaPinNo, int sclPinNo)
    {
        (void)sdaPinNo;
        (void)sclPinNo;

        return true;
    }

    /**
     * Start the two-wire interface.
     *
     * @return Always true, there is no two-wire interface on the host.
     */
    bool begin()
    {
        return true;
    }

    /**
     * Stop the two-wire interface.
     *
     * @return Always true, there is no two-wire interface on the host.
     */
    bool end()
    {
        return true;
    }

    /**
     * Begin a transmission to the device with the given address.
     *
     * @param[in] address   Device address
     */
    void beginTransmission(uint8_t address)
    {
        (void)address;
    }

    /**
     * End the transmission.
     *
     * @return Always 2, which means the device is not available.
     */
    uint8_t endTransmission()
    {
        /* 2 means: received NACK on transmit of address. */
        return 2U;
    }

    /**
     * Request data from the device with the given address.
     *
     * @param[in] address   Device address
     * @param[in] size      Number of requested bytes.
     *
     * @return Always 0, there is no device on the host.
     */
    size_t requestFrom(uint8_t address, size_t size)
    {
        (void)address;
        (void)size;

        return 0U;
    }

    /**
     * Write a single byte.
     *
     * @param[in] data  Data byte
     *
     * @return Always 0, there is no device on the host.
     */
    size_t write(uint8_t data)
    {
        (void)data;

        return 0U;
    }

    /**
     * Get the number of available bytes.
     *
     * @return Always 0, there is no device on the host.
     */
    int available()
    {
        return 0;
    }

    /**
     * Read a single byte.
     *
     * @return Always -1, there is no device on the host.
     */
    int read()
    {
        return -1;
    }

private:

    TwoWire(const TwoWire& wire);
    TwoWire& operator=(const TwoWire& wire);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

/** The two-wire interface, like the Arduino provides it. */
extern TwoWire Wire;

#endif /* WIRE_H */

/** @} */
