/* MIT License
 *
 * Copyright (c) 2019 - 2022 Andreas Merkle <web@blue-andi.de>
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
#include "TestBitmapWidget.h"
#include "TestGfx.h"

#include <unity.h>
#include <BitmapWidget.h>

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
 * Test bitmap widget.
 */
extern void testBitmapWidget()
{
    const uint8_t BITMAP_WIDTH      = TestGfx::HEIGHT;  /* Use height as width here for a square */
    const uint8_t BITMAP_HEIGHT     = TestGfx::HEIGHT;
    const char*   WIDGET_NAME       = "bmpWidgetName";

    TestGfx         testGfx;
    BitmapWidget    bitmapWidget;
    Color           bitmap[BITMAP_WIDTH * BITMAP_HEIGHT];
    uint8_t         x               = 0U;
    uint8_t         y               = 0U;
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
    for(y = 0U; y < BITMAP_HEIGHT; ++y)
    {
        for(x = 0U; x < BITMAP_WIDTH; ++x)
        {
            bitmap[x + y * BITMAP_WIDTH] = x + y * BITMAP_WIDTH;
        }
    }

    /* Set bitmap and read back */
    bitmapWidget.set(bitmap, BITMAP_WIDTH, BITMAP_HEIGHT);
    bitmapPtr = bitmapWidget.get(width, height);
    TEST_ASSERT_EQUAL_UINT16(BITMAP_WIDTH, width);
    TEST_ASSERT_EQUAL_UINT16(BITMAP_HEIGHT, height);

    for(y = 0U; y < BITMAP_HEIGHT; ++y)
    {
        for(x = 0U; x < BITMAP_WIDTH; ++x)
        {
            TEST_ASSERT_EQUAL_UINT32(bitmap[x + y * BITMAP_WIDTH], bitmapPtr[x + y * BITMAP_WIDTH]);
        }
    }

    /* Draw bitmap and verify */
    bitmapWidget.update(testGfx);
    displayBuffer = testGfx.getBuffer();

    for(y = 0U; y < BITMAP_HEIGHT; ++y)
    {
        for(x = 0U; x < BITMAP_WIDTH; ++x)
        {
            TEST_ASSERT_EQUAL_UINT16(x + y * BITMAP_WIDTH, displayBuffer[x + y * TestGfx::WIDTH]);
        }
    }

    return;
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/
