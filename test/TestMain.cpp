/* MIT License
 *
 * Copyright (c) 2019 Andreas Merkle <web@blue-andi.de>
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
@brief  Main test entry point
@author Andreas Merkle <web@blue-andi.de>

@section desc Description
This module provides the main test entry point.

*******************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <unity.h>
#include <stdio.h>

#include <LinkedList.hpp>
#include <Widget.hpp>
#include <Canvas.h>
#include <LampWidget.hpp>
#include <BitmapWidget.hpp>
#include <TextWidget.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/** Mark not used function parameters */
#define NOT_USED(__var) (void)(__var)

/** Get number of array elements. */
#define ARRAY_NUM(__arr)    (sizeof(__arr) / sizeof((__arr)[0]))

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Graphics interface for testing purposes.
 * It provides all relevant methods from the Adafruit GFX, which are used.
 */
class TestGfx : public Adafruit_GFX
{
public:

    /**
     * Constructs a graphic interface for testing purposes.
     */
    TestGfx() :
        Adafruit_GFX(WIDTH, HEIGHT),
        m_buffer(),
        m_callCounterDrawPixel(0)
    {
        uint16_t index = 0u;

        for(index = 0u; index < ARRAY_NUM(m_buffer); ++index)
        {
            m_buffer[index] = 0u;
        }
    }

    /**
     * Destroys the graphic interface.
     */
    ~TestGfx()
    {
    }

    /**
     * Draw a single pixel in the matrix and ensure that the drawing borders
     * are not violated.
     * 
     * @param[in] x     x-coordinate
     * @param[in] y     y-coordinate
     * @param[in] color Pixel color
     */
    void drawPixel(int16_t x, int16_t y, uint16_t color)
    {
        /* Out of bounds check */
        TEST_ASSERT_GREATER_OR_EQUAL_INT16(0, x);
        TEST_ASSERT_GREATER_OR_EQUAL_INT16(0, y);
        TEST_ASSERT_LESS_OR_EQUAL_INT16(WIDTH, x);
        TEST_ASSERT_LESS_OR_EQUAL_INT16(HEIGHT, y);

        m_buffer[x + y * WIDTH] = color;

        ++m_callCounterDrawPixel;

        return;
    }

    /**
     * Get display buffer.
     * 
     * @return Display buffer
     */
    uint16_t* getBuffer(void)
    {
        return m_buffer;
    }

    /**
     * Get call counter of @drawPixel.
     * 
     * @return Call counter
     */
    uint32_t getCallCounterDrawPixel(void) const
    {
        return m_callCounterDrawPixel;
    }

    /**
     * Set call counter of @drawPixel.
     * 
     * @param[in] counter New call counter value
     */
    void setCallCounterDrawPixel(uint32_t counter)
    {
        m_callCounterDrawPixel = counter;
        return;
    }

    /**
     * Dump display buffer to console.
     */
    void dump(void) const
    {
        uint16_t    x   = 0u;
        uint16_t    y   = 0u;

        for(y = 0u; y < HEIGHT; ++y)
        {
            for(x = 0u; x < WIDTH; ++x)
            {
                if (0u < x)
                {
                    printf(" ");
                }

                printf("0x%04X", m_buffer[x + WIDTH * y]);
            }

            printf("\r\n");
        }

        return;
    }

    /**
     * Verify rectangle at given position. It must be there with the given color.
     * 
     * @param[in] posX      Top left x-coordinate
     * @param[in] posY      Top left y-coordinate
     * @param[in] width     Width in pixel
     * @param[in] height    Height in pixel
     * @param[in] color     Color of widget drawing
     */
    void verify(int16_t posX, int16_t posY, uint16_t width, uint16_t height, uint16_t color)
    {
        uint16_t    x   = 0u;
        uint16_t    y   = 0u;

        TEST_ASSERT_LESS_OR_EQUAL(WIDTH, posX + width);
        TEST_ASSERT_LESS_OR_EQUAL(HEIGHT, posY + height);

        for(y = 0u; y < height; ++y)
        {
            for(x = 0u; x < width; ++x)
            {
                if (color != m_buffer[posX + x + (posY + y) * WIDTH])
                {
                    dump();
                    printf("x = %d, y = %d\r\n", posX + x, posY + y);
                }
                TEST_ASSERT_EQUAL_UINT16(color, m_buffer[posX + x + (posY + y) * WIDTH]);
            }
        }

        return;
    }

