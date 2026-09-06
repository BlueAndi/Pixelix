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
 * @file   cencode.h
 * @brief  Base64 encoder for the native environment
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * Counterpart of the libb64 encoder of the Arduino core. The websocket
 * handshake encodes the hashed client key with it, see RFC 6455.
 *
 * @addtogroup TEST
 *
 * @{
 */

#ifndef LIBB64_CENCODE_H
#define LIBB64_CENCODE_H

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

/**
 * Get the buffer size which is required to encode the given number of bytes,
 * without the string termination.
 */
#define base64_encode_expected_len(n) ((((4U * (n)) / 3U) + 3U) & ~3U)

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** Step of the encoding, one base64 character covers parts of two bytes. */
typedef enum
{
    step_A = 0, /**< Next byte provides the first 6 bit. */
    step_B,     /**< Next byte provides the following 6 bit. */
    step_C      /**< Next byte provides the last 6 bit. */

} base64_encodestep;

/** State of the encoding. */
typedef struct
{
    base64_encodestep step;      /**< Current step. */
    char              result;    /**< Bits which are kept for the next step. */
    int               stepcount; /**< Number of encoded characters, used for line breaks. */

} base64_encodestate;

/******************************************************************************
 * Functions
 *****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize the encoder state.
 *
 * @param[out] state    The encoder state.
 */
void base64_init_encodestate(base64_encodestate* state);

/**
 * Encode a single 6-bit value.
 *
 * @param[in] value 6-bit value.
 *
 * @return Base64 character
 */
char base64_encode_value(char value);

/**
 * Encode a block of data.
 *
 * @param[in]  plaintext_in     Data buffer
 * @param[in]  length_in        Data buffer size in byte
 * @param[out] code_out         Encoded data
 * @param[in,out] state_in      The encoder state.
 *
 * @return Number of written characters.
 */
int base64_encode_block(const char* plaintext_in, int length_in, char* code_out, base64_encodestate* state_in);

/**
 * Finish the encoding and write the padding.
 *
 * The encoded data is terminated afterwards, therefore the buffer must provide
 * one character more than the return value. The termination is not counted.
 *
 * @param[out] code_out     Encoded data
 * @param[in,out] state_in  The encoder state.
 *
 * @return Number of written characters.
 */
int base64_encode_blockend(char* code_out, base64_encodestate* state_in);

/**
 * Encode a block of data in one step, incl. the string termination.
 *
 * @param[in]  plaintext_in     Data buffer
 * @param[in]  length_in        Data buffer size in byte
 * @param[out] code_out         Encoded data
 *
 * @return Number of written characters, without the string termination.
 */
int base64_encode_chars(const char* plaintext_in, int length_in, char* code_out);

#ifdef __cplusplus
}
#endif

#endif /* LIBB64_CENCODE_H */

/** @} */
