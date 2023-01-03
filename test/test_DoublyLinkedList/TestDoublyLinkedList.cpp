/* MIT License
 *
 * Copyright (c) 2019 - 2023 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Doubly linked list tests.
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <unity.h>
#include <LinkedList.hpp>
#include <Util.h>

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

static void testDoublyLinkedList();

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/******************************************************************************
 * Public Methods
 *****************************************************************************/

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

/**
 * Main entry point
 *
 * @param[in] argc  Number of command line arguments
 * @param[in] argv  Command line arguments
 */
extern int main(int argc, char **argv)
{
    UTIL_NOT_USED(argc);
    UTIL_NOT_USED(argv);

    UNITY_BEGIN();

    RUN_TEST(testDoublyLinkedList);

    return UNITY_END();
}

/**
 * Setup a test. This function will be called before every test by unity.
 */
extern void setUp(void)
{
    /* Not used. */
}

/**
 * Clean up test. This function will be called after every test by unity.
 */
extern void tearDown(void)
{
    /* Not used. */
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/**
 * Doubly linked list tests.
 */
static void testDoublyLinkedList()
{
    DLinkedList<uint32_t>           list;
    DLinkedListIterator<uint32_t>   it(list);
    uint32_t                        value   = 1U;
    uint32_t                        index   = 0U;
    const uint32_t                  max     = 3U;

    /* List is empty. */
    TEST_ASSERT_FALSE(it.first());
    TEST_ASSERT_FALSE(it.last());
    TEST_ASSERT_NULL(it.current());
    TEST_ASSERT_FALSE(it.next());
    TEST_ASSERT_FALSE(it.prev());
    TEST_ASSERT_EQUAL_UINT32(0u, list.getNumOfElements());

    /* Add one element. */
    TEST_ASSERT_TRUE(list.append(value));
    TEST_ASSERT_EQUAL_UINT32(1u, list.getNumOfElements());

    TEST_ASSERT_TRUE(it.first());
    TEST_ASSERT_NOT_NULL(it.current());
    TEST_ASSERT_EQUAL_INT(value, *it.current());

    TEST_ASSERT_TRUE(it.last());
    TEST_ASSERT_NOT_NULL(it.current());
    TEST_ASSERT_EQUAL_INT(value, *it.current());

    /* Remove element from list. List is now empty. */
    it.remove();
    TEST_ASSERT_EQUAL_UINT32(0u, list.getNumOfElements());

    TEST_ASSERT_FALSE(it.first());
    TEST_ASSERT_FALSE(it.last());
    TEST_ASSERT_FALSE(it.current());
    TEST_ASSERT_FALSE(it.next());
    TEST_ASSERT_FALSE(it.prev());

    /* Add more elements */
    for(index = 1U; index <= max; ++index)
    {
        TEST_ASSERT_TRUE(list.append(index));
        TEST_ASSERT_EQUAL_UINT32(index, list.getNumOfElements());
    }

    TEST_ASSERT_TRUE(it.first());
    TEST_ASSERT_EQUAL_INT(1u, *it.current());

    TEST_ASSERT_TRUE(it.last());
    TEST_ASSERT_EQUAL_INT(max, *it.current());

    /* Select element for element, from head to tail. */
    TEST_ASSERT_TRUE(it.first());
    for(index = 1U; index <= max; ++index)
    {
        TEST_ASSERT_NOT_NULL(it.current());
        TEST_ASSERT_EQUAL_INT(index, *it.current());

        if (index < max)
        {
            TEST_ASSERT_TRUE(it.next());
        }
        else
        {
            TEST_ASSERT_FALSE(it.next());
        }
    }

    /* Select element for element, from tail to head. */
    TEST_ASSERT_TRUE(it.last());
    for(index = max; index > 0U; --index)
    {
        TEST_ASSERT_NOT_NULL(it.current());
        TEST_ASSERT_EQUAL_INT(index, *it.current());

        if (index > 1U)
        {
            TEST_ASSERT_TRUE(it.prev());
        }
        else
        {
            TEST_ASSERT_FALSE(it.prev());
        }
    }

    /* Remove all elements */
    for(index = 1; index <= max; ++index)
    {
        it.remove();
        TEST_ASSERT_EQUAL_UINT32(max - index, list.getNumOfElements());
    }

    TEST_ASSERT_FALSE(it.first());
    TEST_ASSERT_FALSE(it.last());
    TEST_ASSERT_NULL(it.current());
    TEST_ASSERT_FALSE(it.next());
    TEST_ASSERT_FALSE(it.prev());

    /* Insert elements again */
    for(index = 1U; index <= max; ++index)
    {
        TEST_ASSERT_TRUE(list.append(index));
        TEST_ASSERT_EQUAL_UINT32(index, list.getNumOfElements());
    }

    /* Copy it via copy constructor */
    {
        DLinkedList<uint32_t>           copyOfList = list;
        DLinkedListIterator<uint32_t>   itListCopy(copyOfList);

        TEST_ASSERT_TRUE(it.first());
        for(index = 1U; index <= max; ++index)
        {
            TEST_ASSERT_NOT_NULL(itListCopy.current());
            TEST_ASSERT_NOT_NULL(it.current());
            TEST_ASSERT_NOT_EQUAL(itListCopy.current(), it.current());
            TEST_ASSERT_EQUAL_INT(*itListCopy.current(), *it.current());
            (void)itListCopy.next();
            (void)it.next();
        }
    }

    /* Copy it via assignment */
    {
        DLinkedList<uint32_t>           copyOfList;
        DLinkedListIterator<uint32_t>   itListCopy(copyOfList);
        copyOfList = list;

        TEST_ASSERT_TRUE(it.first());
        for(index = 1U; index <= max; ++index)
        {
            TEST_ASSERT_NOT_NULL(itListCopy.current());
            TEST_ASSERT_NOT_NULL(it.current());
            TEST_ASSERT_NOT_EQUAL(itListCopy.current(), it.current());
            TEST_ASSERT_EQUAL_INT(*itListCopy.current(), *it.current());
            (void)itListCopy.next();
            (void)it.next();
        }
    }

    /* Find not existing element */
    TEST_ASSERT_TRUE(it.first());
    TEST_ASSERT_FALSE(it.find(max + 1));

    /* Find existing element */
    TEST_ASSERT_TRUE(it.first());
    TEST_ASSERT_TRUE(it.find(1U));
    TEST_ASSERT_EQUAL(1U, *it.current());

    TEST_ASSERT_TRUE(it.first());
    TEST_ASSERT_TRUE(it.find(max));
    TEST_ASSERT_EQUAL(max, *it.current());

    return;
}
