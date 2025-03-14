/* MIT License
 *
 * Copyright (c) 2019 - 2025 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Test utility functions.
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <unity.h>
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

static void testUtil(void);
static void testMinMax(void);
static void testColorHtml(void);
static void testFormatSpecifierInStr(void);

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

    RUN_TEST(testUtil);
    RUN_TEST(testMinMax);
    RUN_TEST(testColorHtml);
    RUN_TEST(testFormatSpecifierInStr);

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
 * Test utility functions.
 */
static void testUtil(void)
{
    String      hexStr;
    uint8_t     valueUInt8  = 0U;
    uint16_t    valueUInt16 = 0U;
    uint32_t    valueUInt32 = 0U;
    int32_t     valueInt32  = 0;

    /* Test string to 8 bit unsigned integer conversion. */
    TEST_ASSERT_TRUE(Util::strToUInt8("0", valueUInt8));
    TEST_ASSERT_EQUAL_UINT8(0U, valueUInt8);

    valueUInt8 = 0U;
    TEST_ASSERT_TRUE(Util::strToUInt8("255", valueUInt8));
    TEST_ASSERT_EQUAL_UINT8(0xffU, valueUInt8);

    valueUInt8 = 0U;
    TEST_ASSERT_FALSE(Util::strToUInt8("256", valueUInt8));
    TEST_ASSERT_EQUAL_UINT8(0U, valueUInt8);

    valueUInt8 = 0U;
    TEST_ASSERT_FALSE(Util::strToUInt8("-1", valueUInt8));
    TEST_ASSERT_EQUAL_UINT8(0U, valueUInt8);

    /* Test string to 16 bit unsigned integer conversion. */
    TEST_ASSERT_TRUE(Util::strToUInt16("0", valueUInt16));
    TEST_ASSERT_EQUAL_UINT16(0U, valueUInt16);

    valueUInt16 = 0U;
    TEST_ASSERT_TRUE(Util::strToUInt16("65535", valueUInt16));
    TEST_ASSERT_EQUAL_UINT16(0xffffU, valueUInt16);

    valueUInt16 = 0U;
    TEST_ASSERT_FALSE(Util::strToUInt16("65536", valueUInt16));
    TEST_ASSERT_EQUAL_UINT16(0U, valueUInt16);

    valueUInt16 = 0U;
    TEST_ASSERT_FALSE(Util::strToUInt16("-1", valueUInt16));
    TEST_ASSERT_EQUAL_UINT16(0U, valueUInt16);

    /* Test string to 32 bit unsigned integer conversion. */
    TEST_ASSERT_TRUE(Util::strToUInt32("0", valueUInt32));
    TEST_ASSERT_EQUAL_UINT32(0U, valueUInt32);

    valueUInt32 = 0U;
    TEST_ASSERT_TRUE(Util::strToUInt32("4294967295", valueUInt32));
    TEST_ASSERT_EQUAL_UINT32(0xffffffffU, valueUInt32);

    valueUInt32 = 0U;
    TEST_ASSERT_FALSE(Util::strToUInt32("4294967296", valueUInt32));
    TEST_ASSERT_EQUAL_UINT32(0U, valueUInt32);

/* This test fails if executed with mingw, but is successful on CI with gcc. */
#if 0
    valueUInt32 = 0U;
    TEST_ASSERT_FALSE(Util::strToUInt32("-1", valueUInt32));
    TEST_ASSERT_EQUAL_UINT32(0U, valueUInt32);
#endif

    /* Test string to 32 bit signed integer conversion. */
    TEST_ASSERT_TRUE(Util::strToInt32("0", valueInt32));
    TEST_ASSERT_EQUAL_INT32(0, valueInt32);

    valueInt32 = 0;
    TEST_ASSERT_TRUE(Util::strToInt32("1", valueInt32));
    TEST_ASSERT_EQUAL_INT32(1, valueInt32);

    valueInt32 = 0;
    TEST_ASSERT_TRUE(Util::strToInt32("-1", valueInt32));
    TEST_ASSERT_EQUAL_INT32(-1, valueInt32);

    valueInt32 = 0;
    TEST_ASSERT_TRUE(Util::strToInt32("2147483647", valueInt32));
    TEST_ASSERT_EQUAL_INT32(2147483647, valueInt32);

    valueInt32 = 0;
    TEST_ASSERT_TRUE(Util::strToInt32("-2147483648", valueInt32));
    TEST_ASSERT_EQUAL_INT32(-2147483648, valueInt32);

    valueInt32 = 0;
    TEST_ASSERT_FALSE(Util::strToInt32("4294967295", valueInt32));
    TEST_ASSERT_EQUAL_INT32(0, valueInt32);

    /* Test number to hex string conversion */
    TEST_ASSERT_EQUAL_STRING("1", Util::uint32ToHex(0x01).c_str());
    TEST_ASSERT_EQUAL_STRING("a", Util::uint32ToHex(0x0a).c_str());
    TEST_ASSERT_EQUAL_STRING("f", Util::uint32ToHex(0x0f).c_str());
    TEST_ASSERT_EQUAL_STRING("10", Util::uint32ToHex(0x10).c_str());
    TEST_ASSERT_EQUAL_STRING("ffff0000", Util::uint32ToHex(0xffff0000).c_str());
    TEST_ASSERT_EQUAL_STRING("ffffffff", Util::uint32ToHex(0xffffffff).c_str());

    /* Value of empty hex string shall be 0. */
    hexStr.clear();
    TEST_ASSERT_EQUAL_UINT32(0U, Util::hexToUInt32(hexStr));

    /* Several valid tests now. */
    hexStr = "1";
    TEST_ASSERT_EQUAL_UINT32(1U, Util::hexToUInt32(hexStr));
    hexStr = "0x1";
    TEST_ASSERT_EQUAL_UINT32(1U, Util::hexToUInt32(hexStr));
    hexStr = "0X1";
    TEST_ASSERT_EQUAL_UINT32(1U, Util::hexToUInt32(hexStr));
    hexStr = "10";
    TEST_ASSERT_EQUAL_UINT32(16U, Util::hexToUInt32(hexStr));
    hexStr = "0x10";
    TEST_ASSERT_EQUAL_UINT32(16U, Util::hexToUInt32(hexStr));
    hexStr = "0X10";
    TEST_ASSERT_EQUAL_UINT32(16U, Util::hexToUInt32(hexStr));
    hexStr = "1f";
    TEST_ASSERT_EQUAL_UINT32(31U, Util::hexToUInt32(hexStr));
    hexStr = "0x1f";
    TEST_ASSERT_EQUAL_UINT32(31U, Util::hexToUInt32(hexStr));
    hexStr = "0x1F";
    TEST_ASSERT_EQUAL_UINT32(31U, Util::hexToUInt32(hexStr));

    /* Several invalid tests now. */
    hexStr = " 1";
    TEST_ASSERT_EQUAL_UINT32(0U, Util::hexToUInt32(hexStr));
    hexStr = "1 ";
    TEST_ASSERT_EQUAL_UINT32(0U, Util::hexToUInt32(hexStr));
    hexStr = "g";
    TEST_ASSERT_EQUAL_UINT32(0U, Util::hexToUInt32(hexStr));
    hexStr = "G";
    TEST_ASSERT_EQUAL_UINT32(0U, Util::hexToUInt32(hexStr));
    hexStr = "1g";
    TEST_ASSERT_EQUAL_UINT32(0U, Util::hexToUInt32(hexStr));
    hexStr = "1G";
    TEST_ASSERT_EQUAL_UINT32(0U, Util::hexToUInt32(hexStr));
    hexStr = "0y5";
    TEST_ASSERT_EQUAL_UINT32(0U, Util::hexToUInt32(hexStr));
}

