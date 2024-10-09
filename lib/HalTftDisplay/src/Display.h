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
 * @brief  Graphic TFT display
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup gfx
 *
 * @{
 */

#ifndef DISPLAY_H
#define DISPLAY_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <IDisplay.hpp>
#include <ColorDef.hpp>
#include <TFT_eSPI.h>
#include <YAGfxBitmap.h>

#include "Board.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * This display represents a graphic TFT display.
 */
class Display : public IDisplay
{
public:

    /**
     * Get display instance.
     *
     * @return Display
     */
    static Display& getInstance()
    {
        static Display instance; /* singleton idiom to force initialization in the first usage. */

        return instance;
    }

    /**
     * Initialize base driver for the display.
     *
     * @return If successful, returns true otherwise false.
     */
    bool begin() final
    {
        m_tft.init();
        m_tft.fillScreen(TFT_BLACK);
        m_isOn = true;

        return true;
    }

    /**
     * Show framebuffer on physical display. This may be synchronous
     * or asynchronous.
     */
    void show() final;

    /**
     * The display is ready, when the last physical pixel update is finished.
     * A asynchronous display update, triggered by show() can be observed this way.
     *
     * @return If ready for another update via show(), it will return true otherwise false.
     */
    bool isReady() const final
    {
        return true;
    }

    /**
     * Set brightness from 0 to 255.
     * 255 = max. brightness.
     *
     * @param[in] brightness    Brightness value [0; 255]
     */
    void setBrightness(uint8_t brightness) final
    {
        m_brightness = brightness;
    }

    /**
     * Clear LED matrix.
     */
    void clear() final
    {
        m_tft.fillScreen(TFT_BLACK);
        m_ledMatrix.fillScreen(ColorDef::BLACK);
    }

    /**
     * Get width in pixel.
     *
     * @return Canvas width in pixel
     */
    uint16_t getWidth() const final
    {
        return m_ledMatrix.getWidth();
    }

    /**
     * Get height in pixel.
     *
     * @return Canvas height in pixel
     */
    uint16_t getHeight() const final
    {
        return m_ledMatrix.getHeight();
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
        return m_ledMatrix.getColor(x, y);
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
        return m_ledMatrix.getColor(x, y);
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
        return m_ledMatrix.getFrameBufferXAddr(x, y, length, offset);
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
    const Color* getFrameBufferXAddr(int16_t x, int16_t y, uint16_t length, uint16_t& offset) const  final
    {
        return m_ledMatrix.getFrameBufferXAddr(x, y, length, offset);
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
        return m_ledMatrix.getFrameBufferYAddr(x, y, length, offset);
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
        return m_ledMatrix.getFrameBufferYAddr(x, y, length, offset);
    }

    /**
     * Power display off.
     */
    void off() final;

    /**
     * Power display on.
     */
    void on() final;

    /**
     * Is display powered on?
     * 
     * @return If display is powered on, it will return true otherwise false.
     */
    bool isOn() const final;

private:

    /* The below TFT_* definitions are set in platform.ini build_flags */

    /** Display matrix width in pixels (not T-Display width) */
    static const uint16_t   MATRIX_WIDTH        = CONFIG_LED_MATRIX_WIDTH;

    /** Display matrix height in pixels (not T-Display height) */
    static const uint16_t   MATRIX_HEIGHT       = CONFIG_LED_MATRIX_HEIGHT;

    /** Width of a single matrix pixel in T-Display pixels */
    static const int32_t    PIXEL_WIDTH         = TFT_PIXEL_WIDTH;

    /** Height of a single matrix pixel in T-Display pixels */
    static const int32_t    PIXEL_HEIGHT        = TFT_PIXEL_HEIGHT;

    /** Pixel distance in T-Display pixels */
    static const int32_t    PiXEL_DISTANCE      = TFT_PIXEL_DISTANCE;

    /** T-Display x-axis border size in T-Display pixels */
    static const int32_t    BORDER_X            = (TFT_HEIGHT - (MATRIX_WIDTH * (PIXEL_WIDTH + PiXEL_DISTANCE))) / 2;

    /** T-Display y-axis border size in T-Display pixels */
    static const int32_t    BORDER_Y            = (TFT_WIDTH - (MATRIX_HEIGHT * (PIXEL_HEIGHT + PiXEL_DISTANCE))) / 2;

    /** TFT default brightness */
    static const uint8_t    DEFAULT_BRIGHTNESS  = TFT_DEFAULT_BRIGHTNESS;

    TFT_eSPI                                        m_tft;          /**< T-Display driver */
    YAGfxStaticBitmap<MATRIX_WIDTH, MATRIX_HEIGHT>  m_ledMatrix;    /**< Simulated LED matrix framebuffer */
    uint8_t                                         m_brightness;   /**< Display brightness [0; 255] value. 255 = max. brightness. */
    bool                                            m_isOn;         /**< Is display on? */

    /**
     * Construct display.
     */
    Display();

    /**
     * Destroys display.
     */
    ~Display();

    Display(const Display& display);
    Display& operator=(const Display& display);

    /**
     * Draw a single pixel on the display.
     *
     * @param[in] x     x-coordinate
     * @param[in] y     y-coordinate
     * @param[in] color Pixel color in RGB888 format
     */
    void drawPixel(int16_t x, int16_t y, const Color& color) final
    {
        m_ledMatrix.drawPixel(x, y, color);
    }
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* DISPLAY_H */

/** @} */