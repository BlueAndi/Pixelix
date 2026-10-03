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
 * @brief  Display driver for the native environment
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * There is no physical display on the host. The driver keeps the last shown
 * framebuffer, so the webinterface can show it. See the websocket command
 * GETDISP and the display page of the webinterface.
 *
 * @addtogroup TEST
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
#include <YAGfxBitmap.h>
#include <ColorDef.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/
class LedGridSim;

/**
 * Display driver for the native environment.
 *
 * It behaves like a LED matrix of CONFIG_LED_MATRIX_WIDTH x
 * CONFIG_LED_MATRIX_HEIGHT pixels, but without any physical output.
 */
class DisplayDrv : public IDisplayDrv
{
public:

    /**
     * Constructs the display driver.
     */
    DisplayDrv();

    /**
     * Destroys the display driver.
     */
    virtual ~DisplayDrv();

    /**
     * Initialize the display driver.
     *
     * @return If successful, it will return true otherwise false.
     */
    bool begin() final;

    /**
     * Show the framebuffer on the display.
     * The framebuffer is kept, so the webinterface can show it.
     *
     * @param[in] bitmap    Framebuffer to show.
     */
    void show(const YAGfxBitmap& bitmap) final;

    /**
     * The display is ready, when the last pixel update is finished.
     *
     * @return Always true, there is no physical display on the host.
     */
    bool isReady() const final
    {
        return true;
    }

    /**
     * Set the brightness.
     *
     * @param[in] brightness    Brightness value [0; 255]
     */
    void setBrightness(uint8_t brightness) final;

    /**
     * Clear the display.
     */
    void clear() final;

    /**
     * Power the display off.
     */
    void off() final;

    /**
     * Power the display on.
     */
    void on() final;

    /**
     * Is the display powered on?
     *
     * @return If the display is powered on, it will return true otherwise false.
     */
    bool isOn() const final;

private:

    LedGridSim* m_simulationInterface; /**< Simulation interface to show the framebuffer on the host. */

    DisplayDrv(const DisplayDrv& drv);
    DisplayDrv& operator=(const DisplayDrv& drv);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* DISPLAY_DRV_H */

/** @} */
