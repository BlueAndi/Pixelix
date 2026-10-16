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
 * @file   TestIconTextView.cpp
 * @brief  Test the icon and text view layouts.
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <unity.h>
#include <IconTextViewBase.hpp>
#include <ColorDef.hpp>
#include <Util.h>
#include <YAGfxBitmap.h>

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

/**
 * The view under test.
 *
 * The bitmap widget is protected, which lets the test provide an icon from
 * memory instead of from the filesystem.
 */
class TestView : public IconTextView<LAYOUT_32X8>
{
public:

    /**
     * Show an icon which is already in memory.
     *
     * It replaces loadIcon(), which would need the icon in the filesystem.
     *
     * @param[in] bitmap    The icon.
     */
    void setIcon(const YAGfxBitmap& bitmap)
    {
        m_bitmapWidget.set(bitmap);

        if (false == m_isIconVisible)
        {
            m_isIconVisible = true;

            applyScrollLayout();
        }
    }
};

/******************************************************************************
 * Prototypes
 *****************************************************************************/

static void testIconAndText();
static void testIconOnly();
static void testTextOnly();
static void testIconOnlyFullSize();

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/** Width of the icon used by the tests in pixel. */
static const uint16_t ICON_WIDTH  = 8U;

/** Height of the icon used by the tests in pixel. */
static const uint16_t ICON_HEIGHT = 8U;

/** Color of the icon used by the tests. */
static const Color ICON_COLOR     = ColorDef::RED;

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
 *
 * @return Test result
 */
