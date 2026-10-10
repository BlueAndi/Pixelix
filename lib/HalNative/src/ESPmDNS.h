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
 * @file   ESPmDNS.h
 * @brief  Stub for the multicast DNS
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * The webserver is reached by localhost on the host, therefore no service
 * discovery is provided.
 *
 * @addtogroup TEST
 *
 * @{
 */

#ifndef ESP_MDNS_H
#define ESP_MDNS_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <Arduino.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Multicast DNS responder stub.
 */
class MDNSResponder
{
public:

    /**
     * Constructs the multicast DNS responder.
     */
    MDNSResponder()
    {
    }

    /**
     * Destroys the multicast DNS responder.
     */
    ~MDNSResponder()
    {
    }

    /**
     * Start the responder with the given hostname.
     *
     * @param[in] hostname  Hostname
     *
     * @return Always true, there is no service discovery on the host.
     */
    bool begin(const char* hostname)
    {
        (void)hostname;

        return true;
    }

    /**
     * Stop the responder.
     */
    void end()
    {
    }

    /**
     * Announce a service.
     *
     * @param[in] service   Service name
     * @param[in] protocol  Protocol name
     * @param[in] port      Port number
     *
     * @return Always true, there is no service discovery on the host.
     */
    bool addService(const char* service, const char* protocol, uint16_t port)
    {
        (void)service;
        (void)protocol;
        (void)port;

        return true;
    }

private:

    MDNSResponder(const MDNSResponder& responder);
    MDNSResponder& operator=(const MDNSResponder& responder);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

/** The multicast DNS responder, like the Arduino provides it. */
extern MDNSResponder MDNS;

#endif /* ESP_MDNS_H */

/** @} */
