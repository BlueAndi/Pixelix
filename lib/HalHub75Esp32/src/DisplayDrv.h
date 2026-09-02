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
 * @file   DisplayDrv.h
 * @brief  HUB75 matrix display driver
 * @author Mariano Dupont <marianomd@gmail.com>
 *
 * @addtogroup HAL
 *
 * @{
 */

#ifndef DISPLAY_DRV_H
#define DISPLAY_DRV_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <IDisplayDrv.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

#include <ColorDef.hpp>
#include <BoardConstant.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * This display driver controls a HUB75 matrix.
 */
class DisplayDrv : public IDisplayDrv
{
public:

    /**
     * Construct display driver.
     */
    DisplayDrv();

    /**
     * Destroys display driver.
     */
    virtual ~DisplayDrv();

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
     *
     * @param[in] bitmap    Framebuffer to show on physical display.
     */
    void show(const YAGfxBitmap& bitmap) final;

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
     *
     * @param[in] brightness    Brightness value [0; 255]
     */
    void setBrightness(uint8_t brightness) final
    {
        /* To protect the electronic parts, the luminance will be scaled down
         * according to the max. supply current.
         */
        const uint8_t SAFE_LUMINANCE =
            (BoardConstant::SUPPLY_CURRENT_MAX * brightness) /
            (BoardConstant::MAX_CURRENT_PER_LED * CONFIG_LED_MATRIX_WIDTH * CONFIG_LED_MATRIX_HEIGHT);

        m_panel.setBrightness(SAFE_LUMINANCE);
    }

    /**
     * Clear display.
     */
    void clear() final
    {
        m_panel.fillScreen(ColorDef::BLACK);
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

    /** HUB75 I2S pin configuration. */
    static const HUB75_I2S_CFG::i2s_pins I2S_PINS;

    /** HUB75 matrix configuration. */
    static const HUB75_I2S_CFG MATRIX_CFG;

    /** HUB75 panel driver. */
    MatrixPanel_I2S_DMA m_panel;

    /** Is display on? */
    bool m_isOn;

    DisplayDrv(const DisplayDrv& display);
    DisplayDrv& operator=(const DisplayDrv& display);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* DISPLAY_DRV_H */

/** @} */