extern int main(int argc, char** argv)
{
    UTIL_NOT_USED(argc);
    UTIL_NOT_USED(argv);

    UNITY_BEGIN();

    RUN_TEST(testIconAndText);
    RUN_TEST(testIconOnly);
    RUN_TEST(testTextOnly);
    RUN_TEST(testIconOnlyFullSize);

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
 * Get the bounding box of everything which is not black.
 *
 * @param[in]   gfx     The graphics to inspect.
 * @param[out]  left    Left column of the bounding box.
 * @param[out]  top     Top row of the bounding box.
 * @param[out]  right   Right column of the bounding box.
 * @param[out]  bottom  Bottom row of the bounding box.
 *
 * @return If anything is drawn, it will return true otherwise false.
 */
static bool getDrawnArea(YAGfxTest& gfx, int16_t& left, int16_t& top, int16_t& right, int16_t& bottom)
{
    bool    isAnythingDrawn = false;
    int16_t x               = 0;
    int16_t y               = 0;

    left                    = static_cast<int16_t>(gfx.getWidth());
    top                     = static_cast<int16_t>(gfx.getHeight());
    right                   = -1;
    bottom                  = -1;

    for (y = 0; y < static_cast<int16_t>(gfx.getHeight()); ++y)
    {
        for (x = 0; x < static_cast<int16_t>(gfx.getWidth()); ++x)
        {
            if (ColorDef::BLACK != gfx.getColor(x, y))
            {
                if (x < left)
                {
                    left = x;
                }

                if (y < top)
                {
                    top = y;
                }

                if (x > right)
                {
                    right = x;
                }

                if (y > bottom)
                {
                    bottom = y;
                }

                isAnythingDrawn = true;
            }
        }
    }

    return isAnythingDrawn;
}

/**
 * An icon together with a text is shown left aligned in the icon area.
 */
static void testIconAndText()
{
    YAGfxTest                                  gfx;
    TestView                                   view;
    YAGfxStaticBitmap<ICON_WIDTH, ICON_HEIGHT> icon;
    int16_t                                    left   = 0;
    int16_t                                    top    = 0;
    int16_t                                    right  = 0;
    int16_t                                    bottom = 0;

    icon.fillScreen(ICON_COLOR);

    view.init(gfx.getWidth(), gfx.getHeight());
    view.setIcon(icon);
    view.setFormatText("T");
    view.update(gfx);

    TEST_ASSERT_TRUE(getDrawnArea(gfx, left, top, right, bottom));

    /* The icon occupies the left 8 pixels, the text follows right of it. */
    TEST_ASSERT_EQUAL_INT16(0, left);
    TEST_ASSERT_EQUAL_INT16(0, top);
    TEST_ASSERT_EQUAL_INT16(static_cast<int16_t>(ICON_HEIGHT - 1U), bottom);
}

/**
 * An icon without a text is centered over the whole display.
 */
static void testIconOnly()
{
    YAGfxTest                                  gfx;
    TestView                                   view;
    YAGfxStaticBitmap<ICON_WIDTH, ICON_HEIGHT> icon;
    const int16_t                              EXPECTED_LEFT = (CONFIG_LED_MATRIX_WIDTH - ICON_WIDTH) / 2;
    int16_t                                    left          = 0;
    int16_t                                    top           = 0;
    int16_t                                    right         = 0;
    int16_t                                    bottom        = 0;

    icon.fillScreen(ICON_COLOR);

    view.init(gfx.getWidth(), gfx.getHeight());
    view.setIcon(icon);
    view.setFormatText("");
    view.update(gfx);

    TEST_ASSERT_TRUE(getDrawnArea(gfx, left, top, right, bottom));

    TEST_ASSERT_EQUAL_INT16(EXPECTED_LEFT, left);
    TEST_ASSERT_EQUAL_INT16(EXPECTED_LEFT + static_cast<int16_t>(ICON_WIDTH) - 1, right);
    TEST_ASSERT_EQUAL_INT16(0, top);
    TEST_ASSERT_EQUAL_INT16(static_cast<int16_t>(ICON_HEIGHT - 1U), bottom);
}

/**
 * A text without an icon uses the full display width.
 */
static void testTextOnly()
{
    const uint32_t MAX_UPDATES = 100U;
    YAGfxTest      gfx;
    TestView       view;
    uint32_t       updateCnt = 0U;
    bool           isDrawn   = false;
    int16_t        left      = 0;
    int16_t        top       = 0;
    int16_t        right     = 0;
    int16_t        bottom    = 0;

    view.init(gfx.getWidth(), gfx.getHeight());
    view.setFormatText("T");

    /* The text fades in, therefore it takes several updates until it shows up. */
    while ((MAX_UPDATES > updateCnt) && (false == isDrawn))
    {
        view.update(gfx);

        isDrawn = getDrawnArea(gfx, left, top, right, bottom);

        ++updateCnt;
    }

    TEST_ASSERT_TRUE(isDrawn);

    /* Without an icon the text starts at the very left. */
    TEST_ASSERT_EQUAL_INT16(0, left);
}

/**
 * A display filling icon without a text is shown completely and not cropped.
 * This is what the startup logo relies on.
 */
static void testIconOnlyFullSize()
{
    YAGfxTest                                                            gfx;
    TestView                                                             view;
    YAGfxStaticBitmap<CONFIG_LED_MATRIX_WIDTH, CONFIG_LED_MATRIX_HEIGHT> logo;
    int16_t                                                              left   = 0;
    int16_t                                                              top    = 0;
    int16_t                                                              right  = 0;
    int16_t                                                              bottom = 0;

    logo.fillScreen(ICON_COLOR);

    view.init(gfx.getWidth(), gfx.getHeight());
    view.setIcon(logo);
    view.setFormatText("");
    view.update(gfx);

    TEST_ASSERT_TRUE(getDrawnArea(gfx, left, top, right, bottom));

    TEST_ASSERT_EQUAL_INT16(0, left);
    TEST_ASSERT_EQUAL_INT16(0, top);
    TEST_ASSERT_EQUAL_INT16(static_cast<int16_t>(CONFIG_LED_MATRIX_WIDTH - 1U), right);
    TEST_ASSERT_EQUAL_INT16(static_cast<int16_t>(CONFIG_LED_MATRIX_HEIGHT - 1U), bottom);
}
