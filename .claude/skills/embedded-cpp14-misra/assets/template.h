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
 * @file   ClassName.h
 * @brief  Short class/module description
 * @author Author Name <author@example.com>
 *
 * @addtogroup GROUP
 *
 * @{
 */

/* Replace: ClassName, CLASS_NAME, GROUP, @brief text, @author.
 * Keep every section banner, even when the section stays empty.
 */

#ifndef CLASS_NAME_H
#define CLASS_NAME_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Class description: what it owns, what it guarantees, how it is used.
 */
class ClassName
{
public:

    /**
     * Constructs the instance. No hardware access, no allocation.
     */
    ClassName();

    /**
     * Destroys the instance.
     */
    ~ClassName();

    /**
     * Initialize module resources.
     * Safe to call twice; the second call is a no-op and warns.
     *
     * @return If successful, it will return true otherwise false.
     */
    bool init();

    /**
     * Process module periodically.
     * Does nothing until init() succeeded.
     */
    void process();

    /**
     * Get the current counter value.
     *
     * @return Number of processing cycles since init(). Wraps at UINT32_MAX.
     */
    uint32_t getCounter() const;

private:

    /** Maximum number of processing cycles per call. */
    static const uint32_t MAX_CYCLES = 10U;

    /* Members: initialize every one of them in the constructor initializer
     * list, in declaration order.
     */
    bool     m_isInitialized; /**< Initialization flag. */
    uint32_t m_counter;       /**< Number of processing cycles since init(). */

    /* Copy constructor and assignment operator are not supported. */
    ClassName(const ClassName& other);
    ClassName& operator=(const ClassName& other);
};

/******************************************************************************
 * Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* CLASS_NAME_H */

/** @} */
