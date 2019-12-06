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
 * @brief  Main test entry point
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <unity.h>
#include <stdio.h>

#include <LinkedList.hpp>
#include <Widget.hpp>
#include <Canvas.h>
#include <LampWidget.h>
#include <BitmapWidget.h>
#include <TextWidget.h>
#include <Color.h>
#include <StateMachine.hpp>
#include <SimpleTimer.hpp>
#include <ProgressBar.h>
#include <Logging.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/** Mark not used function parameters */
#define NOT_USED(__var)     (void)(__var)

/** Get number of array elements. */
#define ARRAY_NUM(__arr)    (sizeof(__arr) / sizeof((__arr)[0]))

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Logging interface for testing purposes.
 * It provides all relevant methods from the Print class, which are used.
 */
class TestLogger : public Print
{
public:

    /**
     * Constructs a logging interface for testing purposes.
     */
    TestLogger():
        m_buffer()
    {
        uint16_t index = 0u;

        for(index = 0u; index < ARRAY_NUM(m_buffer); ++index)
        {
            m_buffer[index] = 0;
        }
    }

    /**
     * Write a single byte.
     * 
     * @param[in] data The byte to be written.
     * 
     * @return 1.
     */
    size_t write(uint8_t data)
    {
        NOT_USED(data);

        /* Method is not used at all, because the write(const uint8_t*, size_t size)
         * is overwritten, which doesn't use the single byte write method.
         */

        return 0;
    }

    /**
     * Write a single byte.
     * 
     * @param[in] buffer Pointer to the data to be written.
     * @param[in] size the size of the data to be written.
     * 
     * @return The size of the written data.
     */
    size_t write(const uint8_t* buffer, size_t size)
    {
        uint16_t index = 0u;
        
        for(index = 0u; index < ARRAY_NUM(m_buffer); ++index)
        {
            m_buffer[index] = static_cast<char>(buffer[index]);
        }

        return size;
    }

    /**
     * Get the Write buffer.
     * 
     * @return Write buffer
     */
    const char* getBuffer(void)
    {
        return m_buffer;
    }

    /**
     * Destroys the logging interface.
     */
    ~TestLogger( )
    {
    }

private:
    char m_buffer[1024]; /**< Write buffer, containing the logMessage. */
};

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
        m_callCounterDrawPixel(0u)
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
     * @return If the verify is successful, it will return true otherwise false.
     */
    bool verify(int16_t posX, int16_t posY, uint16_t width, uint16_t height, uint16_t color)
    {
        uint16_t    x               = 0u;
        uint16_t    y               = 0u;
        bool        isSuccessful    = true;

        TEST_ASSERT_LESS_OR_EQUAL(WIDTH, posX + width);
        TEST_ASSERT_LESS_OR_EQUAL(HEIGHT, posY + height);

        while((height > y) && (true == isSuccessful))
        {
            x = 0u;
            while((width > x) && (true == isSuccessful))
            {
                if (color != m_buffer[posX + x + (posY + y) * WIDTH])
                {
                    dump();
                    printf("x = %d, y = %d\r\n", posX + x, posY + y);
                    isSuccessful = false;
                }

                ++x;
            }

            ++y;
        }

        return isSuccessful;
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
                gfx.drawPixel(m_posX + x, m_posY + y, m_color.get565());
            }
        }
        
        return;
    }

    /**
     * Get pen color, used to draw the widget.
     * 
     * @return Pen color
     */
    const Color& getPenColor(void) const
    {
        return m_color;
    }

    /**
     * Set pen color, used to draw the widget.
     * 
     * @param[in] color Pen color
     */
    void setPenColor(const Color& color)
    {
        m_color = color;
        return;
    }

    static const uint16_t   WIDTH       = 10u;  /**< Widget width in pixel */
    static const uint16_t   HEIGHT      = 5u;   /**< Widget height in pixel */
    static const char*      WIDGET_TYPE;        /**< Widget type string */

private:

    Color m_color;  /**< Pen color, used to draw the widget. */

};

const char* TestWidget::WIDGET_TYPE = "test";

/**
 * Test state
 */
class TestState : public AbstractState
{
public:

    /**
     * Constructs the test state.
     */
    TestState() :
        m_callCntEntry(0u),
        m_callCntExit(0u),
        m_nextState(NULL)
    {
    }

    /**
     * Destroys the test state.
     */
    ~TestState()
    {
    }

