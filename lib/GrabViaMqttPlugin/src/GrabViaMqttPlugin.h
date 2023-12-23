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
 * @brief  Grab information via REST API plugin
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup plugin
 *
 * @{
 */

#ifndef GRAB_VIA_REST_PLUGIN_H
#define GRAB_VIA_REST_PLUGIN_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include "Plugin.hpp"

#include <WidgetGroup.h>
#include <BitmapWidget.h>
#include <TextWidget.h>
#include <Mutex.hpp>
#include <FileSystem.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Grab information from a MQTT broker and display it.
 */
class GrabViaMqttPlugin : public Plugin, private PluginConfigFsHandler
{
public:

    /**
     * Constructs the plugin.
     *
     * @param[in] name  Plugin name
     * @param[in] uid   Unique id
     */
    GrabViaMqttPlugin(const String& name, uint16_t uid) :
        Plugin(name, uid),
        PluginConfigFsHandler(uid, FILESYSTEM),
        m_fontType(Fonts::FONT_TYPE_DEFAULT),
        m_layoutRight(),
        m_layoutLeft(),
        m_layoutTextOnly(),
        m_iconWidget(),
        m_textWidgetRight("\\calign?"),
        m_textWidgetTextOnly("\\calign?"),
        m_path(),
        m_filter(1024U),
        m_iconPath(),
        m_format("%s"),
        m_multiplier(1.0f),
        m_offset(0.0f),
        m_mutex(),
        m_cfgReloadTimer(),
        m_storeConfigReq(false),
        m_reloadConfigReq(false),
        m_hasTopicChanged(false)
    {
        (void)m_mutex.create();
    }

    /**
     * Destroys the plugin.
     */
    ~GrabViaMqttPlugin()
    {
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
        return new(std::nothrow)GrabViaMqttPlugin(name, uid);
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
    String                  m_path;                 /**< MQTT topic path */
    DynamicJsonDocument     m_filter;               /**< Filter used for the response in JSON format. */
    String                  m_iconPath;             /**< Icon filename with path. */
    String                  m_format;               /**< Format used to embed the retrieved filtered value. */
    float                   m_multiplier;           /**< If grabbed value is a number, it will be multiplied with the multiplier. */
    float                   m_offset;               /**< If grabbed value is a number, the offset will be added after the multiplication with the multiplier. */
    mutable MutexRecursive  m_mutex;                /**< Mutex to protect against concurrent access. */
    SimpleTimer             m_cfgReloadTimer;       /**< Timer is used to cyclic reload the configuration from persistent memory. */
    bool                    m_storeConfigReq;       /**< Is requested to store the configuration in persistent memory? */
    bool                    m_reloadConfigReq;      /**< Is requested to reload the configuration from persistent memory? */
    bool                    m_hasTopicChanged;      /**< Has the topic content changed? */

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
     * Get value from JSON source by the filter.
     * 
     * @param[in]   src     Source in JSON format
     * @param[in]   filter  Filter in JSON format
     * @param[out]  value   Value in JSON format
     */
    void getJsonValueByFilter(JsonObjectConst src, JsonObjectConst filter, JsonVariantConst& value);

    /**
     * Clear the task proxy queue.
     */
    void clearQueue();

    /**
     * Subscribe MQTT topic to be informed about value changes.
     */
    void subscribe();

    /**
     * Unsubscribe MQTT topic to stop on change notifications.
     */
    void unsubscribe();

    /**
     * The MQTT callback is registered by subscription and will be called on change by
     * the MQTT service.
     * 
     * @param[in] topic     Topic
     * @param[in] payload   Topic payload
     * @param[in] size      Topic payload size in byte
     */
    void mqttTopicCallback(const String& topic, const uint8_t* payload, size_t size);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* GRAB_VIA_REST_PLUGIN_H */

/** @} */