    /**
     * Fill display buffer with given color.
     * 
     * @param[in] color Fill color
     */
    void fill(uint16_t color)
    {
        uint16_t    x   = 0u;
        uint16_t    y   = 0u;

        for(y = 0u; y < HEIGHT; ++y)
        {
            for(x = 0u; x < WIDTH; ++x)
            {
                m_buffer[x + y * WIDTH] = color;
            }
        }

        return;
    }

    static const uint16_t   WIDTH   = 32;   /**< Drawing area width in pixel */
    static const uint16_t   HEIGHT  = 8;    /**< Drawing area height in pixel */

private:

    uint16_t    m_buffer[WIDTH * HEIGHT];   /**< Display buffer, containing all pixels. */
    uint32_t    m_callCounterDrawPixel;     /**< Call counter for @drawPixel */

    TestGfx(const TestGfx& gfx);
    TestGfx& operator=(const TestGfx& gfx);

};

/**
 * Widget for test purposes.
 * It draws only a rectangle with a specific color.
 */
class TestWidget : public Widget
{
public:

    /**
     * Constructs a widget for testing purposes at position (0, 0).
     * The drawing pen color is set to black.
     */
    TestWidget() :
        Widget(WIDGET_TYPE, 0, 0),
        m_color(0u)
    {
    }

    /**
     * Destroys widget for testing purposes.
     */
    ~TestWidget()
    {
    }

    /**
     * Update widget drawing.
     * 
     * @param[in] gfx Graphics interface, which to use.
     */
    void update(Adafruit_GFX& gfx)
    {
        int16_t x = 0;
        int16_t y = 0;

        for(y = 0; y < HEIGHT; ++y)
        {
            for(x = 0; x < WIDTH; ++x)
            {
                gfx.drawPixel(m_posX + x, m_posY + y, m_color);
            }
        }
        
        return;
    }

    /**
     * Get pen color, used to draw the widget.
     * 
     * @return Pen color
     */
    uint16_t getPenColor(void) const
    {
        return m_color;
    }

    /**
     * Set pen color, used to draw the widget.
     * 
     * @param[in] color Pen color
     */
    void setPenColor(uint16_t color)
    {
        m_color = color;
        return;
    }

    static const uint16_t   WIDTH       = 10u;  /**< Widget width in pixel */
    static const uint16_t   HEIGHT      = 5u;   /**< Widget height in pixel */
    static const char*      WIDGET_TYPE;        /**< Widget type string */

private:

    uint16_t m_color;   /**< Pen color, used to draw the widget. */

};

const char* TestWidget::WIDGET_TYPE = "test";

/******************************************************************************
 * Prototypes
 *****************************************************************************/

template < typename T >
static T getMin(const T value1, const T value2);

static void testDoublyLinkedList(void);
static void testWidget(void);
static void testCanvas(void);
static void testLampWidget(void);
static void testBitmapWidget(void);
static void testTextWidget(void);

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
    NOT_USED(argc);
    NOT_USED(argv);

    UNITY_BEGIN();

    RUN_TEST(testDoublyLinkedList);
    RUN_TEST(testWidget);
    RUN_TEST(testCanvas);
    RUN_TEST(testLampWidget);
    RUN_TEST(testBitmapWidget);
    RUN_TEST(testTextWidget);

    return UNITY_END();
}

/******************************************************************************
 * Local functions
 *****************************************************************************/

/**
 * Get minimum of two values.
 * 
 * @param[in] value1    Value 1
 * @param[in] value2    Value 2
 * 
 * @return Minimum of value 1 and value 2
 */
template < typename T >
static T getMin(const T value1, const T value2)
{
    if (value1 < value2)
    {
        return value1;
    }

    return value2;
}

/**
 * Doubly linked list tests.
 */
