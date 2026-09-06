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
 * @file   cbuf.h
 * @brief  Circular buffer for test purposes only
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * Counterpart of the Arduino circular buffer.
 *
 * @addtogroup TEST
 *
 * @{
 */

#ifndef CBUF_H
#define CBUF_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <vector>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Circular buffer for test purposes only.
 */
class cbuf
{
public:

    /**
     * Constructs the circular buffer.
     *
     * @param[in] size  Buffer size in byte.
     */
    cbuf(size_t size) :
        m_buffer(size),
        m_readIndex(0U),
        m_writeIndex(0U)
    {
    }

    /**
     * Destroys the circular buffer.
     */
    ~cbuf()
    {
    }

    /**
     * Get the buffer size.
     *
     * Note, this is the capacity and not the number of available bytes, like
     * the Arduino counterpart does it. The webserver derives from it, whether
     * the allocation was successful.
     *
     * @return Buffer size in byte
     */
    size_t size() const
    {
        return m_buffer.size();
    }

    /**
     * Get the number of bytes which are available to read.
     *
     * @return Number of available bytes
     */
    size_t available() const
    {
        return m_writeIndex - m_readIndex;
    }

    /**
     * Get the free space of the buffer.
     *
     * @return Free space in byte
     */
    size_t room() const
    {
        return m_buffer.size() - m_writeIndex;
    }

    /**
     * Increase the buffer size by the given number of bytes.
     *
     * @param[in] addSize   Number of bytes, which to add.
     *
     * @return If successful resized, it will return true otherwise false.
     */
    bool resizeAdd(size_t addSize)
    {
        bool isSuccessful = false;

        if (0U < addSize)
        {
            compact();
            m_buffer.resize(m_buffer.size() + addSize);

            isSuccessful = true;
        }

        return isSuccessful;
    }

    /**
     * Write data to the buffer.
     *
     * @param[in] data  Data buffer
     * @param[in] size  Data buffer size in byte
     *
     * @return Number of written bytes
     */
    size_t write(const char* data, size_t size)
    {
        size_t written = 0U;

        if (nullptr != data)
        {
            written = room();

            if (size < written)
            {
                written = size;
            }

            if (0U < written)
            {
                memcpy(&m_buffer[m_writeIndex], data, written);
                m_writeIndex += written;
            }
        }

        return written;
    }

    /**
     * Read data from the buffer.
     *
     * @param[out] data Data buffer
     * @param[in] size  Data buffer size in byte
     *
     * @return Number of read bytes
     */
    size_t read(char* data, size_t size)
    {
        size_t readCount = available();

        if (size < readCount)
        {
            readCount = size;
        }

        if ((nullptr != data) &&
            (0U < readCount))
        {
            memcpy(data, &m_buffer[m_readIndex], readCount);
            m_readIndex += readCount;

            /* Everything read? Start from the beginning again. */
            if (m_readIndex == m_writeIndex)
            {
                m_readIndex  = 0U;
                m_writeIndex = 0U;
            }
        }

        return readCount;
    }

private:

    std::vector<char> m_buffer;     /**< The buffer. */
    size_t            m_readIndex;  /**< Index of the next byte to read. */
    size_t            m_writeIndex; /**< Index of the next byte to write. */

    cbuf(const cbuf& buffer);
    cbuf& operator=(const cbuf& buffer);

    /**
     * Move the pending data to the beginning of the buffer.
     */
    void compact()
    {
        if (0U < m_readIndex)
        {
            size_t pending = available();

            if (0U < pending)
            {
                memmove(&m_buffer[0U], &m_buffer[m_readIndex], pending);
            }

            m_readIndex  = 0U;
            m_writeIndex = pending;
        }
    }
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* CBUF_H */

/** @} */
