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
 * @brief  Test the text graphic functions.
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <unity.h>
#include <YAGfxText.h>
#include <TomThumb.h>
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

static void testGfxText();

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

    RUN_TEST(testGfxText);

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
 * Test the text graphic functions.
 */
static void testGfxText()
{
    YAGfxTest       testGfx;
    YAGfxText       testGfxText;
    const Color     COLOR               = 0x1234;
    uint16_t        boundingBoxWidth    = 0U;
    uint16_t        boundingBoxHeight   = 0U;
    int16_t         cursorPosX          = 0;
    int16_t         cursorPosY          = 0;
    const GFXglyph* glyph               = nullptr;
    uint8_t         index               = 0U;

    /* Verify cursor position */
    testGfxText.getTextCursorPos(cursorPosX, cursorPosY);
    TEST_ASSERT_EQUAL_INT16(0, cursorPosX);
    TEST_ASSERT_EQUAL_INT16(0, cursorPosY);
    TEST_ASSERT_EQUAL_INT16(0, testGfxText.getTextCursorPosX());
    TEST_ASSERT_EQUAL_INT16(0, testGfxText.getTextCursorPosY());

    testGfxText.setTextCursorPos(1, 2);
    testGfxText.getTextCursorPos(cursorPosX, cursorPosY);
    TEST_ASSERT_EQUAL_INT16(1, cursorPosX);
    TEST_ASSERT_EQUAL_INT16(2, cursorPosY);
    TEST_ASSERT_EQUAL_INT16(1, testGfxText.getTextCursorPosX());
    TEST_ASSERT_EQUAL_INT16(2, testGfxText.getTextCursorPosY());

    /* Draw character, but without font. Nothing shall be shown. */
    testGfxText.setTextCursorPos(0, 6);
    testGfxText.setTextWrap(false);
    testGfxText.setTextColor(COLOR);
    testGfxText.drawChar(testGfx, 'T');
    TEST_ASSERT_TRUE(testGfx.verify(0, 0, YAGfxTest::WIDTH, YAGfxTest::HEIGHT, 0U));

    /* Select font and draw again. The character shall be shown. */
    testGfxText.setFont(&TomThumb);
    testGfxText.setTextWrap(false);
    TEST_ASSERT_TRUE(testGfxText.getTextBoundingBox(testGfx.getWidth(), "Test", boundingBoxWidth, boundingBoxHeight));
    TEST_ASSERT_LESS_OR_EQUAL_UINT16(TomThumb.yAdvance, boundingBoxHeight);

    /* Check text wrap around. */
    testGfx.fillScreen(ColorDef::BLACK);
    glyph = testGfxText.getFont().getGlyph('A');
    TEST_ASSERT_NOT_NULL(glyph);

    testGfxText.setTextWrap(false);
    TEST_ASSERT_TRUE(testGfxText.getTextBoundingBox(glyph->xAdvance, "AA", boundingBoxWidth, boundingBoxHeight));
    TEST_ASSERT_EQUAL_UINT16(1U * TomThumb.yAdvance, boundingBoxHeight);

    testGfxText.setTextWrap(true);
    TEST_ASSERT_TRUE(testGfxText.getTextBoundingBox(glyph->xAdvance, "AA", boundingBoxWidth, boundingBoxHeight));
    TEST_ASSERT_EQUAL_UINT16(2U * TomThumb.yAdvance, boundingBoxHeight);

    return;
}
