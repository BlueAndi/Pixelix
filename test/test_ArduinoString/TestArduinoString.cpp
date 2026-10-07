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
 * @file   TestArduinoString.cpp
 * @brief  Test the String of the Arduino implementation for the native environment.
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * The String is used by the whole application and by the webserver. A deviation
 * from the Arduino behaviour is not visible at compile time, but breaks the
 * application at runtime. Therefore the behaviour is pinned down here.
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <unity.h>
#include <WString.h>
#include <Util.h>
#include <cmath>

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

static void testConstruction();
static void testNumberConstruction();
static void testConcat();
static void testSubstring();
static void testStartsWith();
static void testEndsWith();
static void testIndexOf();
static void testLastIndexOf();
static void testModification();
static void testConversion();

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/** Max. deviation which is accepted, if two floating point values are compared. */
static const double EPSILON = 0.0001;

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

    RUN_TEST(testConstruction);
    RUN_TEST(testNumberConstruction);
    RUN_TEST(testConcat);
    RUN_TEST(testSubstring);
    RUN_TEST(testStartsWith);
    RUN_TEST(testEndsWith);
    RUN_TEST(testIndexOf);
    RUN_TEST(testLastIndexOf);
    RUN_TEST(testModification);
    RUN_TEST(testConversion);

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
 * Test the construction and the assignment.
 */
static void testConstruction()
{
    String empty;
    String fromLiteral("Pixelix");
    String fromNullptr(static_cast<const char*>(nullptr));
    String fromChar('P');
    String fromBuffer("Pixelix", 5U);
    String copy(fromLiteral);

    TEST_ASSERT_TRUE(empty.isEmpty());
    TEST_ASSERT_EQUAL_UINT32(0U, empty.length());
    TEST_ASSERT_EQUAL_STRING("", empty.c_str());

    TEST_ASSERT_EQUAL_STRING("Pixelix", fromLiteral.c_str());
    TEST_ASSERT_EQUAL_UINT32(7U, fromLiteral.length());

    /* A nullptr results in an empty string and must not crash. */
    TEST_ASSERT_EQUAL_STRING("", fromNullptr.c_str());

    TEST_ASSERT_EQUAL_STRING("P", fromChar.c_str());
    TEST_ASSERT_EQUAL_STRING("Pixel", fromBuffer.c_str());
    TEST_ASSERT_EQUAL_STRING("Pixelix", copy.c_str());

    empty = fromLiteral;
    TEST_ASSERT_EQUAL_STRING("Pixelix", empty.c_str());

    empty = "Other";
    TEST_ASSERT_EQUAL_STRING("Other", empty.c_str());

    TEST_ASSERT_TRUE(fromLiteral == copy);
    TEST_ASSERT_TRUE(fromLiteral != empty);

    /* A index out of bounds results in a termination character. */
    TEST_ASSERT_EQUAL_CHAR('P', fromLiteral[0U]);
    TEST_ASSERT_EQUAL_CHAR('x', fromLiteral.charAt(6U));
    TEST_ASSERT_EQUAL_CHAR('\0', fromLiteral[7U]);
    TEST_ASSERT_EQUAL_CHAR('\0', fromLiteral.charAt(100U));
}

/**
 * Test the construction from a number. Every integral type has its own
 * constructor, therefore every one is verified.
 */
static void testNumberConstruction()
{
    TEST_ASSERT_EQUAL_STRING("42", String(static_cast<unsigned char>(42U)).c_str());
    TEST_ASSERT_EQUAL_STRING("-42", String(static_cast<int>(-42)).c_str());
    TEST_ASSERT_EQUAL_STRING("42", String(static_cast<unsigned int>(42U)).c_str());
    TEST_ASSERT_EQUAL_STRING("-42", String(static_cast<long>(-42)).c_str());
    TEST_ASSERT_EQUAL_STRING("42", String(static_cast<unsigned long>(42U)).c_str());
    TEST_ASSERT_EQUAL_STRING("-42", String(static_cast<long long>(-42)).c_str());
    TEST_ASSERT_EQUAL_STRING("42", String(static_cast<unsigned long long>(42U)).c_str());

    /* A size_t must be unambiguous, it is used all over the application. */
    TEST_ASSERT_EQUAL_STRING("42", String(static_cast<size_t>(42U)).c_str());

    /* The number base is considered. Note, the base is a int here and not a
     * unsigned int. The latter would be ambiguous, because it matches the
     * decimal places of the float constructor exactly.
     */
    TEST_ASSERT_EQUAL_STRING("ff", String(255, 16).c_str());
    TEST_ASSERT_EQUAL_STRING("11111111", String(255, 2).c_str());
    TEST_ASSERT_EQUAL_STRING("-ff", String(-255, 16).c_str());

    /* The default is 2 decimal places, like on the Arduino. */
    TEST_ASSERT_EQUAL_STRING("3.14", String(3.14159F).c_str());
    TEST_ASSERT_EQUAL_STRING("3.1416", String(3.14159F, 4U).c_str());
    TEST_ASSERT_EQUAL_STRING("3.14", String(3.14159).c_str());
}

