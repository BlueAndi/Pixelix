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
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "TWTokenizer.h"

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

bool TWTokenizer::parse(TWAbstractSyntaxTree& ast, const String& formattedText)
{
    bool   isSuccessful        = true;
    size_t idx                 = 0U;
    bool   isKeywordBeginFound = false;
    bool   isEscapeActive      = false;
    size_t beginIdx            = 0U;
    String result;

    /* Clear the AST first to ensure there is no old stuff inside. */
    ast.clear();

    /* Walk through formatted text character by character as long as parse
     * error happens or the end is reached.
     */
    while ((true == isSuccessful) && ('\0' != formattedText[idx]))
    {
        /* Escape active? */
        if (true == isEscapeActive)
        {
            /* This character will disable it and will be handled just as
             * text. The escape character itself shall not be in the
             * result text.
             */
            result         += formattedText.substring(beginIdx, idx - 1U);
            result         += formattedText[idx];

            beginIdx        = idx + 1U;
            isEscapeActive  = false;
        }
        /* Escape character found? */
        else if ('\\' == formattedText[idx])
        {
            /* Escape character inside a keyword is not allowed. */
            if (true == isKeywordBeginFound)
            {
                isSuccessful = false;
            }
            else
            {
                isEscapeActive = true;
            }
        }
        /* Line feed found? */
        else if ('\n' == formattedText[idx])
        {
            /* Line feed inside a keyword is not allowed. */
            if (true == isKeywordBeginFound)
            {
                isSuccessful = false;
            }
            else
            {
                isSuccessful = ast.createToken(TWToken::TYPE_LINE_FEED, "\n");

                beginIdx     = idx + 1U;
            }
        }
        /* Begin of keyword found? */
        else if ('{' == formattedText[idx])
        {
            /* Already inside a keyword? */
            if (true == isKeywordBeginFound)
            {
                isSuccessful = false;
            }
            else
            {
                beginIdx            = idx;
                isKeywordBeginFound = true;
            }
        }
        /* End of keyword found? */
        else if ('}' == formattedText[idx])
        {
            /* No keyword started yet? */
            if (false == isKeywordBeginFound)
            {
                isSuccessful = false;
            }
            else
            {
                isSuccessful        = ast.createToken(TWToken::TYPE_KEYWORD, formattedText.substring(beginIdx, idx + 1U));
                beginIdx            = idx + 1U;
                isKeywordBeginFound = false;
            }
        }
        else if (false == isKeywordBeginFound)
        {
            /* End of text reached? */
            if (('{' == formattedText[idx + 1U]) ||
                ('}' == formattedText[idx + 1U]) ||
                ('\n' == formattedText[idx + 1U]) ||
                ('\0' == formattedText[idx + 1U]))
            {
                if (beginIdx <= idx)
                {
                    result += formattedText.substring(beginIdx, idx + 1U);
                }

                if (false == result.isEmpty())
                {
                    isSuccessful = ast.createToken(TWToken::TYPE_TEXT, result);
                    beginIdx     = idx + 1U;
                    result.clear();
                }
            }
        }
        else
        {
            /* Nothing to do. */
        }

        /* If any error happened, remember the index in the formatted text. */
        if (false == isSuccessful)
        {
            m_errorIndex = idx;
        }

        ++idx;
    }

    if (false == result.isEmpty())
    {
        isSuccessful = ast.createToken(TWToken::TYPE_TEXT, result);
    }

    return isSuccessful;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
