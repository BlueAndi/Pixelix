/* MIT License
 *
 * Copyright (c) 2019 - 2026 Andreas Merkle <web@blue-andi.de>
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
 * @file   TestBitmapWidget.cpp
 * @brief  Test bitmap widget.
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <unity.h>
#include <stdio.h>
#include <BitmapWidget.h>
#include <Util.h>
#include <FS.h>

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
static void testBitmapWidgetSupportedTypes();
static void testBitmapWidgetLoadPng();
static void testBitmapWidgetLoadWebp();
static void testBitmapWidgetSwitchFormats();

static void verifyDrawnImage(YAGfxTest& testGfx, const uint32_t* expectedData, uint16_t expectedWidth, uint16_t expectedHeight);

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/** BMP test image fixture, a 2 x 2 image. */
static const char* FILE_NAME_BMP           = "./test/test_BmpImgLoader/test24bpp.bmp";

/** GIF test image fixture, a 10 x 10 image. */
static const char* FILE_NAME_GIF           = "./test/test_GifImgPlayer/TestStatic.gif";

/** PNG test image fixture, a lossless conversion of the GIF fixture. */
static const char* FILE_NAME_PNG           = "./test/test_PngImgLoader/TestStatic.png";

/** WebP test image fixture, a lossless conversion of the GIF fixture. */
static const char* FILE_NAME_WEBP          = "./test/test_WebpImgPlayer/TestStatic.webp";

/** Image file name of a format which Pixelix never supports. */
static const char* FILE_NAME_JPG           = "./test/test_BitmapWidget/TestUnsupported.jpg";

/** Expected image width of the GIF, PNG and WebP fixtures. */
static const uint16_t EXPECTED_DATA_WIDTH  = 10U;

/** Expected image height of the GIF, PNG and WebP fixtures. */
static const uint16_t EXPECTED_DATA_HEIGHT = 10U;

/** Expected image width of the BMP fixture. */
static const uint16_t EXPECTED_BMP_WIDTH   = 2U;

/** Expected image height of the BMP fixture. */
static const uint16_t EXPECTED_BMP_HEIGHT  = 2U;

/**
 * Expected image data of the GIF, PNG and WebP fixtures.
 * Its the very same array as in test_GifImgPlayer/TestGifImgPlayer.cpp, because
 * the PNG and the WebP fixture are lossless conversions of the GIF fixture.
 */
/* clang-format off */
static const uint32_t EXPECTED_DATA[] = {
    /*          0          1          2          3          4          5          6          7          8          9 */
    /* 0 */ 0xFF0000U, 0xFF0000U, 0xFF0000U, 0xFF0000U, 0xFF0000U, 0x0000FFU, 0x0000FFU, 0x0000FFU, 0x0000FFU, 0x0000FFU,
    /* 1 */ 0xFF0000U, 0xFF0000U, 0xFF0000U, 0xFF0000U, 0xFF0000U, 0x0000FFU, 0x0000FFU, 0x0000FFU, 0x0000FFU, 0x0000FFU,
    /* 2 */ 0xFF0000U, 0xFF0000U, 0xFF0000U, 0xFF0000U, 0xFF0000U, 0x0000FFU, 0x0000FFU, 0x0000FFU, 0x0000FFU, 0x0000FFU,
    /* 3 */ 0xFF0000U, 0xFF0000U, 0xFF0000U, 0xFFFFFFU, 0xFFFFFFU, 0xFFFFFFU, 0xFFFFFFU, 0x0000FFU, 0x0000FFU, 0x0000FFU,
    /* 4 */ 0xFF0000U, 0xFF0000U, 0xFF0000U, 0xFFFFFFU, 0xFFFFFFU, 0xFFFFFFU, 0xFFFFFFU, 0x0000FFU, 0x0000FFU, 0x0000FFU,
    /* 5 */ 0x0000FFU, 0x0000FFU, 0x0000FFU, 0xFFFFFFU, 0xFFFFFFU, 0xFFFFFFU, 0xFFFFFFU, 0xFF0000U, 0xFF0000U, 0xFF0000U,
    /* 6 */ 0x0000FFU, 0x0000FFU, 0x0000FFU, 0xFFFFFFU, 0xFFFFFFU, 0xFFFFFFU, 0xFFFFFFU, 0xFF0000U, 0xFF0000U, 0xFF0000U,
    /* 7 */ 0x0000FFU, 0x0000FFU, 0x0000FFU, 0x0000FFU, 0x0000FFU, 0xFF0000U, 0xFF0000U, 0xFF0000U, 0xFF0000U, 0xFF0000U,
    /* 8 */ 0x0000FFU, 0x0000FFU, 0x0000FFU, 0x0000FFU, 0x0000FFU, 0xFF0000U, 0xFF0000U, 0xFF0000U, 0xFF0000U, 0xFF0000U,
    /* 9 */ 0x0000FFU, 0x0000FFU, 0x0000FFU, 0x0000FFU, 0x0000FFU, 0xFF0000U, 0xFF0000U, 0xFF0000U, 0xFF0000U, 0xFF0000U
};
/* clang-format on */

