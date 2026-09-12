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
 * @file   base64.cpp
 * @brief  Base64 encoding/decoding for the native environment
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * Implementation according to RFC 4648.
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "mbedtls/base64.h"

#include <stdint.h>

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

static int16_t base64Value(uint8_t ch);
static int     base64Validate(const unsigned char* src, size_t slen, size_t* symbolCount, size_t* padCount);
static size_t  base64Decode(unsigned char* dst, const unsigned char* src, size_t slen);

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/** The base64 alphabet, see RFC 4648 section 4. */
static const char BASE64_ALPHABET[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/** Value of a character, which is a padding. */
static const int16_t BASE64_PADDING = 64;

/** Value of a character, which is not part of the base64 alphabet. */
static const int16_t BASE64_INVALID = -1;

/** Max. number of padding characters at the end of the encoded data. */
static const size_t MAX_PADDING     = 2U;

/******************************************************************************
 * Public Methods
 *****************************************************************************/

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

extern "C" int mbedtls_base64_encode(unsigned char* dst, size_t dlen, size_t* olen, const unsigned char* src, size_t slen)
{
    int     result       = 0;
    size_t  dummy        = 0U;
    size_t* outLen       = (nullptr == olen) ? &dummy : olen;
    size_t  requiredSize = 0U;

    if ((nullptr == src) || (0U == slen))
    {
        *outLen = 0U;
    }
    else
    {
        /* Every 3 source bytes become 4 destination bytes, a incomplete group
         * is padded. One byte is added for the terminating zero.
         */
        requiredSize = (((slen + 2U) / 3U) * 4U) + 1U;

        if ((nullptr == dst) || (dlen < requiredSize))
        {
            *outLen = requiredSize;
            result  = MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL;
        }
        else
        {
            size_t srcIndex = 0U;
            size_t dstIndex = 0U;

            while (srcIndex < slen)
            {
                uint32_t group     = 0U;
                size_t   available = slen - srcIndex;
                size_t   byteCount = (3U < available) ? 3U : available;
                size_t   byteIndex = 0U;

                while (byteIndex < byteCount)
                {
                    group |= static_cast<uint32_t>(src[srcIndex + byteIndex]) << (16U - (8U * byteIndex));
                    ++byteIndex;
                }

                dst[dstIndex] = static_cast<unsigned char>(BASE64_ALPHABET[(group >> 18U) & 0x3FU]);
                ++dstIndex;
                dst[dstIndex] = static_cast<unsigned char>(BASE64_ALPHABET[(group >> 12U) & 0x3FU]);
                ++dstIndex;
                dst[dstIndex] = (1U < byteCount) ? static_cast<unsigned char>(BASE64_ALPHABET[(group >> 6U) & 0x3FU]) : static_cast<unsigned char>('=');
                ++dstIndex;
                dst[dstIndex] = (2U < byteCount) ? static_cast<unsigned char>(BASE64_ALPHABET[group & 0x3FU]) : static_cast<unsigned char>('=');
                ++dstIndex;

                srcIndex += byteCount;
            }

            dst[dstIndex] = static_cast<unsigned char>('\0');
            *outLen       = dstIndex;
        }
    }

    return result;
}

extern "C" int mbedtls_base64_decode(unsigned char* dst, size_t dlen, size_t* olen, const unsigned char* src, size_t slen)
{
    int     result      = 0;
    size_t  dummy       = 0U;
    size_t* outLen      = (nullptr == olen) ? &dummy : olen;
    size_t  symbolCount = 0U;
    size_t  padCount    = 0U;

    if ((nullptr == src) || (0U == slen))
    {
        *outLen = 0U;
    }
    else
    {
        result = base64Validate(src, slen, &symbolCount, &padCount);

        if (0 != result)
        {
            /* Invalid character, nothing to report about the size. */
        }
        else if (0U == symbolCount)
        {
            *outLen = 0U;
        }
        else
        {
            /* Every symbol carries 6 bit, the padding is not part of the
             * decoded data.
             */
            size_t requiredSize = (((symbolCount * 6U) + 7U) / 8U) - padCount;

            if ((nullptr == dst) || (dlen < requiredSize))
            {
                *outLen = requiredSize;
                result  = MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL;
            }
            else
            {
                *outLen = base64Decode(dst, src, slen);
            }
        }
    }

    return result;
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/**
 * Get the value of a base64 character.
 *
 * @param[in] ch    Character of the encoded data.
 *
 * @return Value 0 - 63, BASE64_PADDING for the padding character or
 *         BASE64_INVALID if the character is not part of the alphabet.
 */
static int16_t base64Value(uint8_t ch)
{
    int16_t value = BASE64_INVALID;

    if ((static_cast<uint8_t>('A') <= ch) && (static_cast<uint8_t>('Z') >= ch))
    {
        value = static_cast<int16_t>(ch - static_cast<uint8_t>('A'));
    }
    else if ((static_cast<uint8_t>('a') <= ch) && (static_cast<uint8_t>('z') >= ch))
    {
        value = static_cast<int16_t>(ch - static_cast<uint8_t>('a') + 26);
    }
    else if ((static_cast<uint8_t>('0') <= ch) && (static_cast<uint8_t>('9') >= ch))
    {
        value = static_cast<int16_t>(ch - static_cast<uint8_t>('0') + 52);
    }
    else if (static_cast<uint8_t>('+') == ch)
    {
        value = 62;
    }
    else if (static_cast<uint8_t>('/') == ch)
    {
        value = 63;
    }
    else if (static_cast<uint8_t>('=') == ch)
    {
        value = BASE64_PADDING;
    }
    else
    {
        /* Not part of the alphabet, it is either a whitespace or invalid. */
    }

    return value;
}

/**
 * Walk through the encoded data and check it for validity.
 *
 * Whitespaces are skipped, like the mbedTLS counterpart does.
 *
 * @param[in]  src          Source buffer, which to check.
 * @param[in]  slen         Source buffer size in byte.
 * @param[out] symbolCount  Number of symbols, incl. the padding.
 * @param[out] padCount     Number of padding characters.
 *
 * @return If the data is valid, it will return 0 otherwise
 *         MBEDTLS_ERR_BASE64_INVALID_CHARACTER.
 */
static int base64Validate(const unsigned char* src, size_t slen, size_t* symbolCount, size_t* padCount)
{
    int    result  = 0;
    size_t index   = 0U;
    size_t symbols = 0U;
    size_t pads    = 0U;

    while (index < slen)
    {
        uint8_t ch = static_cast<uint8_t>(src[index]);

        if ((static_cast<uint8_t>(' ') == ch) ||
            (static_cast<uint8_t>('\t') == ch) ||
            (static_cast<uint8_t>('\r') == ch) ||
            (static_cast<uint8_t>('\n') == ch))
        {
            /* Whitespace is skipped. */
        }
        else
        {
            int16_t value = base64Value(ch);

            if (BASE64_INVALID == value)
            {
                result = MBEDTLS_ERR_BASE64_INVALID_CHARACTER;
                break;
            }

            if (BASE64_PADDING == value)
            {
                ++pads;

                if (MAX_PADDING < pads)
                {
                    result = MBEDTLS_ERR_BASE64_INVALID_CHARACTER;
                    break;
                }
            }
            else if (0U < pads)
            {
                /* Data after the padding is not allowed. */
                result = MBEDTLS_ERR_BASE64_INVALID_CHARACTER;
                break;
            }
            else
            {
                /* Regular data character. */
            }

            ++symbols;
        }

        ++index;
    }

    if (0 == result)
    {
        *symbolCount = symbols;
        *padCount    = pads;
    }

    return result;
}

/**
 * Decode the given data, which was checked for validity before.
 *
 * The destination buffer must be large enough, see mbedtls_base64_decode().
 *
 * @param[out] dst  Destination buffer.
 * @param[in]  src  Source buffer, which to decode.
 * @param[in]  slen Source buffer size in byte.
 *
 * @return Number of written bytes.
 */
static size_t base64Decode(unsigned char* dst, const unsigned char* src, size_t slen)
{
    size_t   srcIndex  = 0U;
    size_t   dstIndex  = 0U;
    uint32_t group     = 0U;
    uint8_t  symbols   = 0U;
    uint8_t  dataCount = 3U;

    while (srcIndex < slen)
    {
        int16_t value = base64Value(static_cast<uint8_t>(src[srcIndex]));

        /* A whitespace is the only invalid character which may appear here,
         * because the data was checked for validity before.
         */
        if (BASE64_INVALID != value)
        {
            if (BASE64_PADDING == value)
            {
                /* The padding carries no data, the group gets shorter. */
                --dataCount;
                group = (group << 6U);
            }
            else
            {
                group = (group << 6U) | static_cast<uint32_t>(value);
            }

            ++symbols;

            if (4U == symbols)
            {
                if (0U < dataCount)
                {
                    dst[dstIndex] = static_cast<unsigned char>((group >> 16U) & 0xFFU);
                    ++dstIndex;
                }

                if (1U < dataCount)
                {
                    dst[dstIndex] = static_cast<unsigned char>((group >> 8U) & 0xFFU);
                    ++dstIndex;
                }

                if (2U < dataCount)
                {
                    dst[dstIndex] = static_cast<unsigned char>(group & 0xFFU);
                    ++dstIndex;
                }

                group   = 0U;
                symbols = 0U;
            }
        }

        ++srcIndex;
    }

    return dstIndex;
}
