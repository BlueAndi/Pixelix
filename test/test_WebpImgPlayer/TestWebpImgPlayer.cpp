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
 * @file   TestWebpImgPlayer.cpp
 * @brief  Test WebP image player.
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <unity.h>
#include <stdio.h>
#include <Arduino.h>
#include <WebpImgPlayer.h>
#include <YAGfxCanvas.h>
#include <Util.h>
#include <FS.h>

#include "../common/YAGfxTest.hpp"

/* The heap usage probe is host specific. Every allocation of the player as
 * well as every allocation of libwebp ends up in the C runtime heap, therefore
 * the C runtime is asked how much of it is in use.
 */
#if defined(_WIN32)
#include <malloc.h>
#define HEAP_USAGE_MEASURABLE (1)
#elif defined(__GLIBC__)
#include <malloc.h>
#define HEAP_USAGE_MEASURABLE (1)
#if defined(__GLIBC_PREREQ)
#if __GLIBC_PREREQ(2, 33)
#define HEAP_USAGE_MALLINFO2  (1)
#endif
#endif
#else
#define HEAP_USAGE_MEASURABLE (0)
#endif

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

static void testWebpImgPlayerStatic();
static void testWebpImgPlayerAnimated();
static void testWebpImgPlayerNotFound();
static void testWebpImgPlayerAlreadyOpen();
static void testWebpImgPlayerTruncated();
static void testWebpImgPlayerNotAWebp();
static void   testWebpImgPlayerCanvasTooBig();
static void   testWebpImgPlayerAlpha();
static void   testWebpImgPlayerLoopCount();
static void   testWebpImgPlayerInfiniteLoop();
static void   testWebpImgPlayerReopenLoop();
static bool   copyFilePart(const char* srcFileName, const char* dstFileName, size_t maxSize);
static bool   writeFile(const char* fileName, const uint8_t* data, size_t size);
static bool   writeVp8xHeaderFile(const char* fileName, uint32_t width, uint32_t height);
static size_t getHeapUsage();

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/** Static WebP test image, a lossless conversion of test_GifImgPlayer/TestStatic.gif. */
static const char* FILE_NAME_STATIC    = "./test/test_WebpImgPlayer/TestStatic.webp";

/** Animated WebP test image, a lossless conversion of test_GifImgPlayer/TestAnimation.gif. */
static const char* FILE_NAME_ANIMATION = "./test/test_WebpImgPlayer/TestAnimation.webp";

/** Animated WebP test image with 3 frames of 40 ms and a loop count of 2. */
static const char* FILE_NAME_LOOP_COUNT = "./test/test_WebpImgPlayer/TestLoopCount.webp";

/** Lossless WebP test image with alpha channel. Left half opaque red, right half alpha 0. */
static const char* FILE_NAME_ALPHA     = "./test/test_WebpImgPlayer/TestAlpha.webp";

/** Temporary file, which gets a hand written WebP header with a huge canvas. */
static const char* FILE_NAME_HUGE_CANVAS = "./test/test_WebpImgPlayer/HugeCanvas.tmp.webp";

/** Temporary file, which gets a truncated copy of the static WebP test image. */
static const char* FILE_NAME_TRUNCATED = "./test/test_WebpImgPlayer/Truncated.tmp.webp";

/** A GIF file, used to verify that a non-WebP file is rejected. */
static const char* FILE_NAME_NO_WEBP   = "./test/test_GifImgPlayer/TestStatic.gif";

/** Expected image width of the static test image. */
static const uint32_t EXPECTED_DATA_WIDTH     = 10U;

/** Expected image height of the static test image. */
static const uint32_t EXPECTED_DATA_HEIGHT    = 10U;

/** Expected image width of the animated test image. */
static const int16_t  EXPECTED_ANIMATION_WIDTH  = 11;

/** Expected image height of the animated test image. */
static const int16_t  EXPECTED_ANIMATION_HEIGHT = 29;

/**
 * Expected image data of the static test image.
 * Its the very same array as in test_GifImgPlayer/TestGifImgPlayer.cpp, because
 * the WebP fixture is a lossless conversion of the GIF fixture.
 */
