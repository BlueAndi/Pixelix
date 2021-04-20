/* MIT License
 *
 * Copyright (c) 2019 - 2021 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Reads temperature and humidity from a sensor and displays it
 * @author Flavio Curti <fcu-github@no-way.org>
 * @addtogroup plugin
 *
 * @{
 */

#ifndef __TEMPHUMIDPLUGIN_H__
#define __TEMPHUMIDPLUGIN_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/
#define DHTTYPE DHTesp::DHT11   // DHT 11 sensor
#define DHTTYPE DHTesp::DHT11   // DHT 11 sensor


/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include "Plugin.hpp"

#include <DHTesp.h>
#include <SimpleTimer.hpp>

#include <Canvas.h>
#include <BitmapWidget.h>
#include <TextWidget.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * TempHumid plugin.
 *
 * This plugins displays the temperature and humidity read from a sensor
 * currently implemented sensors:
 * - DHT Sensors supported by DHTesp library
 * - TODO: sht3x
 */
class TempHumidPlugin : public Plugin
{
public:

    /**
     * Constructs the plugin.
     *
     * @param[in] name  Plugin name
     * @param[in] uid   Unique id
     */
    TempHumidPlugin(const String& name, uint16_t uid) :
        Plugin(name, uid),
        m_textCanvas(nullptr),
        m_iconCanvas(nullptr),
        m_bitmapWidget(),
        m_textWidget("\\calign?"),
        m_text(),
        m_page(0U),
        m_pageTime(),
        m_timer(),
        m_xMutex(nullptr),
        m_dht(),
        m_humid(0),
        m_temp(0),
        m_last(0),
        m_slotInterf(nullptr)
    {
        /* Move the text widget one line lower for better look. */
        m_textWidget.move(0, 1);

        m_xMutex = xSemaphoreCreateMutex();        
    }

    /**
     * Destroys the plugin.
     */
    ~TempHumidPlugin()
    {
        if (nullptr != m_iconCanvas)
        {
            delete m_iconCanvas;
            m_iconCanvas = nullptr;
        }

        if (nullptr != m_textCanvas)
        {
            delete m_textCanvas;
            m_textCanvas = nullptr;
        }

        if (nullptr != m_xMutex)
        {
            vSemaphoreDelete(m_xMutex);
            m_xMutex = nullptr;
        }        
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
        return new TempHumidPlugin(name, uid);
    }

    /**
     * Set the slot interface, which the plugin can used to request information
     * from the slot, it is plugged in.
     *
     * @param[in] slotInterf    Slot interface
     */
    void setSlot(const ISlotPlugin* slotInterf) final;

    /**
     * This method will be called in case the plugin is set active, which means
     * it will be shown on the display in the next step.
     *
     * @param[in] gfx   Display graphics interface
     */
    void active(IGfx& gfx) final;

    /**
     * Update the display.
     * The scheduler will call this method periodically.
     *
     * @param[in] gfx   Display graphics interface
     */
    void update(IGfx& gfx) final;

    /**
     * Start the plugin.
     * Overwrite it if your plugin needs to know that it was installed.
     */
    void start() final;

    /**
     * Process the plugin.
     * Overwrite it if your plugin has cyclic stuff to do without being in a
     * active slot.
     */
    void process(void) final;    

    /**
     * Protect against concurrent access.
     */
    void lock(void) const;

    /**
     * Unprotect against concurrent access.
     */
    void unlock(void) const;

private:
    /**
     * Icon width in pixels.
     */
    static const int16_t        ICON_WIDTH          = 8;

    /**
     * Icon height in pixels.
     */
    static const int16_t        ICON_HEIGHT         = 8;

    /**
     * Path to the temperature icon
     */    
    static const char*          IMAGE_PATH_TEMP_ICON;

    /**
     * Path to the humidity icon
     */
    static const char*          IMAGE_PATH_HUMID_ICON;

    /**
     * Read sensor only every N milliseconds (currently 90 seconds)
     */
    static const uint32_t       SENSOR_UPDATE_PERIOD = (90U * 1000U);

    Canvas*                     m_textCanvas;               /**< Canvas used for the text widget. */
    Canvas*                     m_iconCanvas;               /**< Canvas used for the bitmap widget. */
    BitmapWidget                m_bitmapWidget;             /**< Bitmap widget, used to show the icon. */
    TextWidget                  m_textWidget;               /**< Text widget, used for showing the text. */
    String                      m_text;

    uint8_t                     m_page;                     /**< Number of page, which to show */
    unsigned long               m_pageTime;                 /**< How long to show page (1/4 slot-time or 10s default) */    
    SimpleTimer                 m_timer;                    /**< Timer for changing page */
    SemaphoreHandle_t           m_xMutex;                   /**< Mutex to protect against concurrent access. */

    DHTesp                      m_dht;                      /**< sensor object*/
    float                       m_humid;                    /**< last sensor humidity value*/
    float                       m_temp;                     /**< last sensor temperature value*/
    unsigned long               m_last;                     /**< last sensor read timestamp*/
    const ISlotPlugin*          m_slotInterf;               /**< Slot interface */

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __TEMPHUMIDPLUGIN_H__ */

/** @} */