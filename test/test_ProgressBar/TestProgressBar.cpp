/* MIT License
 *
 * Copyright (c) 2019 - 2023 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Test progress bar.
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <unity.h>
#include <ProgressBar.h>
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

static void testProgressBar();

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

    RUN_TEST(testProgressBar);

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
 * Test progress bar.
 */
static void testProgressBar()
{
    YAGfxTest   testGfx;
    ProgressBar progressBar;
    const char* WIDGET_NAME = "progressBarName";
    int16_t     posX        = 2;
    int16_t     posY        = 2;

    /* Verify widget type name */
    TEST_ASSERT_EQUAL_STRING(ProgressBar::WIDGET_TYPE, progressBar.getType());

    /* No widget name is set, it must be empty. */
    TEST_ASSERT_EQUAL_STRING("", progressBar.getName().c_str());

    /* Set widget name and read back. */
    progressBar.setName(WIDGET_NAME);
    TEST_ASSERT_EQUAL_STRING(WIDGET_NAME, progressBar.getName().c_str());

    /* Find widget with empty name.
     * Expected: Not found
     */
    TEST_ASSERT_NULL(progressBar.find(""));

    /* Find widget with its name.
     * Expected: Widget is found
     */
    TEST_ASSERT_NOT_NULL(progressBar.find(WIDGET_NAME));
    TEST_ASSERT_EQUAL_PTR(&progressBar, progressBar.find(WIDGET_NAME));

    /* Default algorithm: progress bar */

    /* Progress should be now 0% */
    progressBar.update(testGfx);
    TEST_ASSERT_TRUE(testGfx.verify(0, 0, testGfx.getWidth(), testGfx.getHeight(), ColorDef::BLACK));

    /* Set progress bar to 50% */
    progressBar.setProgress(50U);
    progressBar.update(testGfx);
    TEST_ASSERT_TRUE(testGfx.verify(0, 0, testGfx.getWidth() / 2u, testGfx.getHeight(), ColorDef::RED));
    TEST_ASSERT_TRUE(testGfx.verify(testGfx.getWidth() / 2u, 0, testGfx.getWidth() / 2u, testGfx.getHeight(), ColorDef::BLACK));

    /* Set progress bar to 100% */
    progressBar.setProgress(100U);
    progressBar.update(testGfx);
    TEST_ASSERT_TRUE(testGfx.verify(0, 0, testGfx.getWidth(), testGfx.getHeight(), ColorDef::RED));

    /* Clear display */
    testGfx.fill(ColorDef::BLACK);

    /* Widget must be moveable */
    progressBar.move(posX, posY);
    progressBar.setProgress(100U);
    progressBar.update(testGfx);
    TEST_ASSERT_TRUE(testGfx.verify(posX, posY, testGfx.getWidth() - posX, testGfx.getHeight() - posY, ColorDef::RED));
    progressBar.move(0, 0);

    /* Test algorithm: progress pixel wise */
    progressBar.setAlgo(ProgressBar::ALGORITHM_PIXEL_WISE);

    /* Clear display */
    testGfx.fill(ColorDef::BLACK);

    /* Set progress bar to 0% */
    progressBar.setProgress(0U);
    progressBar.update(testGfx);
    TEST_ASSERT_TRUE(testGfx.verify(0, 0, testGfx.getWidth(), testGfx.getHeight(), ColorDef::BLACK));

    /* Set progress bar to 50% */
    progressBar.setProgress(50U);
    progressBar.update(testGfx);
    TEST_ASSERT_TRUE(testGfx.verify(0, 0, testGfx.getWidth(), testGfx.getHeight() / 2u, ColorDef::RED));
    TEST_ASSERT_TRUE(testGfx.verify(0, testGfx.getHeight() / 2u, testGfx.getWidth(), testGfx.getHeight() / 2u, ColorDef::BLACK));

    /* Set progress bar to 100% */
    progressBar.setProgress(100U);
    progressBar.update(testGfx);
    TEST_ASSERT_TRUE(testGfx.verify(0, 0, testGfx.getWidth(), testGfx.getHeight(), ColorDef::RED));

    /* Clear display */
    testGfx.fill(ColorDef::BLACK);

    /* Widget must be moveable */
    progressBar.move(posX, posY);
    progressBar.setProgress(100U);
    progressBar.update(testGfx);
    TEST_ASSERT_TRUE(testGfx.verify(posX, posY, testGfx.getWidth() - posX, testGfx.getHeight() - posY, ColorDef::RED));
    progressBar.move(0, 0);
    
    return;
}