static void testDoublyLinkedList(void)
{
    LinkedList<uint32_t> list;
    uint32_t*            element = NULL;
    uint32_t             value   = 1u;
    uint32_t             index   = 0u;
    const uint32_t       max     = 3u;

    /* List is empty. */
    TEST_ASSERT_NULL(list.first());
    TEST_ASSERT_NULL(list.last());
    TEST_ASSERT_NULL(list.current());
    TEST_ASSERT_FALSE(list.next());
    TEST_ASSERT_FALSE(list.prev());
    TEST_ASSERT_EQUAL_UINT32(0u, list.getNumOfElements());

    /* Add one element. */
    TEST_ASSERT_TRUE(list.append(value));
    TEST_ASSERT_EQUAL_UINT32(1u, list.getNumOfElements());

    TEST_ASSERT_NOT_NULL(list.first());
    TEST_ASSERT_NOT_NULL(list.last());
    TEST_ASSERT_NOT_NULL(list.current());

    TEST_ASSERT_EQUAL_INT(value, *list.first());
    TEST_ASSERT_EQUAL_INT(value, *list.last());
    TEST_ASSERT_EQUAL_INT(value, *list.current());

    TEST_ASSERT_EQUAL_PTR(list.first(), list.last());
    TEST_ASSERT_EQUAL_PTR(list.first(), list.current());

    /* Remove element from list. List is now empty. */
    list.removeSelected();
    TEST_ASSERT_EQUAL_UINT32(0u, list.getNumOfElements());

    TEST_ASSERT_NULL(list.first());
    TEST_ASSERT_NULL(list.last());
    TEST_ASSERT_NULL(list.current());
    TEST_ASSERT_FALSE(list.next());
    TEST_ASSERT_FALSE(list.prev());

    /* Add more elements */
    for(index = 1u; index <= max; ++index)
    {
        TEST_ASSERT_TRUE(list.append(index));
        TEST_ASSERT_EQUAL_UINT32(index, list.getNumOfElements());
    }

    TEST_ASSERT_NOT_NULL(list.first());
    TEST_ASSERT_EQUAL_INT(1u, *list.first());

    TEST_ASSERT_NOT_NULL(list.last());
    TEST_ASSERT_EQUAL_INT(max, *list.last());

    /* Select element for element, from head to tail. */
    for(index = 1u; index <= max; ++index)
    {
        TEST_ASSERT_NOT_NULL(list.current());
        TEST_ASSERT_EQUAL_INT(index, *list.current());

        if (index < max)
        {
            TEST_ASSERT_TRUE(list.next());
        }
        else
        {
            TEST_ASSERT_FALSE(list.next());
        }
        
    }

    /* Select element for element, from tail to head. */
    for(index = max; index > 0u; --index)
    {
        TEST_ASSERT_NOT_NULL(list.current());
        TEST_ASSERT_EQUAL_INT(index, *list.current());

        if (index > 1u)
        {
            TEST_ASSERT_TRUE(list.prev());
        }
        else
        {
            TEST_ASSERT_FALSE(list.prev());
        }
        
    }

    /* Remove all elements */
    for(index = 1; index <= max; ++index)
    {
        list.removeSelected();
        TEST_ASSERT_EQUAL_UINT32(max - index, list.getNumOfElements());
    }

    TEST_ASSERT_NULL(list.first());
    TEST_ASSERT_NULL(list.last());
    TEST_ASSERT_NULL(list.current());
    TEST_ASSERT_FALSE(list.next());
    TEST_ASSERT_FALSE(list.prev());

    /* Insert elements again */
    for(index = 1u; index <= max; ++index)
    {
        TEST_ASSERT_TRUE(list.append(index));
        TEST_ASSERT_EQUAL_UINT32(index, list.getNumOfElements());
    }

    /* Copy it via copy constructor */
    {
        LinkedList<uint32_t> copyOfList = list;

        TEST_ASSERT_TRUE(list.selectFirstElement());
        for(index = 1u; index <= max; ++index)
        {
            TEST_ASSERT_NOT_NULL(copyOfList.current());
            TEST_ASSERT_NOT_NULL(list.current());
            TEST_ASSERT_NOT_EQUAL(copyOfList.current(), list.current());
            TEST_ASSERT_EQUAL_INT(*copyOfList.current(), *list.current());
            copyOfList.next();
            list.next();
        }
    }

    /* Copy it via assignment */
    {
        LinkedList<uint32_t> copyOfList;
        copyOfList = list;

        TEST_ASSERT_TRUE(list.selectFirstElement());
        for(index = 1; index <= max; ++index)
        {
            TEST_ASSERT_NOT_NULL(copyOfList.current());
            TEST_ASSERT_NOT_NULL(list.current());
            TEST_ASSERT_NOT_EQUAL(copyOfList.current(), list.current());
            TEST_ASSERT_EQUAL_INT(*copyOfList.current(), *list.current());
            copyOfList.next();
            list.next();
        }
    }

    /* Find not existing element */
    TEST_ASSERT_TRUE(list.selectFirstElement());
    TEST_ASSERT_FALSE(list.find(max + 1));

    /* Find existing element */
    TEST_ASSERT_TRUE(list.selectFirstElement());
    TEST_ASSERT_TRUE(list.find(*list.first()));
    TEST_ASSERT_EQUAL(list.first(), list.current());

    TEST_ASSERT_TRUE(list.selectFirstElement());
    TEST_ASSERT_TRUE(list.find(*list.last()));
    TEST_ASSERT_EQUAL(list.last(), list.current());

    return;
}