/** Expected image data of the BMP fixture. */
/* clang-format off */
static const uint32_t EXPECTED_BMP_DATA[] = {
    /*          0          1 */
    /* 0 */ 0x0000FFU, 0x00FF00U,
    /* 1 */ 0xFF0000U, 0xFFFFFFU
};
/* clang-format on */

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
    RUN_TEST(testBitmapWidgetSupportedTypes);
    RUN_TEST(testBitmapWidgetLoadPng);
    RUN_TEST(testBitmapWidgetLoadWebp);
    RUN_TEST(testBitmapWidgetSwitchFormats);

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
    const uint16_t  BITMAP_WIDTH    = YAGfxTest::HEIGHT;    /* Use height as width here for a square. */
    const uint16_t  BITMAP_HEIGHT   = YAGfxTest::HEIGHT;
    const char*     WIDGET_NAME     = "bmpWidgetName";

    YAGfxTest                                       testGfx;
    BitmapWidget                                    bitmapWidget(BITMAP_WIDTH, BITMAP_HEIGHT);
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
            const Color&  bitmapColor = bitmap.getColor(x, y);
            const Color&  bitmapWidgetColor = bitmapWidget.get().getColor(x, y);

            TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(bitmapColor), static_cast<uint32_t>(bitmapWidgetColor));
        }
    }

    /* Draw bitmap and verify */
    bitmapWidget.update(testGfx);
    displayBuffer = testGfx.getBuffer();

    for(y = 0; y < BITMAP_HEIGHT; ++y)
    {
        for(x = 0; x < BITMAP_WIDTH; ++x)
        {
            uint32_t    bitmapColor         = x + y * BITMAP_WIDTH;
            Color*      displayBufferColor  = &displayBuffer[x + y * YAGfxTest::WIDTH];

            TEST_ASSERT_EQUAL_UINT16(bitmapColor, static_cast<uint32_t>(*displayBufferColor));
        }
    }
}

/**
 * Test which image types the bitmap widget reports as supported.
 *
 * BMP and GIF are always supported. PNG and WebP only if the corresponding
 * build switch is enabled. A format which Pixelix never supports is rejected
 * in every configuration.
 */
static void testBitmapWidgetSupportedTypes()
{
    size_t expectedCount = 2U; /* BMP and GIF are always supported. */

    TEST_ASSERT_TRUE(BitmapWidget::isImageTypeSupported(FILE_NAME_BMP));
    TEST_ASSERT_TRUE(BitmapWidget::isImageTypeSupported(FILE_NAME_GIF));
    TEST_ASSERT_FALSE(BitmapWidget::isImageTypeSupported(FILE_NAME_JPG));

#if (0 != CONFIG_IMAGE_FORMAT_PNG_ENABLE)

    TEST_ASSERT_TRUE(BitmapWidget::isImageTypeSupported(FILE_NAME_PNG));
    ++expectedCount;

#else /* (0 != CONFIG_IMAGE_FORMAT_PNG_ENABLE) */

    TEST_ASSERT_FALSE(BitmapWidget::isImageTypeSupported(FILE_NAME_PNG));

#endif /* (0 != CONFIG_IMAGE_FORMAT_PNG_ENABLE) */

#if (0 != CONFIG_IMAGE_FORMAT_WEBP_ENABLE)

    TEST_ASSERT_TRUE(BitmapWidget::isImageTypeSupported(FILE_NAME_WEBP));
    ++expectedCount;

#else /* (0 != CONFIG_IMAGE_FORMAT_WEBP_ENABLE) */

    TEST_ASSERT_FALSE(BitmapWidget::isImageTypeSupported(FILE_NAME_WEBP));

#endif /* (0 != CONFIG_IMAGE_FORMAT_WEBP_ENABLE) */

    TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(expectedCount), static_cast<uint32_t>(BitmapWidget::IMAGE_FILE_EXTENSIONS_COUNT));
}

/**
 * Test loading a PNG image into the bitmap widget.
 *
 * If the PNG support is disabled, the widget must refuse the file and stay
 * empty.
 */