/**
 * Test min and max functions.
 */
static void testMinMax(void)
{
    TEST_ASSERT_EQUAL_INT8(1, Util::min(1, 2));
    TEST_ASSERT_EQUAL_INT8(1, Util::min(2, 1));
    TEST_ASSERT_EQUAL_INT8(1, Util::min(1, 1));

    TEST_ASSERT_EQUAL_INT8(2, Util::max(1, 2));
    TEST_ASSERT_EQUAL_INT8(2, Util::max(2, 1));
    TEST_ASSERT_EQUAL_INT8(1, Util::max(1, 1));
}

/**
 * Test color to HTML conversion.
 */
static void testColorHtml(void)
{
    Color   color;
    String  htmlColor;

    /* Test black color */
    color.set(0, 0, 0);
    htmlColor = Util::colorToHtml(color);
    TEST_ASSERT_EQUAL_STRING("#000000", htmlColor.c_str());

    /* Test white color */
    color.set(255, 255, 255);
    htmlColor = Util::colorToHtml(color);
    TEST_ASSERT_EQUAL_STRING("#FFFFFF", htmlColor.c_str());

    /* Test red color */
    color.set(255, 0, 0);
    htmlColor = Util::colorToHtml(color);
    TEST_ASSERT_EQUAL_STRING("#FF0000", htmlColor.c_str());

    /* Test green color */
    color.set(0, 255, 0);
    htmlColor = Util::colorToHtml(color);
    TEST_ASSERT_EQUAL_STRING("#00FF00", htmlColor.c_str());

    /* Test blue color */
    color.set(0, 0, 255);
    htmlColor = Util::colorToHtml(color);
    TEST_ASSERT_EQUAL_STRING("#0000FF", htmlColor.c_str());

    /* Test yellow color */
    color.set(255, 255, 0);
    htmlColor = Util::colorToHtml(color);
    TEST_ASSERT_EQUAL_STRING("#FFFF00", htmlColor.c_str());

    /* Test magenta color */
    color.set(255, 0, 255);
    htmlColor = Util::colorToHtml(color);
    TEST_ASSERT_EQUAL_STRING("#FF00FF", htmlColor.c_str());

    /* Test cyan color */
    color.set(0, 255, 255);
    htmlColor = Util::colorToHtml(color);
    TEST_ASSERT_EQUAL_STRING("#00FFFF", htmlColor.c_str());

    /* Test gray color */
    color.set(128, 128, 128);
    htmlColor = Util::colorToHtml(color);
    TEST_ASSERT_EQUAL_STRING("#808080", htmlColor.c_str());

    /* Test dark red color */
    color.set(128, 0, 0);
    htmlColor = Util::colorToHtml(color);
    TEST_ASSERT_EQUAL_STRING("#800000", htmlColor.c_str());

    /* Test dark green color */
    color.set(0, 128, 0);
    htmlColor = Util::colorToHtml(color);
    TEST_ASSERT_EQUAL_STRING("#008000", htmlColor.c_str());

    /* Test dark blue color */
    color.set(0, 0, 128);
    htmlColor = Util::colorToHtml(color);
    TEST_ASSERT_EQUAL_STRING("#000080", htmlColor.c_str());
}

