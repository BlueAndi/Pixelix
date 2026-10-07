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
 * @file   IDisplayDrv.h
 * @brief  Display driver interface
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup HAL
 *
 * @{
 */

#ifndef IDISPLAYDRV_H
#define IDISPLAYDRV_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <YAGfxBitmap.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * The display driver interface abstracts the hardware specific display driver.
 * It is used by the display interface to access the physical display.
 */
class IDisplayDrv
{
public:

    /**
     * Destroys the display driver interface.
     */
    virtual ~IDisplayDrv()
    {
    }

    /**
     * Initialize base driver for the display.
     *
     * @return If successful, returns true otherwise false.
     */
    virtual bool begin()                           = 0;

    /**
     * Show framebuffer on physical display. This may be synchronous
     * or asynchronous.
     *
     * @param[in] bitmap    Framebuffer to show on physical display.
     */
    virtual void show(const YAGfxBitmap& bitmap)   = 0;

    /**
     * The display is ready, when the last physical pixel update is finished.
     * A asynchronous display update, triggered by show() can be observed this way.
     *
     * @return If ready for another update via show(), it will return true otherwise false.
     */
    virtual bool isReady() const                   = 0;

    /**
     * Set brightness from 0 to 255.
     *
     * @param[in] brightness    Brightness value [0; 255]
     */
    virtual void setBrightness(uint8_t brightness) = 0;

    /**
     * Clear display.
     */
    virtual void clear()                           = 0;

    /**
     * Power display off.
     */
    virtual void off()                             = 0;

    /**
     * Power display on.
     */
    virtual void on()                              = 0;

    /**
     * Is display powered on?
     *
     * @return If display is powered on, it will return true otherwise false.
     */
    virtual bool isOn() const                      = 0;

protected:

    /**
     * Constructs the display driver interface.
     */
    IDisplayDrv()
    {
    }

private:
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* IDISPLAYDRV_H */

/** @} */