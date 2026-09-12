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
 * @file   DNSServer.h
 * @brief  Stub for the DNS server
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * The DNS server is used for the captive portal in wifi access point mode.
 * There is no access point on the host, therefore it does nothing.
 *
 * @addtogroup TEST
 *
 * @{
 */

#ifndef DNS_SERVER_H
#define DNS_SERVER_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <Arduino.h>
#include <IPAddress.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** DNS reply code, compatible to the Arduino DNSServer. */
enum class DNSReplyCode
{
    NoError           = 0, /**< No error */
    FormError         = 1, /**< Format error */
    ServerFailure     = 2, /**< Server failure */
    NonExistentDomain = 3, /**< Domain doesn't exist */
    NotImplemented    = 4, /**< Not implemented */
    Refused           = 5, /**< Refused */
    YXDomain          = 6, /**< Name exists, but it shall not */
    YXRRSet           = 7, /**< Resource record set exists, but it shall not */
    NXRRSet           = 8  /**< Resource record set doesn't exist */
};

/**
 * DNS server stub.
 */
class DNSServer
{
public:

    /**
     * Constructs the DNS server.
     */
    DNSServer()
    {
    }

    /**
     * Destroys the DNS server.
     */
    ~DNSServer()
    {
    }

    /**
     * Start the DNS server.
     *
     * @param[in] port      Port number
     * @param[in] domain    Domain name, which to resolve.
     * @param[in] address   IP address, which every request is resolved to.
     *
     * @return Always true, there is no DNS server on the host.
     */
    bool start(uint16_t port, const String& domain, const IPAddress& address)
    {
        (void)port;
        (void)domain;
        (void)address;

        return true;
    }

    /**
     * Stop the DNS server.
     */
    void stop()
    {
    }

    /**
     * Set the reply code, used for requests which can not be resolved.
     *
     * @param[in] replyCode Reply code
     */
    void setErrorReplyCode(const DNSReplyCode& replyCode)
    {
        (void)replyCode;
    }

    /**
     * Process the next request.
     */
    void processNextRequest()
    {
    }

private:

    DNSServer(const DNSServer& server);
    DNSServer& operator=(const DNSServer& server);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* DNS_SERVER_H */

/** @} */