    /**
     * The entry is called once, a state is entered.
     * 
     * @param[in] sm    Responsible state machine
     */
    void entry(StateMachine& sm)
    {
        ++m_callCntEntry;
        return;
    }

    /**
     * The process routine is called cyclic, as long as the state is active.
     * 
     * @param[in] sm    Responsible state machine
     */

    void process(StateMachine& sm)
    {
        if (NULL != m_nextState)
        {
            sm.setState(*m_nextState);
        }
        
        return;
    }

    /**
     * The exit is called once, a state will be left.
     * 
     * @param[in] sm    Responsible state machine
     */
    void exit(StateMachine& sm)
    {
        ++m_callCntExit;
        return;
    }

    /**
     * Set next state.
     * 
     * @param[in] nextState Next state to go in process method.
     */
    void setState(AbstractState& nextState)
    {
        m_nextState = &nextState;
        return;
    }

    /**
     * Get call counter for entry method.
     * 
     * @return Call counter for entry method.
     */
    uint32_t getCallCntEntry(void) const
    {
        return m_callCntEntry;
    }

    /**
     * Get call counter for exit method.
     * 
     * @return Call counter for exit method.
     */
    uint32_t getCallCntExit(void) const
    {
        return m_callCntExit;
    }

private:

    uint32_t        m_callCntEntry; /**< Call counter of entry method */
    uint32_t        m_callCntExit;  /**< Call counter of exit method */
    AbstractState*  m_nextState;    /**< Next state */
};

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
static void testColor(void);
static void testStateMachine(void);
static void testSimpleTimer(void);
static void testProgressBar(void);
static void testLogging(void);

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
    RUN_TEST(testColor);
    RUN_TEST(testStateMachine);
    RUN_TEST(testSimpleTimer);
    RUN_TEST(testProgressBar);
    RUN_TEST(testLogging);
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
    DLinkedList<uint32_t>   list;
    uint32_t*               element = NULL;
    uint32_t                value   = 1u;
    uint32_t                index   = 0u;
    const uint32_t          max     = 3u;

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
        DLinkedList<uint32_t> copyOfList = list;

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
        DLinkedList<uint32_t> copyOfList;
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
    TestGfx     testGfx;
    TestWidget  testWidget;
    int16_t     posX        = -1;
    int16_t     posY        = -1;
    const Color COLOR       = 0x123456;
    const char* testStr     = "myWidget";

    /* Verify widget type name */
    TEST_ASSERT_EQUAL_STRING(TestWidget::WIDGET_TYPE, testWidget.getType());

    /* No widget name is set, it must be empty. */
    TEST_ASSERT_EQUAL_STRING("", testWidget.getName().c_str());

    /* Set widget name and read back. */
    testWidget.setName(testStr);
    TEST_ASSERT_EQUAL_STRING(testStr, testWidget.getName().c_str());

    /* Find widget with empty name.
     * Expected: Not found
     */
    TEST_ASSERT_NULL(testWidget.find(""));

    /* Find widget with its name.
     * Expected: Widget is found
     */
    TEST_ASSERT_NOT_NULL(testWidget.find(testStr));
    TEST_ASSERT_EQUAL_PTR(&testWidget, testWidget.find(testStr));

    /* Clear name */
    testWidget.setName("");
    TEST_ASSERT_EQUAL_STRING("", testWidget.getName().c_str());

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
    TEST_ASSERT_TRUE(testGfx.verify(posX,
                                    posY,
                                    getMin<uint16_t>(TestGfx::WIDTH - posX, TestWidget::WIDTH), 
                                    getMin<uint16_t>(TestGfx::HEIGHT - posY, TestWidget::HEIGHT),
                                    COLOR.get565()));

    /* Draw widget at position (2, 1) and verify widget movement. */
    posX = 2;
    posY = 1;
    testWidget.move(posX, posY);
    testGfx.fill(0);
    testWidget.update(testGfx);
    TEST_ASSERT_TRUE(testGfx.verify(posX,
                                    posY,
                                    getMin<uint16_t>(TestGfx::WIDTH - posX, TestWidget::WIDTH), 
                                    getMin<uint16_t>(TestGfx::HEIGHT - posY, TestWidget::HEIGHT),
                                    COLOR.get565()));

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
    const Color     WIDGET_COLOR        = 0x123456;
    const char*     CANVAS_NAME         = "canvasWidgetName";
    const char*     TEST_WIDGET_NAME    = "testWidgetName";

    TestGfx     testGfx;
    Canvas      testCanvas(CANVAS_WIDTH, CANVAS_HEIGHT, 0, 0);
    TestWidget  testWidget;
    TestWidget  testWidget2;

    /* Verify widget type name */
    TEST_ASSERT_EQUAL_STRING(Canvas::WIDGET_TYPE, testCanvas.getType());

    /* Canvas contains no other widget, so nothing should be drawn. */
    testGfx.setCallCounterDrawPixel(0);
    testCanvas.update(testGfx);
    TEST_ASSERT_EQUAL_UINT32(0, testGfx.getCallCounterDrawPixel());
    TEST_ASSERT_TRUE(testGfx.verify(0, 0, TestWidget::WIDTH, TestWidget::HEIGHT, 0));

    /* Add widget to canvas, move widget and set draw pen */
    TEST_ASSERT_TRUE(testCanvas.addWidget(testWidget));
    testWidget.move(WIDGET_POS_X, WIDGET_POS_Y);
    testWidget.setPenColor(WIDGET_COLOR);

    /* Draw canvas with widget. Expected is a full drawn widget. */
    testGfx.fill(0);
    testCanvas.update(testGfx);
    TEST_ASSERT_TRUE(testGfx.verify(WIDGET_POS_X, 
                                    WIDGET_POS_Y, 
                                    getMin<uint16_t>(TestWidget::WIDTH, CANVAS_WIDTH - WIDGET_POS_X), 
                                    getMin<uint16_t>(TestWidget::HEIGHT, CANVAS_HEIGHT - WIDGET_POS_Y),
                                    WIDGET_COLOR.get565()));

    /* Move widget outside canvas and try to draw. Expected is no drawing at all. */
    testGfx.fill(0);
    testWidget.move(CANVAS_WIDTH, CANVAS_HEIGHT);
    testCanvas.update(testGfx);
    TEST_ASSERT_TRUE(testGfx.verify(0, 
                                    0, 
                                    CANVAS_WIDTH, 
                                    CANVAS_HEIGHT,
                                    0));

    /* Move widget half outside canvas and draw. Expected is partly drawing. */
    testGfx.fill(0);
    testWidget.move(CANVAS_WIDTH / 2, CANVAS_HEIGHT / 2);
    testCanvas.update(testGfx);
    TEST_ASSERT_TRUE(testGfx.verify(CANVAS_WIDTH / 2, 
                                    CANVAS_HEIGHT / 2, 
                                    CANVAS_WIDTH / 2, 
                                    CANVAS_HEIGHT / 2,
                                    WIDGET_COLOR.get565()));

    /* No widget name is set, it must be empty. */
    TEST_ASSERT_EQUAL_STRING("", testCanvas.getName().c_str());

    /* Set widget name and read back. */
    testCanvas.setName(CANVAS_NAME);
    TEST_ASSERT_EQUAL_STRING(CANVAS_NAME, testCanvas.getName().c_str());

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

    /* Find widget in container and container has no name.
     * Expected: Test widget found
     */
    testWidget.setName(TEST_WIDGET_NAME);
    testCanvas.setName("");
    TEST_ASSERT_NOT_NULL(testCanvas.find(TEST_WIDGET_NAME));
    TEST_ASSERT_EQUAL_PTR(&testWidget, testCanvas.find(TEST_WIDGET_NAME));

    /* Find widget in container, which contains 2 widgets.
     * Expected: Test widget found
     */
    testWidget.setName(TEST_WIDGET_NAME);
    testCanvas.setName("");
    TEST_ASSERT_TRUE(testCanvas.addWidget(testWidget2));
    TEST_ASSERT_NOT_NULL(testCanvas.find(TEST_WIDGET_NAME));
    TEST_ASSERT_EQUAL_PTR(&testWidget, testCanvas.find(TEST_WIDGET_NAME));

    return;
}

