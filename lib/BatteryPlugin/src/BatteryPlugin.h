/* MIT License
 *
 * Copyright (c) 2019 - 2023 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Shows current battery state of charge.
 * @author Andreas Merkle <web@blue-andi.de>
 * @addtogroup plugin
 *
 * @{
 */

#ifndef BATTERYPLUGIN_H
#define BATTERYPLUGIN_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include "Plugin.hpp"

#include <SimpleTimer.hpp>
#include <ISensorChannel.hpp>
#include <Mutex.hpp>
#include <YAGfxBitmap.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * This plugins displays the battery state of charge in a graphical way.
 */
class BatteryPlugin : public Plugin
{
public:

    /**
     * Constructs the plugin.
     *
     * @param[in] name  Plugin name
     * @param[in] uid   Unique id
     */
    BatteryPlugin(const String& name, uint16_t uid) :
        Plugin(name, uid),
        m_batterySymbol(),
        m_mutex(),
        m_sensorUpdateTimer(),
        m_socSensorCh(nullptr),
        m_stateOfCharge(0U),
        m_socBarX(0),
        m_socBarY(0),
        m_socBarWidth(0),
        m_socBarHeight(0)
    {
        (void)m_mutex.create();
    }

    /**
     * Enumeration to choose the page to be displayed.
     */    
    enum PageDisplay
    {
        TEMPERATURE = 0,   /**< Display Temperature in degree Celsius. */
        HUMIDITY,          /**< Display humidity in %. */
        PAGE_MAX           /**< Number of pages, use for simple switch (by using a PAGE_MAX as modulo operator) */ 
    };

    /**
     * Destroys the plugin.
     */
    ~BatteryPlugin()
    {
        m_batterySymbol.release();
        m_mutex.destroy();
    }

    /**
     * Plugin creation method, used to register on the plugin manager.
     *
     * @param[in] name  Plugin name
     * @param[in] uid   Unique id
     *
     * @return If successful, it will return the pointer to the plugin instance, otherwise nullptr.
     */
    static IPluginMaintenance* create(const String& name, uint16_t uid)
    {
        return new(std::nothrow) BatteryPlugin(name, uid);
    }

    /**
     * Start the plugin. This is called only once during plugin lifetime.
     * It can be used as deferred initialization (after the constructor)
     * and provides the canvas size.
     * 
     * If your display layout depends on canvas or font size, calculate it
     * here.
     * 
     * Overwrite it if your plugin needs to know that it was installed.
     * 
     * @param[in] width     Display width in pixel
     * @param[in] height    Display height in pixel
     */
    void start(uint16_t width, uint16_t height) final;

   /**
     * Stop the plugin. This is called only once during plugin lifetime.
     * It can be used as a first clean-up, before the plugin will be destroyed.
     * 
     * Overwrite it if your plugin needs to know that it will be uninstalled.
     */
    void stop() final;

    /**
     * Process the plugin.
     * Overwrite it if your plugin has cyclic stuff to do without being in a
     * active slot.
     * 
     * @param[in] isConnected   The network connection status. If network
     *                          connection is established, it will be true otherwise false.
     */
    void process(bool isConnected) final; 
    
    /**
     * Update the display.
     * The scheduler will call this method periodically.
     *
     * @param[in] gfx   Display graphics interface
     */
    void update(YAGfx& gfx) final;

private:

    /**
     * Read sensor only every 10s.
     */
    static const uint32_t   SENSOR_UPDATE_PERIOD = SIMPLE_TIMER_SECONDS(10U);

    YAGfxDynamicBitmap      m_batterySymbol;        /**< The battery symbol. */
    MutexRecursive          m_mutex;                /**< Mutex to protect against concurrent access. */
    SimpleTimer             m_sensorUpdateTimer;    /**< Time used for cyclic sensor reading. */
    ISensorChannel*         m_socSensorCh;          /**< Battery sensor SOC channel. */
    uint32_t                m_stateOfCharge;        /**< Last read state of charge in [0; 100] %. */
    int16_t                 m_socBarX;              /**< x-coordinate of graphical state of charge bar. */
    int16_t                 m_socBarY;              /**< y-coordinate of graphical state of charge bar. */
    int16_t                 m_socBarWidth;          /**< Width in pixel of state of charge bar. */
    int16_t                 m_socBarHeight;         /**< Height in pixel of state of charge bar. */

    /**
     * Divide and round up.
     * 
     * @param[in]   dividend    The dividend.
     * @param[in]   divisor     The divisor.
     * 
     * @return The result of the divison.
     */
    uint16_t divideAndRound(uint16_t dividend, uint16_t divisor);

    /**
     * Draw the state of charge on the display.
     *
     * @param[in] gfx   Display graphics interface
     */
    void drawStateOfCharge(YAGfx& gfx);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* BATTERYPLUGIN_H */

/** @} */