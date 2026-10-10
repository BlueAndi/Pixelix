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
 * @file   Stream.h
 * @brief  Stream for test purposes only
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup TEST
 *
 * @{
 */

#ifndef STREAM_H
#define STREAM_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>

#include "Print.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * A stream is a bidirectional interface, which can be written and read.
 */
class Stream : public Print
{
public:

    /**
     * Constructs a stream.
     */
    Stream() :
        Print()
    {
    }

    /**
     * Destroys a stream.
     */
    virtual ~Stream()
    {
    }

    /**
     * Get the number of bytes which are available to read.
     *
     * @return Number of available bytes.
     */
    virtual int available() = 0;

    /**
     * Read a single byte.
     *
     * @return Data byte or -1 if nothing is available.
     */
    virtual int read()      = 0;

    /**
     * Get the next byte without removing it.
     *
     * @return Data byte or -1 if nothing is available.
     */
    virtual int peek()      = 0;

    /**
     * Set the timeout for reading.
     *
     * @param[in] timeout   Timeout in ms.
     */
    void setTimeout(unsigned long timeout)
    {
        m_timeout = timeout;
    }

    /**
     * Get the timeout for reading.
     *
     * @return Timeout in ms.
     */
    unsigned long getTimeout() const
    {
        return m_timeout;
    }

    /**
     * Read the given number of bytes.
     * It stops as soon as nothing is available anymore.
     *
     * @param[out] buffer   Data buffer
     * @param[in] length    Number of bytes to read.
     *
     * @return Number of read bytes.
     */
    size_t readBytes(char* buffer, size_t length)
    {
        size_t count = 0U;

        if (nullptr != buffer)
        {
            while (count < length)
            {
                int data = read();

                if (0 > data)
                {
                    break;
                }

                buffer[count] = static_cast<char>(data);
                ++count;
            }
        }

        return count;
    }

    /**
     * Read the given number of bytes.
     * It stops as soon as nothing is available anymore.
     *
     * @param[out] buffer   Data buffer
     * @param[in] length    Number of bytes to read.
     *
     * @return Number of read bytes.
     */
    size_t readBytes(uint8_t* buffer, size_t length)
    {
        return readBytes(reinterpret_cast<char*>(buffer), length);
    }

private:

    unsigned long m_timeout = 1000U; /**< Timeout for reading in ms. */
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* STREAM_H */

/** @} */
