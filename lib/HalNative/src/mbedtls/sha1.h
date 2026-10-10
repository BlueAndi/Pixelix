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
 * @file   sha1.h
 * @brief  SHA-1 for the native environment
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * The websocket handshake hashes the client key, see RFC 6455. This is not a
 * stub, because a wrong hash means no websocket connection and therefore no
 * display preview and no log output in the webinterface.
 *
 * The interface is compatible to the mbedTLS one, which the webserver uses.
 *
 * @addtogroup TEST
 *
 * @{
 */

#ifndef MBEDTLS_SHA1_H
#define MBEDTLS_SHA1_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <stddef.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** SHA-1 context, compatible to the mbedTLS mbedtls_sha1_context. */
typedef struct
{
    uint32_t total[2U];   /**< Number of processed bytes. */
    uint32_t state[5U];   /**< Intermediate digest state. */
    uint8_t  buffer[64U]; /**< Data block which is processed. */

} mbedtls_sha1_context;

/******************************************************************************
 * Functions
 *****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the SHA-1 context.
 *
 * @param[in,out] ctx   SHA-1 context
 */
void mbedtls_sha1_init(mbedtls_sha1_context* ctx);

/**
 * Release the SHA-1 context.
 *
 * @param[in,out] ctx   SHA-1 context
 */
void mbedtls_sha1_free(mbedtls_sha1_context* ctx);

/**
 * Start a SHA-1 calculation.
 *
 * @param[in,out] ctx   SHA-1 context
 *
 * @return If successful, it will return 0 otherwise -1.
 */
int mbedtls_sha1_starts(mbedtls_sha1_context* ctx);

/**
 * Feed data into the SHA-1 calculation.
 *
 * @param[in,out] ctx   SHA-1 context
 * @param[in] input     Data buffer
 * @param[in] ilen      Data buffer size in byte
 *
 * @return If successful, it will return 0 otherwise -1.
 */
int mbedtls_sha1_update(mbedtls_sha1_context* ctx, const uint8_t* input, size_t ilen);

/**
 * Finish the SHA-1 calculation and get the digest.
 *
 * @param[in,out] ctx   SHA-1 context
 * @param[out] output   Digest, 20 byte.
 *
 * @return If successful, it will return 0 otherwise -1.
 */
int mbedtls_sha1_finish(mbedtls_sha1_context* ctx, uint8_t output[20U]);

/**
 * Calculate the SHA-1 digest of the given data in one step.
 *
 * @param[in] input     Data buffer
 * @param[in] ilen      Data buffer size in byte
 * @param[out] output   Digest, 20 byte.
 *
 * @return If successful, it will return 0 otherwise -1.
 */
int mbedtls_sha1(const uint8_t* input, size_t ilen, uint8_t output[20U]);

#ifdef __cplusplus
}
#endif

#endif /* MBEDTLS_SHA1_H */

/** @} */
