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
 * @file   TestColor.cpp
 * @brief  Test color.
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <unity.h>
#include <Rgb888.h>
#include <Rgb565.h>
#include <ColorUtil.hpp>
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

static void testColorUtil();
static void testColor888();
static void testColor565();

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
extern int main(int argc, char** argv)
{
    UTIL_NOT_USED(argc);
    UTIL_NOT_USED(argv);

    UNITY_BEGIN();

    RUN_TEST(testColorUtil);
    RUN_TEST(testColor888);
    RUN_TEST(testColor565);

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
 * Test color utility functions.
 */
static void testColorUtil()
{
    TEST_ASSERT_EQUAL_UINT8(0xffU, ColorUtil::rgb888Red(ColorDef::WHITE));
    TEST_ASSERT_EQUAL_UINT8(0x00U, ColorUtil::rgb888Red(ColorDef::BLACK));
    TEST_ASSERT_EQUAL_UINT8(0xffU, ColorUtil::rgb888Red(ColorDef::RED));
    TEST_ASSERT_EQUAL_UINT8(0x00U, ColorUtil::rgb888Red(ColorDef::GREEN));
    TEST_ASSERT_EQUAL_UINT8(0x00U, ColorUtil::rgb888Red(ColorDef::BLUE));

    TEST_ASSERT_EQUAL_UINT8(0xffU, ColorUtil::rgb888Green(ColorDef::WHITE));
    TEST_ASSERT_EQUAL_UINT8(0x00U, ColorUtil::rgb888Green(ColorDef::BLACK));
    TEST_ASSERT_EQUAL_UINT8(0x00U, ColorUtil::rgb888Green(ColorDef::RED));
    TEST_ASSERT_EQUAL_UINT8(0x80U, ColorUtil::rgb888Green(ColorDef::GREEN));
    TEST_ASSERT_EQUAL_UINT8(0x00U, ColorUtil::rgb888Green(ColorDef::BLUE));

    TEST_ASSERT_EQUAL_UINT8(0xffU, ColorUtil::rgb888Blue(ColorDef::WHITE));
    TEST_ASSERT_EQUAL_UINT8(0x00U, ColorUtil::rgb888Blue(ColorDef::BLACK));
    TEST_ASSERT_EQUAL_UINT8(0x00U, ColorUtil::rgb888Blue(ColorDef::RED));
    TEST_ASSERT_EQUAL_UINT8(0x00U, ColorUtil::rgb888Blue(ColorDef::GREEN));
    TEST_ASSERT_EQUAL_UINT8(0xffU, ColorUtil::rgb888Blue(ColorDef::BLUE));

    TEST_ASSERT_EQUAL_UINT8(0xf8U, ColorUtil::rgb565Red(0xffffU));
    TEST_ASSERT_EQUAL_UINT8(0x00U, ColorUtil::rgb565Red(0x0000U));
    TEST_ASSERT_EQUAL_UINT8(0xf8U, ColorUtil::rgb565Red(0xf800U));
    TEST_ASSERT_EQUAL_UINT8(0x00U, ColorUtil::rgb565Red(0x07e0U));
    TEST_ASSERT_EQUAL_UINT8(0x00U, ColorUtil::rgb565Red(0x001fU));

    TEST_ASSERT_EQUAL_UINT8(0xfcU, ColorUtil::rgb565Green(0xffffU));
    TEST_ASSERT_EQUAL_UINT8(0x00U, ColorUtil::rgb565Green(0x0000U));
    TEST_ASSERT_EQUAL_UINT8(0x00U, ColorUtil::rgb565Green(0xf800U));
    TEST_ASSERT_EQUAL_UINT8(0xfcU, ColorUtil::rgb565Green(0x07e0U));
    TEST_ASSERT_EQUAL_UINT8(0x00U, ColorUtil::rgb565Green(0x001fU));

    TEST_ASSERT_EQUAL_UINT8(0xf8U, ColorUtil::rgb565Blue(0xffffU));
    TEST_ASSERT_EQUAL_UINT8(0x00U, ColorUtil::rgb565Blue(0x0000U));
    TEST_ASSERT_EQUAL_UINT8(0x00U, ColorUtil::rgb565Blue(0xf800U));
    TEST_ASSERT_EQUAL_UINT8(0x00U, ColorUtil::rgb565Blue(0x07e0U));
    TEST_ASSERT_EQUAL_UINT8(0xf8U, ColorUtil::rgb565Blue(0x001fU));

    TEST_ASSERT_EQUAL_UINT16(0x0821U, ColorUtil::to565(0x00080408U));
    TEST_ASSERT_EQUAL_UINT16(0xffffU, ColorUtil::to565(ColorDef::WHITE));
    TEST_ASSERT_EQUAL_UINT16(0x0000U, ColorUtil::to565(ColorDef::BLACK));
    TEST_ASSERT_EQUAL_UINT16(0xf800U, ColorUtil::to565(ColorDef::RED));
    TEST_ASSERT_EQUAL_UINT16(0x0400U, ColorUtil::to565(ColorDef::GREEN));
    TEST_ASSERT_EQUAL_UINT16(0x001fU, ColorUtil::to565(ColorDef::BLUE));

    TEST_ASSERT_EQUAL_UINT32(0x00080408U, ColorUtil::to888(0x0821U));
}

/**
 * Test RGB888 color.
 */
static void testColor888()
{
    Rgb888 myColorA;
    Rgb888 myColorB = ColorDef::TOMATO;
    Rgb888 myColorC = myColorB;

    /* Default color is black */
    TEST_ASSERT_EQUAL_UINT32(0u, myColorA);

    /* Does the color assignment works? */
    TEST_ASSERT_EQUAL_UINT8(ColorUtil::rgb888Red(ColorDef::TOMATO), myColorB.getRed());
    TEST_ASSERT_EQUAL_UINT8(ColorUtil::rgb888Green(ColorDef::TOMATO), myColorB.getGreen());
    TEST_ASSERT_EQUAL_UINT8(ColorUtil::rgb888Blue(ColorDef::TOMATO), myColorB.getBlue());

    /* Does the color assignment via copy constructor works? */
    TEST_ASSERT_EQUAL_UINT8(ColorUtil::rgb888Red(ColorDef::TOMATO), myColorC.getRed());
    TEST_ASSERT_EQUAL_UINT8(ColorUtil::rgb888Green(ColorDef::TOMATO), myColorC.getGreen());
    TEST_ASSERT_EQUAL_UINT8(ColorUtil::rgb888Blue(ColorDef::TOMATO), myColorC.getBlue());

    /* Check the 5-6-5 RGB format conversion. */
    myColorA.set(ColorDef::WHITE);
    TEST_ASSERT_EQUAL_UINT8(0xffu, myColorA.getRed());
    TEST_ASSERT_EQUAL_UINT8(0xffu, myColorA.getGreen());
    TEST_ASSERT_EQUAL_UINT8(0xffu, myColorA.getBlue());
    TEST_ASSERT_EQUAL_UINT16(0xffffu, static_cast<uint16_t>(myColorA));

    myColorA.set(0x00080408U);
    TEST_ASSERT_EQUAL_UINT8(0x08u, myColorA.getRed());
    TEST_ASSERT_EQUAL_UINT8(0x04u, myColorA.getGreen());
    TEST_ASSERT_EQUAL_UINT8(0x08u, myColorA.getBlue());
    TEST_ASSERT_EQUAL_UINT16(0x0821u, myColorA.toRgb565());

    /* Does the color assignment via assignment operator works? */
    myColorA = myColorB;
    TEST_ASSERT_EQUAL_UINT8(myColorB.getRed(), myColorC.getRed());
    TEST_ASSERT_EQUAL_UINT8(myColorB.getGreen(), myColorC.getGreen());
    TEST_ASSERT_EQUAL_UINT8(myColorB.getBlue(), myColorC.getBlue());

    /* Get/Set single colors */
    myColorA.setRed(0x12U);
    myColorA.setGreen(0x34U);
    myColorA.setBlue(0x56U);
    TEST_ASSERT_EQUAL_UINT8(0x12u, myColorA.getRed());
    TEST_ASSERT_EQUAL_UINT8(0x34u, myColorA.getGreen());
    TEST_ASSERT_EQUAL_UINT8(0x56u, myColorA.getBlue());

    /* Dim color 25% darker */
    myColorA = 0xc8c8c8u;
    myColorA.setIntensity(192);
    TEST_ASSERT_EQUAL_UINT8(0x96u, myColorA.getRed());
    TEST_ASSERT_EQUAL_UINT8(0x96u, myColorA.getGreen());
    TEST_ASSERT_EQUAL_UINT8(0x96u, myColorA.getBlue());

    /* Dim a color by 0%, which means no change.
     * And additional check non-destructive base colors.
     */
    myColorA.setIntensity(255);
    TEST_ASSERT_EQUAL_UINT8(0xc8u, myColorA.getRed());
    TEST_ASSERT_EQUAL_UINT8(0xc8u, myColorA.getGreen());
    TEST_ASSERT_EQUAL_UINT8(0xc8u, myColorA.getBlue());
}

/**
 * Test RGB565 color.
 */
static void testColor565()
{
    Rgb565 myColorA;
    Rgb565 myColorB;

    myColorB.fromRgb565(0x0821);

    Rgb565 myColorC = myColorB;

    /* Default color is black */
    TEST_ASSERT_EQUAL_UINT32(0u, myColorA);

    /* Does the color assignment works? */
    TEST_ASSERT_EQUAL_UINT8(ColorUtil::rgb888Red(0x00080408U), myColorB.getRed());
    TEST_ASSERT_EQUAL_UINT8(ColorUtil::rgb888Green(0x00080408U), myColorB.getGreen());
    TEST_ASSERT_EQUAL_UINT8(ColorUtil::rgb888Blue(0x00080408U), myColorB.getBlue());

    /* Does the color assignment via copy constructor works? */
    TEST_ASSERT_EQUAL_UINT8(ColorUtil::rgb888Red(0x00080408U), myColorC.getRed());
    TEST_ASSERT_EQUAL_UINT8(ColorUtil::rgb888Green(0x00080408U), myColorC.getGreen());
    TEST_ASSERT_EQUAL_UINT8(ColorUtil::rgb888Blue(0x00080408U), myColorC.getBlue());

    /* Check the 5-6-5 RGB format conversion. */
    myColorA.set(ColorDef::WHITE);
    TEST_ASSERT_EQUAL_UINT8(0xf8u, myColorA.getRed());
    TEST_ASSERT_EQUAL_UINT8(0xfcu, myColorA.getGreen());
    TEST_ASSERT_EQUAL_UINT8(0xf8u, myColorA.getBlue());
    TEST_ASSERT_EQUAL_UINT16(0xffffu, myColorA.toRgb565());

    myColorA.set(0x00080408U);
    TEST_ASSERT_EQUAL_UINT8(0x08u, myColorA.getRed());
    TEST_ASSERT_EQUAL_UINT8(0x04u, myColorA.getGreen());
    TEST_ASSERT_EQUAL_UINT8(0x08u, myColorA.getBlue());
    TEST_ASSERT_EQUAL_UINT16(0x0821u, myColorA.toRgb565());

    /* Does the color assignment via assignment operator works? */
    myColorA = myColorB;
    TEST_ASSERT_EQUAL_UINT8(myColorB.getRed(), myColorC.getRed());
    TEST_ASSERT_EQUAL_UINT8(myColorB.getGreen(), myColorC.getGreen());
    TEST_ASSERT_EQUAL_UINT8(myColorB.getBlue(), myColorC.getBlue());

    /* Get/Set single colors */
    myColorA.setRed(0x12U);
    myColorA.setGreen(0x34U);
    myColorA.setBlue(0x56U);
    TEST_ASSERT_EQUAL_UINT8(0x10u, myColorA.getRed());
    TEST_ASSERT_EQUAL_UINT8(0x34u, myColorA.getGreen());
    TEST_ASSERT_EQUAL_UINT8(0x50u, myColorA.getBlue());

    /* Dim color 25% darker */
    myColorA = 0xc8c8c8u;
    myColorA.setIntensity(192);
    TEST_ASSERT_EQUAL_UINT8(0x96u, myColorA.getRed());
    TEST_ASSERT_EQUAL_UINT8(0x96u, myColorA.getGreen());
    TEST_ASSERT_EQUAL_UINT8(0x96u, myColorA.getBlue());

    /* Dim a color by 0%, which means no change.
     * And additional check non-destructive base colors.
     */
    myColorA.setIntensity(255);
    TEST_ASSERT_EQUAL_UINT8(0xc8u, myColorA.getRed());
    TEST_ASSERT_EQUAL_UINT8(0xc8u, myColorA.getGreen());
    TEST_ASSERT_EQUAL_UINT8(0xc8u, myColorA.getBlue());
}
