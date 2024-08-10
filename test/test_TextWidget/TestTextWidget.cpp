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
 * @brief  Test text widget.
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <unity.h>
#include <TWTokenizer.h>
#include <TextWidget.h>
#include <Util.h>

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

static void testTokenizer();
static void testTextWidget();

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

    RUN_TEST(testTokenizer);
    RUN_TEST(testTextWidget);

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
 * Test the text widget tokenizer.
 */
static void testTokenizer()
{
    TWTokenizer             tokenizer;
    TWAbstractSyntaxTree    ast;
    TWAbstractSyntaxTree    ast2;

    /* Empty string */
    TEST_ASSERT_EQUAL(true, tokenizer.parse(ast, ""));
    TEST_ASSERT_EQUAL(0U, ast.length());

    /* Only keyword */
    TEST_ASSERT_EQUAL(true, tokenizer.parse(ast, "{abc}"));
    TEST_ASSERT_EQUAL(1U, ast.length());
    TEST_ASSERT_EQUAL(TWToken::TYPE_KEYWORD, ast[0U].getType());
    TEST_ASSERT_EQUAL_STRING("{abc}", ast[0U].getStr().c_str());

    /* Only text */
    TEST_ASSERT_EQUAL(true, tokenizer.parse(ast, "abc"));
    TEST_ASSERT_EQUAL(1U, ast.length());
    TEST_ASSERT_EQUAL(TWToken::TYPE_TEXT, ast[0U].getType());
    TEST_ASSERT_EQUAL_STRING("abc", ast[0U].getStr().c_str());

    /* Only line feed */
    TEST_ASSERT_EQUAL(true, tokenizer.parse(ast, "\n"));
    TEST_ASSERT_EQUAL(1U, ast.length());
    TEST_ASSERT_EQUAL(TWToken::TYPE_LINE_FEED, ast[0U].getType());
    TEST_ASSERT_EQUAL_STRING("\n", ast[0U].getStr().c_str());

    /* Text with escape of character */
    TEST_ASSERT_EQUAL(true, tokenizer.parse(ast, "a\\b"));
    TEST_ASSERT_EQUAL(1U, ast.length());
    TEST_ASSERT_EQUAL(TWToken::TYPE_TEXT, ast[0U].getType());
    TEST_ASSERT_EQUAL_STRING("ab", ast[0U].getStr().c_str());

    /* Text with escaped {} */
    TEST_ASSERT_EQUAL(true, tokenizer.parse(ast, "a\\{b\\}"));
    TEST_ASSERT_EQUAL(1U, ast.length());
    TEST_ASSERT_EQUAL(TWToken::TYPE_TEXT, ast[0U].getType());
    TEST_ASSERT_EQUAL_STRING("a{b}", ast[0U].getStr().c_str());

    /* Order: keyword, text */
    TEST_ASSERT_EQUAL(true, tokenizer.parse(ast, "{a}b"));
    TEST_ASSERT_EQUAL(2U, ast.length());
    TEST_ASSERT_EQUAL(TWToken::TYPE_KEYWORD, ast[0U].getType());
    TEST_ASSERT_EQUAL(TWToken::TYPE_TEXT, ast[1U].getType());
    TEST_ASSERT_EQUAL_STRING("{a}", ast[0U].getStr().c_str());
    TEST_ASSERT_EQUAL_STRING("b", ast[1U].getStr().c_str());

    /* Order: keyword, keyword, text */
    TEST_ASSERT_EQUAL(true, tokenizer.parse(ast, "{a}{b}c"));
    TEST_ASSERT_EQUAL(3U, ast.length());
    TEST_ASSERT_EQUAL(TWToken::TYPE_KEYWORD, ast[0U].getType());
    TEST_ASSERT_EQUAL(TWToken::TYPE_KEYWORD, ast[1U].getType());
    TEST_ASSERT_EQUAL(TWToken::TYPE_TEXT, ast[2U].getType());
    TEST_ASSERT_EQUAL_STRING("{a}", ast[0U].getStr().c_str());
    TEST_ASSERT_EQUAL_STRING("{b}", ast[1U].getStr().c_str());
    TEST_ASSERT_EQUAL_STRING("c", ast[2U].getStr().c_str());

    /* Order: keyword, text, keyword, text */
    TEST_ASSERT_EQUAL(true, tokenizer.parse(ast, "{a}b{c}d"));
    TEST_ASSERT_EQUAL(4U, ast.length());
    TEST_ASSERT_EQUAL(TWToken::TYPE_KEYWORD, ast[0U].getType());
    TEST_ASSERT_EQUAL(TWToken::TYPE_TEXT, ast[1U].getType());
    TEST_ASSERT_EQUAL(TWToken::TYPE_KEYWORD, ast[2U].getType());
    TEST_ASSERT_EQUAL(TWToken::TYPE_TEXT, ast[3U].getType());
    TEST_ASSERT_EQUAL_STRING("{a}", ast[0U].getStr().c_str());
    TEST_ASSERT_EQUAL_STRING("b", ast[1U].getStr().c_str());
    TEST_ASSERT_EQUAL_STRING("{c}", ast[2U].getStr().c_str());
    TEST_ASSERT_EQUAL_STRING("d", ast[3U].getStr().c_str());

    /* Order: keyword, text, line feed, keyword, text */
    TEST_ASSERT_EQUAL(true, tokenizer.parse(ast, "{a}b\n{c}d"));
    TEST_ASSERT_EQUAL(5U, ast.length());
    TEST_ASSERT_EQUAL(TWToken::TYPE_KEYWORD, ast[0U].getType());
    TEST_ASSERT_EQUAL(TWToken::TYPE_TEXT, ast[1U].getType());
    TEST_ASSERT_EQUAL(TWToken::TYPE_LINE_FEED, ast[2U].getType());
    TEST_ASSERT_EQUAL(TWToken::TYPE_KEYWORD, ast[3U].getType());
    TEST_ASSERT_EQUAL(TWToken::TYPE_TEXT, ast[4U].getType());
    TEST_ASSERT_EQUAL_STRING("{a}", ast[0U].getStr().c_str());
    TEST_ASSERT_EQUAL_STRING("b", ast[1U].getStr().c_str());
    TEST_ASSERT_EQUAL_STRING("\n", ast[2U].getStr().c_str());
    TEST_ASSERT_EQUAL_STRING("{c}", ast[3U].getStr().c_str());
    TEST_ASSERT_EQUAL_STRING("d", ast[4U].getStr().c_str());

    /* Text move operator */
    TEST_ASSERT_EQUAL(true, tokenizer.parse(ast, "a{b}"));
    TEST_ASSERT_EQUAL(2U, ast.length());
    TEST_ASSERT_EQUAL(0U, ast2.length());
    ast2 = std::move(ast);
    TEST_ASSERT_EQUAL(0U, ast.length());
    TEST_ASSERT_EQUAL(2U, ast2.length());
    TEST_ASSERT_EQUAL_STRING("a", ast2[0U].getStr().c_str());
    TEST_ASSERT_EQUAL_STRING("{b}", ast2[1U].getStr().c_str());
}

