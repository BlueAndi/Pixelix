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
 * @brief  Widget for test purposes.
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <unity.h>
#include <Util.h>

#include "../common/YAGfxTest.hpp"
#include "../common/TestWidget.hpp"

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

template < typename T >
static T getMin(const T value1, const T value2);
static void testWidget();

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

    RUN_TEST(testWidget);

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
 * Get minimum of two values.
 *
 * @param[in] value1    Value 1
 * @param[in] value2    Value 2
 *
 * @return Minimum of value 1 and value 2
 */
template < typename T >
static T getMin(const T value1, const T value2)
{
    if (value1 < value2)
    {
        return value1;
    }

    return value2;
}

/**
 * Widget tests.
 */
static void testWidget()
{
    YAGfxTest   testGfx;
    TestWidget  testWidget;
    int16_t     posX        = -1;
    int16_t     posY        = -1;
    const Color COLOR       = 0x123456;
    const char* testStr     = "myWidget";

    /* Verify widget type name */
    TEST_ASSERT_EQUAL_STRING(TestWidget::WIDGET_TYPE, testWidget.getType());

    /* No widget name is set, it must be empty. */
    TEST_ASSERT_EQUAL_STRING("", testWidget.getName().c_str());

    /* Set widget name and read back. */
    testWidget.setName(testStr);
    TEST_ASSERT_EQUAL_STRING(testStr, testWidget.getName().c_str());

    /* Find widget with empty name.
     * Expected: Not found
     */
    TEST_ASSERT_NULL(testWidget.find(""));

    /* Find widget with its name.
     * Expected: Widget is found
     */
    TEST_ASSERT_NOT_NULL(testWidget.find(testStr));
    TEST_ASSERT_EQUAL_PTR(&testWidget, testWidget.find(testStr));

    /* Clear name */
    testWidget.setName("");
    TEST_ASSERT_EQUAL_STRING("", testWidget.getName().c_str());

    /* Current position must be (0, 0) */
    testWidget.getPos(posX, posY);
    TEST_ASSERT_EQUAL_INT16(0, posX);
    TEST_ASSERT_EQUAL_INT16(0, posY);

    /* Move widget and verify position again. */
    testWidget.move(10, 20);
    testWidget.getPos(posX, posY);
    TEST_ASSERT_EQUAL_INT16(10, posX);
    TEST_ASSERT_EQUAL_INT16(20, posY);

    /* Verify widget type name */
    TEST_ASSERT_EQUAL_STRING(TestWidget::WIDGET_TYPE, testWidget.getType());

    /* For the whole test, set the widget color. */
    testWidget.setPenColor(COLOR);

    /* Draw widget at position (0, 0) */
    posX = 0;
    posY = 0;
    testWidget.move(posX, posY);
    testGfx.fill(0);
    testWidget.update(testGfx);
    TEST_ASSERT_TRUE(testGfx.verify(posX,
                                    posY,
                                    getMin<uint16_t>(YAGfxTest::WIDTH - posX, TestWidget::WIDTH),
                                    getMin<uint16_t>(YAGfxTest::HEIGHT - posY, TestWidget::HEIGHT),
                                    COLOR));

    /* Draw widget at position (2, 1) and verify widget movement. */
    posX = 2;
    posY = 1;
    testWidget.move(posX, posY);
    testGfx.fill(0);
    testWidget.update(testGfx);
    TEST_ASSERT_TRUE(testGfx.verify(posX,
                                    posY,
                                    getMin<uint16_t>(YAGfxTest::WIDTH - posX, TestWidget::WIDTH),
                                    getMin<uint16_t>(YAGfxTest::HEIGHT - posY, TestWidget::HEIGHT),
                                    COLOR));

    return;
}
