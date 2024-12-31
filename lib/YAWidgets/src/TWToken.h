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
 * @brief  Text widget token
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup GFX
 *
 * @{
 */

#ifndef TWTOKEN_H
#define TWTOKEN_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <WString.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * A text widget token.
 */
class TWToken
{
public:

    /**
     * Token type
     */
    enum Type
    {
        TYPE_KEYWORD = 0, /**< Token type keyword */
        TYPE_TEXT,        /**< Token type text (single line) */
        TYPE_LINE_FEED    /**< Token type line feed (LF) */
    };

    /**
     * Constructs a token.
     *
     * @param[in] tokenType The type of the token.
     * @param[in] str       The string which represents the token.
     */
    TWToken(Type tokenType, const String& str) :
        m_type(tokenType),
        m_str(str)
    {
    }

    /**
     * Constructs a token by assignment.
     *
     * @param[in] other The token which to assign.
     */
    TWToken(const TWToken& other) :
        m_type(other.m_type),
        m_str(other.m_str)
    {
    }

    /**
     * Destroys the token.
     */
    ~TWToken()
    {
    }

    /**
     * Assign other token.
     *
     * @param[in] other The token which to assign.
     *
     * @return Token
     */
    TWToken& operator=(const TWToken& other)
    {
        if (this != &other)
        {
            m_type = other.m_type;
            m_str  = other.m_str;
        }

        return *this;
    }

    /**
     * Get token type.
     *
     * @return Token type
     */
    Type getType() const
    {
        return m_type;
    }

    /**
     * Set token type.
     *
     * @param[in] tokenType The new type which to set.
     */
    void setType(Type tokenType)
    {
        m_type = tokenType;
    }

    /**
     * Get token string.
     *
     * @return Token string
     */
    const String& getStr() const
    {
        return m_str;
    }

    /**
     * Set token string.
     *
     * @param[in] str   Token string which to set.
     */
    void setStr(const String& str)
    {
        m_str = str;
    }

private:

    Type   m_type; /**< The type of the token. */
    String m_str;  /**< String which represents the token. */
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* TWTOKEN_H */

/** @} */