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
 * @brief  Widget group tests.
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <unity.h>
#include <WidgetGroup.h>
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
static void testWidgetGroup();

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

    RUN_TEST(testWidgetGroup);

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
 * Widget group tests.
 */
static void testWidgetGroup()
{
    const uint16_t  CANVAS_WIDTH        = 8;
    const uint16_t  CANVAS_HEIGHT       = 8;
    const int16_t   WIDGET_POS_X        = 2;
    const int16_t   WIDGET_POS_Y        = 2;
    const Color     WIDGET_COLOR        = 0x123456;
    const char*     CANVAS_NAME         = "canvasWidgetName";
    const char*     TEST_WIDGET_NAME    = "testWidgetName";

    YAGfxTest   testGfx;
    WidgetGroup testWGroup(CANVAS_WIDTH, CANVAS_HEIGHT, 0, 0);
    TestWidget  testWidget;
    TestWidget  testWidget2;

    /* Verify widget type name */
    TEST_ASSERT_EQUAL_STRING(WidgetGroup::WIDGET_TYPE, testWGroup.getType());

    /* Canvas contains no other widget, so nothing should be drawn. */
    testGfx.setCallCounterDrawPixel(0);
    testWGroup.update(testGfx);
    TEST_ASSERT_EQUAL_UINT32(0, testGfx.getCallCounterDrawPixel());
    TEST_ASSERT_TRUE(testGfx.verify(0, 0, TestWidget::WIDTH, TestWidget::HEIGHT, 0));

    /* Add widget to canvas, move widget and set draw pen */
    TEST_ASSERT_TRUE(testWGroup.addWidget(testWidget));
    testWidget.move(WIDGET_POS_X, WIDGET_POS_Y);
    testWidget.setPenColor(WIDGET_COLOR);

    /* Draw canvas with widget. Expected is a full drawn widget. */
    testGfx.fill(0);
    testWGroup.update(testGfx);
    TEST_ASSERT_TRUE(testGfx.verify(WIDGET_POS_X,
                                    WIDGET_POS_Y,
                                    getMin<uint16_t>(TestWidget::WIDTH, CANVAS_WIDTH - WIDGET_POS_X),
                                    getMin<uint16_t>(TestWidget::HEIGHT, CANVAS_HEIGHT - WIDGET_POS_Y),
                                    WIDGET_COLOR));

    /* Move widget outside canvas and try to draw. Expected is no drawing at all. */
    testGfx.fill(0);
    testWidget.move(CANVAS_WIDTH, CANVAS_HEIGHT);
    testWGroup.update(testGfx);
    TEST_ASSERT_TRUE(testGfx.verify(0,
                                    0,
                                    CANVAS_WIDTH,
                                    CANVAS_HEIGHT,
                                    0));

    /* Move widget half outside canvas and draw. Expected is partly drawing. */
    testGfx.fill(0);
    testWidget.move(CANVAS_WIDTH / 2, CANVAS_HEIGHT / 2);
    testWGroup.update(testGfx);
    TEST_ASSERT_TRUE(testGfx.verify(CANVAS_WIDTH / 2,
                                    CANVAS_HEIGHT / 2,
                                    CANVAS_WIDTH / 2,
                                    CANVAS_HEIGHT / 2,
                                    WIDGET_COLOR));

    /* No widget name is set, it must be empty. */
    TEST_ASSERT_EQUAL_STRING("", testWGroup.getName().c_str());

    /* Set widget name and read back. */
    testWGroup.setName(CANVAS_NAME);
    TEST_ASSERT_EQUAL_STRING(CANVAS_NAME, testWGroup.getName().c_str());

    /* Find widget with its name.
     * Expected: Widget is found
     */
    TEST_ASSERT_NOT_NULL(testWGroup.find(CANVAS_NAME));
    TEST_ASSERT_EQUAL_PTR(static_cast<Widget*>(&testWGroup), testWGroup.find(CANVAS_NAME));

    /* Find widget in container, but widget has no name.
     * Expected: Test widget not found
     */
    TEST_ASSERT_NULL(testWGroup.find(TEST_WIDGET_NAME));

    /* Find widget in container.
     * Expected: Test widget found
     */
    testWidget.setName(TEST_WIDGET_NAME);
    TEST_ASSERT_NOT_NULL(testWGroup.find(TEST_WIDGET_NAME));
    TEST_ASSERT_EQUAL_PTR(&testWidget, testWGroup.find(TEST_WIDGET_NAME));

    /* Find widget in container and container has no name.
     * Expected: Test widget found
     */
    testWidget.setName(TEST_WIDGET_NAME);
    testWGroup.setName("");
    TEST_ASSERT_NOT_NULL(testWGroup.find(TEST_WIDGET_NAME));
    TEST_ASSERT_EQUAL_PTR(&testWidget, testWGroup.find(TEST_WIDGET_NAME));

    /* Find widget in container, which contains 2 widgets.
     * Expected: Test widget found
     */
    testWidget.setName(TEST_WIDGET_NAME);
    testWGroup.setName("");
    TEST_ASSERT_TRUE(testWGroup.addWidget(testWidget2));
    TEST_ASSERT_NOT_NULL(testWGroup.find(TEST_WIDGET_NAME));
    TEST_ASSERT_EQUAL_PTR(&testWidget, testWGroup.find(TEST_WIDGET_NAME));

    return;
}
