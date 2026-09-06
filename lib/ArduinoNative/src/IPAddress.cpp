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
 * @file   IPAddress.cpp
 * @brief  IP address for test purposes only
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "IPAddress.h"

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and classes
 *****************************************************************************/

/******************************************************************************
 * Prototypes
 *****************************************************************************/

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/** Number of octets of a IPv4 address. */
static const int OCTET_COUNT = 4;

/******************************************************************************
 * Public Methods
 *****************************************************************************/

IPAddress::IPAddress() :
    m_octets{ 0U, 0U, 0U, 0U }
{
}

IPAddress::IPAddress(uint8_t octet1, uint8_t octet2, uint8_t octet3, uint8_t octet4) :
    m_octets{ octet1, octet2, octet3, octet4 }
{
}

IPAddress::IPAddress(uint32_t address) :
    m_octets{ 0U, 0U, 0U, 0U }
{
    *this = address;
}

IPAddress::IPAddress(const IPAddress& other) :
    m_octets{ other.m_octets[0U], other.m_octets[1U], other.m_octets[2U], other.m_octets[3U] }
{
}

IPAddress::~IPAddress()
{
}

IPAddress& IPAddress::operator=(const IPAddress& other)
{
    if (this != &other)
    {
        int index = 0;

        for (index = 0; index < OCTET_COUNT; ++index)
        {
            m_octets[index] = other.m_octets[index];
        }
    }

    return *this;
}

IPAddress& IPAddress::operator=(uint32_t address)
{
    /* The 1st octet is the most significant byte, like the dotted decimal
     * notation shows it.
     */
    m_octets[0U] = static_cast<uint8_t>((address >> 24U) & 0xFFU);
    m_octets[1U] = static_cast<uint8_t>((address >> 16U) & 0xFFU);
    m_octets[2U] = static_cast<uint8_t>((address >> 8U) & 0xFFU);
    m_octets[3U] = static_cast<uint8_t>((address >> 0U) & 0xFFU);

    return *this;
}

bool IPAddress::operator==(const IPAddress& other) const
{
    bool isEqual = true;
    int  index   = 0;

    for (index = 0; index < OCTET_COUNT; ++index)
    {
        if (m_octets[index] != other.m_octets[index])
        {
            isEqual = false;
            break;
        }
    }

    return isEqual;
}

bool IPAddress::operator!=(const IPAddress& other) const
{
    return (false == (*this == other));
}

uint8_t IPAddress::operator[](int index) const
{
    uint8_t octet = 0U;

    if ((0 <= index) &&
        (OCTET_COUNT > index))
    {
        octet = m_octets[index];
    }

    return octet;
}

IPAddress::operator uint32_t() const
{
    uint32_t address  = 0U;

    address          |= static_cast<uint32_t>(m_octets[0U]) << 24U;
    address          |= static_cast<uint32_t>(m_octets[1U]) << 16U;
    address          |= static_cast<uint32_t>(m_octets[2U]) << 8U;
    address          |= static_cast<uint32_t>(m_octets[3U]) << 0U;

    return address;
}

String IPAddress::toString() const
{
    String result;

    result += static_cast<int>(m_octets[0U]);
    result += ".";
    result += static_cast<int>(m_octets[1U]);
    result += ".";
    result += static_cast<int>(m_octets[2U]);
    result += ".";
    result += static_cast<int>(m_octets[3U]);

    return result;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
