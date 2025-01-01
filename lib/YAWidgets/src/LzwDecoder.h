/* MIT License
 *
 * Copyright (c) 2019 - 2025 Andreas Merkle <web@blue-andi.de>
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
 * @brief  LZW decoder for GIF images
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup GFX
 *
 * @{
 */

#ifndef LZW_DECODER_H
#define LZW_DECODER_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <stddef.h>
#include <functional>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * LZW decoder for GIF images.
 * The code was derived from https://www.raygard.net/giflzw/
 * Thanks to Ray Gardner!
 */
class LzwDecoder
{
public:

    /** Prototyp for reading from input stream (code stream). */
    typedef std::function<bool(uint8_t& data)> ReadFromInStream;

    /** Prototype for writing decoded data to output stream. */
    typedef std::function<bool(uint8_t& data)> WriteToOutStream;

    /**
     * Construct a LZW decoder object.
     */
    LzwDecoder() :
        m_isInitialState(true),
        m_lzwMinCodeWidth(0U),
        m_clearCode(0U),
        m_endCode(0U),
        m_nextCode(0U),
        m_maxCode(0U),
        m_codeWidth(0U),
        m_bitsInBuffer(0U),
        m_codeBuffer(0U),
        m_firstByte(0U),
        m_inCode(0U),
        m_prevCode(0U),
        m_codes(nullptr),
        m_stack(nullptr),
        m_stackPtr(nullptr)
    {
    }

    /**
     * Destroy LZW decoder object.
     * 
     */
    ~LzwDecoder()
    {
        if (nullptr != m_codes)
        {
            delete[] m_codes;
        }

        if (nullptr != m_stack)
        {
            delete[] m_stack;
        }
    }

    /**
     * Initialize with the LZW min. code width (number of bits).
     * It will allocate internal memory for the decompression. Use the deInit()
     * after successful decompression to release it again.
     * 
     * @param[in] lzwMinCodeWidth   LZW min. code width
     */
    void init(uint8_t lzwMinCodeWidth);

    /**
     * Decodes input stream (code stream) and write it the output stream.
     * 
     * @param[in] readFromInStreamFunc  Callback used to read from input stream.
     * @param[in] writeToOutStreamFunc  Callback used to write to output stream.
     * 
     * @return If successful decoded, it will return true otherwise false.
     */
    bool decode(const ReadFromInStream& readFromInStreamFunc, const WriteToOutStream& writeToOutStreamFunc);

    /**
     * Deinitialize the LZW decoder.
     * It will release internal allocated memory.
     */
    void deInit();

private:

    /**
     * Max. number of codes.
     * 2^12 = 4096
     */
    static const size_t CODE_LIMIT  = 4096U;

    /**
     * Max. stack size in byte.
     * 2^12 = 4096
     */
    static const size_t STACK_SIZE  = 4096U;

    bool        m_isInitialState;   /**< Is LZW decoder initialization state or not. */
    uint32_t    m_lzwMinCodeWidth;  /**< LZW min. code width in bits */
    uint32_t    m_clearCode;        /**< Code for clear request. */
    uint32_t    m_endCode;          /**< Code which marks the end. */
    uint32_t    m_nextCode;         /**< Next code */
    uint32_t    m_maxCode;          /**< Max. code */
    uint32_t    m_codeWidth;        /**< Code width in bits */
    uint32_t    m_bitsInBuffer;     /**< Number of bits in code buffer */
    uint32_t    m_codeBuffer;       /**< Code buffer used to retrieve a code */
    uint32_t    m_firstByte;        /**< First byte */
    uint32_t    m_inCode;           /**< In code */
    uint32_t    m_prevCode;         /**< Previous code */
    uint32_t*   m_codes;            /**< Array of codes */
    uint8_t*    m_stack;            /**< Stack */
    uint8_t*    m_stackPtr;         /**< Current stack pointer on stack */

    /* Not allowed */
    LzwDecoder(const LzwDecoder& other);
    LzwDecoder& operator=(const LzwDecoder& other);

    /**
     * Clear code tables.
     */
    void clear();

    /**
     * Get the code.
     * 
     * @param[out] code                 The retrieved code.
     * @param[in] readFromInStreamFunc  Callback to read from input stream on demand.
     *
     * @return If successful, it will return true otherwise false.
     */
    bool getCode(uint32_t& code, const ReadFromInStream& readFromInStreamFunc);

    /**
     * Decompress code to output stream.
     * 
     * @param[in] code                  The retrieved code.
     * @param[in] writeToOutStreamFunc  Callback to write to output stream on demand.
     * 
     * @return If successful, it will return true otherwise false.
     */
    bool decompress(uint32_t& code, const WriteToOutStream& writeToOutStreamFunc);

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* LZW_DECODER_H */

/** @} */