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
 * @file   ClassName.cpp
 * @brief  Short implementation description
 * @author Author Name <author@example.com>
 */

/* Replace: ClassName, @brief text, @author.
 * Own header first, then framework headers, then project headers.
 * SortIncludes is off - the order is manual and meaningful.
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "ClassName.h"

#include <Logging.h>

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

ClassName::ClassName() :
    m_isInitialized(false),
    m_counter(0U)
{
}

ClassName::~ClassName()
{
}

bool ClassName::init()
{
    bool isSuccessful = false;

    if (true == m_isInitialized)
    {
        LOG_WARNING("Already initialized.");
    }
    else
    {
        /* Bring every member to a defined state before declaring success. */
        m_counter       = 0U;
        m_isInitialized = true;
        isSuccessful    = true;

        LOG_INFO("Initialized.");
    }

    return isSuccessful;
}

void ClassName::process()
{
    if (false == m_isInitialized)
    {
        /* Guard path: do not process before initialization. */
    }
    else
    {
        ++m_counter;
    }
}

uint32_t ClassName::getCounter() const
{
    return m_counter;
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
