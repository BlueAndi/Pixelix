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
 * @file   WiFiClient.h
 * @brief  TCP client for the native environment
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * Counterpart of the Arduino WiFiClient. In difference to the AsyncTCP of the
 * webserver, this one is blocking. The HTTPClient uses it that way.
 *
 * @addtogroup HAL_NATIVE
 *
 * @{
 */

#ifndef WIFICLIENT_H
#define WIFICLIENT_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <stddef.h>
#include <Stream.h>
#include <WString.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * TCP client, which uses the sockets of the host.
 */
class WiFiClient : public Stream
{
public:

    /**
     * Constructs a client, which is not connected yet.
     */
    WiFiClient();

    /**
     * Destroys the client and closes the connection.
     */
    virtual ~WiFiClient();

    /**
     * Connect to the given host.
     *
     * @param[in] host  Host name or address.
     * @param[in] port  Port of the host.
     *
     * @return If successful connected, it will return 1 otherwise 0.
     */
    virtual int connect(const char* host, uint16_t port);

    /**
     * Connect to the given host.
     *
     * @param[in] host  Host name or address.
     * @param[in] port  Port of the host.
     *
     * @return If successful connected, it will return 1 otherwise 0.
     */
    int connect(const String& host, uint16_t port)
    {
        return connect(host.c_str(), port);
    }

    /**
     * Is the client connected?
     *
     * @return If connected, it will return 1 otherwise 0.
     */
    virtual uint8_t connected();

    /**
     * Close the connection.
     */
    virtual void stop();

    /**
     * Get the number of bytes, which can be read without blocking.
     *
     * @return Number of available bytes.
     */
    int available() final;

    /**
     * Read a single byte.
     *
     * @return Byte or -1 if nothing is available.
     */
    int read() final;

    /**
     * Read the given number of bytes.
     *
     * @param[out] buffer   Data buffer
     * @param[in]  size     Data buffer size in byte
     *
     * @return Number of read bytes or -1 in case of an error.
     */
    virtual int read(uint8_t* buffer, size_t size);

    /**
     * Get the next byte without removing it from the receive buffer.
     *
     * @return Byte or -1 if nothing is available.
     */
    int peek() final;

    /**
     * Write a single byte.
     *
     * @param[in] data  Byte which to write.
     *
     * @return Number of written bytes.
     */
    size_t write(uint8_t data) final;

    /**
     * Write the given number of bytes.
     *
     * @param[in] buffer    Data buffer
     * @param[in] size      Data buffer size in byte
     *
     * @return Number of written bytes.
     */
    size_t write(const uint8_t* buffer, size_t size) final;

    /**
     * Wait until all data is sent. The socket sends it on its own, therefore
     * there is nothing to do.
     */
    void flush()
    {
        /* Nothing to do. */
    }

    /**
     * Set the max. time to wait for data resp. for a connection.
     *
     * @param[in] timeout   Timeout in ms.
     */
    void setTimeout(uint32_t timeout)
    {
        m_timeout = timeout;
    }

    /**
     * Get the max. time to wait for data resp. for a connection.
     *
     * @return Timeout in ms.
     */
    uint32_t getTimeout() const
    {
        return m_timeout;
    }

    /**
     * Is the client connected?
     *
     * @return If connected, it will return true otherwise false.
     */
    operator bool()
    {
        return (0U != connected());
    }

protected:

    /** Max. time to wait for data resp. for a connection in ms. */
    static const uint32_t DEFAULT_TIMEOUT = 5000U;

    int                   m_socket;  /**< Socket of the connection, -1 if not connected. */
    uint32_t              m_timeout; /**< Max. time to wait for data resp. for a connection in ms. */
    int                   m_peeked;  /**< Byte which was read by peek(), -1 if there is none. */

private:

    WiFiClient(const WiFiClient& client);
    WiFiClient& operator=(const WiFiClient& client);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* WIFICLIENT_H */

/** @} */
