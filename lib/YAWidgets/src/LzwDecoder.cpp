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
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "LzwDecoder.h"

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

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void LzwDecoder::init(uint8_t lzwMinCodeWidth)
{
    if (nullptr == m_codes)
    {
        m_codes = new(std::nothrow) uint32_t[CODE_LIMIT];
    }

    if (nullptr == m_stack)
    {
        m_stack = new(std::nothrow) uint8_t[STACK_SIZE];
    }

    m_lzwMinCodeWidth   = lzwMinCodeWidth;
    m_clearCode         = 1U << m_lzwMinCodeWidth;
    m_endCode           = m_clearCode + 1U;
    m_stackPtr          = m_stack;
    m_bitsInBuffer      = 0U;
    clear();
}

bool LzwDecoder::decode(const ReadFromInStream& readFromInStreamFunc, const WriteToOutStream& writeToOutStreamFunc)
{
    bool        isSuccessful    = true;
    bool        isEnd           = false;
    uint32_t    code;

    if ((nullptr == m_codes) ||
        (nullptr == m_stack))
    {
        isSuccessful = false;
    }

    while((false == isEnd) && (true == isSuccessful))
    {
        /* Get code */
        if (false == getCode(code, readFromInStreamFunc))
        {
            isSuccessful = false;
        }
        else
        {
            /* Finished? */
            if (code == m_endCode)
            {
                isEnd = true;
            }
            /* Decoder initialization request? */
            else if (code == m_clearCode)
            {
                clear();
            }
            /* Continue decompression. */
            else if (false == decompress(code, writeToOutStreamFunc))
            {
                isSuccessful = false;
            }
            else
            {
                /* Go ahead. */
                ;
            }
        }
    }

    return isSuccessful;
}

void LzwDecoder::deInit()
{
    if (nullptr != m_codes)
    {
        delete[] m_codes;
        m_codes = nullptr;
    }

    if (nullptr != m_stack)
    {
        delete[] m_stack;
        m_stack = nullptr;
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void LzwDecoder::clear()
{
    m_nextCode          = m_endCode + 1U;
    m_maxCode           = 2U * m_clearCode - 1U;
    m_codeWidth         = m_lzwMinCodeWidth + 1U;
    m_isInitialState    = true;
}

bool LzwDecoder::getCode(uint32_t& code, const ReadFromInStream& readFromInStreamFunc)
{
    bool        isSuccessful    = true;
    uint32_t    codeBitsNeeded  = m_codeWidth;

    code = 0U;

    /* Run as long as code bits are still needed. */
    while((0U < codeBitsNeeded) && (true == isSuccessful))
    {
        /* No bits in the buffer anymore? */
        if (0U == m_bitsInBuffer)
        {
            uint8_t data = 0U;

            /* Read data from code stream. */
            if (false == readFromInStreamFunc(data))
            {
                /* No more data is available, abort now. */
                isSuccessful = false;
            }
            else
            {
                m_codeBuffer = data;
                m_bitsInBuffer = 8U;
            }
        }

        if (true == isSuccessful)
        {
            uint32_t bitsAvailable  = (m_bitsInBuffer < codeBitsNeeded) ? m_bitsInBuffer : codeBitsNeeded;
            uint32_t mask           = (1U << bitsAvailable) - 1U; /* Mask for n bits. */

            code |= (m_codeBuffer & mask) << (m_codeWidth - codeBitsNeeded);

            /* Remove bits from code buffer. */
            m_codeBuffer >>= bitsAvailable;
            m_bitsInBuffer -= bitsAvailable;

            /* Now less bits are needed. */
            codeBitsNeeded -= bitsAvailable;
        }
    }

    return isSuccessful;
}

bool LzwDecoder::decompress(uint32_t& code, const WriteToOutStream& writeToOutStreamFunc)
{
    bool isSuccessful = true;

    if (true == m_isInitialState)
    {
        uint8_t data = code & 0xFFU;

        m_firstByte = code;
        m_prevCode  = code;

        /* Is code invalid? */
        if (code > m_endCode)
        {
            isSuccessful = false;
        }
        else if (false == writeToOutStreamFunc(data))
        {
            isSuccessful = false;
        }
        else
        {
            m_isInitialState = false;
        }
    }
    else
    {
        m_inCode = code;

        /* Handle the KwKwK case. */
        if (code >= m_nextCode)
        {
            /* Invalid data? */
            if (code != m_nextCode)
            {
                isSuccessful = false;
            }
            else
            {
                *m_stackPtr = m_firstByte & 0xFFU;
                ++m_stackPtr;
                code        = m_prevCode;
            }
        }

        if (true == isSuccessful)
        {
            /* "Unwind" code's string to stack. */
            while(code >= m_clearCode)
            {
                /* Heads are packed to left of tails in codes. */
                
                /* Tails */
                *m_stackPtr = m_codes[code] & 0xFFU;
                ++m_stackPtr;

                /* Heads*/
                code = (m_codes[code] >> 8U) & 0x0FFFU;
            }

            m_firstByte = code;
            *m_stackPtr = code & 0xFFU;
            ++m_stackPtr;

            do
            {
                --m_stackPtr;

                if (false == writeToOutStreamFunc(*m_stackPtr))
                {
                    isSuccessful = false;
                }
            }
            while((m_stackPtr > m_stack) && (true == isSuccessful));
        }

        if (true == isSuccessful)
        {
            if (m_nextCode < CODE_LIMIT)
            {
                /* Heads are packed to left of tails in codes. */
                m_codes[m_nextCode] = (m_prevCode << 8U) | code;
                ++m_nextCode;

                if ((m_nextCode > m_maxCode) && (m_nextCode < CODE_LIMIT))
                {
                    m_maxCode = m_maxCode * 2U + 1U;
                    ++m_codeWidth;
                }
            }

            m_prevCode = m_inCode;
        }
    }
    
    return isSuccessful;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
