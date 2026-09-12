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
 * @file   sha1.cpp
 * @brief  SHA-1 for the native environment
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * Implementation according to RFC 3174.
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "mbedtls/sha1.h"

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

static void     processBlock(mbedtls_sha1_context* ctx, const uint8_t data[64U]);
static uint32_t rotateLeft(uint32_t value, uint32_t shift);

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/** Size of a single data block in byte. */
static const size_t BLOCK_SIZE = 64U;

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

extern "C" void mbedtls_sha1_init(mbedtls_sha1_context* ctx)
{
    if (nullptr != ctx)
    {
        memset(ctx, 0, sizeof(mbedtls_sha1_context));
    }
}

extern "C" void mbedtls_sha1_free(mbedtls_sha1_context* ctx)
{
    if (nullptr != ctx)
    {
        memset(ctx, 0, sizeof(mbedtls_sha1_context));
    }
}

extern "C" int mbedtls_sha1_starts(mbedtls_sha1_context* ctx)
{
    int status = -1;

    if (nullptr != ctx)
    {
        ctx->total[0U] = 0U;
        ctx->total[1U] = 0U;

        /* Initial digest state, see RFC 3174. */
        ctx->state[0U] = 0x67452301U;
        ctx->state[1U] = 0xEFCDAB89U;
        ctx->state[2U] = 0x98BADCFEU;
        ctx->state[3U] = 0x10325476U;
        ctx->state[4U] = 0xC3D2E1F0U;

        status         = 0;
    }

    return status;
}

extern "C" int mbedtls_sha1_update(mbedtls_sha1_context* ctx, const uint8_t* input, size_t ilen)
{
    int status = -1;

    if ((nullptr != ctx) &&
        ((nullptr != input) || (0U == ilen)))
    {
        size_t   left = ctx->total[0U] % BLOCK_SIZE;
        size_t   fill = BLOCK_SIZE - left;
        uint32_t oldTotal;

        status          = 0;

        /* Count the processed bytes as 64-bit value. */
        oldTotal        = ctx->total[0U];
        ctx->total[0U] += static_cast<uint32_t>(ilen);

        if (ctx->total[0U] < oldTotal)
        {
            ++ctx->total[1U];
        }

        /* Fill up the pending block first. */
        if ((0U < left) &&
            (ilen >= fill))
        {
            memcpy(&ctx->buffer[left], input, fill);
            processBlock(ctx, ctx->buffer);

            input += fill;
            ilen  -= fill;
            left   = 0U;
        }

        /* Process every complete block. */
        while (BLOCK_SIZE <= ilen)
        {
            processBlock(ctx, input);

            input += BLOCK_SIZE;
            ilen  -= BLOCK_SIZE;
        }

        /* Keep the rest for the next call. */
        if (0U < ilen)
        {
            memcpy(&ctx->buffer[left], input, ilen);
        }
    }

    return status;
}

extern "C" int mbedtls_sha1_finish(mbedtls_sha1_context* ctx, uint8_t output[20U])
{
    int status = -1;

    if ((nullptr != ctx) &&
        (nullptr != output))
    {
        uint8_t  padding[BLOCK_SIZE] = { 0U };
        uint8_t  msgLength[8U];
        uint32_t high   = (ctx->total[0U] >> 29U) | (ctx->total[1U] << 3U);
        uint32_t low    = ctx->total[0U] << 3U;
        size_t   left   = ctx->total[0U] % BLOCK_SIZE;
        size_t   padLen = (56U > left) ? (56U - left) : (120U - left);
        uint32_t idx    = 0U;

        /* The message length in bit as 64-bit big endian value. */
        msgLength[0U]   = static_cast<uint8_t>(high >> 24U);
        msgLength[1U]   = static_cast<uint8_t>(high >> 16U);
        msgLength[2U]   = static_cast<uint8_t>(high >> 8U);
        msgLength[3U]   = static_cast<uint8_t>(high);
        msgLength[4U]   = static_cast<uint8_t>(low >> 24U);
        msgLength[5U]   = static_cast<uint8_t>(low >> 16U);
        msgLength[6U]   = static_cast<uint8_t>(low >> 8U);
        msgLength[7U]   = static_cast<uint8_t>(low);

        padding[0U]     = 0x80U;

        (void)mbedtls_sha1_update(ctx, padding, padLen);
        (void)mbedtls_sha1_update(ctx, msgLength, sizeof(msgLength));

        /* The digest as big endian value. */
        for (idx = 0U; idx < 5U; ++idx)
        {
            output[(idx * 4U) + 0U] = static_cast<uint8_t>(ctx->state[idx] >> 24U);
            output[(idx * 4U) + 1U] = static_cast<uint8_t>(ctx->state[idx] >> 16U);
            output[(idx * 4U) + 2U] = static_cast<uint8_t>(ctx->state[idx] >> 8U);
            output[(idx * 4U) + 3U] = static_cast<uint8_t>(ctx->state[idx]);
        }

        status = 0;
    }

    return status;
}

