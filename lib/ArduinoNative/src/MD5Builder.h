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
 * @file   MD5Builder.h
 * @brief  MD5 for the native environment
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * Counterpart of the Arduino MD5Builder. It is a implementation according to
 * RFC 1321.
 *
 * @addtogroup TEST
 *
 * @{
 */

#ifndef MD5_BUILDER_H
#define MD5_BUILDER_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <stddef.h>

#include "WString.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * MD5 calculation, compatible to the Arduino MD5Builder.
 */
class MD5Builder
{
public:

    /**
     * Constructs the MD5 builder.
     */
    MD5Builder();

    /**
     * Destroys the MD5 builder.
     */
    ~MD5Builder();

    /**
     * Start a new calculation.
     */
    void begin();

    /**
     * Feed data into the calculation.
     *
     * @param[in] data  Data buffer
     * @param[in] len   Data buffer size in byte
     */
    void add(const uint8_t* data, uint16_t len);

    /**
     * Feed a string into the calculation.
     *
     * @param[in] data  Zero terminated string.
     */
    void add(const char* data);

    /**
     * Feed a string into the calculation.
     *
     * @param[in] data  String
     */
    void add(const String& data);

    /**
     * Finish the calculation.
     */
    void calculate();

    /**
     * Get the digest as hexadecimal string.
     *
     * @param[out] output   Digest, 33 byte (32 characters and the termination).
     */
    void getChars(char* output);

    /**
     * Get the digest as hexadecimal string.
     *
     * @return Digest
     */
    String toString();

    /**
     * Get the raw digest.
     *
     * @param[out] output   Digest, 16 byte.
     */
    void getBytes(uint8_t* output);

private:

    /** Size of the digest in byte. */
    static const size_t DIGEST_SIZE = 16U;

    /** Size of a single data block in byte. */
    static const size_t BLOCK_SIZE  = 64U;

    uint32_t            m_state[4U];           /**< Intermediate digest state. */
    uint32_t            m_count[2U];           /**< Number of processed bits. */
    uint8_t             m_buffer[BLOCK_SIZE];  /**< Data block which is processed. */
    uint8_t             m_digest[DIGEST_SIZE]; /**< The calculated digest. */

    MD5Builder(const MD5Builder& builder);
    MD5Builder& operator=(const MD5Builder& builder);

    /**
     * Process a single data block.
     *
     * @param[in] block Data block, 64 byte.
     */
    void processBlock(const uint8_t block[64U]);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* MD5_BUILDER_H */

/** @} */
