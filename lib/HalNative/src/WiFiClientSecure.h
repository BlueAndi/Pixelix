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
 * @file   WiFiClientSecure.h
 * @brief  TCP client with TLS for the native environment
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * Counterpart of the Arduino WiFiClientSecure. The host has no TLS stack
 * available, therefore a connection attempt fails. Everything which uses plain
 * HTTP works, only HTTPS does not. See the note in connect().
 *
 * @addtogroup HAL_NATIVE
 *
 * @{
 */

#ifndef WIFICLIENTSECURE_H
#define WIFICLIENTSECURE_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include "WiFiClient.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * TCP client with TLS, which is not supported on the host.
 */
class WiFiClientSecure : public WiFiClient
{
public:

    /**
     * Constructs a client, which is not connected yet.
     */
    WiFiClientSecure() :
        WiFiClient()
    {
    }

    /**
     * Destroys the client and closes the connection.
     */
    ~WiFiClientSecure()
    {
    }

    /**
     * Connect to the given host.
     *
     * The host has no TLS stack, therefore it always fails. The reason is
     * logged once, so the user knows why a HTTPS request doesn't work.
     *
     * @param[in] host  Host name or address.
     * @param[in] port  Port of the host.
     *
     * @return Always 0, the connection can not be established.
     */
    int connect(const char* host, uint16_t port) final;

    /**
     * Don't verify the certificate of the server.
     *
     * There is no TLS at all, therefore there is nothing to configure.
     */
    void setInsecure()
    {
        /* Nothing to do. */
    }

    /**
     * Set the certificate of the certificate authority.
     *
     * There is no TLS at all, therefore there is nothing to configure.
     *
     * @param[in] rootCA    Certificate in PEM format.
     */
    void setCACert(const char* rootCA)
    {
        (void)rootCA;
    }

private:

    WiFiClientSecure(const WiFiClientSecure& client);
    WiFiClientSecure& operator=(const WiFiClientSecure& client);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* WIFICLIENTSECURE_H */

/** @} */
