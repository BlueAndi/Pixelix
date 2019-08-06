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

#include <LinkedList.hpp>
#include <Widget.hpp>

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
class TestGfx : public IGfx
{
public:

    /**
     * Constructs a graphic interface for testing purposes.
     */
    TestGfx() :
        IGfx(WIDTH, HEIGHT),
        m_buffer()
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
        TEST_ASSERT_LESS_OR_EQUAL(WIDTH, x);
        TEST_ASSERT_LESS_OR_EQUAL(HEIGHT, y);

        m_buffer[x * y] = color;

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

    static const int16_t    WIDTH   = 32;   /**< Drawing area width in pixel */
    static const int16_t    HEIGHT  = 8;    /**< Drawing area height in pixel */

private:

    uint16_t m_buffer[WIDTH * HEIGHT];  /**< Display buffer, containing all pixels. */

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
        Widget(0, 0),
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
    void update(IGfx& gfx)
    {
        int16_t x = 0;
        int16_t y = 0;

        for(x = 0; x < WIDTH; ++x)
        {
            for(y = 0; y < HEIGHT; ++y)
            {
                gfx.drawPixel(x, y, m_color);
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

    static const int16_t    WIDTH   = 10;   /**< Widget width in pixel */
    static const int16_t    HEIGHT  = 5;    /**< Widget height in pixel */

private:

    uint16_t m_color;   /**< Pen color, used to draw the widget. */

};

/******************************************************************************
 * Prototypes
 *****************************************************************************/

static void testDoublyLinkedList(void);
static void verifyWidget(TestGfx& gfx, uint16_t posX, uint16_t posY, uint16_t color);
static void testWidget(void);

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

    return UNITY_END();
}

/******************************************************************************
 * Local functions
 *****************************************************************************/

/**
 * Doubly linked list tests.
 */
static void testDoublyLinkedList(void)
{
    LinkedList<int> list;
    int*            element = NULL;
    int             value   = 1;
    int             index   = 0;
    const int       max     = 3;

    /* List is empty. */
    TEST_ASSERT_NULL(list.first());
    TEST_ASSERT_NULL(list.last());
    TEST_ASSERT_NULL(list.current());
    TEST_ASSERT_FALSE(list.next());
    TEST_ASSERT_FALSE(list.prev());

    /* Add one element. */
    TEST_ASSERT_TRUE(list.append(value));

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

    TEST_ASSERT_NULL(list.first());
    TEST_ASSERT_NULL(list.last());
    TEST_ASSERT_NULL(list.current());
    TEST_ASSERT_FALSE(list.next());
    TEST_ASSERT_FALSE(list.prev());

    /* Add more elements */
    for(index = 1; index <= max; ++index)
    {
        TEST_ASSERT_TRUE(list.append(index));
    }

    TEST_ASSERT_NOT_NULL(list.first());
    TEST_ASSERT_EQUAL_INT(1, *list.first());

    TEST_ASSERT_NOT_NULL(list.last());
    TEST_ASSERT_EQUAL_INT(max, *list.last());

    /* Select element for element, from head to tail. */
    for(index = 1; index <= max; ++index)
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
    for(index = max; index > 0; --index)
    {
        TEST_ASSERT_NOT_NULL(list.current());
        TEST_ASSERT_EQUAL_INT(index, *list.current());

        if (index > 1)
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
    }

    TEST_ASSERT_NULL(list.first());
    TEST_ASSERT_NULL(list.last());
    TEST_ASSERT_NULL(list.current());
    TEST_ASSERT_FALSE(list.next());
    TEST_ASSERT_FALSE(list.prev());

    /* Insert elements again */
    for(index = 1; index <= max; ++index)
    {
        TEST_ASSERT_TRUE(list.append(index));
    }

    /* Copy it via copy constructor */
    {
        LinkedList<int> copyOfList = list;

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

    /* Copy it via assignment */
    {
        LinkedList<int> copyOfList;
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
 * Verify widget at given position. It must be there with the given color.
 * 
 * @param[in] gfx   Graphics interface, including display buffer.
 * @param[in] posX  Top left x-coordinate
 * @param[in] posY  Top left y-coordinate
 * @param[in] color Color of widget drawing
 */
static void verifyWidget(TestGfx& gfx, uint16_t posX, uint16_t posY, uint16_t color)
{
    uint16_t    x       = 0u;
    uint16_t    y       = 0u;
    uint16_t*   buffer  = gfx.getBuffer();

    TEST_ASSERT_NOT_NULL(buffer);

    for(x = 0u; x < TestWidget::WIDTH; ++x)
    {
        for(y = 0u; y < TestWidget::HEIGHT; ++y)
        {
            TEST_ASSERT_EQUAL_UINT16(color, buffer[x * y]);
        }
    }

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
    uint16_t    color       = 0x1234;

    /* Current position must be (0, 0) */
    testWidget.getPos(posX, posY);
    TEST_ASSERT_EQUAL_INT16(0, posX);
    TEST_ASSERT_EQUAL_INT16(0, posY);

    /* Move widget and verify position again. */
    testWidget.move(10, 20);
    testWidget.getPos(posX, posY);
    TEST_ASSERT_EQUAL_INT16(10, posX);
    TEST_ASSERT_EQUAL_INT16(20, posY);

    /* Draw widget, the current widget position must not be considered. */
    testWidget.setPenColor(color);
    testWidget.update(testGfx);
    verifyWidget(testGfx, 0, 0, color);

    return;
}