extern "C" int mbedtls_sha1(const uint8_t* input, size_t ilen, uint8_t output[20U])
{
    mbedtls_sha1_context ctx;
    int                  status = -1;

    mbedtls_sha1_init(&ctx);

    if (0 == mbedtls_sha1_starts(&ctx))
    {
        if (0 == mbedtls_sha1_update(&ctx, input, ilen))
        {
            status = mbedtls_sha1_finish(&ctx, output);
        }
    }

    mbedtls_sha1_free(&ctx);

    return status;
}

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

/**
 * Process a single data block, see RFC 3174 section 6.1.
 *
 * @param[in,out] ctx   SHA-1 context
 * @param[in] data      Data block, 64 byte.
 */
static void processBlock(mbedtls_sha1_context* ctx, const uint8_t data[64U])
{
    uint32_t work[80U];
    uint32_t a   = ctx->state[0U];
    uint32_t b   = ctx->state[1U];
    uint32_t c   = ctx->state[2U];
    uint32_t d   = ctx->state[3U];
    uint32_t e   = ctx->state[4U];
    uint32_t idx = 0U;

    /* The data block as big endian 32-bit words. */
    for (idx = 0U; idx < 16U; ++idx)
    {
        work[idx] = (static_cast<uint32_t>(data[(idx * 4U) + 0U]) << 24U) |
                    (static_cast<uint32_t>(data[(idx * 4U) + 1U]) << 16U) |
                    (static_cast<uint32_t>(data[(idx * 4U) + 2U]) << 8U) |
                    (static_cast<uint32_t>(data[(idx * 4U) + 3U]));
    }

    for (idx = 16U; idx < 80U; ++idx)
    {
        work[idx] = rotateLeft(work[idx - 3U] ^ work[idx - 8U] ^ work[idx - 14U] ^ work[idx - 16U], 1U);
    }

    for (idx = 0U; idx < 80U; ++idx)
    {
        uint32_t func  = 0U;
        uint32_t konst = 0U;
        uint32_t temp  = 0U;

        if (20U > idx)
        {
            func  = (b & c) | ((~b) & d);
            konst = 0x5A827999U;
        }
        else if (40U > idx)
        {
            func  = b ^ c ^ d;
            konst = 0x6ED9EBA1U;
        }
        else if (60U > idx)
        {
            func  = (b & c) | (b & d) | (c & d);
            konst = 0x8F1BBCDCU;
        }
        else
        {
            func  = b ^ c ^ d;
            konst = 0xCA62C1D6U;
        }

        temp = rotateLeft(a, 5U) + func + e + konst + work[idx];
        e    = d;
        d    = c;
        c    = rotateLeft(b, 30U);
        b    = a;
        a    = temp;
    }

    ctx->state[0U] += a;
    ctx->state[1U] += b;
    ctx->state[2U] += c;
    ctx->state[3U] += d;
    ctx->state[4U] += e;
}