static void testBitmapWidgetLoadPng()
{
    YAGfxTest    testGfx;
    BitmapWidget bitmapWidget(YAGfxTest::WIDTH, YAGfxTest::HEIGHT);
    FS           fileSystem;

    TEST_ASSERT_TRUE(bitmapWidget.isEmpty());

#if (0 != CONFIG_IMAGE_FORMAT_PNG_ENABLE)

    TEST_ASSERT_TRUE(bitmapWidget.load(FILE_NAME_PNG, fileSystem));
    TEST_ASSERT_FALSE(bitmapWidget.isEmpty());

    /* A PNG is decoded into the widget own bitmap. */
    TEST_ASSERT_EQUAL_UINT16(EXPECTED_DATA_WIDTH, bitmapWidget.get().getWidth());
    TEST_ASSERT_EQUAL_UINT16(EXPECTED_DATA_HEIGHT, bitmapWidget.get().getHeight());

    testGfx.fillScreen(ColorDef::BLACK);
    bitmapWidget.update(testGfx);
    verifyDrawnImage(testGfx, EXPECTED_DATA, EXPECTED_DATA_WIDTH, EXPECTED_DATA_HEIGHT);

#else /* (0 != CONFIG_IMAGE_FORMAT_PNG_ENABLE) */

    TEST_ASSERT_FALSE(bitmapWidget.load(FILE_NAME_PNG, fileSystem));
    TEST_ASSERT_TRUE(bitmapWidget.isEmpty());

#endif /* (0 != CONFIG_IMAGE_FORMAT_PNG_ENABLE) */
}

/**
 * Test loading a WebP image into the bitmap widget.
 *
 * If the WebP support is disabled, the widget must refuse the file and stay
 * empty.
 */
static void testBitmapWidgetLoadWebp()
{
    YAGfxTest    testGfx;
    BitmapWidget bitmapWidget(YAGfxTest::WIDTH, YAGfxTest::HEIGHT);
    FS           fileSystem;

    TEST_ASSERT_TRUE(bitmapWidget.isEmpty());

#if (0 != CONFIG_IMAGE_FORMAT_WEBP_ENABLE)

    TEST_ASSERT_TRUE(bitmapWidget.load(FILE_NAME_WEBP, fileSystem));
    TEST_ASSERT_FALSE(bitmapWidget.isEmpty());

    /* A WebP is kept in its own player, therefore the widget own bitmap stays
     * released.
     */
    TEST_ASSERT_EQUAL_UINT16(0U, bitmapWidget.get().getWidth());
    TEST_ASSERT_EQUAL_UINT16(0U, bitmapWidget.get().getHeight());

    testGfx.fillScreen(ColorDef::BLACK);
    bitmapWidget.update(testGfx);
    verifyDrawnImage(testGfx, EXPECTED_DATA, EXPECTED_DATA_WIDTH, EXPECTED_DATA_HEIGHT);

#else /* (0 != CONFIG_IMAGE_FORMAT_WEBP_ENABLE) */

    TEST_ASSERT_FALSE(bitmapWidget.load(FILE_NAME_WEBP, fileSystem));
    TEST_ASSERT_TRUE(bitmapWidget.isEmpty());

#endif /* (0 != CONFIG_IMAGE_FORMAT_WEBP_ENABLE) */
}

/**
 * Test switching the image format on one and the same bitmap widget.
 *
 * Every format must draw its own image, nothing of the previously shown image
 * may be left on the display and the widget own bitmap must be released
 * whenever the new format does not use it.
 *
 * Note: What this test can **not** see is whether GifImgPlayer::close() and
 *       WebpImgPlayer::close() really free their internals. BitmapWidget::paint()
 *       dispatches on the current image type, so the previous player is never
 *       asked to play again and a leak inside it would stay invisible here.
 *       Neither player exposes its open or allocation state, therefore the
 *       available test surface ends at the widget own bitmap and at the pixels
 *       on the display.
 */