/**
 * Widget tests.
 */
static void testWidget(void)
{
    TestGfx         testGfx;
    TestWidget      testWidget;
    int16_t         posX        = -1;
    int16_t         posY        = -1;
    const uint16_t  COLOR       = 0x1234;
    const char*     testStr     = "myWidget";

    /* Verify widget type name */
    TEST_ASSERT_EQUAL_STRING(TestWidget::WIDGET_TYPE, testWidget.getType());

    /* No widget name is set, it must be empty. */
    TEST_ASSERT_EQUAL_STRING("", testWidget.getName());

    /* Set widget name and read back. */
    testWidget.setName(testStr);
    TEST_ASSERT_EQUAL_STRING(testStr, testWidget.getName());

    /* Find widget with empty name.
     * Expected: Not found
     */
    TEST_ASSERT_NULL(testWidget.find(NULL));
    TEST_ASSERT_NULL(testWidget.find(""));

    /* Find widget with its name.
     * Expected: Widget is found
     */
    TEST_ASSERT_NOT_NULL(testWidget.find(testStr));
    TEST_ASSERT_EQUAL_PTR(&testWidget, testWidget.find(testStr));

    /* Clear name */
    testWidget.setName(NULL);
    TEST_ASSERT_EQUAL_STRING("", testWidget.getName());

    /* Current position must be (0, 0) */
    testWidget.getPos(posX, posY);
    TEST_ASSERT_EQUAL_INT16(0, posX);
    TEST_ASSERT_EQUAL_INT16(0, posY);

    /* Move widget and verify position again. */
    testWidget.move(10, 20);
    testWidget.getPos(posX, posY);
    TEST_ASSERT_EQUAL_INT16(10, posX);
    TEST_ASSERT_EQUAL_INT16(20, posY);

    /* Verify widget type name */
    TEST_ASSERT_EQUAL_STRING(TestWidget::WIDGET_TYPE, testWidget.getType());

    /* For the whole test, set the widget color. */
    testWidget.setPenColor(COLOR);

    /* Draw widget at position (0, 0) */
    posX = 0;
    posY = 0;
    testWidget.move(posX, posY);
    testGfx.fill(0);
    testWidget.update(testGfx);
    testGfx.verify( posX,
                    posY,
                    getMin<uint16_t>(TestGfx::WIDTH - posX, TestWidget::WIDTH), 
                    getMin<uint16_t>(TestGfx::HEIGHT - posY, TestWidget::HEIGHT),
                    COLOR);

    /* Draw widget at position (2, 1) and verify widget movement. */
    posX = 2;
    posY = 1;
    testWidget.move(posX, posY);
    testGfx.fill(0);
    testWidget.update(testGfx);
    testGfx.verify( posX,
                    posY,
                    getMin<uint16_t>(TestGfx::WIDTH - posX, TestWidget::WIDTH), 
                    getMin<uint16_t>(TestGfx::HEIGHT - posY, TestWidget::HEIGHT),
                    COLOR);

    return;
}