/**
 * Test the concatenation. A number must be appended as number and not as
 * single character, otherwise e.g. the HTTP status line of the webserver
 * gets corrupted.
 */
static void testConcat()
{
    String str;

    TEST_ASSERT_TRUE(str.concat("HTTP/1."));
    TEST_ASSERT_TRUE(str.concat(static_cast<unsigned char>(1U)));
    TEST_ASSERT_TRUE(str.concat(' '));
    TEST_ASSERT_TRUE(str.concat(static_cast<int>(503)));
    TEST_ASSERT_EQUAL_STRING("HTTP/1.1 503", str.c_str());

    str = "";
    TEST_ASSERT_TRUE(str.concat(static_cast<unsigned int>(1U)));
    TEST_ASSERT_TRUE(str.concat(static_cast<long>(2)));
    TEST_ASSERT_TRUE(str.concat(static_cast<unsigned long>(3U)));
    TEST_ASSERT_TRUE(str.concat(static_cast<long long>(4)));
    TEST_ASSERT_TRUE(str.concat(static_cast<unsigned long long>(5U)));
    TEST_ASSERT_EQUAL_STRING("12345", str.c_str());

    str = "";
    TEST_ASSERT_TRUE(str.concat(String("abc")));
    TEST_ASSERT_TRUE(str.concat("def", 2U));
    TEST_ASSERT_EQUAL_STRING("abcde", str.c_str());

    /* A nullptr is not appended and reported as failure. */
    TEST_ASSERT_FALSE(str.concat(static_cast<const char*>(nullptr)));
    TEST_ASSERT_EQUAL_STRING("abcde", str.c_str());

    /* The operators behave like the concat. */
    str  = "value=";
    str += 42;
    TEST_ASSERT_EQUAL_STRING("value=42", str.c_str());

    TEST_ASSERT_EQUAL_STRING("a1", (String("a") + 1).c_str());
    TEST_ASSERT_EQUAL_STRING("1a", (String("1") + String("a")).c_str());
    TEST_ASSERT_EQUAL_STRING("prefix/", ("prefix" + String("/")).c_str());
}

/**
 * Test the substring. The Arduino clamps the indices, the std::string would
 * throw a std::out_of_range and terminate the program.
 */
static void testSubstring()
{
    String str("Pixelix");

    TEST_ASSERT_EQUAL_STRING("Pixelix", str.substring(0U).c_str());
    TEST_ASSERT_EQUAL_STRING("elix", str.substring(3U).c_str());
    TEST_ASSERT_EQUAL_STRING("Pix", str.substring(0U, 3U).c_str());
    TEST_ASSERT_EQUAL_STRING("eli", str.substring(3U, 6U).c_str());

    /* The index at the end of the string is valid and results in a empty string. */
    TEST_ASSERT_EQUAL_STRING("", str.substring(7U).c_str());

    /* Out of bounds must not throw. */
    TEST_ASSERT_EQUAL_STRING("", str.substring(100U).c_str());
    TEST_ASSERT_EQUAL_STRING("", str.substring(100U, 200U).c_str());
    TEST_ASSERT_EQUAL_STRING("elix", str.substring(3U, 100U).c_str());

    /* Swapped indices are corrected. */
    TEST_ASSERT_EQUAL_STRING("eli", str.substring(6U, 3U).c_str());

    TEST_ASSERT_EQUAL_STRING("", String().substring(0U).c_str());
    TEST_ASSERT_EQUAL_STRING("", String().substring(1U, 5U).c_str());
}

/**
 * Test the prefix comparison, including the offset which is easily forgotten.
 */
static void testStartsWith()
{
    String str("/style/style.css");

    TEST_ASSERT_TRUE(0U != str.startsWith(String("/style")));
    TEST_ASSERT_TRUE(0U != str.startsWith(String("/")));
    TEST_ASSERT_TRUE(0U != str.startsWith(String("")));

    /* The webserver derives the routing from this, a false positive would
     * redirect every request.
     */
    TEST_ASSERT_TRUE(0U == str.startsWith(String("//")));
    TEST_ASSERT_TRUE(0U == str.startsWith(String("style")));

    /* A pattern longer than the string never matches. */
    TEST_ASSERT_TRUE(0U == str.startsWith(String("/style/style.css.map")));

    /* The offset is considered. */
    TEST_ASSERT_TRUE(0U != str.startsWith(String("style"), 1U));
    TEST_ASSERT_TRUE(0U != str.startsWith(String("style.css"), 7U));
    TEST_ASSERT_TRUE(0U == str.startsWith(String("style"), 2U));
    TEST_ASSERT_TRUE(0U == str.startsWith(String("css"), 100U));
}

