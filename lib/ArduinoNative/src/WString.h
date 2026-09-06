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
 * @file   WString.h
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
#include <stdio.h>
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
        m_stdStr((nullptr == other) ? "" : other)
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
     * Constructs a string from a buffer with the given length.
     *
     * @param[in] other Data buffer. May be nullptr.
     * @param[in] len   Number of characters.
     */
    String(const char* other, size_t len) :
        m_stdStr((nullptr == other) ? std::string() : std::string(other, len))
    {
    }

    /* The following constructors mirror the Arduino String. Every integral type
     * needs its own overload, otherwise e.g. a int argument would be ambiguous,
     * because it can be converted to more than one of them. The signed and
     * unsigned long long overloads cover size_t on every supported host.
     */

    /**
     * Constructs a string by converting a number.
     *
     * @param[in] number    Number to convert.
     * @param[in] base      Number base.
     */
    String(unsigned char number, unsigned char base = 10U) :
        m_stdStr(numberToString(static_cast<unsigned long long>(number), base))
    {
    }

    /**
     * Constructs a string by converting a number.
     *
     * @param[in] number    Number to convert.
     * @param[in] base      Number base.
     */
    String(int number, unsigned char base = 10U) :
        m_stdStr(signedNumberToString(static_cast<long long>(number), base))
    {
    }

    /**
     * Constructs a string by converting a number.
     *
     * @param[in] number    Number to convert.
     * @param[in] base      Number base.
     */
    String(unsigned int number, unsigned char base = 10U) :
        m_stdStr(numberToString(static_cast<unsigned long long>(number), base))
    {
    }

    /**
     * Constructs a string by converting a number.
     *
     * @param[in] number    Number to convert.
     * @param[in] base      Number base.
     */
    String(long number, unsigned char base = 10U) :
        m_stdStr(signedNumberToString(static_cast<long long>(number), base))
    {
    }

    /**
     * Constructs a string by converting a number.
     *
     * @param[in] number    Number to convert.
     * @param[in] base      Number base.
     */
    String(unsigned long number, unsigned char base = 10U) :
        m_stdStr(numberToString(static_cast<unsigned long long>(number), base))
    {
    }

    /**
     * Constructs a string by converting a number.
     *
     * @param[in] number    Number to convert.
     * @param[in] base      Number base.
     */
    String(long long number, unsigned char base = 10U) :
        m_stdStr(signedNumberToString(number, base))
    {
    }

    /**
     * Constructs a string by converting a number.
     *
     * @param[in] number    Number to convert.
     * @param[in] base      Number base.
     */
    String(unsigned long long number, unsigned char base = 10U) :
        m_stdStr(numberToString(number, base))
    {
    }

    /**
     * Constructs a string by converting a number.
     *
     * @param[in] number        Number to convert.
     * @param[in] decimalPlaces Number of decimal places.
     */
    String(float number, unsigned int decimalPlaces = 2U) :
        m_stdStr(floatToString(static_cast<double>(number), decimalPlaces))
    {
    }

    /**
     * Constructs a string by converting a number.
     *
     * @param[in] number        Number to convert.
     * @param[in] decimalPlaces Number of decimal places.
     */
    String(double number, unsigned int decimalPlaces = 2U) :
        m_stdStr(floatToString(number, decimalPlaces))
    {
    }

    /**
     * Assign a string.
     *
     * @param[in] other String, which to assign.
     *
     * @return String
     */
    String& operator=(const char* other)
    {
        m_stdStr = other;

        return *this;
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
    char operator[](unsigned int index) const
    {
        char singleChar = '\0';

        if (m_stdStr.length() > index)
        {
            singleChar = m_stdStr[index];
        }

        return singleChar;
    }

    String& operator+=(const char* other)
    {
        m_stdStr += other;

        return *this;
    }

    String& operator+=(const String& other)
    {
        m_stdStr += other.m_stdStr;

        return *this;
    }

    String& operator+=(char c)
    {
        m_stdStr += c;

        return *this;
    }

    String& operator+=(int number)
    {
        m_stdStr += std::to_string(number);

        return *this;
    }

    /**
     * Append a number.
     *
     * @param[in] number Number to append.
     *
     * @return String
     */
    String& operator+=(unsigned int number)
    {
        m_stdStr += std::to_string(number);

        return *this;
    }

    /**
     * Append a number.
     *
     * @param[in] number Number to append.
     *
     * @return String
     */
    String& operator+=(long number)
    {
        m_stdStr += std::to_string(number);

        return *this;
    }

    /**
     * Append a number.
     *
     * @param[in] number Number to append.
     *
     * @return String
     */
    String& operator+=(unsigned long number)
    {
        m_stdStr += std::to_string(number);

        return *this;
    }

    /**
     * Append a number.
     *
     * @param[in] number Number to append.
     *
     * @return String
     */
    String& operator+=(long long number)
    {
        m_stdStr += std::to_string(number);

        return *this;
    }

    /**
     * Append a number.
     *
     * @param[in] number Number to append.
     *
     * @return String
     */
    String& operator+=(unsigned long long number)
    {
        m_stdStr += std::to_string(number);

        return *this;
    }

    /**
     * Append a number.
     *
     * @param[in] number Number to append.
     *
     * @return String
     */
    String& operator+=(float number)
    {
        m_stdStr += std::to_string(number);

        return *this;
    }

    /**
     * Append a number.
     *
     * @param[in] number Number to append.
     *
     * @return String
     */
    String& operator+=(double number)
    {
        m_stdStr += std::to_string(number);

        return *this;
    }

    String operator+(const String& other) const
    {
        String tmp  = *this;
        tmp        += other;

        return tmp;
    }

    /* The following operator+() overloads mirror the Arduino String. Every
     * integral type must have its own overload, otherwise e.g. a uint16_t
     * argument is ambiguous, because it can be converted to more than one of
     * the String constructors. Note that adding constructors would not help,
     * because two user-defined conversion sequences via different constructors
     * are not comparable.
     */

    /**
     * Concatenate with a string literal.
     *
     * @param[in] other String literal to append.
     *
     * @return Concatenated string
     */
    String operator+(const char* other) const
    {
        String tmp  = *this;
        tmp        += other;

        return tmp;
    }

    /**
     * Concatenate with a single character.
     *
     * @param[in] c Character to append.
     *
     * @return Concatenated string
     */
    String operator+(char c) const
    {
        String tmp  = *this;
        tmp        += c;

        return tmp;
    }

    /**
     * Concatenate with a number.
     *
     * @param[in] number Number to append.
     *
     * @return Concatenated string
     */
    String operator+(int number) const
    {
        String tmp  = *this;
        tmp        += number;

        return tmp;
    }

    /**
     * Concatenate with a number.
     *
     * @param[in] number Number to append.
     *
     * @return Concatenated string
     */
    String operator+(unsigned int number) const
    {
        String tmp  = *this;
        tmp        += number;

        return tmp;
    }

    /**
     * Concatenate with a number.
     *
     * @param[in] number Number to append.
     *
     * @return Concatenated string
     */
    String operator+(long number) const
    {
        String tmp  = *this;
        tmp        += number;

        return tmp;
    }

    /**
     * Concatenate with a number.
     *
     * @param[in] number Number to append.
     *
     * @return Concatenated string
     */
    String operator+(unsigned long number) const
    {
        String tmp  = *this;
        tmp        += number;

        return tmp;
    }

    /**
     * Concatenate with a number.
     *
     * @param[in] number Number to append.
     *
     * @return Concatenated string
     */
    String operator+(long long number) const
    {
        String tmp  = *this;
        tmp        += number;

        return tmp;
    }

    /**
     * Concatenate with a number.
     *
     * @param[in] number Number to append.
     *
     * @return Concatenated string
     */
    String operator+(unsigned long long number) const
    {
        String tmp  = *this;
        tmp        += number;

        return tmp;
    }

    /**
     * Concatenate with a number.
     *
     * @param[in] number Number to append.
     *
     * @return Concatenated string
     */
    String operator+(float number) const
    {
        String tmp  = *this;
        tmp        += number;

        return tmp;
    }

    /**
     * Concatenate with a number.
     *
     * @param[in] number Number to append.
     *
     * @return Concatenated string
     */
    String operator+(double number) const
    {
        String tmp  = *this;
        tmp        += number;

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
     * Reserve memory for the string.
     *
     * @param[in] size Number of bytes to reserve.
     *
     * @return true on success, otherwise false.
     */
    bool reserve(unsigned int size)
    {
        m_stdStr.reserve(size);

        return (m_stdStr.capacity() >= size) ? true : false;
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
        return substring(index, static_cast<unsigned int>(m_stdStr.length()));
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
        String       result;
        unsigned int length = static_cast<unsigned int>(m_stdStr.length());

        /* Like the Arduino String, the indices are clamped. The std::string
         * would throw a std::out_of_range instead, which would terminate the
         * program.
         */
        if (left > right)
        {
            unsigned int tmp = left;

            left             = right;
            right            = tmp;
        }

        if (left < length)
        {
            if (right > length)
            {
                right = length;
            }

            result = m_stdStr.substr(left, right - left);
        }

        return result;
    }

    /**
     * Starts string with given pattern?
     *
     * @param[in] s2    Pattern
     *
     * @return If string starts with pattern, it will return true otherwise false.
     */
    unsigned char startsWith(const String& s2) const
    {
        return startsWith(s2, 0U);
    }

    /**
     * Starts string with given pattern from offset?
     *
     * @param[in] s2        Pattern
     * @param[in] offset    Offset, where the pattern is expected.
     *
     * @return If string starts with pattern, it will return true otherwise false.
     */
    unsigned char startsWith(const String& s2, unsigned int offset) const
    {
        unsigned char result = 0U;

        if ((offset + s2.length()) <= m_stdStr.length())
        {
            if (0 == m_stdStr.compare(offset, s2.m_stdStr.length(), s2.m_stdStr))
            {
                result = 1U;
            }
        }

        return result;
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
     * Get index of given character.
     *
     * @param[in] ch        Character to search for.
     * @param[in] fromIndex Start index for search.
     *
     * @return If found, it will return the index otherwise -1.
     */
    int indexOf(char ch, unsigned int fromIndex) const
    {
        int    index = -1;
        size_t pos   = m_stdStr.find(ch, fromIndex);

        if (std::string::npos != pos)
        {
            index = pos;
        }

        return index;
    }

    /**
     * Get index of given character.
     *
     * @param[in] ch    Character to search for.
     *
     * @return If found, it will return the index otherwise -1.
     */
    int indexOf(char ch) const
    {
        return indexOf(ch, 0U);
    }

    /**
     * Get index of given string.
     *
     * @param[in] other     String to search for.
     * @param[in] fromIndex Start index for search.
     *
     * @return If found, it will return the index otherwise -1.
     */
    int indexOf(const String& other, unsigned int fromIndex) const
    {
        int    index = -1;
        size_t pos   = m_stdStr.find(other.m_stdStr, fromIndex);

        if (std::string::npos != pos)
        {
            index = pos;
        }

        return index;
    }

    /**
     * Get index of given string.
     *
     * @param[in] other String to search for.
     *
     * @return If found, it will return the index otherwise -1.
     */
    int indexOf(const String& other) const
    {
        return indexOf(other, 0U);
    }

    /**
     * Compare string.
     *
     * @param[in] other String to compare with.
     *
     * @return If equal, it will return true otherwise false.
     */
    bool equals(const String& other) const
    {
        return m_stdStr == other.m_stdStr;
    }

    /**
     * Compare string.
     *
     * @param[in] other String to compare with. May be nullptr.
     *
     * @return If equal, it will return true otherwise false.
     */
    bool equals(const char* other) const
    {
        bool isEqual = false;

        if (nullptr == other)
        {
            /* Guard: a nullptr never equals a string. */
        }
        else
        {
            isEqual = (m_stdStr == other);
        }

        return isEqual;
    }

    /**
     * Append a string.
     *
     * @param[in] other String to append.
     *
     * @return If successful appended, it will return true otherwise false.
     */
    bool concat(const String& other)
    {
        m_stdStr += other.m_stdStr;

        return true;
    }

    /**
     * Append a string.
     *
     * @param[in] other String to append. May be nullptr.
     *
     * @return If successful appended, it will return true otherwise false.
     */
    bool concat(const char* other)
    {
        bool isSuccessful = false;

        if (nullptr == other)
        {
            /* Guard: nothing to append. */
        }
        else
        {
            m_stdStr     += other;
            isSuccessful  = true;
        }

        return isSuccessful;
    }

    /**
     * Append a single character.
     *
     * @param[in] c Character to append.
     *
     * @return If successful appended, it will return true otherwise false.
     */
    bool concat(char c)
    {
        m_stdStr += c;

        return true;
    }

    /**
     * Append the given number of characters.
     *
     * @param[in] other Data buffer. May be nullptr.
     * @param[in] len   Number of characters to append.
     *
     * @return If successful appended, it will return true otherwise false.
     */
    bool concat(const char* other, size_t len)
    {
        bool isSuccessful = false;

        if (nullptr == other)
        {
            /* Guard: nothing to append. */
        }
        else
        {
            m_stdStr.append(other, len);
            isSuccessful = true;
        }

        return isSuccessful;
    }

    /* The following overloads mirror the Arduino String. Every integral type
     * needs its own overload, otherwise e.g. a int argument would be converted
     * to char and appended as a single character instead of a number.
     */

    /**
     * Append a number.
     *
     * @param[in] number Number to append.
     *
     * @return If successful appended, it will return true otherwise false.
     */
    bool concat(unsigned char number)
    {
        m_stdStr += std::to_string(number);

        return true;
    }

    /**
     * Append a number.
     *
     * @param[in] number Number to append.
     *
     * @return If successful appended, it will return true otherwise false.
     */
    bool concat(int number)
    {
        m_stdStr += std::to_string(number);

        return true;
    }

    /**
     * Append a number.
     *
     * @param[in] number Number to append.
     *
     * @return If successful appended, it will return true otherwise false.
     */
    bool concat(unsigned int number)
    {
        m_stdStr += std::to_string(number);

        return true;
    }

    /**
     * Append a number.
     *
     * @param[in] number Number to append.
     *
     * @return If successful appended, it will return true otherwise false.
     */
    bool concat(long number)
    {
        m_stdStr += std::to_string(number);

        return true;
    }

    /**
     * Append a number.
     *
     * @param[in] number Number to append.
     *
     * @return If successful appended, it will return true otherwise false.
     */
    bool concat(unsigned long number)
    {
        m_stdStr += std::to_string(number);

        return true;
    }

    /**
     * Append a number.
     *
     * @param[in] number Number to append.
     *
     * @return If successful appended, it will return true otherwise false.
     */
    bool concat(long long number)
    {
        m_stdStr += std::to_string(number);

        return true;
    }

    /**
     * Append a number.
     *
     * @param[in] number Number to append.
     *
     * @return If successful appended, it will return true otherwise false.
     */
    bool concat(unsigned long long number)
    {
        m_stdStr += std::to_string(number);

        return true;
    }

    /**
     * Append a number.
     *
     * @param[in] number Number to append.
     *
     * @return If successful appended, it will return true otherwise false.
     */
    bool concat(float number)
    {
        m_stdStr += floatToString(static_cast<double>(number), DEFAULT_DECIMAL_PLACES);

        return true;
    }

    /**
     * Append a number.
     *
     * @param[in] number Number to append.
     *
     * @return If successful appended, it will return true otherwise false.
     */
    bool concat(double number)
    {
        m_stdStr += floatToString(number, DEFAULT_DECIMAL_PLACES);

        return true;
    }

    /**
     * Remove all characters from the given index to the end.
     *
     * @param[in] index Index of the first character to remove.
     */
    void remove(unsigned int index)
    {
        if (m_stdStr.length() > index)
        {
            m_stdStr.erase(index);
        }
    }

    /**
     * Remove the given number of characters, starting at the given index.
     *
     * @param[in] index Index of the first character to remove.
     * @param[in] count Number of characters to remove.
     */
    void remove(unsigned int index, unsigned int count)
    {
        if (m_stdStr.length() > index)
        {
            m_stdStr.erase(index, count);
        }
    }

    /**
     * Get last index of given string, searched up to the given index.
     *
     * @param[in] other     String to search for.
     * @param[in] fromIndex Last index which is considered for a match.
     *
     * @return If found, it will return the index otherwise -1.
     */
    int lastIndexOf(const String& other, unsigned int fromIndex) const
    {
        int index  = -1;

        /* The whole string is searched for, not for any of its characters.
         * Therefore rfind() is used and not find_last_of().
         */
        size_t pos = m_stdStr.rfind(other.m_stdStr, fromIndex);

        if (std::string::npos != pos)
        {
            index = static_cast<int>(pos);
        }

        return index;
    }

    /**
     * Get last index of given string.
     *
     * @param[in] other String to search for.
     *
     * @return If found, it will return the index otherwise -1.
     */
    int lastIndexOf(const String& other) const
    {
        return lastIndexOf(other, static_cast<unsigned int>(m_stdStr.length()));
    }

    /**
     * Get last index of given character, searched up to the given index.
     *
     * @param[in] ch        Character to search for.
     * @param[in] fromIndex Last index which is considered for a match.
     *
     * @return If found, it will return the index otherwise -1.
     */
    int lastIndexOf(char ch, unsigned int fromIndex) const
    {
        int    index = -1;
        size_t pos   = m_stdStr.rfind(ch, fromIndex);

        if (std::string::npos != pos)
        {
            index = static_cast<int>(pos);
        }

        return index;
    }

    /**
     * Get last index of given character.
     *
     * @param[in] ch    Character to search for.
     *
     * @return If found, it will return the index otherwise -1.
     */
    int lastIndexOf(char ch) const
    {
        return lastIndexOf(ch, static_cast<unsigned int>(m_stdStr.length()));
    }

    /**
     * Convert every character to lower case.
     */
    void toLowerCase()
    {
        std::transform(m_stdStr.begin(), m_stdStr.end(), m_stdStr.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
    }

    /**
     * Convert every character to upper case.
     */
    void toUpperCase()
    {
        std::transform(m_stdStr.begin(), m_stdStr.end(), m_stdStr.begin(), [](unsigned char c) {
            return static_cast<char>(std::toupper(c));
        });
    }

    /**
     * Remove the leading and trailing whitespaces.
     */
    void trim()
    {
        const char* WHITESPACES = " \t\n\r\f\v";
        size_t      first       = m_stdStr.find_first_not_of(WHITESPACES);

        if (std::string::npos == first)
        {
            m_stdStr.clear();
        }
        else
        {
            size_t last = m_stdStr.find_last_not_of(WHITESPACES);

            m_stdStr    = m_stdStr.substr(first, (last - first) + 1U);
        }
    }

    /**
     * Get the character at the given index.
     *
     * @param[in] index Character index in the string.
     *
     * @return Character
     */
    char charAt(unsigned int index) const
    {
        return (*this)[index];
    }

    /**
     * Compare with another string.
     *
     * @param[in] other String to compare with.
     *
     * @return 0 if equal, negative if less and positive if greater.
     */
    int compareTo(const String& other) const
    {
        return m_stdStr.compare(other.m_stdStr);
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
        return (m_stdStr.length() == other.m_stdStr.length()) &&
               (std::equal(m_stdStr.begin(), m_stdStr.end(), other.m_stdStr.begin(), icharEquals));
    }

    /**
     * Concatenate string literal with String object.
     *
     * @param[in] lhs   String literal
     * @param[in] rhs   String object
     *
     * @return Concatenated string
     */
    friend String operator+(const char* lhs, const String& rhs)
    {
        String tmp(lhs);
        tmp += rhs;

        return tmp;
    }

    /**
     * Convert string to integer.
     *
     * @return Integer value of string.
     */
    long toInt(void) const
    {
        return std::strtol(m_stdStr.c_str(), nullptr, 10);
    }

    /**
     * Convert string to float.
     *
     * @return Float value of string.
     */
    float toFloat(void) const
    {
        return std::strtof(m_stdStr.c_str(), nullptr);
    }

    /**
     * Convert string to double.
     *
     * @return Double value of string.
     */
    double toDouble(void) const
    {
        return std::strtod(m_stdStr.c_str(), nullptr);
    }

    /**
     * Ends string with given suffix?
     *
     * @param[in] suffix    Suffix
     *
     * @return If string ends with suffix, it will return true otherwise false.
     */
    bool endsWith(const String& suffix) const
    {
        bool result = false;

        if (suffix.length() <= length())
        {
            result = 0 == m_stdStr.compare(length() - suffix.length(), suffix.length(), suffix.m_stdStr);
        }

        return result;
    }

private:

    /** Number of decimal places used, if the caller doesn't specify them. */
    static const unsigned int DEFAULT_DECIMAL_PLACES = 2U;

    std::string               m_stdStr; /**< Internal used std string. */

    /**
     * Convert a unsigned number to a string.
     *
     * @param[in] number    Number to convert.
     * @param[in] base      Number base [2; 16].
     *
     * @return Number as string
     */
    static std::string numberToString(unsigned long long number, unsigned char base)
    {
        std::string result;

        if ((2U > base) ||
            (16U < base))
        {
            base = 10U;
        }

        if (0U == number)
        {
            result = "0";
        }
        else
        {
            static const char DIGITS[] = "0123456789abcdef";

            while (0U < number)
            {
                result.insert(result.begin(), DIGITS[number % base]);
                number /= base;
            }
        }

        return result;
    }

    /**
     * Convert a signed number to a string.
     *
     * @param[in] number    Number to convert.
     * @param[in] base      Number base [2; 16].
     *
     * @return Number as string
     */
    static std::string signedNumberToString(long long number, unsigned char base)
    {
        std::string result;

        if (0 > number)
        {
            /* The negation is done on the unsigned type, otherwise the lowest
             * possible value would overflow.
             */
            unsigned long long absValue  = static_cast<unsigned long long>(-(number + 1)) + 1ULL;

            result                       = "-";
            result                      += numberToString(absValue, base);
        }
        else
        {
            result = numberToString(static_cast<unsigned long long>(number), base);
        }

        return result;
    }

    /**
     * Convert a floating point number to a string.
     *
     * @param[in] number        Number to convert.
     * @param[in] decimalPlaces Number of decimal places.
     *
     * @return Number as string
     */
    static std::string floatToString(double number, unsigned int decimalPlaces)
    {
        const size_t BUFFER_SIZE = 64U;
        char         buffer[BUFFER_SIZE];

        (void)snprintf(buffer, sizeof(buffer), "%.*f", static_cast<int>(decimalPlaces), number);

        return std::string(buffer);
    }

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

#endif /* WSTRING_H */

/** @} */