/**
 * Canvas tests.
 */
void testCanvas(void)
{
    const uint16_t  CANVAS_WIDTH        = 8;
    const uint16_t  CANVAS_HEIGHT       = 8;
    const int16_t   WIDGET_POS_X        = 2;
    const int16_t   WIDGET_POS_Y        = 2;
    const uint16_t  WIDGET_COLOR        = 0x1234;
    const char*     CANVAS_NAME         = "canvasWidgetName";
    const char*     TEST_WIDGET_NAME    = "testWidgetName";

    TestGfx     testGfx;
    Canvas      testCanvas(CANVAS_WIDTH, CANVAS_HEIGHT, 0, 0);
    TestWidget  testWidget;

    /* Verify widget type name */
    TEST_ASSERT_EQUAL_STRING(Canvas::WIDGET_TYPE, testCanvas.getType());

    /* Canvas contains no other widget, so nothing should be drawn. */
    testGfx.setCallCounterDrawPixel(0);
    testCanvas.update(testGfx);
    TEST_ASSERT_EQUAL_UINT32(0, testGfx.getCallCounterDrawPixel());
    testGfx.verify(0, 0, TestWidget::WIDTH, TestWidget::HEIGHT, 0);

    /* Add widget to canvas, move widget and set draw pen */
    TEST_ASSERT_TRUE(testCanvas.addWidget(testWidget));
    testWidget.move(WIDGET_POS_X, WIDGET_POS_Y);
    testWidget.setPenColor(WIDGET_COLOR);

    /* Draw canvas with widget. Expected is a full drawn widget. */
    testGfx.fill(0);
    testCanvas.update(testGfx);
    testGfx.verify( WIDGET_POS_X, 
                    WIDGET_POS_Y, 
                    getMin<uint16_t>(TestWidget::WIDTH, CANVAS_WIDTH - WIDGET_POS_X), 
                    getMin<uint16_t>(TestWidget::HEIGHT, CANVAS_HEIGHT - WIDGET_POS_Y),
                    WIDGET_COLOR);

    /* Move widget outside canvas and try to draw. Expected is no drawing at all. */
    testGfx.fill(0);
    testWidget.move(CANVAS_WIDTH, CANVAS_HEIGHT);
    testCanvas.update(testGfx);
    testGfx.verify( 0, 
                    0, 
                    CANVAS_WIDTH, 
                    CANVAS_HEIGHT,
                    0);

    /* Move widget half outside canvas and draw. Expected is partly drawing. */
    testGfx.fill(0);
    testWidget.move(CANVAS_WIDTH / 2, CANVAS_HEIGHT / 2);
    testCanvas.update(testGfx);
    testGfx.verify( CANVAS_WIDTH / 2, 
                    CANVAS_HEIGHT / 2, 
                    CANVAS_WIDTH / 2, 
                    CANVAS_HEIGHT / 2,
                    WIDGET_COLOR);

    /* No widget name is set, it must be empty. */
    TEST_ASSERT_EQUAL_STRING("", testCanvas.getName());

    /* Set widget name and read back. */
    testCanvas.setName(CANVAS_NAME);
    TEST_ASSERT_EQUAL_STRING(CANVAS_NAME, testCanvas.getName());

    /* Find widget with empty name.
     * Expected: Not found
     */
    TEST_ASSERT_NULL(testCanvas.find(NULL));
    TEST_ASSERT_NULL(testCanvas.find(""));

    /* Find widget with its name.
     * Expected: Widget is found
     */
    TEST_ASSERT_NOT_NULL(testCanvas.find(CANVAS_NAME));
    TEST_ASSERT_EQUAL_PTR(static_cast<Widget*>(&testCanvas), testCanvas.find(CANVAS_NAME));

    /* Find widget in container, but widget has no name.
     * Expected: Test widget not found
     */
    TEST_ASSERT_NULL(testCanvas.find(TEST_WIDGET_NAME));

    /* Find widget in container.
     * Expected: Test widget found
     */
    testWidget.setName(TEST_WIDGET_NAME);
    TEST_ASSERT_NOT_NULL(testCanvas.find(TEST_WIDGET_NAME));
    TEST_ASSERT_EQUAL_PTR(&testWidget, testCanvas.find(TEST_WIDGET_NAME));

    return;
}

