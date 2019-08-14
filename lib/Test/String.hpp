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
@brief  Abstract graphics interface for test
@author Andreas Merkle <web@blue-andi.de>

@section desc Description
This module provides the general graphics interface for testing purposes.

*******************************************************************************/
/** @defgroup igfx Abstract graphics interface for test
 * This module provides the abstract graphics interface for testing purposes.
 *
 * @{
 */

#ifndef __STRING_HPP__
#define __STRING_HPP__

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
        m_buffer(NULL)
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

private:

    const char* m_buffer;   /**< String buffer */

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __STRING_HPP__ */

/** @} */