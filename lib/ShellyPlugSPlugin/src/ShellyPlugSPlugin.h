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
 * @brief  ShellyPlugSPlugin plugin
 * @author Yann Le Glaz <yann_le@web.de>

 *
 * @addtogroup plugin
 *
 * @{
 */

#ifndef __SHELLYPLUGSPLUGIN_H__
#define __SHELLYPLUGSPLUGIN_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "AsyncHttpClient.h"
#include "Plugin.hpp"

#include <WidgetGroup.h>
#include <BitmapWidget.h>
#include <stdint.h>
#include <TextWidget.h>
#include <SimpleTimer.hpp>
#include <TaskProxy.hpp>
#include <Mutex.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Shows the current AC power being drawn via a Shelly PlugS, in watts.
 */
class ShellyPlugSPlugin : public Plugin
{
public:

    /**
     * Constructs the plugin.
     *
     * @param[in] name  Plugin name
     * @param[in] uid   Unique id
     */
    ShellyPlugSPlugin(const String& name, uint16_t uid) :
        Plugin(name, uid),
        m_fontType(Fonts::FONT_TYPE_DEFAULT),
        m_textCanvas(),
        m_iconCanvas(),
        m_bitmapWidget(),
        m_textWidget("?"),
        m_ipAddress("192.168.1.123"), /* Example data */
        m_client(),
        m_mutex(),
        m_requestTimer(),
        m_taskProxy()
    {
        (void)m_mutex.create();
    }

    /**
     * Destroys the plugin.
     */
    ~ShellyPlugSPlugin()
    {
        m_client.regOnResponse(nullptr);
        m_client.regOnClosed(nullptr);
        m_client.regOnError(nullptr);

        /* Abort any pending TCP request to avoid getting a callback after the
         * object is destroyed.
         */
        m_client.end();
        
        clearQueue();
        
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
        return new ShellyPlugSPlugin(name, uid);
    }

    /**
     * Get font type.
     * 
     * @return The font type the plugin uses.
     */
    Fonts::FontType getFontType() const final
    {
        return m_fontType;
    }

    /**
     * Set font type.
     * The plugin may skip the font type in case it gets conflicts with the layout.
     * 
     * A font type change will only be considered if it is set before the start()
     * method is called!
     * 
     * @param[in] fontType  The font type which the plugin shall use.
     */
    void setFontType(Fonts::FontType fontType) final
    {
        m_fontType = fontType;
        return;
    }

    /**
     * Get plugin topics, which can be get/set via different communication
     * interfaces like REST, websocket, MQTT, etc.
     * 
     * Example:
     * {
     *     "topics": [
     *         "/text"
     *     ]
     * }
     * 
     * @param[out] topics   Topis in JSON format
     */
    void getTopics(JsonArray& topics) const final;

    /**
     * Get a topic data.
     * Note, currently only JSON format is supported.
     * 
     * @param[in]   topic   The topic which data shall be retrieved.
     * @param[out]  value   The topic value in JSON format.
     * 
     * @return If successful it will return true otherwise false.
     */
    bool getTopic(const String& topic, JsonObject& value) const final;

    /**
     * Set a topic data.
     * Note, currently only JSON format is supported.
     * 
     * @param[in]   topic   The topic which data shall be retrieved.
     * @param[in]   value   The topic value in JSON format.
     * 
     * @return If successful it will return true otherwise false.
     */
    bool setTopic(const String& topic, const JsonObject& value) final;

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
     * Image path within the filesystem.
     */
    static const char*      IMAGE_PATH;

    /**
     * Plugin topic, used for parameter exchange.
     */
    static const char*      TOPIC;

    /**
     * Period in ms for requesting power consumption from the Shelly PlugS.
     * This is used in case the last request to the server was successful.
     */
    static const uint32_t   UPDATE_PERIOD       = SIMPLE_TIMER_SECONDS(15U);

    /**
     * Short period in ms for requesting power consumption from the Shelly PlugS.
     * This is used in case the request to the server failed.
     */
    static const uint32_t   UPDATE_PERIOD_SHORT = SIMPLE_TIMER_SECONDS(10U);

    Fonts::FontType         m_fontType;         /**< Font type which shall be used if there is no conflict with the layout. */
    WidgetGroup             m_textCanvas;       /**< Canvas used for the text widget. */
    WidgetGroup             m_iconCanvas;       /**< Canvas used for the bitmap widget. */
    BitmapWidget            m_bitmapWidget;     /**< Bitmap widget, used to show the icon. */
    TextWidget              m_textWidget;       /**< Text widget, used for showing the text. */
    String                  m_ipAddress;        /**< IP-address of the ShellyPlugS server. */
    AsyncHttpClient         m_client;           /**< Asynchronous HTTP client. */
    mutable MutexRecursive  m_mutex;            /**< Mutex to protect against concurrent access. */
    SimpleTimer             m_requestTimer;     /**< Timer is used for cyclic ShellyPlugS  http request. */

    /**
     * Defines the message types, which are necessary for HTTP client/server handling.
     */
    enum MsgType
    {
        MSG_TYPE_INVALID = 0,   /**< Invalid message type. */
        MSG_TYPE_RSP            /**< A response, caused by a previous request. */
    };

    /**
     * A message for HTTP client/server handling.
     */
    struct Msg
    {
        MsgType                 type;   /**< Message type */
        DynamicJsonDocument*    rsp;    /**< Response, only valid if message type is a response. */

        /**
         * Constructs a message.
         */
        Msg() :
            type(MSG_TYPE_INVALID),
            rsp(nullptr)
        {
        }
    }; 

    /**
     * Task proxy used to decouple server responses, which happen in a different task context.
     */
    TaskProxy<Msg, 2U, 0U> m_taskProxy;

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
     * Handle a web response from the server.
     * 
     * @param[in] jsonDoc   Web response as JSON document
     */
    void handleWebResponse(DynamicJsonDocument& jsonDoc);

    /**
     * Saves current configuration to JSON file.
     */
    bool saveConfiguration() const;

    /**
     * Load configuration from JSON file.
     */
    bool loadConfiguration();

    /**
     * Clear the task proxy queue.
     */
    void clearQueue();
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __SHELLYPLUGSPLUGIN_H__ */

/** @} */