/**
 * Test lamp widget.
 */
static void testLampWidget(void)
{
    const uint16_t  COLOR_OFF   = 0x1111;
    const uint16_t  COLOR_ON    = 0x2222;
    const char*     WIDGET_NAME = "lampWidgetName";

    TestGfx         testGfx;
    LampWidget      lampWidget(false, COLOR_OFF, COLOR_ON);
    int16_t         posX        = 0;
    int16_t         posY        = 0;

    /* Verify widget type name */
    TEST_ASSERT_EQUAL_STRING(LampWidget::WIDGET_TYPE, lampWidget.getType());

    /* No widget name is set, it must be empty. */
    TEST_ASSERT_EQUAL_STRING("", lampWidget.getName());

    /* Set widget name and read back. */
    lampWidget.setName(WIDGET_NAME);
    TEST_ASSERT_EQUAL_STRING(WIDGET_NAME, lampWidget.getName());

    /* Find widget with empty name.
     * Expected: Not found
     */
    TEST_ASSERT_NULL(lampWidget.find(NULL));
    TEST_ASSERT_NULL(lampWidget.find(""));

    /* Find widget with its name.
     * Expected: Widget is found
     */
    TEST_ASSERT_NOT_NULL(lampWidget.find(WIDGET_NAME));
    TEST_ASSERT_EQUAL_PTR(&lampWidget, lampWidget.find(WIDGET_NAME));

    /* Draw widget in off state and verify */
    lampWidget.update(testGfx);
    lampWidget.getPos(posX, posY);
    testGfx.verify( posX,
                    posY,
                    LampWidget::WIDTH,
                    LampWidget::HEIGHT,
                    COLOR_OFF);

    /* Draw widget in on state and verify */
    lampWidget.setOnState(true);
    lampWidget.update(testGfx);
    lampWidget.getPos(posX, posY);
    testGfx.verify( posX,
                    posY,
                    LampWidget::WIDTH,
                    LampWidget::HEIGHT,
                    COLOR_ON);

    /* Draw widget in off state and verify */
    lampWidget.setOnState(false);
    lampWidget.update(testGfx);
    lampWidget.getPos(posX, posY);
    testGfx.verify( posX,
                    posY,
                    LampWidget::WIDTH,
                    LampWidget::HEIGHT,
                    COLOR_OFF);

    /* Move widget and draw in off state again. */
    testGfx.fill(0);
    lampWidget.move(2,2);
    lampWidget.update(testGfx);
    lampWidget.getPos(posX, posY);
    testGfx.verify( posX,
                    posY,
                    LampWidget::WIDTH,
                    LampWidget::HEIGHT,
                    COLOR_OFF);

    return;
}

/**
 * Test bitmap widget.
 */
