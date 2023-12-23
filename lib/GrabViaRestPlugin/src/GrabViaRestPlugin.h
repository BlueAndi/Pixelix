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
 * @brief  Grab information via MQTT plugin
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup plugin
 *
 * @{
 */

#ifndef GRAB_VIA_MQTT_PLUGIN_H
#define GRAB_VIA_MQTT_PLUGIN_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include "Plugin.hpp"
#include "AsyncHttpClient.h"

#include <WidgetGroup.h>
#include <BitmapWidget.h>
#include <TextWidget.h>
#include <TaskProxy.hpp>
#include <Mutex.hpp>
#include <FileSystem.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Capture information from a remote server by using REST API.
 */
class GrabViaRestPlugin : public Plugin, private PluginConfigFsHandler
{
public:

    /**
     * Constructs the plugin.
     *
     * @param[in] name  Plugin name
     * @param[in] uid   Unique id
     */
    GrabViaRestPlugin(const String& name, uint16_t uid) :
        Plugin(name, uid),
        PluginConfigFsHandler(uid, FILESYSTEM),
        m_fontType(Fonts::FONT_TYPE_DEFAULT),
        m_layoutRight(),
        m_layoutLeft(),
        m_layoutTextOnly(),
        m_iconWidget(),
        m_textWidgetRight("\\calign?"),
        m_textWidgetTextOnly("\\calign?"),
        m_method("GET"),
        m_url(),
        m_filter(1024U),
        m_client(),
        m_iconPath(),
        m_format("%s"),
        m_multiplier(1.0f),
        m_offset(0.0f),
        m_requestTimer(),
        m_mutex(),
        m_isConnectionError(false),
        m_cfgReloadTimer(),
        m_storeConfigReq(false),
        m_reloadConfigReq(false),
        m_hasTopicChanged(false),
        m_taskProxy()
    {
        (void)m_mutex.create();
    }

    /**
     * Destroys the plugin.
     */
    ~GrabViaRestPlugin()
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
        return new(std::nothrow)GrabViaRestPlugin(name, uid);
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
     * By default a topic is readable and writeable.
     * This can be set explicit with the "access" key with the following possible
     * values:
     * - Only readable: "r"
     * - Only writeable: "w"
     * - Readable and writeable: "rw"
     * 
     * Example:
     * {
     *     "topics": [{
     *         "name": "/text",
     *         "access": "r"
     *     }]
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
    bool setTopic(const String& topic, const JsonObjectConst& value) final;

    /**
     * Is the topic content changed since last time?
     * Every readable volatile topic shall support this. Otherwise the topic
     * handlers might not be able to provide updated information.
     * 
     * @param[in] topic The topic which to check.
     * 
     * @return If the topic content changed since last time, it will return true otherwise false.
     */
    bool hasTopicChanged(const String& topic) final;
    
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
     * Icon width in pixels.
     */
    static const uint16_t   ICON_WIDTH          = 8U;

    /**
     * Icon height in pixels.
     */
    static const uint16_t   ICON_HEIGHT         = 8U;

    /**
     * Plugin topic, used to read/write the configuration.
     */
    static const char*      TOPIC_CONFIG;

    /**
     * Period in ms for requesting data from server.
     * This is used in case the last request to the server was successful.
     */
    static const uint32_t   UPDATE_PERIOD       = SIMPLE_TIMER_MINUTES(2U);

    /**
     * Short period in ms for requesting data from server.
     * This is used in case the request to the server failed.
     */
    static const uint32_t   UPDATE_PERIOD_SHORT = SIMPLE_TIMER_SECONDS(10U);

    /**
     * The configuration in the persistent memory shall be cyclic loaded.
     * This mechanism ensure that manual changes in the file are considered.
     * This is the reload period in ms.
     */
    static const uint32_t   CFG_RELOAD_PERIOD   = SIMPLE_TIMER_SECONDS(30U);

    Fonts::FontType         m_fontType;             /**< Font type which shall be used if there is no conflict with the layout. */
    WidgetGroup             m_layoutRight;          /**< Canvas used for the text widget in a layout with icon on the left side. */
    WidgetGroup             m_layoutLeft;           /**< Canvas used for the bitmap widget in a layout with text on the right side. */
    WidgetGroup             m_layoutTextOnly;       /**< Canvas used in case only text is shown. */
    BitmapWidget            m_iconWidget;           /**< Bitmap widget, used to show the icon. */
    TextWidget              m_textWidgetRight;      /**< Text widget, used in layout with icon. */
    TextWidget              m_textWidgetTextOnly;   /**< Text widget, used in layout without icon. */
    String                  m_method;               /**< HTTP method. */
    String                  m_url;                  /**< REST URL. */
    DynamicJsonDocument     m_filter;               /**< Filter used for the response in JSON format. */
    AsyncHttpClient         m_client;               /**< Asynchronous HTTP client. */
    String                  m_iconPath;             /**< Icon filename with path. */
    String                  m_format;               /**< Format used to embed the retrieved filtered value. */
    float                   m_multiplier;           /**< If grabbed value is a number, it will be multiplied with the multiplier. */
    float                   m_offset;               /**< If grabbed value is a number, the offset will be added after the multiplication with the multiplier. */
    SimpleTimer             m_requestTimer;         /**< Timer used for cyclic request of new data. */
    mutable MutexRecursive  m_mutex;                /**< Mutex to protect against concurrent access. */
    bool                    m_isConnectionError;    /**< Is connection error happened? */
    SimpleTimer             m_cfgReloadTimer;       /**< Timer is used to cyclic reload the configuration from persistent memory. */
    bool                    m_storeConfigReq;       /**< Is requested to store the configuration in persistent memory? */
    bool                    m_reloadConfigReq;      /**< Is requested to reload the configuration from persistent memory? */
    bool                    m_hasTopicChanged;      /**< Has the topic content changed? */

    /**
     * Defines the message types, which are necessary for HTTP client/server handling.
     */
    enum MsgType
    {
        MSG_TYPE_INVALID = 0,   /**< Invalid message type. */
        MSG_TYPE_RSP,           /**< A response, caused by a previous request. */
        MSG_TYPE_CONN_CLOSED,   /**< The connection is closed. */
        MSG_TYPE_CONN_ERROR     /**< A connection error happened. */
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
     * Request to store configuration to persistent memory.
     */
    void requestStoreToPersistentMemory();

    /**
     * Get configuration in JSON.
     * 
     * @param[out] cfg  Configuration
     */
    void getConfiguration(JsonObject& cfg) const final;

    /**
     * Set configuration in JSON.
     * 
     * @param[in] cfg   Configuration
     * 
     * @return If successful set, it will return true otherwise false.
     */
    bool setConfiguration(JsonObjectConst& cfg) final;

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
     * Handle asynchronous web response from the server.
     * This will be called in LwIP context! Don't modify any member here directly!
     * 
     * @param[in] jsonDoc   Web response as JSON document
     */
    void handleAsyncWebResponse(const HttpResponse& rsp);

    /**
     * Get value from JSON source by the filter.
     * 
     * @param[in]   src     Source in JSON format
     * @param[in]   filter  Filter in JSON format
     * @param[out]  value   Value in JSON format
     */
    void getJsonValueByFilter(JsonObjectConst src, JsonObjectConst filter, JsonVariantConst& value);

    /**
     * Handle a web response from the server.
     * 
     * @param[in] jsonDoc   Web response as JSON document
     */
    void handleWebResponse(const DynamicJsonDocument& jsonDoc);
    
    /**
     * Clear the task proxy queue.
     */
    void clearQueue();
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* GRAB_VIA_MQTT_PLUGIN_H */

/** @} */
