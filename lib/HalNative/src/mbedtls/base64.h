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
 * @file   base64.h
 * @brief  Base64 encoding/decoding for the native environment
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * The MQTT API transfers files base64 encoded. This is not a stub, because a
 * wrong decoding means a corrupt file.
 *
 * The interface is compatible to the mbedTLS one, which the application uses
 * on the target.
 *
 * @addtogroup TEST
 *
 * @{
 */

#ifndef MBEDTLS_BASE64_H
#define MBEDTLS_BASE64_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stddef.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/** The destination buffer is too small, see the required size in olen. */
#define MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL (-0x002A)

/** The source contains a character, which is not part of the base64 alphabet. */
#define MBEDTLS_ERR_BASE64_INVALID_CHARACTER (-0x002C)

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Encode the given data base64.
 *
 * Call it with dst = nullptr and dlen = 0 to determine the required buffer
 * size, which is written to olen. Note, it contains the terminating zero.
 *
 * @param[out] dst      Destination buffer, zero terminated. May be nullptr.
 * @param[in]  dlen     Destination buffer size in byte.
 * @param[out] olen     Number of written bytes resp. the required buffer size.
 * @param[in]  src      Source buffer, which to encode. May be nullptr if slen is 0.
 * @param[in]  slen     Source buffer size in byte.
 *
 * @return If successful, it will return 0 otherwise
 *         MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL.
 */
int mbedtls_base64_encode(unsigned char* dst, size_t dlen, size_t* olen, const unsigned char* src, size_t slen);

/**
 * Decode the given base64 encoded data.
 *
 * Call it with dst = nullptr and dlen = 0 to determine the required buffer
 * size, which is written to olen. In this case
 * MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL is returned, which is not a error.
 * Whitespaces in the source are skipped, like the mbedTLS counterpart does.
 *
 * @param[out] dst      Destination buffer. May be nullptr.
 * @param[in]  dlen     Destination buffer size in byte.
 * @param[out] olen     Number of written bytes resp. the required buffer size.
 * @param[in]  src      Source buffer, which to decode. May be nullptr if slen is 0.
 * @param[in]  slen     Source buffer size in byte.
 *
 * @return If successful, it will return 0 otherwise
 *         MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL or
 *         MBEDTLS_ERR_BASE64_INVALID_CHARACTER.
 */
int mbedtls_base64_decode(unsigned char* dst, size_t dlen, size_t* olen, const unsigned char* src, size_t slen);

#ifdef __cplusplus
}
#endif

#endif /* MBEDTLS_BASE64_H */

/** @} */
