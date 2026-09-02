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
 * @file   ExampleTemplate.hpp
 * @brief  Header-only / template-style class description.
 * @author Author Name <author@example.com>
 *
 * @addtogroup GROUP
 *
 * @{
 */

#ifndef EXAMPLE_TEMPLATE_HPP
#define EXAMPLE_TEMPLATE_HPP

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Template class description.
 *
 * @tparam T Element type handled by the instance.
 */
template<typename T>
class ExampleTemplate
{
public:

    /**
     * Constructs the instance.
     */
    ExampleTemplate() :
        m_isInitialized(false),
        m_counter(0U)
    {
    }

    /**
     * Destroys the instance.
     */
    ~ExampleTemplate()
    {
    }

    /**
     * Initialize module resources.
     *
     * @return If successful, it will return true otherwise false.
     */
    bool init()
    {
        bool isSuccessful = true;

        if (true == m_isInitialized)
        {
            /* Already initialized. */
        }
        else
        {
            m_counter       = 0U;
            m_isInitialized = true;
        }

        return isSuccessful;
    }

    /**
     * Process module periodically.
     */
    void process()
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

    /**
     * Check whether a value is valid for this instance.
     *
     * @param[in] value Value to validate.
     *
     * @return If value is valid, it will return true otherwise false.
     */
    bool isValid(const T& value) const
    {
        bool isValid = false;

        if (false == m_isInitialized)
        {
            /* Guard path. */
        }
        else
        {
            /* Placeholder defensive check; adapt to the concrete type semantics. */
            isValid = (nullptr != &value);
        }

        return isValid;
    }

private:

    bool     m_isInitialized; /**< Initialization flag. */
    uint32_t m_counter;       /**< Internal counter.    */
};

#endif /* EXAMPLE_TEMPLATE_HPP */

/** @} */
