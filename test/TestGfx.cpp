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
 * @brief  Graphics interface for testing purposes.
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "TestGfx.h"

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
 * Test the graphic functions.
 */
extern void testGfx()
{
    TestGfx     testGfx;
    const Color COLOR   = 0x1234;
    int16_t     x       = 0;
    int16_t     y       = 0;
    Color       color   = 0U;
    YAGfxStaticBitmap<TestGfx::WIDTH, TestGfx::HEIGHT>  bitmap;

    /* Verify screen size */
    TEST_ASSERT_EQUAL_UINT16(TestGfx::WIDTH, testGfx.getWidth());
    TEST_ASSERT_EQUAL_UINT16(TestGfx::HEIGHT, testGfx.getHeight());

    /* Test drawing a single pixel and read color back. */
    testGfx.drawPixel(0, 0, COLOR);
    TEST_ASSERT_EQUAL_UINT16(COLOR, testGfx.getColor(0, 0));

    /* Clear screen */
    testGfx.fillScreen(0U);
    TEST_ASSERT_TRUE(testGfx.verify(0, 0, TestGfx::WIDTH, TestGfx::HEIGHT, 0U));

    /* Test drawing a vertical line. */
    testGfx.drawVLine(0, 0, TestGfx::HEIGHT, COLOR);
    TEST_ASSERT_TRUE(testGfx.verify(0, 0, 1U, TestGfx::HEIGHT, COLOR));
    TEST_ASSERT_TRUE(testGfx.verify(1, 0, TestGfx::WIDTH - 1U, TestGfx::HEIGHT, 0U));

    /* Clear screen */
    testGfx.fillScreen(0U);
    TEST_ASSERT_TRUE(testGfx.verify(0, 0, TestGfx::WIDTH, TestGfx::HEIGHT, 0U));

    /* Test drawing a horizontal line. */
    testGfx.drawHLine(0, 0, TestGfx::WIDTH, COLOR);
    TEST_ASSERT_TRUE(testGfx.verify(0, 0, TestGfx::WIDTH, 1U, COLOR));
    TEST_ASSERT_TRUE(testGfx.verify(0, 1, TestGfx::WIDTH, TestGfx::HEIGHT - 1U, 0U));

    /* Clear screen */
    testGfx.fillScreen(0U);
    TEST_ASSERT_TRUE(testGfx.verify(0, 0, TestGfx::WIDTH, TestGfx::HEIGHT, 0U));

    /* Test drawing lines. */
    testGfx.drawLine(0, 0, TestGfx::WIDTH - 1, 0, COLOR);
    TEST_ASSERT_TRUE(testGfx.verify(0, 0, TestGfx::WIDTH - 1U, 0U, COLOR));

    testGfx.drawLine(0, TestGfx::HEIGHT - 1 , TestGfx::WIDTH - 1, TestGfx::HEIGHT - 1, COLOR);
    TEST_ASSERT_TRUE(testGfx.verify(0, TestGfx::HEIGHT - 1, TestGfx::WIDTH, 1U, COLOR));

    testGfx.drawLine(0, 1, 0, TestGfx::HEIGHT - 2, COLOR);
    TEST_ASSERT_TRUE(testGfx.verify(0, 1, 1U, TestGfx::HEIGHT - 2U, COLOR));

    testGfx.drawLine(TestGfx::WIDTH - 1, 1, TestGfx::WIDTH - 1U, TestGfx::HEIGHT - 2U, COLOR);
    TEST_ASSERT_TRUE(testGfx.verify(TestGfx::WIDTH - 1, 1, 1U, TestGfx::HEIGHT - 2U, COLOR));

    TEST_ASSERT_TRUE(testGfx.verify(1, 1, TestGfx::WIDTH - 2U, TestGfx::HEIGHT - 2U, 0U));

    /* Clear screen */
    testGfx.fillScreen(0U);
    TEST_ASSERT_TRUE(testGfx.verify(0, 0, TestGfx::WIDTH, TestGfx::HEIGHT, 0U));

    /* Test drawing a rectangle */
    testGfx.drawRectangle(0, 0, TestGfx::WIDTH, TestGfx::HEIGHT, COLOR);
    TEST_ASSERT_TRUE(testGfx.verify(0, 0, TestGfx::WIDTH, 1U, COLOR));
    TEST_ASSERT_TRUE(testGfx.verify(0, TestGfx::HEIGHT - 1, TestGfx::WIDTH, 1U, COLOR));
    TEST_ASSERT_TRUE(testGfx.verify(0, 1, 1U, TestGfx::HEIGHT - 2, COLOR));
    TEST_ASSERT_TRUE(testGfx.verify(TestGfx::WIDTH - 1, 1, 1U, TestGfx::HEIGHT - 2U, COLOR));
    TEST_ASSERT_TRUE(testGfx.verify(1, 1, TestGfx::WIDTH - 2U, TestGfx::HEIGHT - 2U, 0U));

    /* Clear screen */
    testGfx.fillScreen(0U);
    TEST_ASSERT_TRUE(testGfx.verify(0, 0, TestGfx::WIDTH, TestGfx::HEIGHT, 0U));

    /* Fill rectangle */
    testGfx.fillRect(0, 0, TestGfx::WIDTH / 2U, TestGfx::HEIGHT / 2U, COLOR);
    TEST_ASSERT_TRUE(testGfx.verify(0, 0, TestGfx::WIDTH / 2, TestGfx::HEIGHT / 2, COLOR));
    TEST_ASSERT_TRUE(testGfx.verify(TestGfx::WIDTH / 2, 0, TestGfx::WIDTH / 2, TestGfx::HEIGHT / 2, 0U));
    TEST_ASSERT_TRUE(testGfx.verify(0, TestGfx::HEIGHT / 2, TestGfx::WIDTH / 2, TestGfx::HEIGHT / 2, 0U));

    /* Clear screen */
    testGfx.fillScreen(0U);
    TEST_ASSERT_TRUE(testGfx.verify(0, 0, TestGfx::WIDTH, TestGfx::HEIGHT, 0U));

    /* Test drawing a bitmap. */
    for(y = 0; y < TestGfx::HEIGHT; ++y)
    {
        for(x = 0U; x < TestGfx::WIDTH; ++x)
        {
            bitmap.drawPixel(x, y, rand() % 0xFFFFU);
        }
    }

    testGfx.drawBitmap(0, 0, bitmap);

    for(y = 0U; y < TestGfx::HEIGHT; ++y)
    {
        for(x = 0U; x < TestGfx::WIDTH; ++x)
        {
            TEST_ASSERT_EQUAL_UINT16(bitmap.getColor(x, y), testGfx.getColor(x, y));
        }
    }

    /* Clear screen */
    testGfx.fillScreen(0U);
    TEST_ASSERT_TRUE(testGfx.verify(0, 0, TestGfx::WIDTH, TestGfx::HEIGHT, 0U));

    return;
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/