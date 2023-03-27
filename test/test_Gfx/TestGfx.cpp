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
 * @brief  Graphics interface for testing purposes.
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <unity.h>
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

static void testGfx();

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

    RUN_TEST(testGfx);

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
 * Test the graphic functions.
 */
static void testGfx()
{
    YAGfxTest   testGfx;
    const Color COLOR   = 0x1234;
    int16_t     x       = 0;
    int16_t     y       = 0;
    Color       color   = 0U;
    YAGfxStaticBitmap<YAGfxTest::WIDTH, YAGfxTest::HEIGHT>  bitmap;

    /* Verify screen size */
    TEST_ASSERT_EQUAL_UINT16(YAGfxTest::WIDTH, testGfx.getWidth());
    TEST_ASSERT_EQUAL_UINT16(YAGfxTest::HEIGHT, testGfx.getHeight());

    /* Test drawing a single pixel and read color back. */
    testGfx.drawPixel(0, 0, COLOR);
    TEST_ASSERT_EQUAL_UINT16(COLOR, testGfx.getColor(0, 0));

    /* Clear screen */
    testGfx.fillScreen(0U);
    TEST_ASSERT_TRUE(testGfx.verify(0, 0, YAGfxTest::WIDTH, YAGfxTest::HEIGHT, 0U));

    /* Test drawing a vertical line. */
    testGfx.drawVLine(0, 0, YAGfxTest::HEIGHT, COLOR);
    TEST_ASSERT_TRUE(testGfx.verify(0, 0, 1U, YAGfxTest::HEIGHT, COLOR));
    TEST_ASSERT_TRUE(testGfx.verify(1, 0, YAGfxTest::WIDTH - 1U, YAGfxTest::HEIGHT, 0U));

    /* Clear screen */
    testGfx.fillScreen(0U);
    TEST_ASSERT_TRUE(testGfx.verify(0, 0, YAGfxTest::WIDTH, YAGfxTest::HEIGHT, 0U));

    /* Test drawing a horizontal line. */
    testGfx.drawHLine(0, 0, YAGfxTest::WIDTH, COLOR);
    TEST_ASSERT_TRUE(testGfx.verify(0, 0, YAGfxTest::WIDTH, 1U, COLOR));
    TEST_ASSERT_TRUE(testGfx.verify(0, 1, YAGfxTest::WIDTH, YAGfxTest::HEIGHT - 1U, 0U));

    /* Clear screen */
    testGfx.fillScreen(0U);
    TEST_ASSERT_TRUE(testGfx.verify(0, 0, YAGfxTest::WIDTH, YAGfxTest::HEIGHT, 0U));

    /* Test drawing lines. */
    testGfx.drawLine(0, 0, YAGfxTest::WIDTH - 1, 0, COLOR);
    TEST_ASSERT_TRUE(testGfx.verify(0, 0, YAGfxTest::WIDTH - 1U, 0U, COLOR));

    testGfx.drawLine(0, YAGfxTest::HEIGHT - 1 , YAGfxTest::WIDTH - 1, YAGfxTest::HEIGHT - 1, COLOR);
    TEST_ASSERT_TRUE(testGfx.verify(0, YAGfxTest::HEIGHT - 1, YAGfxTest::WIDTH, 1U, COLOR));

    testGfx.drawLine(0, 1, 0, YAGfxTest::HEIGHT - 2, COLOR);
    TEST_ASSERT_TRUE(testGfx.verify(0, 1, 1U, YAGfxTest::HEIGHT - 2U, COLOR));

    testGfx.drawLine(YAGfxTest::WIDTH - 1, 1, YAGfxTest::WIDTH - 1U, YAGfxTest::HEIGHT - 2U, COLOR);
    TEST_ASSERT_TRUE(testGfx.verify(YAGfxTest::WIDTH - 1, 1, 1U, YAGfxTest::HEIGHT - 2U, COLOR));

    TEST_ASSERT_TRUE(testGfx.verify(1, 1, YAGfxTest::WIDTH - 2U, YAGfxTest::HEIGHT - 2U, 0U));

    /* Clear screen */
    testGfx.fillScreen(0U);
    TEST_ASSERT_TRUE(testGfx.verify(0, 0, YAGfxTest::WIDTH, YAGfxTest::HEIGHT, 0U));

    /* Test drawing a rectangle */
    testGfx.drawRectangle(0, 0, YAGfxTest::WIDTH, YAGfxTest::HEIGHT, COLOR);
    TEST_ASSERT_TRUE(testGfx.verify(0, 0, YAGfxTest::WIDTH, 1U, COLOR));
    TEST_ASSERT_TRUE(testGfx.verify(0, YAGfxTest::HEIGHT - 1, YAGfxTest::WIDTH, 1U, COLOR));
    TEST_ASSERT_TRUE(testGfx.verify(0, 1, 1U, YAGfxTest::HEIGHT - 2, COLOR));
    TEST_ASSERT_TRUE(testGfx.verify(YAGfxTest::WIDTH - 1, 1, 1U, YAGfxTest::HEIGHT - 2U, COLOR));
    TEST_ASSERT_TRUE(testGfx.verify(1, 1, YAGfxTest::WIDTH - 2U, YAGfxTest::HEIGHT - 2U, 0U));

    /* Clear screen */
    testGfx.fillScreen(0U);
    TEST_ASSERT_TRUE(testGfx.verify(0, 0, YAGfxTest::WIDTH, YAGfxTest::HEIGHT, 0U));

    /* Test drawing a circle */
    testGfx.drawCircle(YAGfxTest::WIDTH / 2, YAGfxTest::HEIGHT / 2, 0U, COLOR);
    TEST_ASSERT_TRUE(testGfx.getColor(YAGfxTest::WIDTH / 2, YAGfxTest::HEIGHT / 2) == COLOR);

    testGfx.fillScreen(0U);
    testGfx.drawCircle(YAGfxTest::WIDTH / 2, YAGfxTest::HEIGHT / 2, 1U, COLOR);
    TEST_ASSERT_TRUE(testGfx.getColor(YAGfxTest::WIDTH / 2, YAGfxTest::HEIGHT / 2 + 1) == COLOR);
    TEST_ASSERT_TRUE(testGfx.getColor(YAGfxTest::WIDTH / 2, YAGfxTest::HEIGHT / 2 - 1) == COLOR);
    TEST_ASSERT_TRUE(testGfx.getColor(YAGfxTest::WIDTH / 2 + 1, YAGfxTest::HEIGHT / 2) == COLOR);
    TEST_ASSERT_TRUE(testGfx.getColor(YAGfxTest::WIDTH / 2 - 1, YAGfxTest::HEIGHT / 2) == COLOR);

    /* Clear screen */
    testGfx.fillScreen(0U);
    TEST_ASSERT_TRUE(testGfx.verify(0, 0, YAGfxTest::WIDTH, YAGfxTest::HEIGHT, 0U));

    /* Fill rectangle */
    testGfx.fillRect(0, 0, YAGfxTest::WIDTH / 2U, YAGfxTest::HEIGHT / 2U, COLOR);
    TEST_ASSERT_TRUE(testGfx.verify(0, 0, YAGfxTest::WIDTH / 2, YAGfxTest::HEIGHT / 2, COLOR));
    TEST_ASSERT_TRUE(testGfx.verify(YAGfxTest::WIDTH / 2, 0, YAGfxTest::WIDTH / 2, YAGfxTest::HEIGHT / 2, 0U));
    TEST_ASSERT_TRUE(testGfx.verify(0, YAGfxTest::HEIGHT / 2, YAGfxTest::WIDTH / 2, YAGfxTest::HEIGHT / 2, 0U));

    /* Clear screen */
    testGfx.fillScreen(0U);
    TEST_ASSERT_TRUE(testGfx.verify(0, 0, YAGfxTest::WIDTH, YAGfxTest::HEIGHT, 0U));

    /* Test drawing a bitmap. */
    for(y = 0; y < YAGfxTest::HEIGHT; ++y)
    {
        for(x = 0U; x < YAGfxTest::WIDTH; ++x)
        {
            bitmap.drawPixel(x, y, rand() % 0xFFFFU);
        }
    }

    testGfx.drawBitmap(0, 0, bitmap);

    for(y = 0U; y < YAGfxTest::HEIGHT; ++y)
    {
        for(x = 0U; x < YAGfxTest::WIDTH; ++x)
        {
            TEST_ASSERT_EQUAL_UINT16(bitmap.getColor(x, y), testGfx.getColor(x, y));
        }
    }

    /* Clear screen */
    testGfx.fillScreen(0U);
    TEST_ASSERT_TRUE(testGfx.verify(0, 0, YAGfxTest::WIDTH, YAGfxTest::HEIGHT, 0U));

    return;
}
