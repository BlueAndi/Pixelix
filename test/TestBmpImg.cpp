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
 * @brief  Test bitmap image loader.
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "TestBmpImg.h"
#include "TestGfx.h"

#include <unity.h>
#include <FS.h>
#include <BmpImg.h>

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
 * Test bitmap image loader.
 */
extern void testBmpImg()
{
    BmpImg  image;
    FS      localFileSystem;

    /* If no image is loaded, width and height shall be 0. */
    TEST_ASSERT_EQUAL_UINT16(0, image.getWidth());
    TEST_ASSERT_EQUAL_UINT16(0, image.getHeight());

    /* Load test image:
     * 2x2 pixels
     * (0, 0) blue
     * (1, 0) green
     * (0, 1) red
     * (1, 1) white
     * 24 bpp, no compression
     * No color palette
     */
    TEST_ASSERT_EQUAL(BmpImg::RET_OK, image.load(localFileSystem, "./test/test24bpp.bmp"));
    TEST_ASSERT_EQUAL_UINT16(2, image.getWidth());
    TEST_ASSERT_EQUAL_UINT16(2, image.getHeight());
    TEST_ASSERT_NOT_EQUAL(nullptr, image.get(0, 0));
    TEST_ASSERT_EQUAL_UINT32(0x0000ff, *image.get(0, 0));
    TEST_ASSERT_NOT_EQUAL(nullptr, image.get(1, 0));
    TEST_ASSERT_EQUAL_UINT32(0x00ff00, *image.get(1, 0));
    TEST_ASSERT_NOT_EQUAL(nullptr, image.get(0, 1));
    TEST_ASSERT_EQUAL_UINT32(0xff0000, *image.get(0, 1));
    TEST_ASSERT_NOT_EQUAL(nullptr, image.get(1, 1));
    TEST_ASSERT_EQUAL_UINT32(0xffffff, *image.get(1, 1));

    /* Load test image:
     * 2x2 pixels
     * (0, 0) blue
     * (1, 0) green
     * (0, 1) red
     * (1, 1) white
     * 32 bpp, bitfield (not supported)
     * No color palette
     */
    TEST_ASSERT_EQUAL(BmpImg::RET_FILE_FORMAT_UNSUPPORTED, image.load(localFileSystem, "./test/test32bpp.bmp"));
    TEST_ASSERT_EQUAL(nullptr, image.get());
    TEST_ASSERT_EQUAL_UINT16(0, image.getWidth());
    TEST_ASSERT_EQUAL_UINT16(0, image.getHeight());

    /* Load valid bitmap file. */
    TEST_ASSERT_EQUAL(BmpImg::RET_OK, image.load(localFileSystem, "./test/test24bpp.bmp"));

    /* Construct bitmap image by copy. */
    {
        BmpImg  newImage(image);

        TEST_ASSERT_EQUAL_UINT16(2, newImage.getWidth());
        TEST_ASSERT_EQUAL_UINT16(2, newImage.getHeight());
        TEST_ASSERT_NOT_EQUAL(nullptr, newImage.get(0, 0));
        TEST_ASSERT_EQUAL_UINT32(0x0000ff, *newImage.get(0, 0));
        TEST_ASSERT_NOT_EQUAL(nullptr, newImage.get(1, 0));
        TEST_ASSERT_EQUAL_UINT32(0x00ff00, *newImage.get(1, 0));
        TEST_ASSERT_NOT_EQUAL(nullptr, newImage.get(0, 1));
        TEST_ASSERT_EQUAL_UINT32(0xff0000, *newImage.get(0, 1));
        TEST_ASSERT_NOT_EQUAL(nullptr, newImage.get(1, 1));
        TEST_ASSERT_EQUAL_UINT32(0xffffff, *newImage.get(1, 1));
    }

    /* Assign bitmap image. */
    {
        BmpImg  newImage;

        newImage = image;

        TEST_ASSERT_EQUAL_UINT16(2, newImage.getWidth());
        TEST_ASSERT_EQUAL_UINT16(2, newImage.getHeight());
        TEST_ASSERT_NOT_EQUAL(nullptr, newImage.get(0, 0));
        TEST_ASSERT_EQUAL_UINT32(0x0000ff, *newImage.get(0, 0));
        TEST_ASSERT_NOT_EQUAL(nullptr, newImage.get(1, 0));
        TEST_ASSERT_EQUAL_UINT32(0x00ff00, *newImage.get(1, 0));
        TEST_ASSERT_NOT_EQUAL(nullptr, newImage.get(0, 1));
        TEST_ASSERT_EQUAL_UINT32(0xff0000, *newImage.get(0, 1));
        TEST_ASSERT_NOT_EQUAL(nullptr, newImage.get(1, 1));
        TEST_ASSERT_EQUAL_UINT32(0xffffff, *newImage.get(1, 1));
    }

    return;
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/
