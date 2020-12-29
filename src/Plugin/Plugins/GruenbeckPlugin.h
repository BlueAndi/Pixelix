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
 * @brief  Gruenbeck plugin
 * @author Yann Le Glaz <yann_le@web.de>

 *
 * @addtogroup plugin
 *
 * @{
 */

#ifndef __GRUENBECKPLUGIN_H__
#define __GRUENBECKPLUGIN_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "AsyncHttpClient.h"
#include <stdint.h>
#include "Plugin.hpp"
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
 * Shows the remaining system capacity (parameter = D_Y_10_1 ) 
 * of the Gruenbeck softliQ SC18 via the system's RESTful webservice.
 */
class GruenbeckPlugin : public Plugin
{
public:

    /**
     * Constructs the plugin.
     *
     * @param[in] name  Plugin name
     * @param[in] uid   Unique id
     */
    GruenbeckPlugin(const String& name, uint16_t uid) :
        Plugin(name, uid),
        m_textCanvas(nullptr),
        m_iconCanvas(nullptr),
        m_bitmapWidget(),
        m_textWidget("\\calign?"),
        m_ipAddress("192.168.0.16"),
        m_configurationFilename(),
        m_httpResponseReceived(false),
        m_relevantResponsePart(),
        m_requestTimer(),
        m_url(),
        m_callbackWebHandler(nullptr),
        m_xMutex(nullptr)
    {
        /* Move the text widget one line lower for better look. */
        m_textWidget.move(0, 1);

        m_xMutex = xSemaphoreCreateMutex();
    }

    /**
     * Destroys the plugin.
     */
    ~GruenbeckPlugin()
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
        return new GruenbeckPlugin(name, uid);
    }

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
     * Update the display.
     * The scheduler will call this method periodically.
     *
     * @param[in] gfx   Display graphics interface
     */
    void update(IGfx& gfx);

   /**
     * Stop the plugin.
     * Overwrite it if your plugin needs to know that it will be uninstalled.
     */
    void stop() override;

    /**
     * Start the plugin.
     * Overwrite it if your plugin needs to know that it was installed.
     */
    void start() override;
    
    /**
     * Process the plugin.
     * Overwrite it if your plugin has cyclic stuff to do without being in a
     * active slot.
     */
    void process(void);

    /**
     * Get ip-address.
     * 
     * @return IP-address
     */
    String getIPAddress() const;

    /**
     * Set ip-address.
     * 
     * @param[in] ipAddress IP-address
     */
    void setIPAddress(const String& ipAddress);

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
     * Image path within the SPIFFS.
     */
    static const char*      IMAGE_PATH;

    /**
     * Configuration path within the SPIFFS.
     */
    static const char*      CONFIG_PATH;

    /**
     * Period in ms for requesting data from server.
     * This is used in case the last request to the server was successful.
     */
    static const uint32_t   UPDATE_PERIOD       = (60U * 1000U);

    /**
     * Short period in ms for requesting data from server.
     * This is used in case the request to the server failed.
     */
    static const uint32_t   UPDATE_PERIOD_SHORT = (10U * 1000U);

    Canvas*                     m_textCanvas;               /**< Canvas used for the text widget. */
    Canvas*                     m_iconCanvas;               /**< Canvas used for the bitmap widget. */
    BitmapWidget                m_bitmapWidget;             /**< Bitmap widget, used to show the icon. */
    TextWidget                  m_textWidget;               /**< Text widget, used for showing the text. */
    String                      m_ipAddress;                /**< IP-address of the Gruenbeck server. */
    String                      m_configurationFilename;    /**< String used for specifying the configuration filename. */
    bool                        m_httpResponseReceived;     /**< Flag to indicate a received HTTP response. */
    String                      m_relevantResponsePart;     /**< String used for the relevant part of the HTTP response. */
    AsyncHttpClient             m_client;                   /**< Asynchronous HTTP client. */
    SimpleTimer                 m_requestTimer;             /**< Timer, used for cyclic request of new data. */
    String                      m_url;                      /**< REST API URL */
    AsyncCallbackWebHandler*    m_callbackWebHandler;       /**< Callback web handler */
    SemaphoreHandle_t           m_xMutex;                   /**< Mutex to protect against concurrent access. */

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
    bool requestNewData(void);

    /**
     * Register callback function on response reception.
     */
    void registerResponseCallback(void);

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

#endif  /* __GRUENBECKPLUGIN_H__ */

/** @} */