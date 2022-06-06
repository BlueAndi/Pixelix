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
 * @brief  Test bitmap image loader.
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "TestBmpImgLoader.h"
#include "TestGfx.h"

#include <unity.h>
#include <FS.h>
#include <BmpImgLoader.h>
#include <YAGfxBitmap.h>

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

static void testBmpImgLoader();

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

extern int testSuiteBmpImgLoader()
{
    UNITY_BEGIN();

    RUN_TEST(testBmpImgLoader);

    return UNITY_END();
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/**
 * Test bitmap image loader.
 */
static void testBmpImgLoader()
{
    BmpImgLoader        loader;
    YAGfxDynamicBitmap  bitmap;
    FS                  localFileSystem;

    /* Load test image:
     * 2x2 pixels
     * (0, 0) blue
     * (1, 0) green
     * (0, 1) red
     * (1, 1) white
     * 24 bpp, no compression
     * No color palette
     */
    TEST_ASSERT_EQUAL(BmpImgLoader::RET_OK, loader.load(localFileSystem, "./test/test24bpp.bmp", bitmap));
    TEST_ASSERT_EQUAL_UINT16(2, bitmap.getWidth());
    TEST_ASSERT_EQUAL_UINT16(2, bitmap.getHeight());
    TEST_ASSERT_EQUAL_UINT32(0x0000ff, bitmap.getColor(0, 0));
    TEST_ASSERT_EQUAL_UINT32(0x00ff00, bitmap.getColor(1, 0));
    TEST_ASSERT_EQUAL_UINT32(0xff0000, bitmap.getColor(0, 1));
    TEST_ASSERT_EQUAL_UINT32(0xffffff, bitmap.getColor(1, 1));

    /* Load test image:
     * 2x2 pixels
     * (0, 0) blue
     * (1, 0) green
     * (0, 1) red
     * (1, 1) white
     * 32 bpp, bitfield (not supported)
     * No color palette
     */
    TEST_ASSERT_EQUAL(BmpImgLoader::RET_FILE_FORMAT_UNSUPPORTED, loader.load(localFileSystem, "./test/test32bpp.bmp", bitmap));
    TEST_ASSERT_FALSE(bitmap.isAllocated());
    TEST_ASSERT_EQUAL_UINT16(0, bitmap.getWidth());
    TEST_ASSERT_EQUAL_UINT16(0, bitmap.getHeight());

    /* Load valid bitmap file. */
    TEST_ASSERT_EQUAL(BmpImgLoader::RET_OK, loader.load(localFileSystem, "./test/test24bpp.bmp", bitmap));

    return;
}