static const uint32_t EXPECTED_DATA[]      = {
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
extern int main(int argc, char** argv)
{
    UTIL_NOT_USED(argc);
    UTIL_NOT_USED(argv);

    UNITY_BEGIN();

    RUN_TEST(testWebpImgPlayerStatic);
    RUN_TEST(testWebpImgPlayerAnimated);
    RUN_TEST(testWebpImgPlayerNotFound);
    RUN_TEST(testWebpImgPlayerAlreadyOpen);
    RUN_TEST(testWebpImgPlayerTruncated);
    RUN_TEST(testWebpImgPlayerNotAWebp);
    RUN_TEST(testWebpImgPlayerCanvasTooBig);
    RUN_TEST(testWebpImgPlayerAlpha);
    RUN_TEST(testWebpImgPlayerLoopCount);
    RUN_TEST(testWebpImgPlayerInfiniteLoop);
    RUN_TEST(testWebpImgPlayerReopenLoop);

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
 * Test WebP image player with a static WebP image.
 */
static void testWebpImgPlayerStatic()
{
    WebpImgPlayer webpImgPlayer;
    YAGfxTest     testGfx;
    YAGfxCanvas   canvas(&testGfx, 0, 0, YAGfxTest::WIDTH, YAGfxTest::HEIGHT);
    FS            fileSystem;
    int32_t       x;
    int32_t       y;
    int32_t       width  = (YAGfxTest::WIDTH > EXPECTED_DATA_WIDTH) ? EXPECTED_DATA_WIDTH : YAGfxTest::WIDTH;
    int32_t       height = (YAGfxTest::HEIGHT > EXPECTED_DATA_HEIGHT) ? EXPECTED_DATA_HEIGHT : YAGfxTest::HEIGHT;

    TEST_ASSERT_EQUAL(WebpImgPlayer::RET_OK, webpImgPlayer.open(fileSystem, FILE_NAME_STATIC));
    TEST_ASSERT_EQUAL_INT16(static_cast<int16_t>(EXPECTED_DATA_WIDTH), webpImgPlayer.getWidth());
    TEST_ASSERT_EQUAL_INT16(static_cast<int16_t>(EXPECTED_DATA_HEIGHT), webpImgPlayer.getHeight());
    TEST_ASSERT_EQUAL(false, webpImgPlayer.isAnimation());
    TEST_ASSERT_EQUAL(true, webpImgPlayer.play(canvas));

    for (y = 0; y < height; ++y)
    {
        for (x = 0; x < width; ++x)
        {
            Color& color = testGfx.getColor(x, y);

            printf("%u, %u / 0x%X <-> 0x%X\n", x, y, EXPECTED_DATA[x + y * EXPECTED_DATA_WIDTH], static_cast<uint32_t>(color));

            TEST_ASSERT_EQUAL_UINT32(EXPECTED_DATA[x + y * EXPECTED_DATA_WIDTH], static_cast<uint32_t>(color));
        }
    }

    /* Verify that a 2nd play() call will redraw the image. */
    testGfx.fillScreen(ColorDef::BLACK);
    TEST_ASSERT_EQUAL(true, webpImgPlayer.play(canvas));
    webpImgPlayer.close();

    for (y = 0; y < height; ++y)
    {
        for (x = 0; x < width; ++x)
        {
            Color& color = testGfx.getColor(x, y);

            TEST_ASSERT_EQUAL_UINT32(EXPECTED_DATA[x + y * EXPECTED_DATA_WIDTH], static_cast<uint32_t>(color));
        }
    }

    /* After close() the player shall be reset. */
    TEST_ASSERT_EQUAL_INT16(0, webpImgPlayer.getWidth());
    TEST_ASSERT_EQUAL_INT16(0, webpImgPlayer.getHeight());
}

/**
 * Test WebP image player with an animated WebP image.
 */
static void testWebpImgPlayerAnimated()
{
    WebpImgPlayer  webpImgPlayer;
    YAGfxTest      testGfx;
    YAGfxCanvas    canvas(&testGfx, 0, 0, YAGfxTest::WIDTH, YAGfxTest::HEIGHT);
    FS             fileSystem;
    uint32_t       playCount   = 0U;
    uint32_t       startTime   = 0U;
    uint32_t       elapsedTime = 0U;

    /* Upper bound in ms, which protects against a never finishing animation. */
    const uint32_t TIMEOUT     = 30000U;

    /* The animation has 3 frames with 1000 ms, 500 ms and 1000 ms duration.
     * A complete cycle therefore takes at least 2500 ms. Some slack is
     * considered for the ms resolution of the timer.
     */
    const uint32_t CYCLE_TIME_MIN = 2400U;

    TEST_ASSERT_EQUAL(WebpImgPlayer::RET_OK, webpImgPlayer.open(fileSystem, FILE_NAME_ANIMATION));
    TEST_ASSERT_EQUAL(true, webpImgPlayer.isAnimation());
    TEST_ASSERT_EQUAL_INT16(EXPECTED_ANIMATION_WIDTH, webpImgPlayer.getWidth());
    TEST_ASSERT_EQUAL_INT16(EXPECTED_ANIMATION_HEIGHT, webpImgPlayer.getHeight());
    TEST_ASSERT_EQUAL(false, webpImgPlayer.isAnimationFinished());

    startTime = millis();

    while ((false == webpImgPlayer.isAnimationFinished()) && (TIMEOUT > elapsedTime))
    {
        TEST_ASSERT_EQUAL(true, webpImgPlayer.play(canvas));
        ++playCount;
        delay(1U);
        elapsedTime = millis() - startTime;
    }

    printf("Animation cycle: %u play() calls in %u ms\n", playCount, elapsedTime);

    TEST_ASSERT_EQUAL(true, webpImgPlayer.isAnimationFinished());
    TEST_ASSERT_GREATER_THAN_UINT32(0U, playCount);

    /* The frame durations of the WebP stream shall be honoured. */
    TEST_ASSERT_GREATER_OR_EQUAL_UINT32(CYCLE_TIME_MIN, elapsedTime);

    webpImgPlayer.close();
}

/**
 * Test WebP image player with a not existing file.
 */
static void testWebpImgPlayerNotFound()
{
    WebpImgPlayer webpImgPlayer;
    YAGfxTest     testGfx;
    YAGfxCanvas   canvas(&testGfx, 0, 0, YAGfxTest::WIDTH, YAGfxTest::HEIGHT);
    FS            fileSystem;

    TEST_ASSERT_EQUAL(WebpImgPlayer::RET_FILE_NOT_FOUND, webpImgPlayer.open(fileSystem, "./test/test_WebpImgPlayer/DoesNotExist.webp"));
    TEST_ASSERT_EQUAL_INT16(0, webpImgPlayer.getWidth());
    TEST_ASSERT_EQUAL_INT16(0, webpImgPlayer.getHeight());
    TEST_ASSERT_EQUAL(false, webpImgPlayer.play(canvas));

    webpImgPlayer.close();
}

/**
 * Test WebP image player with a 2nd open() without a close() in between.
 */
static void testWebpImgPlayerAlreadyOpen()
{
    WebpImgPlayer webpImgPlayer;
    FS            fileSystem;

    TEST_ASSERT_EQUAL(WebpImgPlayer::RET_OK, webpImgPlayer.open(fileSystem, FILE_NAME_STATIC));
    TEST_ASSERT_EQUAL(WebpImgPlayer::RET_FILE_ALREADY_OPENED, webpImgPlayer.open(fileSystem, FILE_NAME_STATIC));

    /* The already opened image shall not be destroyed by the rejected open(). */
    TEST_ASSERT_EQUAL_INT16(static_cast<int16_t>(EXPECTED_DATA_WIDTH), webpImgPlayer.getWidth());

    webpImgPlayer.close();

    /* After close() the file can be opened again. */
    TEST_ASSERT_EQUAL(WebpImgPlayer::RET_OK, webpImgPlayer.open(fileSystem, FILE_NAME_STATIC));

    webpImgPlayer.close();
}

/**
 * Test WebP image player with a truncated WebP file.
 */
static void testWebpImgPlayerTruncated()
{
    WebpImgPlayer webpImgPlayer;
    YAGfxTest     testGfx;
    YAGfxCanvas   canvas(&testGfx, 0, 0, YAGfxTest::WIDTH, YAGfxTest::HEIGHT);
    FS            fileSystem;
    size_t        fullSize = 0U;
    FILE*         fd       = fopen(FILE_NAME_STATIC, "rb");

    TEST_ASSERT_NOT_NULL(fd);
    (void)fseek(fd, 0, SEEK_END);
    fullSize = static_cast<size_t>(ftell(fd));
    (void)fclose(fd);

    TEST_ASSERT_GREATER_THAN_UINT32(0U, fullSize);

    /* Cut the file to the half of its length. */
    TEST_ASSERT_EQUAL(true, copyFilePart(FILE_NAME_STATIC, FILE_NAME_TRUNCATED, fullSize / 2U));

    TEST_ASSERT_NOT_EQUAL(WebpImgPlayer::RET_OK, webpImgPlayer.open(fileSystem, FILE_NAME_TRUNCATED));
    TEST_ASSERT_EQUAL_INT16(0, webpImgPlayer.getWidth());
    TEST_ASSERT_EQUAL_INT16(0, webpImgPlayer.getHeight());
    TEST_ASSERT_EQUAL(false, webpImgPlayer.isAnimation());
    TEST_ASSERT_EQUAL(false, webpImgPlayer.play(canvas));

    webpImgPlayer.close();

    (void)remove(FILE_NAME_TRUNCATED);
}

/**
 * Test WebP image player with a file which is no WebP file at all.
 */
static void testWebpImgPlayerNotAWebp()
{
    WebpImgPlayer webpImgPlayer;
    FS            fileSystem;

    TEST_ASSERT_EQUAL(WebpImgPlayer::RET_FILE_FORMAT_INVALID, webpImgPlayer.open(fileSystem, FILE_NAME_NO_WEBP));
    TEST_ASSERT_EQUAL_INT16(0, webpImgPlayer.getWidth());
    TEST_ASSERT_EQUAL_INT16(0, webpImgPlayer.getHeight());

    webpImgPlayer.close();
}

/**
 * Test WebP image player with a WebP file which declares a canvas larger than
 * the 16-bit bitmap dimensions can represent.
 *
 * WebP's VP8X chunk carries the canvas size in two 24-bit fields, so a 30 byte
 * file can claim a canvas of millions of pixels. The dimensions used here are
 * chosen so that a narrowing to int16_t would wrap to a harmless looking 10,
 * which is exactly the silent failure the representability guard prevents:
 * without it a 10 pixel wide bitmap would be created and reported as success
 * for a 65546 pixel wide image.
 */
static void testWebpImgPlayerCanvasTooBig()
{
    WebpImgPlayer  webpImgPlayer;
    YAGfxTest      testGfx;
    YAGfxCanvas    canvas(&testGfx, 0, 0, YAGfxTest::WIDTH, YAGfxTest::HEIGHT);
    FS             fileSystem;

    /* 65546 & 0xFFFF == 10. The other dimension stays small, so that the
     * canvas area keeps below libwebp's own MAX_IMAGE_AREA limit and libwebp
     * accepts the header. The player's guard is then the thing which rejects
     * it.
     */
    const uint32_t DIM_TOO_BIG = 65546U;
    const uint32_t DIM_SMALL   = 4U;

    /* Too wide. */
    TEST_ASSERT_EQUAL(true, writeVp8xHeaderFile(FILE_NAME_HUGE_CANVAS, DIM_TOO_BIG, DIM_SMALL));
    TEST_ASSERT_EQUAL(WebpImgPlayer::RET_IMG_TOO_BIG, webpImgPlayer.open(fileSystem, FILE_NAME_HUGE_CANVAS));

    /* The player shall be left closed. */
    TEST_ASSERT_EQUAL_INT16(0, webpImgPlayer.getWidth());
    TEST_ASSERT_EQUAL_INT16(0, webpImgPlayer.getHeight());
    TEST_ASSERT_EQUAL(false, webpImgPlayer.isAnimation());
    TEST_ASSERT_EQUAL(false, webpImgPlayer.play(canvas));

    /* Too high. */
    TEST_ASSERT_EQUAL(true, writeVp8xHeaderFile(FILE_NAME_HUGE_CANVAS, DIM_SMALL, DIM_TOO_BIG));
    TEST_ASSERT_EQUAL(WebpImgPlayer::RET_IMG_TOO_BIG, webpImgPlayer.open(fileSystem, FILE_NAME_HUGE_CANVAS));

    TEST_ASSERT_EQUAL_INT16(0, webpImgPlayer.getWidth());
    TEST_ASSERT_EQUAL_INT16(0, webpImgPlayer.getHeight());
    TEST_ASSERT_EQUAL(false, webpImgPlayer.play(canvas));

    webpImgPlayer.close();

    (void)remove(FILE_NAME_HUGE_CANVAS);
}

/**
 * Test WebP image player with a WebP image which contains an alpha channel.
 * Transparent pixels must be composited against black and not show the color
 * which is stored underneath them.
 */
static void testWebpImgPlayerAlpha()
{
    WebpImgPlayer webpImgPlayer;
    YAGfxTest     testGfx;
    YAGfxCanvas   canvas(&testGfx, 0, 0, YAGfxTest::WIDTH, YAGfxTest::HEIGHT);
    FS            fileSystem;
    int16_t       x;
    int16_t       y;

    /* The fixture is 10x10. Its left half is opaque red, its right half is
     * green with alpha 0. The green must never become visible.
     */
    const int16_t  WIDTH          = 10;
    const int16_t  HEIGHT         = 10;
    const int16_t  OPAQUE_WIDTH   = 5;
    const uint32_t COLOR_OPAQUE   = 0xFF0000U;
    const uint32_t COLOR_COMPOSED = 0x000000U;

    /* The test canvas may be smaller than the image. */
    const int16_t  CHECK_WIDTH    = (static_cast<int16_t>(YAGfxTest::WIDTH) > WIDTH) ? WIDTH : static_cast<int16_t>(YAGfxTest::WIDTH);
    const int16_t  CHECK_HEIGHT   = (static_cast<int16_t>(YAGfxTest::HEIGHT) > HEIGHT) ? HEIGHT : static_cast<int16_t>(YAGfxTest::HEIGHT);

    testGfx.fillScreen(ColorDef::WHITE);

    TEST_ASSERT_EQUAL(WebpImgPlayer::RET_OK, webpImgPlayer.open(fileSystem, FILE_NAME_ALPHA));
    TEST_ASSERT_EQUAL_INT16(WIDTH, webpImgPlayer.getWidth());
    TEST_ASSERT_EQUAL_INT16(HEIGHT, webpImgPlayer.getHeight());
    TEST_ASSERT_EQUAL(true, webpImgPlayer.play(canvas));

    for (y = 0; y < CHECK_HEIGHT; ++y)
    {
        for (x = 0; x < CHECK_WIDTH; ++x)
        {
            const uint32_t expected = (OPAQUE_WIDTH > x) ? COLOR_OPAQUE : COLOR_COMPOSED;
            Color&         color    = testGfx.getColor(x, y);

            TEST_ASSERT_EQUAL_UINT32(expected, static_cast<uint32_t>(color));
        }
    }

    webpImgPlayer.close();
}

/**
 * Copy at most the given number of bytes from one file to another one.
 *
 * @param[in] srcFileName   Name of the source file.
 * @param[in] dstFileName   Name of the destination file.
 * @param[in] maxSize       Max. number of bytes to copy.
 *
 * @return If successful, it will return true otherwise false.
 */
static bool copyFilePart(const char* srcFileName, const char* dstFileName, size_t maxSize)
{
    bool   isSuccessful = false;
    FILE*  src          = fopen(srcFileName, "rb");
    FILE*  dst          = nullptr;

    if (nullptr != src)
    {
        dst = fopen(dstFileName, "wb");

        if (nullptr != dst)
        {
            size_t  remaining = maxSize;
            uint8_t buffer[64U];

            isSuccessful = true;

            while ((0U < remaining) && (true == isSuccessful))
            {
                const size_t chunkSize = (sizeof(buffer) < remaining) ? sizeof(buffer) : remaining;
                const size_t readSize  = fread(buffer, 1U, chunkSize, src);

                if ((0U == readSize) ||
                    (readSize != fwrite(buffer, 1U, readSize, dst)))
                {
                    isSuccessful = false;
                }
                else
                {
                    remaining -= readSize;
                }
            }

            (void)fclose(dst);
        }

        (void)fclose(src);
    }

    return isSuccessful;
}

/**
 * Test that the loop count of the WebP stream is honoured. The animation shall
 * run exactly as often as the stream demands and then stop.
 */
static void testWebpImgPlayerLoopCount()
{
    WebpImgPlayer  webpImgPlayer;
    YAGfxTest      testGfx;
    YAGfxCanvas    canvas(&testGfx, 0, 0, YAGfxTest::WIDTH, YAGfxTest::HEIGHT);
    FS             fileSystem;
    uint32_t       cycleCount  = 0U;
    uint32_t       elapsedTime = 0U;
    uint32_t       startTime   = 0U;

    /* The fixture has 3 frames of 40 ms and a loop count of 2, which means
     * 240 ms in total. Observed much longer, to prove that no 3rd cycle
     * follows.
     */
    const uint32_t OBSERVATION_TIME  = 2000U;
    const uint32_t EXPECTED_CYCLES   = 2U;

    TEST_ASSERT_EQUAL(WebpImgPlayer::RET_OK, webpImgPlayer.open(fileSystem, FILE_NAME_LOOP_COUNT));
    TEST_ASSERT_EQUAL(true, webpImgPlayer.isAnimation());

    startTime = millis();

    while (OBSERVATION_TIME > elapsedTime)
    {
        TEST_ASSERT_EQUAL(true, webpImgPlayer.play(canvas));

        if (true == webpImgPlayer.isAnimationFinished())
        {
            ++cycleCount;
        }

        delay(1U);
        elapsedTime = millis() - startTime;
    }

    printf("Loop count test: %u finished cycles in %u ms\n", cycleCount, elapsedTime);

    TEST_ASSERT_EQUAL_UINT32(EXPECTED_CYCLES, cycleCount);

    webpImgPlayer.close();
}

/**
 * Test that the infinite animation mode overrules the loop count of the WebP
 * stream. The very same fixture which stops after its 2 cycles in
 * testWebpImgPlayerLoopCount() shall never stop here.
 *
 * BitmapWidget enables this mode for every image it shows, so that a WebP in a
 * slot animates as long as the slot is shown, exactly like a GIF does.
 */
static void testWebpImgPlayerInfiniteLoop()
{
    WebpImgPlayer  webpImgPlayer;
    YAGfxTest      testGfx;
    YAGfxCanvas    canvas(&testGfx, 0, 0, YAGfxTest::WIDTH, YAGfxTest::HEIGHT);
    FS             fileSystem;
    uint32_t       cycleCount  = 0U;
    uint32_t       elapsedTime = 0U;
    uint32_t       startTime   = 0U;

    /* The fixture has 3 frames of 40 ms and a loop count of 2. Without the
     * infinite animation mode it stops after 2 cycles, see
     * testWebpImgPlayerLoopCount(). Observed over the same time, which is good
     * for 16 cycles of 120 ms.
     */
    const uint32_t OBSERVATION_TIME   = 2000U;
    const uint32_t STREAM_LOOP_COUNT  = 2U;

    webpImgPlayer.setInfiniteAnimation(true);

    TEST_ASSERT_EQUAL(WebpImgPlayer::RET_OK, webpImgPlayer.open(fileSystem, FILE_NAME_LOOP_COUNT));
    TEST_ASSERT_EQUAL(true, webpImgPlayer.isAnimation());

    startTime = millis();

    while (OBSERVATION_TIME > elapsedTime)
    {
        TEST_ASSERT_EQUAL(true, webpImgPlayer.play(canvas));

        if (true == webpImgPlayer.isAnimationFinished())
        {
            ++cycleCount;
        }

        delay(1U);
        elapsedTime = millis() - startTime;
    }

    printf("Infinite loop test: %u finished cycles in %u ms\n", cycleCount, elapsedTime);

    /* More cycles than the stream allows proves that the mode overrules it.
     * The exact number depends on the scheduling of the host, therefore it is
     * not compared for equality.
     */
    TEST_ASSERT_GREATER_THAN_UINT32(STREAM_LOOP_COUNT, cycleCount);

    webpImgPlayer.close();
}

/**
 * Test that a repeated open()/play()/close() of an animated WebP image does
 * not leak memory.
 */
static void testWebpImgPlayerReopenLoop()
{
#if (0 == HEAP_USAGE_MEASURABLE)

    TEST_IGNORE_MESSAGE("No heap usage probe available for this host.");

#else /* (0 != HEAP_USAGE_MEASURABLE) */

    WebpImgPlayer  webpImgPlayer;
    YAGfxTest      testGfx;
    YAGfxCanvas    canvas(&testGfx, 0, 0, YAGfxTest::WIDTH, YAGfxTest::HEIGHT);
    FS             fileSystem;
    uint32_t       cycle;
    size_t         heapAfterFirstClose = 0U;
    size_t         heapAfterLastClose  = 0U;

    const uint32_t CYCLES              = 50U;

    for (cycle = 0U; cycle < CYCLES; ++cycle)
    {
        TEST_ASSERT_EQUAL(WebpImgPlayer::RET_OK, webpImgPlayer.open(fileSystem, FILE_NAME_ANIMATION));
        TEST_ASSERT_EQUAL(true, webpImgPlayer.play(canvas));
        webpImgPlayer.close();

        /* The first cycle warms up everything which is allocated only once,
         * like the stdio buffers or the libwebp lookup tables.
         */
        if (0U == cycle)
        {
            heapAfterFirstClose = getHeapUsage();
        }
    }

    heapAfterLastClose = getHeapUsage();

    printf("Heap in use after 1st close: %u byte, after %u. close: %u byte\n",
        static_cast<unsigned int>(heapAfterFirstClose),
        static_cast<unsigned int>(CYCLES),
        static_cast<unsigned int>(heapAfterLastClose));

    TEST_ASSERT_GREATER_THAN_UINT32(0U, static_cast<uint32_t>(heapAfterFirstClose));

#if defined(_WIN32)

    /* The MSVCRT heap walk sums the blocks which are marked as in use, one by
     * one. Nothing but an allocation of this test changes that sum, therefore
     * it is compared for equality.
     */
    TEST_ASSERT_EQUAL_UINT32(static_cast<uint32_t>(heapAfterFirstClose), static_cast<uint32_t>(heapAfterLastClose));

#else /* !defined(_WIN32) */

    /* glibc reports the sum over its in-use chunks in uordblks. A chunk which
     * sits in a fastbin still counts as in use there, until malloc_consolidate()
     * folds it into a free chunk, which can happen at any allocation. The value
     * therefore moves by some kByte in both directions without a single byte
     * having leaked, so equality would make this test flaky on the CI host.
     *
     * The leak this test exists to catch is of a different order: an image
     * buffer, a decoder or the file data, which the mutation check measured at
     * about 3.3 kByte per cycle, i.e. roughly 160 kByte over the 49 cycles
     * which follow the first one. A bound of 64 kByte total growth is far below
     * that and far above the consolidation noise.
     */
    const size_t HEAP_GROWTH_MAX = 65536U;
    const size_t heapGrowth      = (heapAfterLastClose > heapAfterFirstClose)
                                     ? (heapAfterLastClose - heapAfterFirstClose)
                                     : 0U;

    printf("Heap growth over %u cycles: %u byte, tolerated: %u byte\n",
        static_cast<unsigned int>(CYCLES - 1U),
        static_cast<unsigned int>(heapGrowth),
        static_cast<unsigned int>(HEAP_GROWTH_MAX));

    TEST_ASSERT_LESS_OR_EQUAL_UINT32(static_cast<uint32_t>(HEAP_GROWTH_MAX), static_cast<uint32_t>(heapGrowth));

#endif /* !defined(_WIN32) */

#endif /* (0 != HEAP_USAGE_MEASURABLE) */
}

/**
 * Get the number of bytes which are currently in use on the C runtime heap.
 *
 * @return Number of bytes in use. 0 if the host provides no probe.
 */
static size_t getHeapUsage()
{
    size_t usage = 0U;

#if defined(_WIN32)

    _HEAPINFO      heapInfo;
    uint32_t       blockCount     = 0U;

    /* Bound the walk, so that a corrupted heap can not block the test. */
    const uint32_t BLOCK_COUNT_MAX = 10000000U;

    heapInfo._pentry               = nullptr;

    while ((BLOCK_COUNT_MAX > blockCount) &&
           (_HEAPOK == _heapwalk(&heapInfo)))
    {
        if (_USEDENTRY == heapInfo._useflag)
        {
            usage += heapInfo._size;
        }

        ++blockCount;
    }

#elif defined(__GLIBC__)

#if defined(HEAP_USAGE_MALLINFO2)
    const struct mallinfo2 info = mallinfo2();
#else
    const struct mallinfo  info = mallinfo();
#endif

    usage = static_cast<size_t>(info.uordblks);

#endif

    return usage;
}

/**
 * Write the given bytes to a file, replacing any existing one.
 *
 * @param[in] fileName  Name of the destination file.
 * @param[in] data      Bytes to write. Must not be nullptr.
 * @param[in] size      Number of bytes to write.
 *
 * @return If successful, it will return true otherwise false.
 */
static bool writeFile(const char* fileName, const uint8_t* data, size_t size)
{
    bool  isSuccessful = false;
    FILE* fd           = fopen(fileName, "wb");

    if (nullptr != fd)
    {
        if (size == fwrite(data, 1U, size, fd))
        {
            isSuccessful = true;
        }

        (void)fclose(fd);
    }

    return isSuccessful;
}

/**
 * Write a minimal WebP file which consists of nothing but a RIFF container
 * with a single VP8X chunk, declaring the given canvas size.
 *
 * The animation flag is set, because libwebp then reports the canvas size from
 * the VP8X header alone, without any image data having to follow.
 *
 * @param[in] fileName  Name of the destination file.
 * @param[in] width     Canvas width in pixels. 1 .. 16777216.
 * @param[in] height    Canvas height in pixels. 1 .. 16777216.
 *
 * @return If successful, it will return true otherwise false.
 */
static bool writeVp8xHeaderFile(const char* fileName, uint32_t width, uint32_t height)
{
    const uint32_t widthMinusOne  = width - 1U;
    const uint32_t heightMinusOne = height - 1U;

    /* "RIFF" <riff size> "WEBP" "VP8X" <chunk size> <flags> <w-1> <h-1> */
    const uint8_t  header[]       = {
        'R', 'I', 'F', 'F',
        22U, 0U, 0U, 0U, /* RIFF size: 4 ("WEBP") + 8 (chunk header) + 10 */
        'W', 'E', 'B', 'P',
        'V', 'P', '8', 'X',
        10U, 0U, 0U, 0U, /* VP8X payload size */
        0x02U,           /* Flags: animation */
        0U, 0U, 0U,      /* Reserved */
        static_cast<uint8_t>(widthMinusOne & 0xFFU),
        static_cast<uint8_t>((widthMinusOne >> 8U) & 0xFFU),
        static_cast<uint8_t>((widthMinusOne >> 16U) & 0xFFU),
        static_cast<uint8_t>(heightMinusOne & 0xFFU),
        static_cast<uint8_t>((heightMinusOne >> 8U) & 0xFFU),
        static_cast<uint8_t>((heightMinusOne >> 16U) & 0xFFU)
    };

    return writeFile(fileName, header, sizeof(header));
}
