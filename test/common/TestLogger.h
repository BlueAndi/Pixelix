/* MIT License
 *
 * Copyright (c) 2019 - 2023 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Logging interface for testing purposes.
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup common
 *
 * @{
 */

#ifndef TEST_LOGGER_H
#define TEST_LOGGER_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <Util.h>
#include <Print.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Logging interface for testing purposes.
 * It provides all relevant methods from the Print class, which are used.
 */
class TestLogger : public Print
{
public:

    /**
     * Constructs a logging interface for testing purposes.
     */
    TestLogger():
        m_buffer()
    {
        uint16_t index = 0U;

        for(index = 0U; index < UTIL_ARRAY_NUM(m_buffer); ++index)
        {
            m_buffer[index] = 0;
        }
    }

    /**
     * Write a single byte.
     *
     * @param[in] data The byte to be written.
     *
     * @return 1.
     */
    size_t write(uint8_t data)
    {
        UTIL_NOT_USED(data);

        /* Method is not used at all, because the write(const uint8_t*, size_t size)
         * is overwritten, which doesn't use the single byte write method.
         */

        return 0;
    }

    /**
     * Write a single byte.
     *
     * @param[in] buffer Pointer to the data to be written.
     * @param[in] size the size of the data to be written.
     *
     * @return The size of the written data.
     */
    size_t write(const uint8_t* buffer, size_t size)
    {
        uint16_t index = 0U;

        for(index = 0U; index < UTIL_ARRAY_NUM(m_buffer); ++index)
        {
            m_buffer[index] = static_cast<char>(buffer[index]);
        }

        return size;
    }

    /**
     * Get the Write buffer.
     *
     * @return Write buffer
     */
    const char* getBuffer()
    {
        return m_buffer;
    }

    /**
     * Clear internal buffer.
     */
    void clear()
    {
        m_buffer[0] = '\0';
    }

    /**
     * Destroys the logging interface.
     */
    ~TestLogger( )
    {
    }

private:
    char m_buffer[1024]; /**< Write buffer, containing the logMessage. */
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* TEST_LOGGER_H */

/** @} */