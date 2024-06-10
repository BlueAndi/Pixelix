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
 * @brief  Test LZW decoder.
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <unity.h>
#include <LzwDecoder.h>
#include <Util.h>

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

static void testLzwDecoder();

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/** LZW min. code length = 2 bits */
static const uint8_t    INPUT_DATA[] =
{
    0x8C, 0x2D, 0x99, 0x87, 0x2A, 0x1C, 0xDC, 0x33, 0xA0, 0x02, 0x75, 0xEC, 0x95, 0xFA, 0xA8, 0xDE, 0x60, 0x8C, 0x04, 0x91, 0x4C, 0x01
};

/** Expected image width. */
static const uint32_t   EXPECTED_DATA_WIDTH = 10U;

/** Expected image data.  */
static const uint8_t    EXPECTED_DATA[] =
{
    /*        0     1     2     3     4     5     6     7     8     9 */
    /* 0 */ 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02,
    /* 1 */ 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02,
    /* 2 */ 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02,
    /* 3 */ 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x02,
    /* 4 */ 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x02,
    /* 5 */ 0x02, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01,
    /* 6 */ 0x02, 0x02, 0x02, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01,
    /* 7 */ 0x02, 0x02, 0x02, 0x02, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01,
    /* 8 */ 0x02, 0x02, 0x02, 0x02, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01,
    /* 9 */ 0x02, 0x02, 0x02, 0x02, 0x02, 0x01, 0x01, 0x01, 0x01, 0x01
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

    RUN_TEST(testLzwDecoder);

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
 * Test LZW decoder.
 */
static void testLzwDecoder()
{
    LzwDecoder  lzwDecoder;
    size_t      srcIndex = 0;
    size_t      dstIndex = 0;
    uint8_t     lzwMinCodeLength = 2;
    uint8_t     dstBuffer[256];
    bool        result = false;

    lzwDecoder.init(lzwMinCodeLength);

    result = lzwDecoder.decode(
        [&srcIndex](uint8_t& data) -> bool
        {
            TEST_ASSERT_LESS_THAN_INT32(sizeof(INPUT_DATA), srcIndex);

            data = INPUT_DATA[srcIndex];
            ++srcIndex;

            printf("In: 0x%02X\n", data);

            return true;
        },
        [&dstIndex](uint8_t data) -> bool
        {
            TEST_ASSERT_LESS_THAN_INT32(sizeof(EXPECTED_DATA), dstIndex);

            printf("Out (%u, %u): 0x%02X\n", dstIndex % EXPECTED_DATA_WIDTH, dstIndex / EXPECTED_DATA_WIDTH, data);

            TEST_ASSERT_EQUAL_UINT8(EXPECTED_DATA[dstIndex], data);
            ++dstIndex;

            return true;
        }
    );

    TEST_ASSERT_EQUAL(true, result);
    TEST_ASSERT_EQUAL(sizeof(INPUT_DATA), srcIndex);
}
