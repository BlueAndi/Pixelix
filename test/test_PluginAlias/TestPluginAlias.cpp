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
 * @file   TestPluginAlias.cpp
 * @brief  Test the plugin alias rules in the native environment.
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <unity.h>
#include <PluginAlias.hpp>
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

static void testIsValidLength();
static void testIsValidCharacterSet();
static void testSanitizeReplaces();
static void testSanitizeTruncates();
static void testSanitizeKeepsTooShort();
static void testMakeDefault();

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
 * Main entry point.
 *
 * @param[in] argc  Number of command line arguments
 * @param[in] argv  Command line arguments
 *
 * @return Test result
 */
extern int main(int argc, char** argv)
{
    UTIL_NOT_USED(argc);
    UTIL_NOT_USED(argv);

    UNITY_BEGIN();

    RUN_TEST(testIsValidLength);
    RUN_TEST(testIsValidCharacterSet);
    RUN_TEST(testSanitizeReplaces);
    RUN_TEST(testSanitizeTruncates);
    RUN_TEST(testSanitizeKeepsTooShort);
    RUN_TEST(testMakeDefault);

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
 * Test the accepted alias length boundaries.
 */
static void testIsValidLength()
{
    TEST_ASSERT_FALSE(PluginAlias::isValid(""));
    TEST_ASSERT_FALSE(PluginAlias::isValid("ab"));
    TEST_ASSERT_TRUE(PluginAlias::isValid("abc"));
    TEST_ASSERT_TRUE(PluginAlias::isValid("12345678901234567890"));   /* 20 characters */
    TEST_ASSERT_FALSE(PluginAlias::isValid("123456789012345678901")); /* 21 characters */
}

/**
 * Test the accepted alias character set.
 */
static void testIsValidCharacterSet()
{
    TEST_ASSERT_TRUE(PluginAlias::isValid("Clock_1"));
    TEST_ASSERT_TRUE(PluginAlias::isValid("my-clock"));
    TEST_ASSERT_FALSE(PluginAlias::isValid("my clock"));
    TEST_ASSERT_FALSE(PluginAlias::isValid("my.clock"));
    TEST_ASSERT_FALSE(PluginAlias::isValid("my/clock"));
    TEST_ASSERT_FALSE(PluginAlias::isValid("my+clock"));
    TEST_ASSERT_FALSE(PluginAlias::isValid("my#clock"));
}

/**
 * Test that sanitizing replaces forbidden characters instead of removing them.
 */
static void testSanitizeReplaces()
{
    TEST_ASSERT_EQUAL_STRING("My_Clock", PluginAlias::sanitize("My Clock").c_str());
    TEST_ASSERT_EQUAL_STRING("___", PluginAlias::sanitize("+++").c_str());
    TEST_ASSERT_TRUE(PluginAlias::isValid(PluginAlias::sanitize("+++")));

    /* An umlaut is two bytes in UTF-8 and therefore becomes two underscores. */
    TEST_ASSERT_EQUAL_STRING("Gr__n", PluginAlias::sanitize("Gr\xC3\xBCn").c_str());
}

/**
 * Test that sanitizing truncates to the maximum length.
 */
static void testSanitizeTruncates()
{
    TEST_ASSERT_EQUAL_STRING("12345678901234567890", PluginAlias::sanitize("123456789012345678901234").c_str());
}

/**
 * Test that sanitizing does not repair a too short alias.
 */
static void testSanitizeKeepsTooShort()
{
    TEST_ASSERT_EQUAL_STRING("ab", PluginAlias::sanitize("ab").c_str());
    TEST_ASSERT_FALSE(PluginAlias::isValid(PluginAlias::sanitize("ab")));
}

/**
 * Test the generated default alias.
 */
static void testMakeDefault()
{
    TEST_ASSERT_EQUAL_STRING("DateTime1", PluginAlias::makeDefault("DateTimePlugin", 1U).c_str());
    TEST_ASSERT_EQUAL_STRING("IconText2", PluginAlias::makeDefault("IconTextPlugin", 2U).c_str());
    TEST_ASSERT_EQUAL_STRING("ChicagoBusTracker1", PluginAlias::makeDefault("ChicagoBusTrackerPlugin", 1U).c_str());

    /* A type name without the suffix is used as it is. */
    TEST_ASSERT_EQUAL_STRING("Foobar3", PluginAlias::makeDefault("Foobar", 3U).c_str());

    /* A nullptr must not crash and must yield a valid alias. */
    TEST_ASSERT_TRUE(PluginAlias::isValid(PluginAlias::makeDefault(nullptr, 1U)));

    /* A very long type name is truncated so that the index still fits. */
    TEST_ASSERT_EQUAL(PluginAlias::MAX_LENGTH,
        PluginAlias::makeDefault("AVeryLongPluginTypeNamePlugin", 7U).length());
    TEST_ASSERT_TRUE(PluginAlias::isValid(PluginAlias::makeDefault("AVeryLongPluginTypeNamePlugin", 7U)));

    /* A type name that is only the suffix still yields a valid alias. */
    TEST_ASSERT_TRUE(PluginAlias::isValid(PluginAlias::makeDefault("Plugin", 1U)));

    /* A very short remainder is padded, so the result is never too short. */
    TEST_ASSERT_TRUE(PluginAlias::isValid(PluginAlias::makeDefault("XPlugin", 1U)));
}
