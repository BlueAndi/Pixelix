/* MIT License
 *
 * Copyright (c) 2019 - 2022 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Main test entry point
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <unity.h>

#include "TestLogger.h"
#include "TestGfx.h"
#include "TestDoublyLinkedList.h"
#include "TestGfxText.h"
#include "TestWidget.h"
#include "TestWidgetGroup.h"
#include "TestLampWidget.h"
#include "TestBitmapWidget.h"
#include "TestTextWidget.h"
#include "TestColor.h"
#include "TestStateMachine.h"
#include "TestSimpleTimer.h"
#include "TestProgressBar.h"
#include "TestLogging.h"
#include "TestUtil.h"
#include "TestBmpImgLoader.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/******************************************************************************
 * Prototypes
 *****************************************************************************/

/******************************************************************************
 * Variables
 *****************************************************************************/

/******************************************************************************
 * External functions
 *****************************************************************************/

/**
 * Main entry point
 *
 * @param[in] argc  Number of command line arguments
 * @param[in] argv  Command line arguments
 */
int main(int argc, char **argv)
{
    UTIL_NOT_USED(argc);
    UTIL_NOT_USED(argv);

    UNITY_BEGIN();

    RUN_TEST(testDoublyLinkedList);
    RUN_TEST(testGfx);
    RUN_TEST(testGfxText);
    RUN_TEST(testWidget);
    RUN_TEST(testWidgetGroup);
    RUN_TEST(testLampWidget);
    RUN_TEST(testBmpImgLoader);
    RUN_TEST(testBitmapWidget);
    RUN_TEST(testTextWidget);
    RUN_TEST(testColor);
    RUN_TEST(testStateMachine);
    RUN_TEST(testSimpleTimer);
    RUN_TEST(testProgressBar);
    RUN_TEST(testLogging);
    RUN_TEST(testUtil);

    return UNITY_END();
}

/**
 * Setup a test. This function will be called before every test by unity.
 */
void setUp(void)
{
    /* Not used. */
}

/**
 * Clean up test. This function will be called after every test by unity.
 */
void tearDown(void)
{
    /* Not used. */
}

/******************************************************************************
 * Local functions
 *****************************************************************************/
