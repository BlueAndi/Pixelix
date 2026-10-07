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
 * @brief  Graphic TFT display driver
 * @author Andreas Merkle <web@blue-andi.de>
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
#include <ColorDef.hpp>
#include <TFT_eSPI.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * This display driver controls a graphic TFT display.
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
        m_tft.init();
        m_tft.fillScreen(TFT_BLACK);
        m_isOn = true;

        return true;
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
     * 255 = max. brightness.
     *
     * @param[in] brightness    Brightness value [0; 255]
     */
    void setBrightness(uint8_t brightness) final
    {
        m_brightness = brightness;
    }

    /**
     * Clear display.
     */
    void clear() final
    {
        m_tft.fillScreen(TFT_BLACK);
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
    static const uint16_t MATRIX_WIDTH      = CONFIG_LED_MATRIX_WIDTH;

    /** Display matrix height in pixels (not T-Display height) */
    static const uint16_t MATRIX_HEIGHT     = CONFIG_LED_MATRIX_HEIGHT;

    /** Width of a single matrix pixel in T-Display pixels */
    static const int32_t PIXEL_WIDTH        = TFT_PIXEL_WIDTH;

    /** Height of a single matrix pixel in T-Display pixels */
    static const int32_t PIXEL_HEIGHT       = TFT_PIXEL_HEIGHT;

    /** Pixel distance in T-Display pixels */
    static const int32_t PIXEL_DISTANCE     = TFT_PIXEL_DISTANCE;

    /** T-Display x-axis border size in T-Display pixels */
    static const int32_t BORDER_X           = (TFT_HEIGHT - (MATRIX_WIDTH * (PIXEL_WIDTH + PIXEL_DISTANCE))) / 2;

    /** T-Display y-axis border size in T-Display pixels */
    static const int32_t BORDER_Y           = (TFT_WIDTH - (MATRIX_HEIGHT * (PIXEL_HEIGHT + PIXEL_DISTANCE))) / 2;

    /** TFT default brightness */
    static const uint8_t DEFAULT_BRIGHTNESS = TFT_DEFAULT_BRIGHTNESS;

    TFT_eSPI             m_tft;        /**< T-Display driver */
    uint8_t              m_brightness; /**< Display brightness [0; 255] value. 255 = max. brightness. */
    bool                 m_isOn;       /**< Is display on? */

    DisplayDrv(const DisplayDrv& display);
    DisplayDrv& operator=(const DisplayDrv& display);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* DISPLAY_DRV_H */

/** @} */