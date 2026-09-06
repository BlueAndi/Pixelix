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
 * @file   cencode.cpp
 * @brief  Base64 encoder for the native environment
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * Implementation according to RFC 4648. In difference to the original libb64
 * no line break is inserted, like the Arduino core variant does it.
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "libb64/cencode.h"

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

/** The base64 alphabet, see RFC 4648. */
static const char BASE64_ALPHABET[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/** Padding character, used to fill up the last block. */
static const char BASE64_PADDING    = '=';

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

extern "C" void base64_init_encodestate(base64_encodestate* state)
{
    if (nullptr != state)
    {
        state->step      = step_A;
        state->result    = 0;
        state->stepcount = 0;
    }
}

extern "C" char base64_encode_value(char value)
{
    char result = BASE64_PADDING;

    if ((0 <= value) &&
        (63 >= value))
    {
        result = BASE64_ALPHABET[static_cast<int>(value)];
    }

    return result;
}

extern "C" int base64_encode_block(const char* plaintext_in, int length_in, char* code_out, base64_encodestate* state_in)
{
    int count = 0;

    if ((nullptr != plaintext_in) &&
        (nullptr != code_out) &&
        (nullptr != state_in) &&
        (0 < length_in))
    {
        const char* plainChar = plaintext_in;
        const char* plainEnd  = plaintext_in + length_in;
        char*       codeChar  = code_out;
        char        result    = state_in->result;
        char        fragment  = 0;

        switch (state_in->step)
        {
            while (true)
            {
            case step_A:
                if (plainEnd == plainChar)
                {
                    state_in->result = result;
                    state_in->step   = step_A;
                    break;
                }

                fragment    = *plainChar++;
                result      = (fragment & 0x0FC) >> 2;
                *codeChar++ = base64_encode_value(result);
                result      = (fragment & 0x003) << 4;

                /* fall through */

            case step_B:
                if (plainEnd == plainChar)
                {
                    state_in->result = result;
                    state_in->step   = step_B;
                    break;
                }

                fragment     = *plainChar++;
                result      |= (fragment & 0x0F0) >> 4;
                *codeChar++  = base64_encode_value(result);
                result       = (fragment & 0x00F) << 2;

                /* fall through */

            case step_C:
                if (plainEnd == plainChar)
                {
                    state_in->result = result;
                    state_in->step   = step_C;
                    break;
                }

                fragment     = *plainChar++;
                result      |= (fragment & 0x0C0) >> 6;
                *codeChar++  = base64_encode_value(result);
                result       = (fragment & 0x03F) >> 0;
                *codeChar++  = base64_encode_value(result);

                ++state_in->stepcount;
            }
        }

        count = static_cast<int>(codeChar - code_out);
    }

    return count;
}

extern "C" int base64_encode_blockend(char* code_out, base64_encodestate* state_in)
{
    int count = 0;

    if ((nullptr != code_out) &&
        (nullptr != state_in))
    {
        char* codeChar = code_out;

        switch (state_in->step)
        {
        case step_B:
            *codeChar++ = base64_encode_value(state_in->result);
            *codeChar++ = BASE64_PADDING;
            *codeChar++ = BASE64_PADDING;
            break;

        case step_C:
            *codeChar++ = base64_encode_value(state_in->result);
            *codeChar++ = BASE64_PADDING;
            break;

        case step_A:
        default:
            /* Nothing to pad. */
            break;
        }

        count     = static_cast<int>(codeChar - code_out);

        /* The encoded data is terminated, like the Arduino implementation does
         * it. The caller may use the buffer as string, e.g. the webserver does
         * it for the websocket handshake. The termination is not counted.
         */
        *codeChar = '\0';
    }

    return count;
}

extern "C" int base64_encode_chars(const char* plaintext_in, int length_in, char* code_out)
{
    base64_encodestate state;
    int                count = 0;

    base64_init_encodestate(&state);

    count  = base64_encode_block(plaintext_in, length_in, code_out, &state);
    count += base64_encode_blockend(&code_out[count], &state);

    if (nullptr != code_out)
    {
        code_out[count] = '\0';
    }

    return count;
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/
