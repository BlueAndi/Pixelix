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
 * @brief  DHT-11 Plugin
 * @author Flavio Curti <fcu-github@no-way.org>
 *
 * @addtogroup plugin
 *
 * @{
 */

#ifndef __DHTPLUGIN_H__
#define __DHTPLUGIN_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/
#define DHTPIN 5U     // GPIO 5
#define DHTTYPE DHT11   // DHT 11

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include "Plugin.hpp"

#include <SimpleTimer.hpp>
#include <DHT.h>

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
 * Test plugin.
 *
 * Use this plugin to check whether your LED matrix topology (layout) is
 * correct or not.
 */
class DHTPlugin : public Plugin
{
public:

    /**
     * Constructs the plugin.
     *
     * @param[in] name  Plugin name
     * @param[in] uid   Unique id
     */
    DHTPlugin(const String& name, uint16_t uid) :
        Plugin(name, uid),
        m_textCanvas(nullptr),
        m_iconCanvas(nullptr),
        m_bitmapWidget(),
        m_textWidget("\\calign?"),
        m_text(),
        m_page(0U),
        m_timer(),
        m_xMutex(nullptr),
        m_dht(DHTPIN, DHTTYPE),
        m_humid(0),
        m_temp(0),
        m_last(0)
    {
        /* Move the text widget one line lower for better look. */
        m_textWidget.move(0, 1);

        m_xMutex = xSemaphoreCreateMutex();        
    }

    /**
     * Destroys the plugin.
     */
    ~DHTPlugin()
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
        return new DHTPlugin(name, uid);
    }

    /**
     * Update the display.
     * The scheduler will call this method periodically.
     *
     * @param[in] gfx   Display graphics interface
     */
    void update(IGfx& gfx) final;
    
    void active(IGfx& gfx) final;

    void start() final;
    
    void process() final;    

    /**
     * Protect against concurrent access.
     */
    void lock(void) const;

    /**
     * Unprotect against concurrent access.
     */
    void unlock(void) const;

private:
    static const uint16_t   ICON_WIDTH          = 8U;
    static const uint16_t   ICON_HEIGHT         = 8U;
    static const char*      IMAGE_PATH_TEMP_ICON;
    static const char*      IMAGE_PATH_HUMID_ICON;

    static const uint32_t   UPDATE_PERIOD       = (90U * 1000U); // read values only every 90 seconds
    Canvas*                     m_textCanvas;               /**< Canvas used for the text widget. */
    Canvas*                     m_iconCanvas;               /**< Canvas used for the bitmap widget. */
    BitmapWidget                m_bitmapWidget;             /**< Bitmap widget, used to show the icon. */
    TextWidget                  m_textWidget;               /**< Text widget, used for showing the text. */
    String                      m_text;

    uint8_t                     m_page;     /**< Number of page, which to show */
    SimpleTimer                 m_timer;    /**< Timer for changing page */
    SemaphoreHandle_t           m_xMutex;                   /**< Mutex to protect against concurrent access. */

    DHT                         m_dht;
    float                       m_humid;
    float                       m_temp;
    unsigned long               m_last;

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __DHTPLUGIN_H__ */

/** @} */