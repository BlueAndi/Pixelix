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
 * @file   MD5Builder.cpp
 * @brief  MD5 for the native environment
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * Implementation according to RFC 1321.
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "MD5Builder.h"

#include <stdio.h>
#include <string.h>

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

static uint32_t rotateLeft(uint32_t value, uint32_t shift);

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/** Per round shift amounts, see RFC 1321. */
static const uint32_t SHIFTS[64U] = {
    7U, 12U, 17U, 22U, 7U, 12U, 17U, 22U, 7U, 12U, 17U, 22U, 7U, 12U, 17U, 22U, 5U, 9U, 14U, 20U, 5U, 9U, 14U, 20U, 5U, 9U, 14U, 20U, 5U, 9U, 14U, 20U, 4U, 11U, 16U, 23U, 4U, 11U, 16U, 23U, 4U, 11U, 16U, 23U, 4U, 11U, 16U, 23U, 6U, 10U, 15U, 21U, 6U, 10U, 15U, 21U, 6U, 10U, 15U, 21U, 6U, 10U, 15U, 21U
};

/** Binary integer part of the sines of integers, see RFC 1321. */
static const uint32_t SINES[64U] = {
    0xD76AA478U, 0xE8C7B756U, 0x242070DBU, 0xC1BDCEEEU, 0xF57C0FAFU, 0x4787C62AU, 0xA8304613U, 0xFD469501U, 0x698098D8U, 0x8B44F7AFU, 0xFFFF5BB1U, 0x895CD7BEU, 0x6B901122U, 0xFD987193U, 0xA679438EU, 0x49B40821U, 0xF61E2562U, 0xC040B340U, 0x265E5A51U, 0xE9B6C7AAU, 0xD62F105DU, 0x02441453U, 0xD8A1E681U, 0xE7D3FBC8U, 0x21E1CDE6U, 0xC33707D6U, 0xF4D50D87U, 0x455A14EDU, 0xA9E3E905U, 0xFCEFA3F8U, 0x676F02D9U, 0x8D2A4C8AU, 0xFFFA3942U, 0x8771F681U, 0x6D9D6122U, 0xFDE5380CU, 0xA4BEEA44U, 0x4BDECFA9U, 0xF6BB4B60U, 0xBEBFBC70U, 0x289B7EC6U, 0xEAA127FAU, 0xD4EF3085U, 0x04881D05U, 0xD9D4D039U, 0xE6DB99E5U, 0x1FA27CF8U, 0xC4AC5665U, 0xF4292244U, 0x432AFF97U, 0xAB9423A7U, 0xFC93A039U, 0x655B59C3U, 0x8F0CCC92U, 0xFFEFF47DU, 0x85845DD1U, 0x6FA87E4FU, 0xFE2CE6E0U, 0xA3014314U, 0x4E0811A1U, 0xF7537E82U, 0xBD3AF235U, 0x2AD7D2BBU, 0xEB86D391U
};

/******************************************************************************
 * Public Methods
 *****************************************************************************/

MD5Builder::MD5Builder() :
    m_state{ 0U },
    m_count{ 0U },
    m_buffer{ 0U },
    m_digest{ 0U }
{
    begin();
}

MD5Builder::~MD5Builder()
{
}

void MD5Builder::begin()
{
    m_count[0U] = 0U;
    m_count[1U] = 0U;

    /* Initial digest state, see RFC 1321. */
    m_state[0U] = 0x67452301U;
    m_state[1U] = 0xEFCDAB89U;
    m_state[2U] = 0x98BADCFEU;
    m_state[3U] = 0x10325476U;

    memset(m_digest, 0, sizeof(m_digest));
}

void MD5Builder::add(const uint8_t* data, uint16_t len)
{
    if (nullptr != data)
    {
        size_t   index    = (m_count[0U] >> 3U) % BLOCK_SIZE;
        uint32_t lenBits  = static_cast<uint32_t>(len) << 3U;
        size_t   partLen  = BLOCK_SIZE - index;
        size_t   offset   = 0U;

        m_count[0U]      += lenBits;

        if (m_count[0U] < lenBits)
        {
            ++m_count[1U];
        }

        m_count[1U] += static_cast<uint32_t>(len) >> 29U;

        if (len >= partLen)
        {
            memcpy(&m_buffer[index], data, partLen);
            processBlock(m_buffer);

            for (offset = partLen; (offset + BLOCK_SIZE) <= len; offset += BLOCK_SIZE)
            {
                processBlock(&data[offset]);
            }

            index = 0U;
        }

        if (offset < len)
        {
            memcpy(&m_buffer[index], &data[offset], len - offset);
        }
    }
}