/**
 * Test text widget.
 */
static void testTextWidget()
{
    YAGfxTest   testGfx;
    TextWidget  textWidget(YAGfxTest::WIDTH, YAGfxTest::HEIGHT);
    String      testStr     = "test";
    const Color TEXT_COLOR  = 0x123456;
    const char* WIDGET_NAME = "textWidgetName";

    /* Verify widget type name */
    TEST_ASSERT_EQUAL_STRING(TextWidget::WIDGET_TYPE, textWidget.getType());

    /* No widget name is set, it must be empty. */
    TEST_ASSERT_EQUAL_STRING("", textWidget.getName().c_str());

    /* Set widget name and read back. */
    textWidget.setName(WIDGET_NAME);
    TEST_ASSERT_EQUAL_STRING(WIDGET_NAME, textWidget.getName().c_str());

    /* Find widget with empty name.
     * Expected: Not found
     */
    TEST_ASSERT_NULL(textWidget.find(""));

    /* Find widget with its name.
     * Expected: Widget is found
     */
    TEST_ASSERT_NOT_NULL(textWidget.find(WIDGET_NAME));
    TEST_ASSERT_EQUAL_PTR(&textWidget, textWidget.find(WIDGET_NAME));

    /* Default string is empty */
    TEST_ASSERT_EQUAL_STRING("", textWidget.getStr().c_str());

    /* Set/Get string */
    textWidget.setFormatStr(testStr);
    TEST_ASSERT_EQUAL_STRING(testStr.c_str(), textWidget.getStr().c_str());

    /* Default string color */
    TEST_ASSERT_EQUAL_UINT32(TextWidget::DEFAULT_TEXT_COLOR, textWidget.getTextColor());

    /* Set/Get text color */
    textWidget.setTextColor(TEXT_COLOR);
    TEST_ASSERT_EQUAL_UINT32(TEXT_COLOR, textWidget.getTextColor());

    /* Check for default font */
    TEST_ASSERT_NOT_NULL(textWidget.getFont().getGfxFont());
    TEST_ASSERT_EQUAL_PTR(TextWidget::DEFAULT_FONT.getGfxFont(), textWidget.getFont().getGfxFont());

    /* Font shall be used for drawing */
    textWidget.update(testGfx);
    TEST_ASSERT_NOT_NULL(textWidget.getFont().getGfxFont());
    TEST_ASSERT_EQUAL_PTR(TextWidget::DEFAULT_FONT.getGfxFont(), textWidget.getFont().getGfxFont());

    /* Set text with valid format keyword and get text without format keyword back. */
    textWidget.setFormatStr("{hl}Hello World!");
    TEST_ASSERT_EQUAL_STRING("Hello World!", textWidget.getStr().c_str());

    textWidget.setFormatStr("{hc}Hello World!");
    TEST_ASSERT_EQUAL_STRING("Hello World!", textWidget.getStr().c_str());

    textWidget.setFormatStr("{hr}Hello World!");
    TEST_ASSERT_EQUAL_STRING("Hello World!", textWidget.getStr().c_str());

    textWidget.setFormatStr("{vt}Hello World!");
    TEST_ASSERT_EQUAL_STRING("Hello World!", textWidget.getStr().c_str());

    textWidget.setFormatStr("{vc}Hello World!");
    TEST_ASSERT_EQUAL_STRING("Hello World!", textWidget.getStr().c_str());

    textWidget.setFormatStr("{vb}Hello World!");
    TEST_ASSERT_EQUAL_STRING("Hello World!", textWidget.getStr().c_str());

    textWidget.setFormatStr("{#FF00FF}Hello World!");
    TEST_ASSERT_EQUAL_STRING("Hello World!", textWidget.getStr().c_str());

    /* Set text with invalid format keyword and get text without format keyword back. */
    textWidget.setFormatStr("{abcd}Hello World!");
    TEST_ASSERT_EQUAL_STRING("Hello World!", textWidget.getStr().c_str());

    textWidget.setFormatStr("{abcd}Hello {abcd}World!");
    TEST_ASSERT_EQUAL_STRING("Hello World!", textWidget.getStr().c_str());

    /* Set text with escaped format keyword and get text with format keyword back. */
    textWidget.setFormatStr("\\{#FF00FF\\}Hello World!");
    TEST_ASSERT_EQUAL_STRING("{#FF00FF}Hello World!", textWidget.getStr().c_str());

    /* Set text with character coe format keyword and get text without format keyword back. */
    textWidget.setFormatStr("{0x41} Hello World!");
    TEST_ASSERT_EQUAL_STRING("A Hello World!", textWidget.getStr().c_str());
}
