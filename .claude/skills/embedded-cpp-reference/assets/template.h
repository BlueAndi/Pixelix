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
 * @file   ExampleClass.h
 * @brief  Short class/module description.
 * @author Author Name <author@example.com>
 *
 * @addtogroup GROUP
 *
 * @{
 */

#ifndef EXAMPLE_CLASS_H
#define EXAMPLE_CLASS_H

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
 * Short class description.
 */
class ExampleClass
{
public:

    /**
     * Constructs the instance.
     */
    ExampleClass();

    /**
     * Destroys the instance.
     */
    ~ExampleClass();

    /**
     * Initialize module resources.
     *
     * @return If successful, it will return true otherwise false.
     */
    bool init();

    /**
     * Process module periodically.
     */
    void process();

private:

    bool     m_isInitialized; /**< Initialization flag. */
    uint32_t m_counter;       /**< Internal counter.    */

    /* An instance shall not be copied. */
    ExampleClass(const ExampleClass& other);
    ExampleClass& operator=(const ExampleClass& other);
};

/******************************************************************************
 * Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* EXAMPLE_CLASS_H */

/** @} */
