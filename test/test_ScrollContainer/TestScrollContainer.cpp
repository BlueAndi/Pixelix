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
 * @file   TestScrollContainer.cpp
 * @brief  Test scroll container.
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <unity.h>
#include <Arduino.h>
#include <ScrollContainer.h>
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

/**
 * Widget for test purposes, which fills its complete canvas with one color.
 * Additionally it counts how often it was painted.
 */
class ColorWidget : public Widget
{
public:

    /**
     * Constructs a color widget.
     *
     * @param[in] width     Widget width in pixel.
     * @param[in] height    Widget height in pixel.
     * @param[in] x         Upper left corner (x-coordinate) in the canvas.
     * @param[in] y         Upper left corner (y-coordinate) in the canvas.
     */
    ColorWidget(uint16_t width = 1U, uint16_t height = 1U, int16_t x = 0, int16_t y = 0) :
        Widget(WIDGET_TYPE, width, height, x, y),
        m_color(0U),
        m_paintCnt(0U)
    {
    }

    /**
     * Destroys the color widget.
     */
    ~ColorWidget()
    {
    }

    /**
     * Set the color, which is used to fill the widget canvas.
     *
     * @param[in] color Fill color.
     */
    void setColor(const Color& color)
    {
        m_color = color;
    }

    /**
     * Get the number of paint calls.
     *
     * @return Number of paint calls.
     */
    uint32_t getPaintCnt() const
    {
        return m_paintCnt;
    }

    /** Widget type string. */
    static constexpr const char* WIDGET_TYPE = "colorWidget";

private:

    Color    m_color;    /**< Color, used to fill the widget canvas. */
    uint32_t m_paintCnt; /**< Counts how often the widget was painted. */

    /**
     * Paint the widget.
     *
     * @param[in] gfx   Graphics interface, which is the widget canvas.
     */
    void paint(YAGfx& gfx) override
    {
        gfx.fillRect(0, 0, getWidth(), getHeight(), m_color);

        ++m_paintCnt;
    }
};

/******************************************************************************
 * Prototypes
 *****************************************************************************/

static bool isAreaFilled(YAGfxTest& gfx, int16_t posX, int16_t posY, uint16_t width, uint16_t height, const Color& color);
static bool isOutsideOfContainerFilled(YAGfxTest& gfx, const Color& color);
static void waitForScrollPause();
static void scrollAndObserve(ScrollContainer& container, YAGfxTest& gfx, bool& isHeadShown, bool& isTailShown, bool& isOutsideKept);

static void testChildHandling();
static void testScrollingState();
static void testStaticContent();
static void testHorizontalScrolling();
static void testVerticalScrolling();

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/** Container x-coordinate in the canvas. */
static const int16_t CONTAINER_X       = 4;

/** Container y-coordinate in the canvas. */
static const int16_t CONTAINER_Y       = 2;

/** Container width in pixel, which is the visible part of the content. */
static const uint16_t CONTAINER_WIDTH  = 8U;

/** Container height in pixel, which is the visible part of the content. */
static const uint16_t CONTAINER_HEIGHT = 4U;

/** Background color, used to detect any unexpected drawing. */
static const Color COLOR_BACKGROUND    = 0x111111U;

/** Color of the first part of the content. */
static const Color COLOR_HEAD          = 0xFF0000U;

/** Color of the second part of the content, which is outside the visible area at the begin. */
static const Color COLOR_TAIL          = 0x00FF00U;

/** Max. number of scroll steps, used to avoid a infinite test loop. */
static const uint32_t MAX_SCROLL_STEPS = 200U;

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

    RUN_TEST(testChildHandling);
    RUN_TEST(testScrollingState);
    RUN_TEST(testStaticContent);
    RUN_TEST(testHorizontalScrolling);
    RUN_TEST(testVerticalScrolling);

    return UNITY_END();
}

/**
 * Setup a test. This function will be called before every test by unity.
 */
extern void setUp(void)
{
    /* The scroll pause is a global setting, therefore it is set to the fastest
     * one to keep the test duration short.
     */
    TEST_ASSERT_TRUE(ScrollContainer::setScrollPause(ScrollController::MIN_SCROLL_PAUSE));
}

/**
 * Clean up test. This function will be called after every test by unity.
 */