/**
 * Test format specifier in string.
 */
static void testFormatSpecifierInStr(void)
{
    String  str;

    str = "Hello %s";
    TEST_ASSERT_TRUE(Util::isFormatSpecifierInStr(str, 's'));

    str = "Hello %d";
    TEST_ASSERT_TRUE(Util::isFormatSpecifierInStr(str, 'd'));

    str = "Hello %f";
    TEST_ASSERT_TRUE(Util::isFormatSpecifierInStr(str, 'f'));

    str = "Hello %d %s";
    TEST_ASSERT_TRUE(Util::isFormatSpecifierInStr(str, 'd'));
    TEST_ASSERT_TRUE(Util::isFormatSpecifierInStr(str, 's'));

    str = "Hello %d %s";
    TEST_ASSERT_TRUE(Util::isFormatSpecifierInStr(str, 'd'));
    TEST_ASSERT_TRUE(Util::isFormatSpecifierInStr(str, 's'));

    str = "Hello %d %s";
    TEST_ASSERT_TRUE(Util::isFormatSpecifierInStr(str, 'd'));
    TEST_ASSERT_TRUE(Util::isFormatSpecifierInStr(str, 's'));

    str = "Hello %d %s";
    TEST_ASSERT_TRUE(Util::isFormatSpecifierInStr(str, 'd'));
    TEST_ASSERT_TRUE(Util::isFormatSpecifierInStr(str, 's'));

    str = "Hello %d %s";
    TEST_ASSERT_TRUE(Util::isFormatSpecifierInStr(str, 'd'));
    TEST_ASSERT_TRUE(Util::isFormatSpecifierInStr(str, 's'));

    str = "Hello %d %s";
    TEST_ASSERT_TRUE(Util::isFormatSpecifierInStr(str, 'd'));
    TEST_ASSERT_TRUE(Util::isFormatSpecifierInStr(str, 's'));

    str = "Hello %d %s";
    TEST_ASSERT_TRUE(Util::isFormatSpecifierInStr(str, 'd'));
    TEST_ASSERT_TRUE(Util::isFormatSpecifierInStr(str, 's'));

    str = "Hello %d %s";
    TEST_ASSERT_TRUE(Util::isFormatSpecifierInStr(str, 'd'));
    TEST_ASSERT_TRUE(Util::isFormatSpecifierInStr(str, 's'));

    str = "Hello %d %s";
    TEST_ASSERT_TRUE(Util::isFormatSpecifierInStr(str, 'd'));
    TEST_ASSERT_TRUE(Util::isFormatSpecifierInStr(str, 's'));

    str = "Hello %d %s";
    TEST_ASSERT_TRUE(Util::isFormatSpecifierInStr(str, 'd'));
    TEST_ASSERT_TRUE(Util::isFormatSpecifierInStr(str, 's'));

    str = "Hello %.5s";
    TEST_ASSERT_TRUE(Util::isFormatSpecifierInStr(str, 's'));

    str = "Hello %0.2f";
    TEST_ASSERT_TRUE(Util::isFormatSpecifierInStr(str, 'f'));
}