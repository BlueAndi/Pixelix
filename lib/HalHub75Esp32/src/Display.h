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
 * @brief  HUB75 matrix display
 * @author Mariano Dupont <marianomd@gmail.com>
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
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

#include <ColorDef.hpp>
#include <YAGfxBitmap.h>

#include "Board.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * This display represents a HUB75 matrix.
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
        return m_panel.begin();
    }

    /**
     * Show framebuffer on physical display. This may be synchronous
     * or asynchronous.
     */
    void show() final
    {
        int16_t y;
        int16_t x;

        for(y = 0; y < Board::LedMatrix::height; ++y)
        {
            for(x = 0; x < Board::LedMatrix::width; ++x)
            {
                Color& color = m_ledMatrix.getColor(x, y);

#if CONFIG_DISPLAY_ROTATE180 != 0
                m_panel.drawPixelRGB888(
                    Board::LedMatrix::width - x - 1, 
                    Board::LedMatrix::height - y - 1,
                    color.getRed(), color.getGreen(), color.getBlue());
#else
                m_panel.drawPixelRGB888(x, y, color.getRed(), color.getGreen(), color.getBlue());
#endif
            }
        }
    }

    /**
     * The display is ready, when the last physical pixel update is finished.
     * A asynchronous display update, triggered by show() can be observed this way.
     *
     * @return If ready for another update via show(), it will return true otherwise false.
     */
    bool isReady() const final
    {
        /* Nothing to do. */
        return true;
    }

    /**
     * Set brightness from 0 to 255.
     *
     * @param[in] brightness    Brightness value [0; 255]
     */
    void setBrightness(uint8_t brightness) final
    {
        /* To protect the electronic parts, the luminance will be scaled down
         * according to the max. supply current.
         */
        const uint8_t SAFE_LUMINANCE =
            (Board::LedMatrix::supplyCurrentMax * brightness) /
            (Board::LedMatrix::maxCurrentPerLed * Board::LedMatrix::width *Board::LedMatrix::height);

        m_panel.setBrightness(SAFE_LUMINANCE);
    }

    /**
     * Clear display.
     */
    void clear() final
    {
        m_ledMatrix.fillScreen(ColorDef::BLACK);
    }

    /**
     * Get width in pixel.
     *
     * @return Canvas width in pixel
     */
    uint16_t getWidth() const final
    {
        return Board::LedMatrix::width;
    }

    /**
     * Get height in pixel.
     *
     * @return Canvas height in pixel
     */
    uint16_t getHeight() const final
    {
        return Board::LedMatrix::height;
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

    /**
     * HUB75 I2S pin configuration.
     */
    static const HUB75_I2S_CFG::i2s_pins    I2S_PINS;

    /**
     * HUB75 matrix configuration.
     */
    static const HUB75_I2S_CFG              MATRIX_CFG;

    /**
     * HUB75 panel driver.
     */
    MatrixPanel_I2S_DMA                                                     m_panel;

    /**
     * The LED matrix framebuffer.
     * This is the drawback for the direct color manipulation via getColor().
     */
    YAGfxStaticBitmap<Board::LedMatrix::width, Board::LedMatrix::height>    m_ledMatrix;

    /**
     * Is display on?
     */
    bool                                                                    m_isOn;

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