/* MIT License
 *
 * Copyright (c) 2019 Andreas Merkle <web@blue-andi.de>
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
@brief  Main test entry point
@author Andreas Merkle <web@blue-andi.de>

@section desc Description
This module provides the main test entry point.

*******************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <unity.h>

#include "../Common/LinkedList.hpp"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/******************************************************************************
 * Prototypes
 *****************************************************************************/

void testDoublyLinkedList(void);

/******************************************************************************
 * Variables
 *****************************************************************************/

/******************************************************************************
 * External functions
 *****************************************************************************/

/**
 * Main entry point
 * 
 * @param[in] argc  Number of command line arguments
 * @param[in] argv  Command line arguments
 */
int main(int argc, char **argv)
{
    UNITY_BEGIN();

    RUN_TEST(testDoublyLinkedList);

    UNITY_END();

    return 0;
}

/******************************************************************************
 * Local functions
 *****************************************************************************/

/**
 * Doubly linked list tests.
 */
void testDoublyLinkedList(void)
{
    LinkedList<int> list;

    /* List is empty */
    TEST_ASSERT_EQUAL_PTR(NULL, list.first());
    TEST_ASSERT_EQUAL_PTR(NULL, list.last());
    TEST_ASSERT_EQUAL_PTR(NULL, list.current());
    TEST_ASSERT_FALSE(list.next());
    TEST_ASSERT_FALSE(list.prev());

    return;
}