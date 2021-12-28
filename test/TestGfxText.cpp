/* MIT License
 *
 * Copyright (c) 2019 - 2021 Andreas Merkle <web@blue-andi.de>
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
#include "TestGfxText.h"
#include "TestGfx.h"

#include <unity.h>
#include <YAGfxText.h>
#include <TomThumb.h>

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
 * Test the text graphic functions.
 */
extern void testGfxText()
{
    TestGfx     testGfx;
    YAGfxText   testGfxText;
    const Color COLOR       = 0x1234;
    uint16_t    width       = 0U;
    uint16_t    height      = 0U;
    int16_t     cursorPosX  = 0;
    int16_t     cursorPosY  = 0;

    /* Verify cursor positon */
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
    TEST_ASSERT_TRUE(testGfx.verify(0, 0, TestGfx::WIDTH, TestGfx::HEIGHT, 0U));

    /* Select font and draw again. The character shall be shown. */
    testGfxText.setFont(&TomThumb);
    TEST_ASSERT_TRUE(testGfxText.getTextBoundingBox(testGfx.getWidth(), testGfx.getHeight(), "Test", width, height));

    return;
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/