extern void tearDown(void)
{
    TEST_ASSERT_TRUE(ScrollContainer::setScrollPause(ScrollController::DEFAULT_SCROLL_PAUSE));
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/**
 * Is the given area filled with the given color? In contrast to
 * YAGfxTest::verify() it keeps quiet, therefore it can be used to observe
 * every single scroll step.
 *
 * @param[in] gfx       Graphics interface with the pixel buffer.
 * @param[in] posX      Top left x-coordinate.
 * @param[in] posY      Top left y-coordinate.
 * @param[in] width     Width in pixel.
 * @param[in] height    Height in pixel.
 * @param[in] color     Color which is expected.
 *
 * @return If the whole area has the given color, it will return true otherwise false.
 */
static bool isAreaFilled(YAGfxTest& gfx, int16_t posX, int16_t posY, uint16_t width, uint16_t height, const Color& color)
{
    bool     isFilled = true;
    uint16_t x;
    uint16_t y;

    for (y = 0U; (y < height) && (true == isFilled); ++y)
    {
        for (x = 0U; (x < width) && (true == isFilled); ++x)
        {
            if (color != gfx.getColor(static_cast<int16_t>(posX + x), static_cast<int16_t>(posY + y)))
            {
                isFilled = false;
            }
        }
    }

    return isFilled;
}

/**
 * Is everything outside of the container area filled with the given color?
 * Used to verify that the container never draws outside of its own area.
 *
 * @param[in] gfx   Graphics interface with the pixel buffer.
 * @param[in] color Color which is expected.
 *
 * @return If everything outside of the container has the given color, it will return true otherwise false.
 */
static bool isOutsideOfContainerFilled(YAGfxTest& gfx, const Color& color)
{
    bool    isFilled = true;
    int16_t x;
    int16_t y;

    for (y = 0; (y < static_cast<int16_t>(YAGfxTest::HEIGHT)) && (true == isFilled); ++y)
    {
        for (x = 0; (x < static_cast<int16_t>(YAGfxTest::WIDTH)) && (true == isFilled); ++x)
        {
            bool isInsideOfContainer = (CONTAINER_X <= x) &&
                                       (x < (CONTAINER_X + static_cast<int16_t>(CONTAINER_WIDTH))) &&
                                       (CONTAINER_Y <= y) &&
                                       (y < (CONTAINER_Y + static_cast<int16_t>(CONTAINER_HEIGHT)));

            if ((false == isInsideOfContainer) &&
                (color != gfx.getColor(x, y)))
            {
                isFilled = false;
            }
        }
    }

    return isFilled;
}

/**
 * Wait until the next scroll step will take place. Busy waiting is used on
 * purpose, because the native time base is derived from the process clock and
 * therefore sleeping might not increase it.
 */
static void waitForScrollPause()
{
    const uint32_t startTime = millis();

    while (ScrollController::MIN_SCROLL_PAUSE > (millis() - startTime))
    {
        ;
    }
}

/**
 * Scroll the content one complete cycle and observe what becomes visible.
 *
 * @param[in]   container       Container which to scroll.
 * @param[in]   gfx             Graphics interface, which is cleared before every step.
 * @param[out]  isHeadShown     The first part of the content was complete visible.
 * @param[out]  isTailShown     The second part of the content was complete visible.
 * @param[out]  isOutsideKept   Nothing was drawn outside of the container area.
 */
static void scrollAndObserve(ScrollContainer& container, YAGfxTest& gfx, bool& isHeadShown, bool& isTailShown, bool& isOutsideKept)
{
    uint32_t stepCnt = 0U;

    isHeadShown      = false;
    isTailShown      = false;
    isOutsideKept    = true;

    while ((0U == container.getScrollingCount()) &&
           (MAX_SCROLL_STEPS > stepCnt))
    {
        waitForScrollPause();

        gfx.fillScreen(COLOR_BACKGROUND);
        container.update(gfx);

        if (true == isAreaFilled(gfx, CONTAINER_X, CONTAINER_Y, CONTAINER_WIDTH, CONTAINER_HEIGHT, COLOR_HEAD))
        {
            isHeadShown = true;
        }

        if (true == isAreaFilled(gfx, CONTAINER_X, CONTAINER_Y, CONTAINER_WIDTH, CONTAINER_HEIGHT, COLOR_TAIL))
        {
            isTailShown = true;
        }

        if (false == isOutsideOfContainerFilled(gfx, COLOR_BACKGROUND))
        {
            isOutsideKept = false;
        }

        ++stepCnt;
    }

    /* The scroll cycle must be finished, otherwise the observation is useless. */
    TEST_ASSERT_LESS_THAN_UINT32(MAX_SCROLL_STEPS, stepCnt);
}

/**
 * Test adding and removing of child widgets.
 */
static void testChildHandling()
{
    YAGfxTest       testGfx;
    ScrollContainer container(CONTAINER_WIDTH, CONTAINER_HEIGHT, CONTAINER_X, CONTAINER_Y);
    ColorWidget     children[ScrollContainer::MAX_CHILDREN + 1U];
    uint8_t         idx;

    /* Verify widget type name. */
    TEST_ASSERT_EQUAL_STRING(ScrollContainer::WIDGET_TYPE, container.getType());

    /* The container takes up to MAX_CHILDREN child widgets. */
    for (idx = 0U; idx < ScrollContainer::MAX_CHILDREN; ++idx)
    {
        children[idx].setColor(COLOR_HEAD);

        TEST_ASSERT_TRUE(container.add(children[idx]));
    }

    /* One more child widget must be rejected. */
    TEST_ASSERT_FALSE(container.add(children[ScrollContainer::MAX_CHILDREN]));

    /* Every added child widget must be painted. */
    container.disableScrolling();
    container.update(testGfx);

    for (idx = 0U; idx < ScrollContainer::MAX_CHILDREN; ++idx)
    {
        TEST_ASSERT_EQUAL_UINT32(1U, children[idx].getPaintCnt());
    }

    TEST_ASSERT_EQUAL_UINT32(0U, children[ScrollContainer::MAX_CHILDREN].getPaintCnt());

    /* After clearing, no child widget is painted anymore and the container
     * takes new ones.
     */
    container.clear();
    container.update(testGfx);

    for (idx = 0U; idx < ScrollContainer::MAX_CHILDREN; ++idx)
    {
        TEST_ASSERT_EQUAL_UINT32(1U, children[idx].getPaintCnt());
    }

    TEST_ASSERT_TRUE(container.add(children[ScrollContainer::MAX_CHILDREN]));

    container.update(testGfx);

    TEST_ASSERT_EQUAL_UINT32(1U, children[ScrollContainer::MAX_CHILDREN].getPaintCnt());
}

/**
 * Test enabling and disabling of the scrolling.
 */
static void testScrollingState()
{
    YAGfxTest       testGfx;
    const uint16_t  SMALL_WIDTH   = 4U;
    const uint16_t  CONTENT_WIDTH = 2U * SMALL_WIDTH;
    ScrollContainer container(SMALL_WIDTH, CONTAINER_HEIGHT, CONTAINER_X, CONTAINER_Y);
    ColorWidget     content(CONTENT_WIDTH, CONTAINER_HEIGHT, 0, 0);
    bool            isScrollingEnabled = true;
    uint32_t        scrollingCnt       = UINT32_MAX;
    uint32_t        stepCnt            = 0U;

    content.setColor(COLOR_HEAD);
    (void)container.add(content);

    /* Content fits into the visible area, therefore no scrolling. */
    container.setContentSize(SMALL_WIDTH);
    container.enableScrolling(ScrollController::DIRECTION_HORIZONTAL);
    TEST_ASSERT_FALSE(container.isScrollingEnabled());

    /* Content is larger than the visible area, therefore scrolling. */
    container.setContentSize(CONTENT_WIDTH);
    container.enableScrolling(ScrollController::DIRECTION_HORIZONTAL);
    TEST_ASSERT_TRUE(container.isScrollingEnabled());

    /* The scroll information must be available. */
    TEST_ASSERT_TRUE(container.getScrollInfo(isScrollingEnabled, scrollingCnt));
    TEST_ASSERT_TRUE(isScrollingEnabled);
    TEST_ASSERT_EQUAL_UINT32(0U, scrollingCnt);

    /* Scroll one complete cycle. */
    while ((0U == container.getScrollingCount()) &&
           (MAX_SCROLL_STEPS > stepCnt))
    {
        waitForScrollPause();
        container.update(testGfx);

        ++stepCnt;
    }

    TEST_ASSERT_EQUAL_UINT32(1U, container.getScrollingCount());

    /* If the visible area changes during runtime, the scrolling must be
     * restarted. Otherwise the content would be scrolled with the offsets of
     * the previous layout.
     */
    container.setWidth(SMALL_WIDTH + 1U);
    container.enableScrolling(ScrollController::DIRECTION_HORIZONTAL);
    TEST_ASSERT_TRUE(container.isScrollingEnabled());
    TEST_ASSERT_EQUAL_UINT32(0U, container.getScrollingCount());

    /* If the visible area gets large enough, the scrolling must stop. */
    container.setWidth(CONTENT_WIDTH);
    container.enableScrolling(ScrollController::DIRECTION_HORIZONTAL);
    TEST_ASSERT_FALSE(container.isScrollingEnabled());

    /* Explicit disabling must work in any case. */
    container.setWidth(SMALL_WIDTH);
    container.enableScrolling(ScrollController::DIRECTION_HORIZONTAL);
    TEST_ASSERT_TRUE(container.isScrollingEnabled());

    container.disableScrolling();
    TEST_ASSERT_FALSE(container.isScrollingEnabled());
    TEST_ASSERT_EQUAL_UINT32(0U, container.getScrollingCount());
}

/**
 * Test the static content, which is not scrolled. The content must be clipped
 * to the container area.
 */
static void testStaticContent()
{
    YAGfxTest       testGfx;
    const uint16_t  CONTENT_WIDTH = 2U * CONTAINER_WIDTH;
    ScrollContainer container(CONTAINER_WIDTH, CONTAINER_HEIGHT, CONTAINER_X, CONTAINER_Y);
    ColorWidget     content(CONTENT_WIDTH, CONTAINER_HEIGHT, 0, 0);

    content.setColor(COLOR_HEAD);
    (void)container.add(content);

    /* The content size is larger than the container, but the scrolling is
     * disabled. The content is drawn from the begin on.
     */
    container.setContentSize(CONTENT_WIDTH);
    container.disableScrolling();

    testGfx.fillScreen(COLOR_BACKGROUND);
    container.update(testGfx);

    TEST_ASSERT_TRUE(testGfx.verify(CONTAINER_X, CONTAINER_Y, CONTAINER_WIDTH, CONTAINER_HEIGHT, COLOR_HEAD));
    TEST_ASSERT_TRUE(isOutsideOfContainerFilled(testGfx, COLOR_BACKGROUND));
}

/**
 * Test scrolling from right to left. The complete content must become visible
 * during one scroll cycle, especially the part which is outside of the visible
 * area at the begin.
 */
static void testHorizontalScrolling()
{
    YAGfxTest       testGfx;
    const uint16_t  CONTENT_WIDTH = 2U * CONTAINER_WIDTH;
    ScrollContainer container(CONTAINER_WIDTH, CONTAINER_HEIGHT, CONTAINER_X, CONTAINER_Y);
    ColorWidget     head(CONTAINER_WIDTH, CONTAINER_HEIGHT, 0, 0);
    ColorWidget     tail(CONTAINER_WIDTH, CONTAINER_HEIGHT, static_cast<int16_t>(CONTAINER_WIDTH), 0);
    bool            isHeadShown   = false;
    bool            isTailShown   = false;
    bool            isOutsideKept = false;

    head.setColor(COLOR_HEAD);
    tail.setColor(COLOR_TAIL);
    (void)container.add(head);
    (void)container.add(tail);

    container.setContentSize(CONTENT_WIDTH);
    container.enableScrolling(ScrollController::DIRECTION_HORIZONTAL);
    TEST_ASSERT_TRUE(container.isScrollingEnabled());

    /* Scrolling starts outside of the visible area, therefore nothing is shown. */
    testGfx.fillScreen(COLOR_BACKGROUND);
    container.update(testGfx);
    TEST_ASSERT_TRUE(testGfx.verify(CONTAINER_X, CONTAINER_Y, CONTAINER_WIDTH, CONTAINER_HEIGHT, COLOR_BACKGROUND));

    scrollAndObserve(container, testGfx, isHeadShown, isTailShown, isOutsideKept);

    TEST_ASSERT_TRUE(isHeadShown);
    TEST_ASSERT_TRUE(isTailShown);
    TEST_ASSERT_TRUE(isOutsideKept);
}

/**
 * Test scrolling from bottom to top. The complete content must become visible
 * during one scroll cycle, especially the part which is outside of the visible
 * area at the begin.
 */
static void testVerticalScrolling()
{
    YAGfxTest       testGfx;
    const uint16_t  CONTENT_HEIGHT = 2U * CONTAINER_HEIGHT;
    ScrollContainer container(CONTAINER_WIDTH, CONTAINER_HEIGHT, CONTAINER_X, CONTAINER_Y);
    ColorWidget     head(CONTAINER_WIDTH, CONTAINER_HEIGHT, 0, 0);
    ColorWidget     tail(CONTAINER_WIDTH, CONTAINER_HEIGHT, 0, static_cast<int16_t>(CONTAINER_HEIGHT));
    bool            isHeadShown   = false;
    bool            isTailShown   = false;
    bool            isOutsideKept = false;

    head.setColor(COLOR_HEAD);
    tail.setColor(COLOR_TAIL);
    (void)container.add(head);
    (void)container.add(tail);

    container.setContentSize(CONTENT_HEIGHT);
    container.enableScrolling(ScrollController::DIRECTION_VERTICAL);
    TEST_ASSERT_TRUE(container.isScrollingEnabled());

    /* Scrolling starts outside of the visible area, therefore nothing is shown. */
    testGfx.fillScreen(COLOR_BACKGROUND);
    container.update(testGfx);
    TEST_ASSERT_TRUE(testGfx.verify(CONTAINER_X, CONTAINER_Y, CONTAINER_WIDTH, CONTAINER_HEIGHT, COLOR_BACKGROUND));

    scrollAndObserve(container, testGfx, isHeadShown, isTailShown, isOutsideKept);

    TEST_ASSERT_TRUE(isHeadShown);
    TEST_ASSERT_TRUE(isTailShown);
    TEST_ASSERT_TRUE(isOutsideKept);
}
