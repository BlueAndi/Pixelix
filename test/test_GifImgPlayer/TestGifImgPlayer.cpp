/* MIT License
 *
 * Copyright (c) 2019 - 2024 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Test GIF image player.
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <unity.h>
#include <unistd.h>
#include <GifImgPlayer.h>
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

static void testGifImgPlayerStatic();
static void testGifImgPlayerAnimated();

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/** Expected image width. */
static const uint32_t   EXPECTED_DATA_WIDTH = 10U;

/** Expected image data.  */
static const uint32_t   EXPECTED_DATA[] =
{
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
extern int main(int argc, char **argv)
{
    UTIL_NOT_USED(argc);
    UTIL_NOT_USED(argv);

    UNITY_BEGIN();

    RUN_TEST(testGifImgPlayerStatic);
    RUN_TEST(testGifImgPlayerAnimated);

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
 * Test GIF image player with a static GIF image.
 */
static void testGifImgPlayerStatic()
{
    GifImgPlayer    gifImgPlayer;
    YAGfxTest       testGfx;
    YAGfxCanvas     canvas(&testGfx, 0, 0, YAGfxTest::WIDTH, YAGfxTest::HEIGHT);
    FS              fileSystem;
    int32_t         x;
    int32_t         y;
    int32_t         width   = (YAGfxTest::WIDTH > EXPECTED_DATA_WIDTH) ? EXPECTED_DATA_WIDTH : YAGfxTest::WIDTH;
    int32_t         height  = (YAGfxTest::HEIGHT > EXPECTED_DATA_WIDTH) ? EXPECTED_DATA_WIDTH : YAGfxTest::HEIGHT;
    
    TEST_ASSERT_EQUAL(GifImgPlayer::RET_OK, gifImgPlayer.open(fileSystem, "./test/test_GifImgPlayer/TestStatic.gif"));
    TEST_ASSERT_EQUAL(true, gifImgPlayer.play(canvas));
    gifImgPlayer.close();

    for(y = 0; y < height; ++y)
    {
        for(x = 0; x < width; ++x)
        {
            printf("%u, %u\n", x, y);

            TEST_ASSERT_EQUAL_UINT32(EXPECTED_DATA[x + y * EXPECTED_DATA_WIDTH], static_cast<uint32_t>(testGfx.getColor(x, y)));
        }
    }
}

/**
 * Test GIF image player with a animated GIF image.
 */
static void testGifImgPlayerAnimated()
{
    GifImgPlayer    gifImgPlayer;
    YAGfxTest       testGfx;
    YAGfxCanvas     canvas(&testGfx, 0, 0, YAGfxTest::WIDTH, YAGfxTest::HEIGHT);
    FS              fileSystem;
    
    TEST_ASSERT_EQUAL(GifImgPlayer::RET_OK, gifImgPlayer.open(fileSystem, "./test/test_GifImgPlayer/TestAnimation.gif"));
    TEST_ASSERT_EQUAL(true, gifImgPlayer.play(canvas));
    usleep(200000);
    TEST_ASSERT_EQUAL(true, gifImgPlayer.play(canvas));
    usleep(200000);
    TEST_ASSERT_EQUAL(true, gifImgPlayer.play(canvas));
    usleep(200000);
    TEST_ASSERT_EQUAL(true, gifImgPlayer.play(canvas));
    gifImgPlayer.close();
}