static void testBitmapWidgetSwitchFormats()
{
    YAGfxTest    testGfx;
    BitmapWidget bitmapWidget(YAGfxTest::WIDTH, YAGfxTest::HEIGHT);
    FS           fileSystem;

    /* GIF image, kept in the GIF player. */
    TEST_ASSERT_TRUE(bitmapWidget.load(FILE_NAME_GIF, fileSystem));
    TEST_ASSERT_FALSE(bitmapWidget.isEmpty());
    TEST_ASSERT_EQUAL_UINT16(0U, bitmapWidget.get().getWidth());

    testGfx.fillScreen(ColorDef::BLACK);
    bitmapWidget.update(testGfx);
    verifyDrawnImage(testGfx, EXPECTED_DATA, EXPECTED_DATA_WIDTH, EXPECTED_DATA_HEIGHT);

#if (0 != CONFIG_IMAGE_FORMAT_WEBP_ENABLE)

    /* WebP image, kept in the WebP player. The widget own bitmap stays
     * released.
     */
    TEST_ASSERT_TRUE(bitmapWidget.load(FILE_NAME_WEBP, fileSystem));
    TEST_ASSERT_FALSE(bitmapWidget.isEmpty());
    TEST_ASSERT_EQUAL_UINT16(0U, bitmapWidget.get().getWidth());

    testGfx.fillScreen(ColorDef::BLACK);
    bitmapWidget.update(testGfx);
    verifyDrawnImage(testGfx, EXPECTED_DATA, EXPECTED_DATA_WIDTH, EXPECTED_DATA_HEIGHT);

#endif /* (0 != CONFIG_IMAGE_FORMAT_WEBP_ENABLE) */

#if (0 != CONFIG_IMAGE_FORMAT_PNG_ENABLE)

    /* PNG image, decoded into the widget own bitmap. */
    TEST_ASSERT_TRUE(bitmapWidget.load(FILE_NAME_PNG, fileSystem));
    TEST_ASSERT_FALSE(bitmapWidget.isEmpty());
    TEST_ASSERT_EQUAL_UINT16(EXPECTED_DATA_WIDTH, bitmapWidget.get().getWidth());

    testGfx.fillScreen(ColorDef::BLACK);
    bitmapWidget.update(testGfx);
    verifyDrawnImage(testGfx, EXPECTED_DATA, EXPECTED_DATA_WIDTH, EXPECTED_DATA_HEIGHT);

#endif /* (0 != CONFIG_IMAGE_FORMAT_PNG_ENABLE) */

    /* BMP image, a different image with a different size. Nothing of the
     * previously shown image may be left on the display.
     */
    TEST_ASSERT_TRUE(bitmapWidget.load(FILE_NAME_BMP, fileSystem));
    TEST_ASSERT_FALSE(bitmapWidget.isEmpty());
    TEST_ASSERT_EQUAL_UINT16(EXPECTED_BMP_WIDTH, bitmapWidget.get().getWidth());

    testGfx.fillScreen(ColorDef::BLACK);
    bitmapWidget.update(testGfx);
    verifyDrawnImage(testGfx, EXPECTED_BMP_DATA, EXPECTED_BMP_WIDTH, EXPECTED_BMP_HEIGHT);

    /* Clearing the widget releases the last used decoder as well. */
    bitmapWidget.clear(ColorDef::BLACK);
    TEST_ASSERT_TRUE(bitmapWidget.isEmpty());
    TEST_ASSERT_EQUAL_UINT16(0U, bitmapWidget.get().getWidth());
}

/**
 * Verify the image which the bitmap widget has drawn.
 *
 * Outside of the image area the display must be black, therefore a previously
 * shown image which was not released would be detected.
 *
 * @param[in] testGfx           Graphics interface which the widget has drawn to.
 * @param[in] expectedData      Expected image data, one 32 bit color per pixel.
 * @param[in] expectedWidth     Expected image width in pixels.
 * @param[in] expectedHeight    Expected image height in pixels.
 */
static void verifyDrawnImage(YAGfxTest& testGfx, const uint32_t* expectedData, uint16_t expectedWidth, uint16_t expectedHeight)
{
    int16_t x = 0;
    int16_t y = 0;

    for (y = 0; y < static_cast<int16_t>(YAGfxTest::HEIGHT); ++y)
    {
        for (x = 0; x < static_cast<int16_t>(YAGfxTest::WIDTH); ++x)
        {
            uint32_t expectedColor = 0U; /* Black outside of the image area. */

            if ((x < static_cast<int16_t>(expectedWidth)) &&
                (y < static_cast<int16_t>(expectedHeight)))
            {
                expectedColor = expectedData[x + y * static_cast<int16_t>(expectedWidth)];
            }

            if (expectedColor != static_cast<uint32_t>(testGfx.getColor(x, y)))
            {
                printf("%d, %d / 0x%X <-> 0x%X\n", x, y, expectedColor, static_cast<uint32_t>(testGfx.getColor(x, y)));
            }

            TEST_ASSERT_EQUAL_UINT32(expectedColor, static_cast<uint32_t>(testGfx.getColor(x, y)));
        }
    }
}
