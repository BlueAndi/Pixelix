/* MIT License
 *
 * Copyright (c) 2019 Andreas Merkle <web@blue-andi.de>
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
 * @brief  String implementation for test
 * @author Andreas Merkle <web@blue-andi.de>
 * 
 * @addtogroup test
 *
 * @{
 */

#ifndef __WSTRING_H__
#define __WSTRING_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdlib.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * String class for test purposes only.
 */
class String
{
public:

    /**
     * Constructs a string.
     */
    String() :
        m_buffer("")
    {
    }

    /**
     * Destroys a string.
     */
    ~String()
    {
    }

    /**
     * Constructs a string by copying another.
     * 
     * @param[in] str String to copy
     */
    String(const String& str) :
        m_buffer(str.m_buffer)
    {
    }

    /**
     * Constructs a string by copying another.
     * 
     * @param[in] str String to copy
     */
    String(const char* str) :
        m_buffer(str)
    {
    }

    /**
     * Assign a string.
     * 
     * @param[in] str String, which to assign.
     *
     * @return String
     */
    String& operator=(const String& str)
    {
        m_buffer = str.m_buffer;

        return *this;
    }

    /**
     * Get string as char array.
     * 
     * @return Char array
     */
    const char* c_str(void) const
    {
        return m_buffer;
    }

    /**
     * Get string length.
     * 
     * @return String length
     */
    unsigned int length(void) const
    {
        unsigned int    length  = 0;
        const char*     ptr     = m_buffer;

        if (NULL != m_buffer)
        {
            while('\0' != *ptr)
            {
                ++length;
                ++ptr;
            }
        }

        return length;
    }

private:

    const char* m_buffer;   /**< String buffer */

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __WSTRING_H__ */

/** @} */