/**
 * Test lamp widget.
 */
static void testLampWidget(void)
{
    const Color     COLOR_OFF   = 0x111111;
    const Color     COLOR_ON    = 0x222222;
    const char*     WIDGET_NAME = "lampWidgetName";

    TestGfx         testGfx;
    LampWidget      lampWidget(false, COLOR_OFF, COLOR_ON);
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
                                    LampWidget::WIDTH,
                                    LampWidget::HEIGHT,
                                    COLOR_OFF.get565()));

    /* Draw widget in on state and verify */
    lampWidget.setOnState(true);
    lampWidget.update(testGfx);
    lampWidget.getPos(posX, posY);
    TEST_ASSERT_TRUE(testGfx.verify(posX,
                                    posY,
                                    LampWidget::WIDTH,
                                    LampWidget::HEIGHT,
                                    COLOR_ON.get565()));

    /* Draw widget in off state and verify */
    lampWidget.setOnState(false);
    lampWidget.update(testGfx);
    lampWidget.getPos(posX, posY);
    TEST_ASSERT_TRUE(testGfx.verify(posX,
                                    posY,
                                    LampWidget::WIDTH,
                                    LampWidget::HEIGHT,
                                    COLOR_OFF.get565()));

    /* Move widget and draw in off state again. */
    testGfx.fill(0);
    lampWidget.move(2,2);
    lampWidget.update(testGfx);
    lampWidget.getPos(posX, posY);
    TEST_ASSERT_TRUE(testGfx.verify(posX,
                                    posY,
                                    LampWidget::WIDTH,
                                    LampWidget::HEIGHT,
                                    COLOR_OFF.get565()));

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
    TEST_ASSERT_EQUAL_STRING("", bitmapWidget.getName().c_str());

    /* Set widget name and read back. */
    bitmapWidget.setName(WIDGET_NAME);
    TEST_ASSERT_EQUAL_STRING(WIDGET_NAME, bitmapWidget.getName().c_str());

    /* Find widget with empty name.
     * Expected: Not found
     */
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
    TestGfx     testGfx;
    TextWidget  textWidget;
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
    textWidget.setStr(testStr);
    TEST_ASSERT_EQUAL_STRING(testStr.c_str(), textWidget.getStr().c_str());

    /* Default string color */
    TEST_ASSERT_EQUAL_UINT16(ColorDef::get565(TextWidget::DEFAULT_TEXT_COLOR), textWidget.getTextColor().get565());

    /* Set/Get text color */
    textWidget.setTextColor(TEXT_COLOR);
    TEST_ASSERT_EQUAL_UINT16(TEXT_COLOR.get565(), textWidget.getTextColor().get565());

    /* Check for default font */
    TEST_ASSERT_NOT_NULL(textWidget.getFont());
    TEST_ASSERT_EQUAL_PTR(TextWidget::DEFAULT_FONT, textWidget.getFont());
    
    /* Font shall be used for drawing */
    textWidget.update(testGfx);
    TEST_ASSERT_NOT_NULL(testGfx.getFont());
    TEST_ASSERT_EQUAL_PTR(TextWidget::DEFAULT_FONT, testGfx.getFont());

    return;
}

