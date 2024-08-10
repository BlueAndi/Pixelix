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
 *
 * @addtogroup gfx
 *
 * @{
 */

#ifndef TWABSTRACT_SYNTAX_TREE_H
#define TWABSTRACT_SYNTAX_TREE_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <WString.h>
#include <TWToken.h>
#include <vector>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * A text widget token.
 */
class TWAbstractSyntaxTree
{
public:

    /**
     * Constructs a abstract syntax tree (AST).
     */
    TWAbstractSyntaxTree() :
        m_tokenTrash(TWToken::TYPE_KEYWORD, ""),
        m_tokens()
    {
    }

    /**
     * Construct the AST by assignment.
     *
     * @param[in] other The other AST which to assign.
     */
    TWAbstractSyntaxTree(const TWAbstractSyntaxTree& other) :
        m_tokenTrash(other.m_tokenTrash),
        m_tokens()
    {
        copy(other.m_tokens);
    }

    /**
     * Destroys the abstract syntax tree.
     */
    ~TWAbstractSyntaxTree()
    {
        clear();
    }

    /**
     * Assign other AST.
     *
     * @param[in] other The other AST which to assign.
     *
     * @return The abstract syntax tree.
     */
    TWAbstractSyntaxTree& operator=(const TWAbstractSyntaxTree& other);

    /**
     * Move other AST.
     *
     * @param[in] other The other AST which to move.
     *
     * @return The abstract syntax tree.
     */
    TWAbstractSyntaxTree& operator=(TWAbstractSyntaxTree&& other) noexcept;

    /**
     * Clear AST.
     */
    void clear();

    /**
     * Create a token add it to the AST.
     *
     * @param[in]   tokenType   The type of the token.
     * @param[in]   str         The string which represents the token.
     *
     * @return If successful created and added, it will return true otherwise false.
     */
    bool createToken(TWToken::Type tokenType, const String& str);

    /**
     * Get number of tokens in the AST.
     *
     * @return Number of tokens
     */
    uint32_t length() const;

    /**
     * Access the token by its indizes.
     *
     * If index is out of bounds, a empty keyword token will be returned.
     *
     * @param[in] index The token index in the AST.
     *
     * @return Token
     */
    const TWToken& operator[](uint32_t index) const;

    /**
     * Access the token by its indizes.
     *
     * If index is out of bounds, a empty keyword token will be returned.
     *
     * @param[in] index The token index in the AST.
     *
     * @return Token
     */
    TWToken& operator[](uint32_t index);

private:

    /** Token list */
    typedef std::vector<TWToken*> TokenList;

    TWToken                       m_tokenTrash; /**< Used for invalid token access. */
    TokenList                     m_tokens;     /**< Token AST */

    /**
     * Copy tokens from other AST.
     *
     * @param[in] other Other AST which to copy.
     */
    void copy(const TokenList& other);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* TWABSTRACT_SYNTAX_TREE_H */

/** @} */