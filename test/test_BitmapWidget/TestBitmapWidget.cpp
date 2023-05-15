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
 * @brief  Test bitmap widget.
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <unity.h>
#include <BitmapWidget.h>
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

static void testBitmapWidget();

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

    RUN_TEST(testBitmapWidget);

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
 * Test bitmap widget.
 */
static void testBitmapWidget()
{
    const uint16_t BITMAP_WIDTH     = YAGfxTest::HEIGHT;    /* Use height as width here for a square */
    const uint16_t BITMAP_HEIGHT    = YAGfxTest::HEIGHT;
    const char*   WIDGET_NAME       = "bmpWidgetName";

    YAGfxTest                                       testGfx;
    BitmapWidget                                    bitmapWidget;
    YAGfxStaticBitmap<BITMAP_WIDTH, BITMAP_HEIGHT>  bitmap;

    int16_t         x               = 0;
    int16_t         y               = 0;
    const Color*    bitmapPtr       = nullptr;
    uint16_t        width           = 0U;
    uint16_t        height          = 0U;
    Color*          displayBuffer   = nullptr;

    /* Verify widget type name */
    TEST_ASSERT_EQUAL_STRING(BitmapWidget::WIDGET_TYPE, bitmapWidget.getType());

    /* No widget name is set, it must be empty. */
    TEST_ASSERT_EQUAL_STRING("", bitmapWidget.getName().c_str());

    /* Set widget name and read back. */
    bitmapWidget.setName(WIDGET_NAME);
    TEST_ASSERT_EQUAL_STRING(WIDGET_NAME, bitmapWidget.getName().c_str());

    /* Find widget with empty name.
     * Expected: Not found
     */
    TEST_ASSERT_NULL(bitmapWidget.find(""));

    /* Find widget with its name.
     * Expected: Widget is found
     */
    TEST_ASSERT_NOT_NULL(bitmapWidget.find(WIDGET_NAME));
    TEST_ASSERT_EQUAL_PTR(&bitmapWidget, bitmapWidget.find(WIDGET_NAME));

    /* Create bitmap */
    for(y = 0; y < BITMAP_HEIGHT; ++y)
    {
        for(x = 0; x < BITMAP_WIDTH; ++x)
        {
            bitmap.drawPixel(x, y, x + y * BITMAP_WIDTH);
        }
    }

    /* Set bitmap and read back */
    bitmapWidget.set(bitmap);
    TEST_ASSERT_EQUAL_UINT16(BITMAP_WIDTH, bitmapWidget.get().getWidth());
    TEST_ASSERT_EQUAL_UINT16(BITMAP_HEIGHT, bitmapWidget.get().getHeight());

    for(y = 0; y < BITMAP_HEIGHT; ++y)
    {
        for(x = 0; x < BITMAP_WIDTH; ++x)
        {
            TEST_ASSERT_EQUAL_UINT32(bitmap.getColor(x, y), bitmapWidget.get().getColor(x, y));
        }
    }

    /* Draw bitmap and verify */
    bitmapWidget.update(testGfx);
    displayBuffer = testGfx.getBuffer();

    for(y = 0; y < BITMAP_HEIGHT; ++y)
    {
        for(x = 0; x < BITMAP_WIDTH; ++x)
        {
            TEST_ASSERT_EQUAL_UINT16(x + y * BITMAP_WIDTH, displayBuffer[x + y * YAGfxTest::WIDTH]);
        }
    }

    return;
}