/**
 * Test color.
 */
static void testColor(void)
{
    Color myColorA;
    Color myColorB  = ColorDef::TOMATO;
    Color myColorC  = myColorB;

    /* Default color is black */
    TEST_ASSERT_EQUAL_UINT16(0u, myColorA.get565());

    /* Does the color assignment works? */
    TEST_ASSERT_EQUAL_UINT8(ColorDef::getRed(ColorDef::TOMATO), myColorB.getRed());
    TEST_ASSERT_EQUAL_UINT8(ColorDef::getGreen(ColorDef::TOMATO), myColorB.getGreen());
    TEST_ASSERT_EQUAL_UINT8(ColorDef::getBlue(ColorDef::TOMATO), myColorB.getBlue());

    /* Does the color assignment via copy constructor works? */
    TEST_ASSERT_EQUAL_UINT8(ColorDef::getRed(ColorDef::TOMATO), myColorC.getRed());
    TEST_ASSERT_EQUAL_UINT8(ColorDef::getGreen(ColorDef::TOMATO), myColorC.getGreen());
    TEST_ASSERT_EQUAL_UINT8(ColorDef::getBlue(ColorDef::TOMATO), myColorC.getBlue());

    /* Check the 5-6-5 RGB format conversion. */
    myColorA.set(0x00ffffffu);
    TEST_ASSERT_EQUAL_UINT8(0xffu, myColorA.getRed());
    TEST_ASSERT_EQUAL_UINT8(0xffu, myColorA.getGreen());
    TEST_ASSERT_EQUAL_UINT8(0xffu, myColorA.getBlue());
    TEST_ASSERT_EQUAL_UINT16(0xffffu, myColorA.get565());

    myColorA.set(0x00080408u);
    TEST_ASSERT_EQUAL_UINT8(0x08u, myColorA.getRed());
    TEST_ASSERT_EQUAL_UINT8(0x04u, myColorA.getGreen());
    TEST_ASSERT_EQUAL_UINT8(0x08u, myColorA.getBlue());
    TEST_ASSERT_EQUAL_UINT16(0x0821, myColorA.get565());

    /* Does the color assignment via assignment operator works? */
    myColorA = myColorB;
    TEST_ASSERT_EQUAL_UINT8(myColorB.getRed(), myColorC.getRed());
    TEST_ASSERT_EQUAL_UINT8(myColorB.getGreen(), myColorC.getGreen());
    TEST_ASSERT_EQUAL_UINT8(myColorB.getBlue(), myColorC.getBlue());

    /* Get/Set single colors */
    myColorA.setRed(0x12u);
    myColorA.setGreen(0x34u);
    myColorA.setBlue(0x56u);
    TEST_ASSERT_EQUAL_UINT8(0x12u, myColorA.getRed());
    TEST_ASSERT_EQUAL_UINT8(0x34u, myColorA.getGreen());
    TEST_ASSERT_EQUAL_UINT8(0x56u, myColorA.getBlue());

    return;
}

