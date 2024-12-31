/* MIT License
 *
 * Copyright (c) 2019 - 2024 Andreas Merkle <web@blue-andi.de>
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
 * @addtogroup TEST
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
#include <string>
#include <cctype>
#include <algorithm>

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
        m_stdStr()
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
     * @param[in] other String to copy
     */
    String(const String& other) :
        m_stdStr(other.m_stdStr)
    {
    }

    /**
     * Constructs a string by copying another.
     *
     * @param[in] other String to copy
     */
    String(const char* other) :
        m_stdStr(other)
    {
    }

    /**
     * Constructs a string by copying another.
     *
     * @param[in] other String to copy
     */
    String(const std::string& other) :
        m_stdStr(other)
    {
    }

    /**
     * Constructs a string by copying a single character.
     *
     * @param[in] c Single character
     */
    String(char c) :
        m_stdStr(1, c)
    {
    }

    /**
     * Assign a string.
     *
     * @param[in] other String, which to assign.
     *
     * @return String
     */
    String& operator=(const String& other)
    {
        if (this != &other)
        {
            m_stdStr = other.m_stdStr;
        }

        return *this;
    }

    /**
     * Compare two strings.
     *
     * @param[in] other String, which to compare with.
     *
     * @return If the strings are equal, it will return true otherwise false.
     */
    bool operator==(const String& other) const
    {
        return m_stdStr == other.m_stdStr;
    }

    /**
     * Compare two strings.
     *
     * @param[in] other String, which to compare with.
     *
     * @return If the strings are equal, it will return true otherwise false.
     */
    bool operator!=(const String& other) const
    {
       return m_stdStr != other.m_stdStr;
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

        if (m_stdStr.length() > index)
        {
            singleChar = m_stdStr[index];
        }

        return singleChar;
    }

    String& operator +=(const String& other)
    {
        m_stdStr += other.m_stdStr;

        return *this;
    }

    String& operator +=(char c)
    {
        m_stdStr += c;

        return *this;
    }

    String& operator +=(int number)
    {
        m_stdStr += std::to_string(number);

        return *this;
    }

    String operator +(const String& other) const
    {
        String tmp = *this;
        tmp += other;

        return tmp;
    }

    /**
     * Get string as char array.
     *
     * @return Char array
     */
    const char* c_str() const
    {
        return m_stdStr.c_str();
    }

    /**
     * Get string length.
     *
     * @return String length
     */
    unsigned int length() const
    {
        return m_stdStr.length();
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
        return m_stdStr.substr(index);
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
        return m_stdStr.substr(left, right - left);
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
        return 0 == m_stdStr.rfind(s2.c_str(), 0);
    }

    /**
     * Clear string.
     */
    void clear()
    {
        m_stdStr.clear();
    }

    /**
     * Is string empty?
     * 
     * @return If empty, it will return true otherwise false.
     */
    bool isEmpty() const
    {
        return m_stdStr.empty();
    }

    /**
     * Get last index of given string.
     * 
     * @param[in] other   String to search for.
     * 
     * @return If found, it will return the index otherwise -1.
     */
    int lastIndexOf(const String& other) const
    {
        int     index   = -1;
        size_t  pos     = m_stdStr.find_last_of(other.m_stdStr);

        if (std::string::npos != pos)
        {
            index = pos;
        }

        return index;
    }

    /**
     * Compare string case insenstive.
     * 
     * @param[in] other   String to compare with.
     * 
     * @return If equal, it will return true otherwise false.
     */
    bool equalsIgnoreCase(const String& other) const
    {
        return  (m_stdStr.length() == other.m_stdStr.length()) &&
                (std::equal(m_stdStr.begin(), m_stdStr.end(), other.m_stdStr.begin(), icharEquals));
    }

private:

    std::string m_stdStr;   /**< Internal used std string. */

    /**
     * Compare single characters case insensitive.
     * 
     * @param[in] ch1   Character 1
     * @param[in] ch2   Character 2
     *
     * @return If equal, it will return true otherwise false.
     */
    static bool icharEquals(unsigned char ch1, unsigned char ch2)
    {
        return std::tolower(ch1) == std::tolower(ch2);
    }
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* WSTRING_H */

/** @} */