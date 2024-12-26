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
 * @brief  Graphics interface for testing purposes.
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup common
 *
 * @{
 */

#ifndef YAGFX_TEST_HPP
#define YAGFX_TEST_HPP

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <unity.h>
#include <stdio.h>
#include <Util.h>
#include <YAGfx.h>
#include <YAGfxBitmap.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Graphics interface for testing purposes.
 * It provides all relevant methods from the Adafruit GFX, which are used.
 */
class YAGfxTest : public YAGfx
{
public:

    /**
     * Constructs a graphic interface for testing purposes.
     */
    YAGfxTest() :
        YAGfx(),
        m_buffer(),
        m_callCounterDrawPixel(0U)
    {
        uint16_t index = 0U;

        for(index = 0U; index < UTIL_ARRAY_NUM(m_buffer); ++index)
        {
            m_buffer[index] = 0U;
        }
    }

    /**
     * Destroys the graphic interface.
     */
    ~YAGfxTest()
    {
    }

    /**
     * Get with in pixels.
     * 
     * @return Width in pixels
     */
    uint16_t getWidth() const final
    {
        return WIDTH;
    }

    /**
     * Get height in pixels.
     * 
     * @return Height in pixels
     */
    uint16_t getHeight() const final
    {
        return HEIGHT;
    }

    /**
     * Get pixel color at given position.
     *
     * @param[in] x x-coordinate
     * @param[in] y y-coordinate
     *
     * @return Color in RGB888 format.
     */
    Color& getColor(int16_t x, int16_t y) final
    {
        /* Out of bounds check */
        TEST_ASSERT_GREATER_OR_EQUAL_INT16(0, x);
        TEST_ASSERT_GREATER_OR_EQUAL_INT16(0, y);
        TEST_ASSERT_LESS_OR_EQUAL_INT16(WIDTH, x);
        TEST_ASSERT_LESS_OR_EQUAL_INT16(HEIGHT, y);

        return m_buffer[x + y * WIDTH];
    }

    /**
     * Get pixel color at given position.
     *
     * @param[in] x x-coordinate
     * @param[in] y y-coordinate
     *
     * @return Color in RGB888 format.
     */
    const Color& getColor(int16_t x, int16_t y) const final
    {
        /* Out of bounds check */
        TEST_ASSERT_GREATER_OR_EQUAL_INT16(0, x);
        TEST_ASSERT_GREATER_OR_EQUAL_INT16(0, y);
        TEST_ASSERT_LESS_OR_EQUAL_INT16(WIDTH, x);
        TEST_ASSERT_LESS_OR_EQUAL_INT16(HEIGHT, y);

        return m_buffer[x + y * WIDTH];
    }
    
    /**
     * Draw a single pixel in the matrix and ensure that the drawing borders
     * are not violated.
     *
     * @param[in] x     x-coordinate
     * @param[in] y     y-coordinate
     * @param[in] color Pixel color
     */
    void drawPixel(int16_t x, int16_t y, const Color& color) final
    {
        if ((0 > x) ||
            (0 > y) ||
            (WIDTH < x) ||
            (HEIGHT < y))
        {
            dumpSimple();
        }

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
    Color* getBuffer()
    {
        return m_buffer;
    }

    /**
     * Get call counter of @drawPixel.
     *
     * @return Call counter
     */
    uint32_t getCallCounterDrawPixel() const
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
    void dump() const
    {
        uint16_t    x   = 0U;
        uint16_t    y   = 0U;

        for(y = 0U; y < HEIGHT; ++y)
        {
            for(x = 0U; x < WIDTH; ++x)
            {
                if (0u < x)
                {
                    ::printf(" ");
                }

                ::printf("0x%04X", static_cast<uint32_t>(m_buffer[x + WIDTH * y]));
            }

            ::printf("\n");
        }

        return;
    }

    /**
     * Dump display buffer to console by using a "*" for a coloured pixel.
     */
    void dumpSimple() const
    {
        uint16_t    x   = 0U;
        uint16_t    y   = 0U;

        for(y = 0U; y < HEIGHT; ++y)
        {
            for(x = 0U; x < WIDTH; ++x)
            {
                if (0U == m_buffer[x + WIDTH * y])
                {
                    ::printf("_");
                }
                else
                {
                    ::printf("*");
                }
            }

            ::printf("\n");
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
    bool verify(int16_t posX, int16_t posY, uint16_t width, uint16_t height, const Color& color)
    {
        uint16_t    x               = 0U;
        uint16_t    y               = 0U;
        bool        isSuccessful    = true;

        TEST_ASSERT_LESS_OR_EQUAL(WIDTH, posX + width);
        TEST_ASSERT_LESS_OR_EQUAL(HEIGHT, posY + height);

        while((height > y) && (true == isSuccessful))
        {
            x = 0U;
            while((width > x) && (true == isSuccessful))
            {
                if (color != m_buffer[posX + x + (posY + y) * WIDTH])
                {
                    dump();
                    ::printf("x = %d, y = %d\r\n", posX + x, posY + y);
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
    void fill(const Color& color)
    {
        uint16_t    x   = 0U;
        uint16_t    y   = 0U;

        for(y = 0U; y < HEIGHT; ++y)
        {
            for(x = 0U; x < WIDTH; ++x)
            {
                m_buffer[x + y * WIDTH] = color;
            }
        }

        return;
    }

    /**
     * Get the address inside the framebuffer at certain coordinates.
     * If the requested length is not available, it will return nullptr.
     * 
     * To address pixel by pixel on the x-axis, the returned offset shall be considered.
     * Otherwise its not guaranteed to address out of bounds!
     * 
     * @param[in] x         x-coordinate
     * @param[in] y         y-coordinate
     * @param[in] length    Requested number of colors on x-axis.
     * @param[out] offset   Address offset in pixel which to use to calculate address of next pixel.
     * 
     * @return Address in the framebuffer or nullptr.
     */
    Color* getFrameBufferXAddr(int16_t x, int16_t y, uint16_t length, uint16_t& offset) final
    {
        Color* addr = nullptr;

        if ((0 <= x) &&
            (0 <= y) &&
            (WIDTH >= (x + length)) &&
            (HEIGHT > y))
        {
            addr    = &m_buffer[x + y * WIDTH];
            offset  = 1U;
        }

        return addr;
    }

    /**
     * Get the address inside the framebuffer at certain coordinates.
     * If the requested length is not available, it will return nullptr.
     * 
     * To address pixel by pixel on the x-axis, the returned offset shall be considered.
     * Otherwise its not guaranteed to address out of bounds!
     * 
     * @param[in] x         x-coordinate
     * @param[in] y         y-coordinate
     * @param[in] length    Requested number of colors on x-axis.
     * @param[out] offset   Address offset in pixel which to use to calculate address of next pixel.
     * 
     * @return Address in the framebuffer or nullptr.
     */
    const Color* getFrameBufferXAddr(int16_t x, int16_t y, uint16_t length, uint16_t& offset) const final
    {
        const Color* addr = nullptr;

        if ((0 <= x) &&
            (0 <= y) &&
            (WIDTH >= (x + length)) &&
            (HEIGHT > y))
        {
            addr    = &m_buffer[x + y * WIDTH];
            offset  = 1U;
        }

        return addr;
    }

    /**
     * Get the address inside the framebuffer at certain coordinates.
     * If the requested length is not available, it will return nullptr.
     * 
     * To address pixel by pixel on the y-axis, the returned offset shall be considered.
     * Otherwise its not guaranteed to address out of bounds!
     * 
     * @param[in] x         x-coordinate
     * @param[in] y         y-coordinate
     * @param[in] length    Requested number of colors on y-axis.
     * @param[out] offset   Address offset in pixel which to use to calculate address of next pixel.
     * 
     * @return Address in the framebuffer or nullptr.
     */
    Color* getFrameBufferYAddr(int16_t x, int16_t y, uint16_t length, uint16_t& offset) final
    {
        Color* addr = nullptr;

        if ((0 <= x) &&
            (0 <= y) &&
            (WIDTH > x) &&
            (HEIGHT >= (y + length)))
        {
            addr    = &m_buffer[x + y * WIDTH];
            offset  = WIDTH;
        }

        return addr;
    }

    /**
     * Get the address inside the framebuffer at certain coordinates.
     * If the requested length is not available, it will return nullptr.
     * 
     * To address pixel by pixel on the y-axis, the returned offset shall be considered.
     * Otherwise its not guaranteed to address out of bounds!
     * 
     * @param[in] x         x-coordinate
     * @param[in] y         y-coordinate
     * @param[in] length    Requested number of colors on y-axis.
     * @param[out] offset   Address offset in pixel which to use to calculate address of next pixel.
     * 
     * @return Address in the framebuffer or nullptr.
     */
    const Color* getFrameBufferYAddr(int16_t x, int16_t y, uint16_t length, uint16_t& offset) const final
    {
        const Color* addr = nullptr;

        if ((0 <= x) &&
            (0 <= y) &&
            (WIDTH > x) &&
            (HEIGHT >= (y + length)))
        {
            addr    = &m_buffer[x + y * WIDTH];
            offset  = WIDTH;
        }

        return addr;
    }

    static const uint16_t   WIDTH   = 32U;  /**< Drawing area width in pixel */
    static const uint16_t   HEIGHT  = 8U;   /**< Drawing area height in pixel */

private:

    Color       m_buffer[WIDTH * HEIGHT];   /**< Display buffer, containing all pixels. */
    uint32_t    m_callCounterDrawPixel;     /**< Call counter for @drawPixel */

    YAGfxTest(const YAGfxTest& gfx);
    YAGfxTest& operator=(const YAGfxTest& gfx);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* YAGFX_TEST_HPP */

/** @} */