/* MIT License
 *
 * Copyright (c) 2019 - 2020 Andreas Merkle <web@blue-andi.de>
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
#include <string.h>

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
        m_size(1u),
        m_buffer(new char[m_size])
    {
        if (NULL == m_buffer)
        {
            m_size = 0u;
        }
        else
        {
            m_buffer[0] = '\0';
        }
    }

    /**
     * Destroys a string.
     */
    ~String()
    {
        if (NULL != m_buffer)
        {
            delete[] m_buffer;
            m_buffer = NULL;
            m_size = 0u;
        }
    }

    /**
     * Constructs a string by copying another.
     * 
     * @param[in] str String to copy
     */
    String(const String& str) :
        m_size(str.m_size),
        m_buffer(NULL)
    {
        if ((0 == str.m_size) ||
            (NULL == str.m_buffer))
        {
            m_buffer = new char[1u];
            
            if (NULL == m_buffer)
            {
                m_size = 0u;
            }
            else
            {
                m_size = 1u;
                m_buffer[0u] = '\0';
            }
        }
        else
        {
            m_buffer = new char[str.m_size];

            if (NULL == m_buffer)
            {
                m_size = 0u;
            }
            else
            {
                m_size = str.m_size;
                strcpy(m_buffer, str.m_buffer);
            }
        }
    }

    /**
     * Constructs a string by copying another.
     * 
     * @param[in] str String to copy
     */
    String(const char* str) :
        m_size((NULL == str) ? 1u : (strlen(str) + 1u)),
        m_buffer(new char[m_size])
    {
        if (NULL == m_buffer)
        {
            m_size = 0u;
        }
        else if (NULL == str)
        {
            m_buffer[0u] = '\0';
        }
        else
        {
            strcpy(m_buffer, str);
        }
    }

    /**
     * Constructs a string by copying a single character.
     * 
     * @param[in] c Single character
     */
    String(char c) :
        m_size(2u),
        m_buffer(new char[m_size])
    {
        if (NULL == m_buffer)
        {
            m_size = 0u;
        }
        else
        {
            m_buffer[0] = c;
            m_buffer[1] = '\0';
        }
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
        if (this != &str)
        {
            if (NULL != m_buffer)
            {
                delete[] m_buffer;
                m_buffer = NULL;
            }

            m_size = str.m_size;

            if (0u < m_size)
            {
                m_buffer = new char[m_size];

                if (NULL == m_buffer)
                {
                    m_size = 0u;
                }
                else
                {
                    memcpy(m_buffer, str.m_buffer, m_size);
                }
            }
        }

        return *this;
    }

    /**
     * Compare two strings.
     * 
     * @param[in] str String, which to compare with.
     * 
     * @return If the strings are equal, it will return true otherwise false.
     */
    bool operator==(const String& str) const
    {
        bool result = false;

        if (0 == strcmp(str.c_str(), m_buffer))
        {
            result = true;
        }

        return result;
    }

    /**
     * Compare two strings.
     * 
     * @param[in] str String, which to compare with.
     * 
     * @return If the strings are equal, it will return true otherwise false.
     */
    bool operator!=(const String& str) const
    {
        return (*this == str) ? false : true;
    }

    /**
     * Get character at given index.
     * If the index is out of bounds, it will return '\0'.
     * 
     * @param[in] index Character index in the string.
     * 
     * @return Character
     */
    char operator [](unsigned int index) const
    {
        char singleChar = '\0';

        if (length() > index)
        {
            singleChar = m_buffer[index];
        }

        return singleChar;
    }

    String& operator +=(const String& str)
    {
        if (NULL != str.m_buffer)
        {
            char* tmp = new char[m_size + str.m_size];

            if (NULL != tmp)
            {
                strcpy(tmp, m_buffer);
                strcat(tmp, str.m_buffer);

                delete[] m_buffer;
                m_buffer = tmp;
                m_size += str.m_size;
            }
        }

        return *this;
    }

    String& operator +=(char c)
    {
        char* tmp = new char[m_size + 1];

        if (NULL != tmp)
        {
            char cBuff[2] = { c, '\0'};

            strcpy(tmp, m_buffer);
            strcat(tmp, cBuff);

            delete[] m_buffer;
            m_buffer = tmp;
            ++m_size;
        }

        return *this;
    }

    String operator +(const String& str) const
    {
        String tmp = *this;
        tmp += str;

        return tmp;
    }

    /**
     * Get string as char array.
     * 
     * @return Char array
     */
    const char* c_str(void) const
    {
        static const char*  emptyStr    = "";
        const char*         buffer      = m_buffer;

        if (NULL == buffer)
        {
            buffer = emptyStr;
        }

        return buffer;
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

    /**
     * Return the substring between left and right index.
     * 
     * @param[in] left  Index left
     * @param[in] right Index right
     * 
     * @return Substring
     */
    String substring(unsigned int left, unsigned int right) const
    {
        String              out;
        const unsigned int  len = length();

        if (left > right)
        {
            unsigned int temp = right;
            right = left;
            left = temp;
        }

        if (len > left)
        {
            if (length() < right)
            {
                right = len;
            }

            char temp = m_buffer[right];
            m_buffer[right] = '\0';

            out = &m_buffer[left];

            m_buffer[right] = temp;
        }

        return out;
    }

private:

    size_t  m_size;     /**< String buffer size */
    char*   m_buffer;   /**< String buffer */

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __WSTRING_H__ */

/** @} */