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
 * @file   Client.h
 * @brief  Network client interface for the native environment
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * Counterpart of the Arduino Client. It is the abstraction every protocol
 * library expects, e.g. the MQTT client gets a Client reference and doesn't
 * care whether there is a plain socket or TLS behind it.
 *
 * @addtogroup TEST
 *
 * @{
 */

#ifndef CLIENT_H
#define CLIENT_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <stddef.h>

#include "IPAddress.h"
#include "Stream.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * A client is a stream, which is connected to a remote peer.
 */
class Client : public Stream
{
public:

    /**
     * Constructs a client.
     */
    Client() :
        Stream()
    {
    }

    /**
     * Destroys a client.
     */
    virtual ~Client()
    {
    }

    /**
     * Connect to the given host.
     *
     * @param[in] ip    Host address.
     * @param[in] port  Port of the host.
     *
     * @return If successful connected, it will return 1 otherwise 0.
     */
    virtual int connect(IPAddress ip, uint16_t port)         = 0;

    /**
     * Connect to the given host.
     *
     * @param[in] host  Host name or address.
     * @param[in] port  Port of the host.
     *
     * @return If successful connected, it will return 1 otherwise 0.
     */
    virtual int connect(const char* host, uint16_t port)     = 0;

    /**
     * Write a single byte.
     *
     * @param[in] data  Byte which to write.
     *
     * @return Number of written bytes.
     */
    virtual size_t write(uint8_t data)                       = 0;

    /**
     * Write the given number of bytes.
     *
     * @param[in] buffer    Data buffer
     * @param[in] size      Data buffer size in byte
     *
     * @return Number of written bytes.
     */
    virtual size_t write(const uint8_t* buffer, size_t size) = 0;

    /**
     * Get the number of bytes, which can be read without blocking.
     *
     * @return Number of available bytes.
     */
    virtual int available()                                  = 0;

    /**
     * Read a single byte.
     *
     * @return Byte or -1 if nothing is available.
     */
    virtual int read()                                       = 0;

    /**
     * Read the given number of bytes.
     *
     * @param[out] buffer   Data buffer
     * @param[in]  size     Data buffer size in byte
     *
     * @return Number of read bytes or -1 in case of an error.
     */
    virtual int read(uint8_t* buffer, size_t size)           = 0;

    /**
     * Get the next byte without removing it from the receive buffer.
     *
     * @return Byte or -1 if nothing is available.
     */
    virtual int peek()                                       = 0;

    /**
     * Wait until all data is sent.
     */
    virtual void flush()                                     = 0;

    /**
     * Close the connection.
     */
    virtual void stop()                                      = 0;

    /**
     * Is the client connected?
     *
     * @return If connected, it will return 1 otherwise 0.
     */
    virtual uint8_t connected()                              = 0;

    /**
     * Is the client connected?
     *
     * @return If connected, it will return true otherwise false.
     */
    virtual operator bool()                                  = 0;
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* CLIENT_H */

/** @} */
