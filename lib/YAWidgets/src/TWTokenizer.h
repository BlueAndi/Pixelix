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
 * @brief  Text widget tokenizer
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup gfx
 *
 * @{
 */

#ifndef TWTOKENIZER_H
#define TWTOKENIZER_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <TWToken.h>
#include <TWAbstractSyntaxTree.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * A text widget tokenizer.
 *
 * KEYWORD = '{'...'}'
 * LINE_FEED = '\n'
 * ESCAPE = '\\'
 * TEXT = ...
 *
 */
class TWTokenizer
{
public:

    /**
     * Constructs a tokenizer.
     */
    TWTokenizer() :
        m_errorIndex(0U)
    {
    }

    /**
     * Destroys the tokenizer.
     */
    ~TWTokenizer()
    {
    }

    /**
     * Parse a formatted text to create the abstract syntax tree.
     *
     * @param[out]  ast             The abstract syntax tree, which will be setup.
     * @param[in]   formattedText   The text which will be parsed.
     *
     * @return If successful, it will return true otherwise false.
     */
    bool parse(TWAbstractSyntaxTree& ast, const String& formattedText);

    /**
     * Get the error index, which indicates where the parser detected the error.
     * Its only valid if the parse() method failed.
     *
     * @return Index
     */
    uint32_t getErrorIndex() const
    {
        return m_errorIndex;
    }

private:

    uint32_t m_errorIndex; /**< Index in the formatted text, where the error happened. */
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* TWTOKENIZER_H */

/** @} */