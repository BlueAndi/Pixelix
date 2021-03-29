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
 * @brief  Weather Plugin
 * @author Flavio Curti <fcu-github@no-way.org>

 *
 * @addtogroup plugin
 *
 * @{
 */

#ifndef __WEATHERPLUGIN_H__
#define __WEATHERPLUGIN_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "AsyncHttpClient.h"
#include "Plugin.hpp"

#include <Canvas.h>
#include <BitmapWidget.h>
#include <stdint.h>
#include <TextWidget.h>
#include <SimpleTimer.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Shows the forecast for a number of days for a certain location via openwheater
 *
 * At the first installation a json document is generated to the /configuration/UUID.json
 * in the filesystem, where the APIKEY, Location and number of days have to be configured.
 *
 * Powered by https://openweathermap.org/
 */
class WeatherPlugin : public Plugin
{
public:

    /**
     * Constructs the plugin.
     *
     * @param[in] name  Plugin name
     * @param[in] uid   Unique id
     */
    WeatherPlugin(const String& name, uint16_t uid) :
        Plugin(name, uid),
        m_textCanvas(nullptr),
        m_iconCanvas(nullptr),
        m_bitmapWidget(),
        m_textWidget("\\calign?"),
        m_location("Zurich, CH"), /* Example data */
        m_days("3"), /* Example data */        
        m_apikey("APIKEY"), /* Example data */        
        m_configurationFilename(""),
        m_httpResponseReceived(false),
        m_relevantResponsePart(""),
        m_url(),
        m_callbackWebHandler(nullptr),
        m_xMutex(nullptr),
        m_requestTimer()
    {
        /* Move the text widget one line lower for better look. */
        m_textWidget.move(0, 1);

        m_xMutex = xSemaphoreCreateMutex();
    }

    /**
     * Destroys the plugin.
     */
    ~WeatherPlugin()
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
        return new WeatherPlugin(name, uid);
    }

    /**
     * Register web interface, e.g. REST API functionality.
     *
     * @param[in] srv       Webserver
     * @param[in] baseUri   Base URI, use this and append plugin specific part.
     */
    void registerWebInterface(AsyncWebServer& srv, const String& baseUri) final;

    /**
     * Unregister web interface.
     *
     * @param[in] srv   Webserver
     */
    void unregisterWebInterface(AsyncWebServer& srv) final;

    /**
     * This method will be called in case the plugin is set active, which means
     * it will be shown on the display in the next step.
     *
     * @param[in] gfx   Display graphics interface
     */
    void active(IGfx& gfx) final;

    /**
     * This method will be called in case the plugin is set inactive, which means
     * it won't be shown on the display anymore.
     */
    void inactive() final;

    /**
     * Update the display.
     * The scheduler will call this method periodically.
     *
     * @param[in] gfx   Display graphics interface
     */
    void update(IGfx& gfx) final;

   /**
     * Stop the plugin.
     * Overwrite it if your plugin needs to know that it will be uninstalled.
     */
    void stop() final;

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
     * Get location.
     *
     * @param[out] location     Location
     */
    void getLocation(String& location) const;

    /**
     * Set location.
     */
    void setLocation(const String& location);

    /**
     * Get API-Key.
     *
     * @param[out] apikey     API-Key
     */
    void getApikey(String& apikey) const;

    /**
     * Set location.
     */
    void setApikey(const String& apikey);

    /**
     * Get days of forecast.
     *
     * @param[out] days         Number of days
     */
    void getDays(String& days) const;

    /**
     * Set location.
     */
    void setDays(const String& days);

private:

    /**
     * Icon width in pixels.
     */
    static const int16_t    ICON_WIDTH          = 8;

    /**
     * Icon height in pixels.
     */
    static const int16_t    ICON_HEIGHT         = 8;

    /**
     * Image path within the filesystem.
     */
    static const char*      IMAGE_PATH;

    /**
     * Configuration path within the filesystem.
     */
    static const char*      CONFIG_PATH;

    /**
     * Period in ms for requesting weather from server (4 Hours)
     * This is used in case the last request to the server was successful.
     */
    static const uint32_t   UPDATE_PERIOD       = (4U * 60U * 60U * 1000U);

    /**
     * Short period in ms for requesting weather from server.
     * This is used in case the request to the server failed.
     */
    static const uint32_t   UPDATE_PERIOD_SHORT = (60U * 1000U);

    Canvas*                     m_textCanvas;               /**< Canvas used for the text widget. */
    Canvas*                     m_iconCanvas;               /**< Canvas used for the bitmap widget. */
    BitmapWidget                m_bitmapWidget;             /**< Bitmap widget, used to show the icon. */
    TextWidget                  m_textWidget;               /**< Text widget, used for showing the text. */
    String                      m_location;                 /**< weather location */
    String                      m_days;                     /**< weather location */
    String                      m_apikey;                   /**< weather location */    
    String                      m_configurationFilename;    /**< String used for specifying the configuration filename. */
    bool                        m_httpResponseReceived;     /**< Flag to indicate a received HTTP response. */
    String                      m_relevantResponsePart;     /**< String used for the relevant part of the HTTP response. */
    AsyncHttpClient             m_client;                   /**< Asynchronous HTTP client. */
    SimpleTimer                 m_requestDataTimer;         /**< Timer, used for cyclic request of new data. */
    String                      m_url;                      /**< REST API URL */
    AsyncCallbackWebHandler*    m_callbackWebHandler;       /**< Callback web handler */
    SemaphoreHandle_t           m_xMutex;                   /**< Mutex to protect against concurrent access. */
    SimpleTimer                 m_requestTimer;             /**< Timer is used for cyclic weather http request. */

    /**
     * Instance specific web request handler, called by the static web request
     * handler. It will really handle the request.
     *
     * @param[in] request   Web request
     */
    void webReqHandler(AsyncWebServerRequest *request);

    /**
     * Request new data.
     *
     * @return If successful it will return true otherwise false.
     */
    bool startHttpRequest(void);

    /**
     * Register callback function on response reception.
     */
    void initHttpClient(void);

    /**
     * Saves current configuration to JSON file.
     */
    bool saveConfiguration();

    /**
     * Load configuration from JSON file.
     */
    bool loadConfiguration();

    /**
     * If configuration directory doesn't exists, it will be created.
     * Otherwise nothing happens.
     */
    void createConfigDirectory();

    /**
     * Protect against concurrent access.
     */
    void lock(void) const;

    /**
     * Unprotect against concurrent access.
     */
    void unlock(void) const;
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __WEATHERPLUGIN_H__ */

/** @} */