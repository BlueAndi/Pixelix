/* MIT License
 *
 * Copyright (c) 2019 - 2020 Andreas Merkle <web@blue-andi.de>
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
 * @brief  VOLUMIO plugin
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup plugin
 *
 * @{
 */

#ifndef __VOLUMIOPLUGIN_H__
#define __VOLUMIOPLUGIN_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include "Plugin.hpp"
#include "AsyncHttpClient.h"

#include <Canvas.h>
#include <BitmapWidget.h>
#include <TextWidget.h>
#include <SPIFFS.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Shows the current state of VOLUMIO and the title of the played music.
 * If the VOLUMIO server is offline, the plugin gets automatically disabled,
 * otherwise enabled.
 *
 * Change VOLUMIO host address via REST API:
 * Text: POST \c "<base-uri>/host?set=<host-address>"
 */
class VolumioPlugin : public Plugin
{
public:

    /**
     * Constructs the plugin.
     *
     * @param[in] name  Plugin name
     * @param[in] uid   Unique id
     */
    VolumioPlugin(const String& name, uint16_t uid) :
        Plugin(name, uid),
        m_textCanvas(nullptr),
        m_iconCanvas(nullptr),
        m_bitmapWidget(),
        m_textWidget("\\calign?"),
        m_volumioHost("volumio.fritz.box"),
        m_configurationFilename(),
        m_urlIcon(),
        m_urlText(),
        m_requestTimer(),
        m_offlineTimer(),
        m_url(),
        m_callbackWebHandler(nullptr),
        m_xMutex(nullptr),
        m_isConnectionError(false),
        m_lastSeekValue(0U)
    {
        /* Move the text widget one line lower for better look. */
        m_textWidget.move(0, 1);

        m_xMutex = xSemaphoreCreateMutex();
    }

    /**
     * Destroys the plugin.
     */
    ~VolumioPlugin()
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
        return new VolumioPlugin(name, uid);
    }

    /**
     * Start the plugin.
     * Overwrite it if your plugin needs to know that it was installed.
     */
    void start() override;

    /**
     * Stop the plugin.
     * Overwrite it if your plugin needs to know that it will be uninstalled.
     */
    void stop() override;

    /**
     * Process the plugin.
     * Overwrite it if your plugin has cyclic stuff to do without being in a
     * active slot.
     */
    void process(void);
    
    /**
     * This method will be called in case the plugin is set active, which means
     * it will be shown on the display in the next step.
     *
     * @param[in] gfx   Display graphics interface
     */
    void active(IGfx& gfx) override;

    /**
     * This method will be called in case the plugin is set inactive, which means
     * it won't be shown on the display anymore.
     */
    void inactive() override;

    /**
     * Register web interface, e.g. REST API functionality.
     *
     * @param[in] srv       Webserver
     * @param[in] baseUri   Base URI, use this and append plugin specific part.
     */
    void registerWebInterface(AsyncWebServer& srv, const String& baseUri) override;

    /**
     * Unregister web interface.
     *
     * @param[in] srv   Webserver
     */
    void unregisterWebInterface(AsyncWebServer& srv) override;

    /**
     * Update the display.
     * The scheduler will call this method periodically.
     *
     * @param[in] gfx   Display graphics interface
     */
    void update(IGfx& gfx);

    /**
     * Get VOLUMIO host address.
     * 
     * @return VOLUMIO host address.
     */
    String getHost() const;

    /**
     * Set VOLUMIO host address.
     * 
     * @param[in] host  VOLUMIO host address
     */
    void setHost(const String& host);

private:

    /**
     * Icon width in pixels.
     */
    static const uint16_t   ICON_WIDTH          = 8U;

    /**
     * Icon height in pixels.
     */
    static const uint16_t   ICON_HEIGHT         = 8U;

    /**
     * Image path within the filesystem to standard icon.
     */
    static const char*      IMAGE_PATH_STD_ICON;

    /**
     * Image path within the filesystem to "stop" icon.
     */
    static const char*      IMAGE_PATH_STOP_ICON;

    /**
     * Image path within the filesystem to "play" icon.
     */
    static const char*      IMAGE_PATH_PLAY_ICON;

    /**
     * Image path within the filesystem to "pause" icon.
     */
    static const char*      IMAGE_PATH_PAUSE_ICON;

    /**
     * Configuration path within the SPIFFS.
     */
    static const char*      CONFIG_PATH;

    /**
     * Period in ms for requesting data from server.
     * This is used in case the last request to the server was successful.
     * The period is shorter than the UPDATE_PERIOD_SHORT, because if the music
     * changes, the display shall be updated more or less immediately.
     */
    static const uint32_t   UPDATE_PERIOD       = (2U * 1000U);

    /**
     * Short period in ms for requesting data from server.
     * This is used in case the request to the server failed.
     */
    static const uint32_t   UPDATE_PERIOD_SHORT = (10U * 1000U);

    /**
     * Period in ms after which the plugin gets automatically disabled if no new
     * data is available.
     */
    static const uint32_t   OFFLINE_PERIOD      = (60U * 1000U);

    Canvas*                     m_textCanvas;               /**< Canvas used for the text widget. */
    Canvas*                     m_iconCanvas;               /**< Canvas used for the bitmap widget. */
    BitmapWidget                m_bitmapWidget;             /**< Bitmap widget, used to show the icon. */
    TextWidget                  m_textWidget;               /**< Text widget, used for showing the text. */
    String                      m_volumioHost;              /**< Host address of the VOLUMIO server. */
    String                      m_configurationFilename;    /**< String used for specifying the configuration filename. */
    String                      m_urlIcon;                  /**< REST API URL for updating the icon */
    String                      m_urlText;                  /**< REST API URL for updating the text */
    AsyncHttpClient             m_client;                   /**< Asynchronous HTTP client. */
    SimpleTimer                 m_requestTimer;             /**< Timer used for cyclic request of new data. */
    SimpleTimer                 m_offlineTimer;             /**< Timer used for offline detection. */
    String                      m_url;                      /**< REST API URL */
    AsyncCallbackWebHandler*    m_callbackWebHandler;       /**< Callback web handler */
    SemaphoreHandle_t           m_xMutex;                   /**< Mutex to protect against concurrent access. */
    bool                        m_isConnectionError;        /**< Is connection error happened? */
    uint32_t                    m_lastSeekValue;            /**< Last seek value, retrieved from VOLUMIO. Used to cross-check the provided status. */

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

#endif  /* __VOLUMIOPLUGIN_H__ */

/** @} */