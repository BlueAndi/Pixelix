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

#include <LinkedList.hpp>

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

    return UNITY_END();
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
    int*            element = NULL;
    int             value   = 1;
    int             index   = 0;
    const int       max     = 3;

    /* List is empty. */
    TEST_ASSERT_EQUAL_PTR(NULL, list.first());
    TEST_ASSERT_EQUAL_PTR(NULL, list.last());
    TEST_ASSERT_EQUAL_PTR(NULL, list.current());
    TEST_ASSERT_FALSE(list.next());
    TEST_ASSERT_FALSE(list.prev());

    /* Add one element. */
    list.append(value);

    TEST_ASSERT_NOT_EQUAL(NULL, list.first());
    TEST_ASSERT_NOT_EQUAL(NULL, list.last());
    TEST_ASSERT_NOT_EQUAL(NULL, list.current());

    TEST_ASSERT_EQUAL_INT(value, *list.first());
    TEST_ASSERT_EQUAL_INT(value, *list.last());
    TEST_ASSERT_EQUAL_INT(value, *list.current());

    TEST_ASSERT_EQUAL_PTR(list.first(), list.last());
    TEST_ASSERT_EQUAL_PTR(list.first(), list.current());

    /* Remove element from list. List is now empty. */
    list.removeSelected();

    TEST_ASSERT_EQUAL_PTR(NULL, list.first());
    TEST_ASSERT_EQUAL_PTR(NULL, list.last());
    TEST_ASSERT_EQUAL_PTR(NULL, list.current());
    TEST_ASSERT_FALSE(list.next());
    TEST_ASSERT_FALSE(list.prev());

    /* Add more elements */
    for(index = 1; index <= max; ++index)
    {
        list.append(index);
    }

    TEST_ASSERT_NOT_EQUAL(NULL, list.first());
    TEST_ASSERT_EQUAL_INT(1, *list.first());

    TEST_ASSERT_NOT_EQUAL(NULL, list.last());
    TEST_ASSERT_EQUAL_INT(max, *list.last());

    /* Select element for element, from head to tail. */
    for(index = 1; index <= max; ++index)
    {
        TEST_ASSERT_NOT_EQUAL(NULL, list.current());
        TEST_ASSERT_EQUAL_INT(index, *list.current());

        if (index < max)
        {
            TEST_ASSERT_TRUE(list.next());
        }
        else
        {
            TEST_ASSERT_FALSE(list.next());
        }
        
    }

    /* Select element for element, from tail to head. */
    for(index = max; index > 0; --index)
    {
        TEST_ASSERT_NOT_EQUAL(NULL, list.current());
        TEST_ASSERT_EQUAL_INT(index, *list.current());

        if (index > 1)
        {
            TEST_ASSERT_TRUE(list.prev());
        }
        else
        {
            TEST_ASSERT_FALSE(list.prev());
        }
        
    }

    /* Remove all elements */
    for(index = 1; index <= max; ++index)
    {
        list.removeSelected();
    }

    TEST_ASSERT_EQUAL_PTR(NULL, list.first());
    TEST_ASSERT_EQUAL_PTR(NULL, list.last());
    TEST_ASSERT_EQUAL_PTR(NULL, list.current());
    TEST_ASSERT_FALSE(list.next());
    TEST_ASSERT_FALSE(list.prev());

    /* Insert elements again */
    for(index = 1; index <= max; ++index)
    {
        list.append(index);
    }

    /* Copy it via copy constructor */
    {
        LinkedList<int> copyOfList = list;

        list.selectFirstElement();
        for(index = 1; index <= max; ++index)
        {
            TEST_ASSERT_NOT_NULL(copyOfList.current());
            TEST_ASSERT_NOT_NULL(list.current());
            TEST_ASSERT_NOT_EQUAL(copyOfList.current(), list.current());
            TEST_ASSERT_EQUAL_INT(*copyOfList.current(), *list.current());
            copyOfList.next();
            list.next();
        }
    }

    /* Copy it via assignment */
    {
        LinkedList<int> copyOfList;
        copyOfList = list;

        list.selectFirstElement();
        for(index = 1; index <= max; ++index)
        {
            TEST_ASSERT_NOT_NULL(copyOfList.current());
            TEST_ASSERT_NOT_NULL(list.current());
            TEST_ASSERT_NOT_EQUAL(copyOfList.current(), list.current());
            TEST_ASSERT_EQUAL_INT(*copyOfList.current(), *list.current());
            copyOfList.next();
            list.next();
        }
    }

    return;
}