/**
 * Test the suffix comparison.
 */
static void testEndsWith()
{
    String str("/index.html");

    TEST_ASSERT_TRUE(str.endsWith(String(".html")));
    TEST_ASSERT_TRUE(str.endsWith(String("/index.html")));
    TEST_ASSERT_TRUE(str.endsWith(String("")));
    TEST_ASSERT_FALSE(str.endsWith(String(".htm")));
    TEST_ASSERT_FALSE(str.endsWith(String("/index.html.map")));
}

/**
 * Test the forward search.
 */
static void testIndexOf()
{
    String str("/plugins/IconTextPlugin.html");

    TEST_ASSERT_EQUAL_INT(0, str.indexOf('/'));
    TEST_ASSERT_EQUAL_INT(8, str.indexOf('/', 1U));
    TEST_ASSERT_EQUAL_INT(-1, str.indexOf('?'));

    TEST_ASSERT_EQUAL_INT(1, str.indexOf(String("plugins")));
    TEST_ASSERT_EQUAL_INT(-1, str.indexOf(String("plugins"), 2U));
    TEST_ASSERT_EQUAL_INT(-1, str.indexOf(String("missing")));
}

/**
 * Test the backward search. The whole pattern must be searched for and not
 * any of its characters, otherwise the webserver treats every route as a
 * wildcard route.
 */
static void testLastIndexOf()
{
    String str("/images/2023/pic.png");

    TEST_ASSERT_EQUAL_INT(12, str.lastIndexOf('/'));
    TEST_ASSERT_EQUAL_INT(7, str.lastIndexOf('/', 11U));
    TEST_ASSERT_EQUAL_INT(-1, str.lastIndexOf('?'));

    TEST_ASSERT_EQUAL_INT(16, str.lastIndexOf(String(".png")));
    TEST_ASSERT_EQUAL_INT(-1, str.lastIndexOf(String(".jpg")));

    /* Any of the characters '/', '*' and '.' is contained, the pattern is not. */
    TEST_ASSERT_EQUAL_INT(-1, str.lastIndexOf(String("/*.")));
    TEST_ASSERT_EQUAL_INT(-1, String("/").lastIndexOf(String("/*.")));
    TEST_ASSERT_EQUAL_INT(7, String("/images/*.png").lastIndexOf(String("/*.")));
}

/**
 * Test the methods which modify the string in place.
 */
static void testModification()
{
    String str("Pixelix");

    str.remove(3U);
    TEST_ASSERT_EQUAL_STRING("Pix", str.c_str());

    str = "Pixelix";
    str.remove(3U, 2U);
    TEST_ASSERT_EQUAL_STRING("Pixix", str.c_str());

    /* Out of bounds must not throw. */
    str.remove(100U);
    TEST_ASSERT_EQUAL_STRING("Pixix", str.c_str());

    str = "Pixelix";
    str.toUpperCase();
    TEST_ASSERT_EQUAL_STRING("PIXELIX", str.c_str());
    str.toLowerCase();
    TEST_ASSERT_EQUAL_STRING("pixelix", str.c_str());

    str = " \t Pixelix \r\n";
    str.trim();
    TEST_ASSERT_EQUAL_STRING("Pixelix", str.c_str());

    str = " \t \r\n";
    str.trim();
    TEST_ASSERT_EQUAL_STRING("", str.c_str());

    str = "Pixelix";
    str.clear();
    TEST_ASSERT_TRUE(str.isEmpty());
}

/**
 * Test the comparison and the conversion to a number.
 */
static void testConversion()
{
    TEST_ASSERT_EQUAL_INT32(42, String("42").toInt());
    TEST_ASSERT_EQUAL_INT32(-42, String("-42").toInt());
    TEST_ASSERT_EQUAL_INT32(0, String("abc").toInt());
    TEST_ASSERT_EQUAL_FLOAT(3.14F, String("3.14").toFloat());

    /* The unity double support is disabled, therefore compared by hand. */
    TEST_ASSERT_TRUE(EPSILON > std::fabs(3.14 - String("3.14").toDouble()));

    TEST_ASSERT_TRUE(String("Pixelix").equals(String("Pixelix")));
    TEST_ASSERT_TRUE(String("Pixelix").equals("Pixelix"));
    TEST_ASSERT_FALSE(String("Pixelix").equals("pixelix"));
    TEST_ASSERT_TRUE(String("Pixelix").equalsIgnoreCase(String("pIXELIX")));
    TEST_ASSERT_FALSE(String("Pixelix").equalsIgnoreCase(String("Pixeli")));

    TEST_ASSERT_EQUAL_INT(0, String("b").compareTo(String("b")));
    TEST_ASSERT_TRUE(0 > String("a").compareTo(String("b")));
    TEST_ASSERT_TRUE(0 < String("c").compareTo(String("b")));
}
