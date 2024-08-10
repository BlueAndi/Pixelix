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
 * @brief  Text widget abstract syntax tree
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "TWAbstractSyntaxTree.h"

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

TWAbstractSyntaxTree& TWAbstractSyntaxTree::operator=(const TWAbstractSyntaxTree& other)
{
    if (this != &other)
    {
        m_tokenTrash = other.m_tokenTrash;

        clear();
        copy(other.m_tokens);
    }

    return *this;
}

TWAbstractSyntaxTree& TWAbstractSyntaxTree::operator=(TWAbstractSyntaxTree&& other) noexcept
{
    if (this != &other)
    {
        m_tokenTrash = other.m_tokenTrash;
        m_tokens     = std::move(other.m_tokens);
        other.m_tokens.clear();
    }

    return *this;
}

void TWAbstractSyntaxTree::clear()
{
    TokenList::iterator it = m_tokens.begin();

    while (it != m_tokens.end())
    {
        TWToken* token = *it;

        it             = m_tokens.erase(it);

        if (nullptr != token)
        {
            delete token;
        }
    }
}

bool TWAbstractSyntaxTree::createToken(TWToken::Type tokenType, const String& str)
{
    bool     isSuccessful = false;
    TWToken* token        = new (std::nothrow) TWToken(tokenType, str);

    if (nullptr != token)
    {
        m_tokens.push_back(token);
        isSuccessful = true;
    }

    return isSuccessful;
}

uint32_t TWAbstractSyntaxTree::length() const
{
    return m_tokens.size();
}

const TWToken& TWAbstractSyntaxTree::operator[](uint32_t index) const
{
    const TWToken* token = &m_tokenTrash;

    if (m_tokens.size() > index)
    {
        token = m_tokens[index];
    }

    return *token;
}

TWToken& TWAbstractSyntaxTree::operator[](uint32_t index)
{
    TWToken* token = &m_tokenTrash;

    if (m_tokens.size() > index)
    {
        token = m_tokens[index];
    }

    return *token;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void TWAbstractSyntaxTree::copy(const TokenList& other)
{
    TokenList::const_iterator it = other.begin();

    while (it != other.end())
    {
        TWToken* token = *it;

        if (nullptr != token)
        {
            (void)createToken(token->getType(), token->getStr());
        }
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
