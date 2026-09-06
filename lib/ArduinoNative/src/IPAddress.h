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
 * @file   IPAddress.h
 * @brief  IP address for test purposes only
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup TEST
 *
 * @{
 */

#ifndef IPADDRESS_H
#define IPADDRESS_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>

#include "WString.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * IPv4 address for test purposes only.
 */
class IPAddress
{
public:

    /**
     * Constructs a IP address 0.0.0.0.
     */
    IPAddress();

    /**
     * Constructs a IP address by its single octets.
     *
     * @param[in] octet1    1st octet
     * @param[in] octet2    2nd octet
     * @param[in] octet3    3rd octet
     * @param[in] octet4    4th octet
     */
    IPAddress(uint8_t octet1, uint8_t octet2, uint8_t octet3, uint8_t octet4);

    /**
     * Constructs a IP address by a 32-bit address in host byte order.
     *
     * @param[in] address   IP address
     */
    IPAddress(uint32_t address);

    /**
     * Constructs a IP address by copying another.
     *
     * @param[in] other IP address to copy
     */
    IPAddress(const IPAddress& other);

    /**
     * Destroys the IP address.
     */
    ~IPAddress();

    /**
     * Assign a IP address.
     *
     * @param[in] other IP address, which to assign.
     *
     * @return IP address
     */
    IPAddress& operator=(const IPAddress& other);

    /**
     * Assign a IP address by a 32-bit address in host byte order.
     *
     * @param[in] address   IP address, which to assign.
     *
     * @return IP address
     */
    IPAddress& operator=(uint32_t address);

    /**
     * Compare two IP addresses.
     *
     * @param[in] other IP address, which to compare with.
     *
     * @return If the addresses are equal, it will return true otherwise false.
     */
    bool operator==(const IPAddress& other) const;

    /**
     * Compare two IP addresses.
     *
     * @param[in] other IP address, which to compare with.
     *
     * @return If the addresses are different, it will return true otherwise false.
     */
    bool operator!=(const IPAddress& other) const;

    /**
     * Get a single octet of the IP address.
     *
     * @param[in] index Octet index [0; 3].
     *
     * @return Octet
     */
    uint8_t operator[](int index) const;

    /**
     * Get the IP address as 32-bit value in host byte order.
     *
     * @return IP address
     */
    operator uint32_t() const;

    /**
     * Get the IP address in the dotted decimal notation.
     *
     * @return IP address
     */
    String toString() const;

private:

    uint8_t m_octets[4U]; /**< The single octets of the IP address. */
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* IPADDRESS_H */

/** @} */
