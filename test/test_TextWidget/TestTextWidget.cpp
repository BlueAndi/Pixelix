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
 * @brief  Test text widget.
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <unity.h>
#include <TextWidget.h>
#include <Util.h>

#include "../common/YAGfxTest.hpp"

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

static void testTextWidget();

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

    RUN_TEST(testTextWidget);

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
 * Test text widget.
 */
static void testTextWidget()
{
    YAGfxTest   testGfx;
    TextWidget  textWidget;
    String      testStr     = "test";
    const Color TEXT_COLOR  = 0x123456;
    const char* WIDGET_NAME = "textWidgetName";

    /* Verify widget type name */
    TEST_ASSERT_EQUAL_STRING(TextWidget::WIDGET_TYPE, textWidget.getType());

    /* No widget name is set, it must be empty. */
    TEST_ASSERT_EQUAL_STRING("", textWidget.getName().c_str());

    /* Set widget name and read back. */
    textWidget.setName(WIDGET_NAME);
    TEST_ASSERT_EQUAL_STRING(WIDGET_NAME, textWidget.getName().c_str());

    /* Find widget with empty name.
     * Expected: Not found
     */
    TEST_ASSERT_NULL(textWidget.find(""));

    /* Find widget with its name.
     * Expected: Widget is found
     */
    TEST_ASSERT_NOT_NULL(textWidget.find(WIDGET_NAME));
    TEST_ASSERT_EQUAL_PTR(&textWidget, textWidget.find(WIDGET_NAME));

    /* Default string is empty */
    TEST_ASSERT_EQUAL_STRING("", textWidget.getStr().c_str());

    /* Set/Get string */
    textWidget.setFormatStr(testStr);
    TEST_ASSERT_EQUAL_STRING(testStr.c_str(), textWidget.getStr().c_str());

    /* Default string color */
    TEST_ASSERT_EQUAL_UINT32(TextWidget::DEFAULT_TEXT_COLOR, textWidget.getTextColor());

    /* Set/Get text color */
    textWidget.setTextColor(TEXT_COLOR);
    TEST_ASSERT_EQUAL_UINT32(TEXT_COLOR, textWidget.getTextColor());

    /* Check for default font */
    TEST_ASSERT_NOT_NULL(textWidget.getFont().getGfxFont());
    TEST_ASSERT_EQUAL_PTR(TextWidget::DEFAULT_FONT.getGfxFont(), textWidget.getFont().getGfxFont());

    /* Font shall be used for drawing */
    textWidget.update(testGfx);
    TEST_ASSERT_NOT_NULL(textWidget.getFont().getGfxFont());
    TEST_ASSERT_EQUAL_PTR(TextWidget::DEFAULT_FONT.getGfxFont(), textWidget.getFont().getGfxFont());

    /* Set text with format tag and get text without format tag back. */
    textWidget.setFormatStr("\\#FF00FFHello World!");
    TEST_ASSERT_EQUAL_STRING("Hello World!", textWidget.getStr().c_str());

    /* Set text with non-escaped format tag and get text back, which must contain it. */
    textWidget.setFormatStr("#FF00FFHello World!");
    TEST_ASSERT_EQUAL_STRING("#FF00FFHello World!", textWidget.getStr().c_str());

    /* Set text with invalid format tag and get text back, which must contain it. */
    textWidget.setFormatStr("\\#ZZ00FFHello World!");
    TEST_ASSERT_EQUAL_STRING("#ZZ00FFHello World!", textWidget.getStr().c_str());

    /* Set text with invalid format tag and get text back, which must contain it. */
    textWidget.setFormatStr("\\#FF00FYeah!");
    TEST_ASSERT_EQUAL_STRING("#FF00FYeah!", textWidget.getStr().c_str());

    return;
}
