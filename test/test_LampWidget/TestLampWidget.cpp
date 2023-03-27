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
 * @brief  Test lamp widget.
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <unity.h>
#include <LampWidget.h>
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

static void testLampWidget();

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

    RUN_TEST(testLampWidget);

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
 * Test lamp widget.
 */
static void testLampWidget()
{
    const Color     COLOR_OFF   = 0x111111;
    const Color     COLOR_ON    = 0x222222;
    const char*     WIDGET_NAME = "lampWidgetName";
    const uint8_t   WIDTH =  4u;

    YAGfxTest       testGfx;
    LampWidget      lampWidget(false, COLOR_OFF, COLOR_ON, WIDTH);
    int16_t         posX        = 0;
    int16_t         posY        = 0;

    /* Verify widget type name */
    TEST_ASSERT_EQUAL_STRING(LampWidget::WIDGET_TYPE, lampWidget.getType());

    /* No widget name is set, it must be empty. */
    TEST_ASSERT_EQUAL_STRING("", lampWidget.getName().c_str());

    /* Set widget name and read back. */
    lampWidget.setName(WIDGET_NAME);
    TEST_ASSERT_EQUAL_STRING(WIDGET_NAME, lampWidget.getName().c_str());

    /* Find widget with empty name.
     * Expected: Not found
     */
    TEST_ASSERT_NULL(lampWidget.find(""));

    /* Find widget with its name.
     * Expected: Widget is found
     */
    TEST_ASSERT_NOT_NULL(lampWidget.find(WIDGET_NAME));
    TEST_ASSERT_EQUAL_PTR(&lampWidget, lampWidget.find(WIDGET_NAME));

    /* Draw widget in off state and verify */
    lampWidget.update(testGfx);
    lampWidget.getPos(posX, posY);
    TEST_ASSERT_TRUE(testGfx.verify(posX,
                                    posY,
                                    LampWidget::DEFAULT_WIDTH,
                                    LampWidget::HEIGHT,
                                    COLOR_OFF));

    /* Draw widget in on state and verify */
    lampWidget.setOnState(true);
    lampWidget.update(testGfx);
    lampWidget.getPos(posX, posY);
    TEST_ASSERT_TRUE(testGfx.verify(posX,
                                    posY,
                                    LampWidget::DEFAULT_WIDTH,
                                    LampWidget::HEIGHT,
                                    COLOR_ON));

    /* Draw widget in off state and verify */
    lampWidget.setOnState(false);
    lampWidget.update(testGfx);
    lampWidget.getPos(posX, posY);
    TEST_ASSERT_TRUE(testGfx.verify(posX,
                                    posY,
                                    LampWidget::DEFAULT_WIDTH,
                                    LampWidget::HEIGHT,
                                    COLOR_OFF));

    /* Move widget and draw in off state again. */
    testGfx.fill(0);
    lampWidget.move(2,2);
    lampWidget.update(testGfx);
    lampWidget.getPos(posX, posY);
    TEST_ASSERT_TRUE(testGfx.verify(posX,
                                    posY,
                                    LampWidget::DEFAULT_WIDTH,
                                    LampWidget::HEIGHT,
                                    COLOR_OFF));

    return;
}