/**
 * Test the abstract state machine.
 */
static void testStateMachine(void)
{
    TestState       stateA;
    TestState       stateB;
    StateMachine    sm;

    /* State machine has no state yet. */
    TEST_ASSERT_NULL(sm.getState());

    /* Add state A, but don't process it. */
    sm.setState(stateA);
    TEST_ASSERT_NULL(sm.getState());
    TEST_ASSERT_EQUAL_UINT32(0u, stateA.getCallCntEntry());
    TEST_ASSERT_EQUAL_UINT32(0u, stateA.getCallCntExit());

    /* Process it once.
     * Expectation:
     * The entry part is called once and the process part.
     */
    sm.process();
    TEST_ASSERT_EQUAL_UINT32(1u, stateA.getCallCntEntry());
    TEST_ASSERT_EQUAL_UINT32(0u, stateA.getCallCntExit());
    TEST_ASSERT_EQUAL_PTR(static_cast<AbstractState*>(&stateA), sm.getState());

    /* Process it a 2nd time.
     * Expectation:
     * Only the process part is called.
     */
    sm.process();
    TEST_ASSERT_EQUAL_UINT32(1u, stateA.getCallCntEntry());
    TEST_ASSERT_EQUAL_UINT32(0u, stateA.getCallCntExit());

    /* Transistion from A to B. */
    stateA.setState(stateB);
    sm.process();
    sm.process();
    TEST_ASSERT_EQUAL_UINT32(1u, stateA.getCallCntEntry());
    TEST_ASSERT_EQUAL_UINT32(1u, stateA.getCallCntExit());
    TEST_ASSERT_EQUAL_UINT32(1u, stateB.getCallCntEntry());
    TEST_ASSERT_EQUAL_UINT32(0u, stateB.getCallCntExit());

    /* Transistion from B to A. */
    stateB.setState(stateA);
    sm.process();
    sm.process();
    TEST_ASSERT_EQUAL_UINT32(2u, stateA.getCallCntEntry());
    TEST_ASSERT_EQUAL_UINT32(1u, stateA.getCallCntExit());
    TEST_ASSERT_EQUAL_UINT32(1u, stateB.getCallCntEntry());
    TEST_ASSERT_EQUAL_UINT32(1u, stateB.getCallCntExit());

    return;
}

/**
 * Test simple timer.
 */
static void testSimpleTimer(void)
{
    SimpleTimer testTimer;

    /* Timer must be stopped */
    TEST_ASSERT_FALSE(testTimer.isTimerRunning());
    TEST_ASSERT_FALSE(testTimer.isTimeout());

    /* Start and check */
    testTimer.start(0u);
    TEST_ASSERT_TRUE(testTimer.isTimerRunning());
    TEST_ASSERT_TRUE(testTimer.isTimeout());
    TEST_ASSERT_TRUE(testTimer.isTimerRunning());

    /* Stop timer and check again */
    testTimer.stop();
    TEST_ASSERT_FALSE(testTimer.isTimerRunning());
    TEST_ASSERT_FALSE(testTimer.isTimeout());

    /* Restart timer */
    testTimer.restart();
    TEST_ASSERT_TRUE(testTimer.isTimerRunning());
    TEST_ASSERT_TRUE(testTimer.isTimeout());

    /* Start timer and start it again after timeout. */
    testTimer.start(0u);
    TEST_ASSERT_TRUE(testTimer.isTimeout());
    testTimer.start(100u);
    TEST_ASSERT_FALSE(testTimer.isTimeout());
    testTimer.stop();

    return;
}