void MD5Builder::add(const char* data)
{
    if (nullptr != data)
    {
        add(reinterpret_cast<const uint8_t*>(data), static_cast<uint16_t>(strlen(data)));
    }
}

void MD5Builder::add(const String& data)
{
    add(data.c_str());
}

void MD5Builder::calculate()
{
    uint8_t  padding[BLOCK_SIZE] = { 0U };
    uint8_t  lengthBits[8U];
    size_t   index  = (m_count[0U] >> 3U) % BLOCK_SIZE;
    size_t   padLen = (56U > index) ? (56U - index) : (120U - index);
    uint32_t idx    = 0U;

    /* The message length in bit as 64-bit little endian value. */
    for (idx = 0U; idx < 4U; ++idx)
    {
        lengthBits[idx]      = static_cast<uint8_t>(m_count[0U] >> (8U * idx));
        lengthBits[idx + 4U] = static_cast<uint8_t>(m_count[1U] >> (8U * idx));
    }

    padding[0U] = 0x80U;

    add(padding, static_cast<uint16_t>(padLen));
    add(lengthBits, sizeof(lengthBits));

    /* The digest as little endian value. */
    for (idx = 0U; idx < 4U; ++idx)
    {
        m_digest[(idx * 4U) + 0U] = static_cast<uint8_t>(m_state[idx] >> 0U);
        m_digest[(idx * 4U) + 1U] = static_cast<uint8_t>(m_state[idx] >> 8U);
        m_digest[(idx * 4U) + 2U] = static_cast<uint8_t>(m_state[idx] >> 16U);
        m_digest[(idx * 4U) + 3U] = static_cast<uint8_t>(m_state[idx] >> 24U);
    }
}

void MD5Builder::getChars(char* output)
{
    if (nullptr != output)
    {
        size_t idx = 0U;

        for (idx = 0U; idx < DIGEST_SIZE; ++idx)
        {
            (void)snprintf(&output[idx * 2U], 3U, "%02x", m_digest[idx]);
        }

        output[DIGEST_SIZE * 2U] = '\0';
    }
}

String MD5Builder::toString()
{
    char buffer[(DIGEST_SIZE * 2U) + 1U];

    getChars(buffer);

    return String(buffer);
}

void MD5Builder::getBytes(uint8_t* output)
{
    if (nullptr != output)
    {
        memcpy(output, m_digest, DIGEST_SIZE);
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void MD5Builder::processBlock(const uint8_t block[64U])
{
    uint32_t work[16U];
    uint32_t a   = m_state[0U];
    uint32_t b   = m_state[1U];
    uint32_t c   = m_state[2U];
    uint32_t d   = m_state[3U];
    uint32_t idx = 0U;

    /* The data block as little endian 32-bit words. */
    for (idx = 0U; idx < 16U; ++idx)
    {
        work[idx] = (static_cast<uint32_t>(block[(idx * 4U) + 0U]) << 0U) |
                    (static_cast<uint32_t>(block[(idx * 4U) + 1U]) << 8U) |
                    (static_cast<uint32_t>(block[(idx * 4U) + 2U]) << 16U) |
                    (static_cast<uint32_t>(block[(idx * 4U) + 3U]) << 24U);
    }

    for (idx = 0U; idx < 64U; ++idx)
    {
        uint32_t func = 0U;
        uint32_t g    = 0U;
        uint32_t temp = 0U;

        if (16U > idx)
        {
            func = (b & c) | ((~b) & d);
            g    = idx;
        }
        else if (32U > idx)
        {
            func = (d & b) | ((~d) & c);
            g    = ((5U * idx) + 1U) % 16U;
        }
        else if (48U > idx)
        {
            func = b ^ c ^ d;
            g    = ((3U * idx) + 5U) % 16U;
        }
        else
        {
            func = c ^ (b | (~d));
            g    = (7U * idx) % 16U;
        }

        temp = d;
        d    = c;
        c    = b;
        b    = b + rotateLeft(a + func + SINES[idx] + work[g], SHIFTS[idx]);
        a    = temp;
    }

    m_state[0U] += a;
    m_state[1U] += b;
    m_state[2U] += c;
    m_state[3U] += d;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/**
 * Rotate the given value to the left.
 *
 * @param[in] value Value which to rotate.
 * @param[in] shift Number of bits to rotate.
 *
 * @return Rotated value
 */
static uint32_t rotateLeft(uint32_t value, uint32_t shift)
{
    return (value << shift) | (value >> (32U - shift));
}
