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
 * @brief  String implementation for test
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup test
 *
 * @{
 */

#ifndef WSTRING_H
#define WSTRING_H

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
        m_size(1U),
        m_buffer(new char[m_size])
    {
        if (nullptr == m_buffer)
        {
            m_size = 0U;
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
        if (nullptr != m_buffer)
        {
            delete[] m_buffer;
            m_buffer = nullptr;
            m_size = 0U;
        }
    }

    /**
     * Constructs a string by copying another.
     *
     * @param[in] str String to copy
     */
    String(const String& str) :
        m_size(str.m_size),
        m_buffer(nullptr)
    {
        if ((0 == str.m_size) ||
            (nullptr == str.m_buffer))
        {
            m_buffer = new char[1u];

            if (nullptr == m_buffer)
            {
                m_size = 0U;
            }
            else
            {
                m_size = 1U;
                m_buffer[0u] = '\0';
            }
        }
        else
        {
            m_buffer = new char[str.m_size];

            if (nullptr == m_buffer)
            {
                m_size = 0U;
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
        m_size((nullptr == str) ? 1u : (strlen(str) + 1U)),
        m_buffer(new char[m_size])
    {
        if (nullptr == m_buffer)
        {
            m_size = 0U;
        }
        else if (nullptr == str)
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
        m_size(2U),
        m_buffer(new char[m_size])
    {
        if (nullptr == m_buffer)
        {
            m_size = 0U;
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
            if (nullptr != m_buffer)
            {
                delete[] m_buffer;
                m_buffer = nullptr;
            }

            m_size = str.m_size;

            if (0u < m_size)
            {
                m_buffer = new char[m_size];

                if (nullptr == m_buffer)
                {
                    m_size = 0U;
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
        if (nullptr != str.m_buffer)
        {
            char* tmp = new char[m_size + str.m_size];

            if (nullptr != tmp)
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

        if (nullptr != tmp)
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
    const char* c_str() const
    {
        static const char*  emptyStr    = "";
        const char*         buffer      = m_buffer;

        if (nullptr == buffer)
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
    unsigned int length() const
    {
        unsigned int    length  = 0;
        const char*     ptr     = m_buffer;

        if (nullptr != m_buffer)
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
     * Return the substring from index to the end.
     *
     * @param[in] index Index.
     *
     * @return Substring
     */
    String substring(unsigned int index) const
    {
        return substring(index, length());
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

    /**
     * Starts string with given pattern?
     *
     * @param[in] s2    Pattern
     *
     * @return If string starts with pattern, it will return true otherwise false.
     */
    unsigned char startsWith(const String &s2) const
    {
        if(length() < s2.length())
        {
            return 0U;
        }

        return startsWith(s2, 0);
    }

    /**
     * Starts string with given pattern from offset?
     *
     * @param[in] s2        Pattern
     * @param[in] offset    Offset
     *
     * @return If string starts with pattern, it will return true otherwise false.
     */
    unsigned char startsWith(const String &s2, unsigned int offset) const
    {
        if((offset > static_cast<unsigned int>(length() - s2.length())) ||
           (nullptr == m_buffer) ||
           (nullptr == s2.m_buffer))
        {
            return 0;
        }

        return 0 == strncmp(&m_buffer[offset], s2.m_buffer, s2.length());
    }

    /**
     * Clear string.
     */
    void clear()
    {
        if (nullptr != m_buffer)
        {
            m_buffer[0] = '\0';
        }
    }

    /**
     * Is string empty?
     * 
     * @return If empty, it will return true otherwise false.
     */
    bool isEmpty() const
    {
        bool isEmptyFlag = true;

        if ((nullptr != m_buffer) &&
            (0U < m_size) &&
            ('\0' != m_buffer[0]))
        {
            isEmptyFlag = false;
        }

        return isEmptyFlag;
    }

private:

    size_t  m_size;     /**< String buffer size */
    char*   m_buffer;   /**< String buffer */

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* WSTRING_H */

/** @} */