/**
 * Test progress bar.
 */
static void testProgressBar(void)
{
    TestGfx     testGfx;
    ProgressBar progressBar;
    const char* WIDGET_NAME = "progressBarName";

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

    /* Progress should be now 0% */
    progressBar.update(testGfx);
    TEST_ASSERT_TRUE(testGfx.verify(0, 0, testGfx.width(), testGfx.height(), ColorDef::get565(ColorDef::BLACK)));

    /* Set progress bar to 50% */
    progressBar.setProgress(50u);
    progressBar.update(testGfx);
    TEST_ASSERT_TRUE(testGfx.verify(0, 0, testGfx.width(), testGfx.height() / 2u, ColorDef::get565(ColorDef::RED)));
    TEST_ASSERT_TRUE(testGfx.verify(0, testGfx.height() / 2u, testGfx.width(), testGfx.height() / 2u, ColorDef::get565(ColorDef::BLACK)));

    /* Set progress bar to 100% */
    progressBar.setProgress(100u);
    progressBar.update(testGfx);
    TEST_ASSERT_TRUE(testGfx.verify(0, 0, testGfx.width(), testGfx.height(), ColorDef::get565(ColorDef::RED)));

    return;
}

/**
 * Test Logging.
 * 
 * Note: In order to let these tests pass the formating of the expected logMessage has to stay directly after the LogMessage (__LINE__-1), 
 * otherwise the tests will fail.
 */
static void testLogging(void)
{
    TestLogger      myTestLogger;
    const char*     printBuffer     = NULL;
    const char*     TEST_STRING_1   = "TestMessage";
    const String    TEST_STRING_2   = "TestMessageAsString";
    char            expectedLogMessage[52];
    int             lineNo          = 0;
    
    /* Check intial LogLevel. */
    Logging::getInstance().init(&myTestLogger);
    TEST_ASSERT_EQUAL(Logging::getInstance().getLogLevel() , Logging::LOGLEVEL_ERROR);

    /* Set LogLevel to LOGLEVEL_INFO. */
    Logging::getInstance().setLogLevel(Logging::LOGLEVEL_INFO);
    TEST_ASSERT_EQUAL(Logging::getInstance().getLogLevel() , Logging::LOGLEVEL_INFO);
    
    /* Set LogLevel to LOGLEVEL_ERROR and trigger a LOG_INFO message. */
    Logging::getInstance().setLogLevel(Logging::LOGLEVEL_ERROR);
    LOG_INFO(TEST_STRING_1);
    snprintf(expectedLogMessage, sizeof(expectedLogMessage), "%s", "");
    printBuffer = myTestLogger.getBuffer();
    TEST_ASSERT_EQUAL_STRING(expectedLogMessage, printBuffer);

    /* Check expected error log output, with type const char* string. */
    LOG_ERROR(TEST_STRING_1); lineNo = __LINE__;
    snprintf(expectedLogMessage, sizeof(expectedLogMessage), "ERROR: TestMain.cpp:%d %s\r\n", lineNo, TEST_STRING_1);
    printBuffer = myTestLogger.getBuffer();

    /* Skip timestamp */
    while(('\0' != *printBuffer) && (' ' != *printBuffer))
    {
        ++printBuffer;
    }
    
    if (' ' == *printBuffer)
    {
        ++printBuffer;
    }

    TEST_ASSERT_EQUAL_STRING(expectedLogMessage, printBuffer);

    /* Check expected error log output, with type const String string. */
    LOG_ERROR(TEST_STRING_2);  lineNo = __LINE__;
    snprintf(expectedLogMessage, sizeof(expectedLogMessage), "ERROR: TestMain.cpp:%d %s\r\n", lineNo, TEST_STRING_2.c_str());
    printBuffer = myTestLogger.getBuffer();

    /* Skip timestamp */
    while(('\0' != *printBuffer) && (' ' != *printBuffer))
    {
        ++printBuffer;
    }

    if (' ' == *printBuffer)
    {
        ++printBuffer;
    }

    TEST_ASSERT_EQUAL_STRING(expectedLogMessage, printBuffer);

    return;
}