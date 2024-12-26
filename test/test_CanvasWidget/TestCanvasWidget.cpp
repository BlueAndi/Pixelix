/* MIT License
 *
 * Copyright (c) 2019 - 2024 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Test canvas widget.
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <unity.h>
#include <CanvasWidget.h>
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

static void testCanvasWidget();

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

    RUN_TEST(testCanvasWidget);

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
 * Test canvas widget.
 */
static void testCanvasWidget()
{
    YAGfxTest       testGfx;
    CanvasWidget    canvasWidget;
    const char*     WIDGET_NAME     = "progressBarName";
    Color*          displayBuffer   = testGfx.getBuffer();
    const Color     TEST_COLOR      = 0x00112233;

    /* Verify widget type name */
    TEST_ASSERT_EQUAL_STRING(CanvasWidget::WIDGET_TYPE, canvasWidget.getType());

    /* No widget name is set, it must be empty. */
    TEST_ASSERT_EQUAL_STRING("", canvasWidget.getName().c_str());

    /* Set widget name and read back. */
    canvasWidget.setName(WIDGET_NAME);
    TEST_ASSERT_EQUAL_STRING(WIDGET_NAME, canvasWidget.getName().c_str());

    /* Default width/height will be 0. */
    TEST_ASSERT_EQUAL_UINT16(0U, canvasWidget.getWidth());
    TEST_ASSERT_EQUAL_UINT16(0U, canvasWidget.getHeight());

    /* Change width/height to max. */
    canvasWidget.setWidth(YAGfxTest::WIDTH);
    canvasWidget.setHeight(YAGfxTest::HEIGHT);
    TEST_ASSERT_EQUAL_UINT16(YAGfxTest::WIDTH, canvasWidget.getWidth());
    TEST_ASSERT_EQUAL_UINT16(YAGfxTest::HEIGHT, canvasWidget.getHeight());

    /* Draw and verify. */
    canvasWidget.drawPixel(0, 0, TEST_COLOR);
    canvasWidget.update(testGfx);
    TEST_ASSERT_EQUAL_UINT32(TEST_COLOR, canvasWidget.getColor(0, 0));
    TEST_ASSERT_EQUAL_UINT32(TEST_COLOR, displayBuffer[0]);
}