static void testBitmapWidget(void)
{
    const uint8_t BITMAP_WIDTH      = TestGfx::HEIGHT;  /* Use height as width here for a square */
    const uint8_t BITMAP_HEIGHT     = TestGfx::HEIGHT;
    const char*   WIDGET_NAME       = "bmpWidgetName";

    TestGfx         testGfx;
    BitmapWidget    bitmapWidget;
    uint16_t        bitmap[BITMAP_WIDTH * BITMAP_HEIGHT];
    uint8_t         x               = 0u;
    uint8_t         y               = 0u;
    const uint16_t* bitmapPtr       = NULL;
    uint16_t        width           = 0u;
    uint16_t        height          = 0u;
    uint16_t*       displayBuffer   = NULL;

    /* Verify widget type name */
    TEST_ASSERT_EQUAL_STRING(BitmapWidget::WIDGET_TYPE, bitmapWidget.getType());

    /* No widget name is set, it must be empty. */
    TEST_ASSERT_EQUAL_STRING("", bitmapWidget.getName());

    /* Set widget name and read back. */
    bitmapWidget.setName(WIDGET_NAME);
    TEST_ASSERT_EQUAL_STRING(WIDGET_NAME, bitmapWidget.getName());

    /* Find widget with empty name.
     * Expected: Not found
     */
    TEST_ASSERT_NULL(bitmapWidget.find(NULL));
    TEST_ASSERT_NULL(bitmapWidget.find(""));

    /* Find widget with its name.
     * Expected: Widget is found
     */
    TEST_ASSERT_NOT_NULL(bitmapWidget.find(WIDGET_NAME));
    TEST_ASSERT_EQUAL_PTR(&bitmapWidget, bitmapWidget.find(WIDGET_NAME));

    /* Create bitmap */
    for(y = 0u; y < BITMAP_HEIGHT; ++y)
    {
        for(x = 0u; x < BITMAP_WIDTH; ++x)
        {
            bitmap[x + y * BITMAP_WIDTH] = x + y * BITMAP_WIDTH;
        }
    }

    /* Set bitmap and read back */
    bitmapWidget.set(bitmap, BITMAP_WIDTH, BITMAP_HEIGHT);
    bitmapPtr = bitmapWidget.get(width, height);
    TEST_ASSERT_EQUAL_PTR(bitmap, bitmapPtr);
    TEST_ASSERT_EQUAL_UINT16(BITMAP_WIDTH, width);
    TEST_ASSERT_EQUAL_UINT16(BITMAP_HEIGHT, height);

    /* Draw bitmap and verify */
    bitmapWidget.update(testGfx);
    displayBuffer = testGfx.getBuffer();

    for(y = 0u; y < BITMAP_HEIGHT; ++y)
    {
        for(x = 0u; x < BITMAP_WIDTH; ++x)
        {
            TEST_ASSERT_EQUAL_UINT16(x + y * BITMAP_WIDTH, displayBuffer[x + y * TestGfx::WIDTH]);
        }
    }

    return;
}

/**
 * Test text widget.
 */
static void testTextWidget(void)
{
    TestGfx         testGfx;
    TextWidget      textWidget;
    String          testStr     = "test";
    const uint16_t  TEXT_COLOR  = 0x1234;
    const char*     WIDGET_NAME = "textWidgetName";

    /* Verify widget type name */
    TEST_ASSERT_EQUAL_STRING(TextWidget::WIDGET_TYPE, textWidget.getType());

    /* No widget name is set, it must be empty. */
    TEST_ASSERT_EQUAL_STRING("", textWidget.getName());

    /* Set widget name and read back. */
    textWidget.setName(WIDGET_NAME);
    TEST_ASSERT_EQUAL_STRING(WIDGET_NAME, textWidget.getName());

    /* Find widget with empty name.
     * Expected: Not found
     */
    TEST_ASSERT_NULL(textWidget.find(NULL));
    TEST_ASSERT_NULL(textWidget.find(""));

    /* Find widget with its name.
     * Expected: Widget is found
     */
    TEST_ASSERT_NOT_NULL(textWidget.find(WIDGET_NAME));
    TEST_ASSERT_EQUAL_PTR(&textWidget, textWidget.find(WIDGET_NAME));

    /* Default string is empty */
    TEST_ASSERT_EQUAL_STRING("", textWidget.getStr().c_str());

    /* Set/Get string */
    textWidget.setStr(testStr);
    TEST_ASSERT_EQUAL_STRING(testStr.c_str(), textWidget.getStr().c_str());

    /* Default string color */
    TEST_ASSERT_EQUAL_UINT16(TextWidget::DEFAULT_TEXT_COLOR, textWidget.getTextColor());

    /* Set/Get text color */
    textWidget.setTextColor(TEXT_COLOR);
    TEST_ASSERT_EQUAL_UINT16(TEXT_COLOR, textWidget.getTextColor());

    return;
}