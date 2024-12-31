/* MIT License
 *
 * Copyright (c) 2019 - 2024 Andreas Merkle <web@blue-andi.de>
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
 * @brief  JustText plugin
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup PLUGIN
 *
 * @{
 */

#ifndef JUSTTEXTPLUGIN_H
#define JUSTTEXTPLUGIN_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "./internal/View.h"

#include <stdint.h>
#include <PluginWithConfig.hpp>
#include <Mutex.hpp>
#include <FileSystem.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Shows text over the whole display.
 * If the text is too long for the display width, it automatically scrolls.
 */
class JustTextPlugin : public PluginWithConfig
{
public:

    /**
     * Constructs the plugin.
     *
     * @param[in] name  Plugin name (must exist over lifetime)
     * @param[in] uid   Unique id
     */
    JustTextPlugin(const char* name, uint16_t uid) :
        PluginWithConfig(name, uid, FILESYSTEM),
        m_view(),
        m_formatTextStored(),
        m_mutex(),
        m_hasTopicChanged(false)
    {
        (void)m_mutex.create();
    }

    /**
     * Destroys the plugin.
     */
    ~JustTextPlugin()
    {
        m_mutex.destroy();
    }

    /**
     * Plugin creation method, used to register on the plugin manager.
     *
     * @param[in] name  Plugin name (must exist over lifetime)
     * @param[in] uid   Unique id
     *
     * @return If successful, it will return the pointer to the plugin instance, otherwise nullptr.
     */
    static IPluginMaintenance* create(const char* name, uint16_t uid)
    {
        return new (std::nothrow) JustTextPlugin(name, uid);
    }

    /**
     * Is plugin enabled or not?
     *
     * @return If plugin is enabled, it will return true otherwise false.
     */
    bool isEnabled() const final;

    /**
     * Get font type.
     *
     * @return The font type the plugin uses.
     */
    Fonts::FontType getFontType() const final
    {
        return m_view.getFontType();
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
        m_view.setFontType(fontType);
    }

    /**
     * Get plugin topics, which can be get/set via different communication
     * interfaces like REST, websocket, MQTT, etc.
     * 
     * Example:
     * <code>{.json}
     * {
     *     "topics": [
     *         "/text"
     *     ]
     * }
     * </code>
     * 
     * By default a topic is readable and writeable.
     * This can be set explicit with the "access" key with the following possible
     * values:
     * - Only readable: "r"
     * - Only writeable: "w"
     * - Readable and writeable: "rw"
     * 
     * Example:
     * <code>{.json}
     * {
     *     "topics": [{
     *         "name": "/text",
     *         "access": "r"
     *     }]
     * }
     * </code>
     * 
     * Homeassistant MQTT discovery support can be added with the "ha" key.
     * <code>{.json}
     * {
     *     "topics": [{
     *         "name": "/text",
     *         "ha": {
     *             ... everything here will be used for MQTT discovery ...
     *         }
     *     }]
     * }
     * </code>
     * 
     * Additional information can be loaded from a file too. It will be appended
     * to the topic data (parallel to "name" and "access"). If a file is used,
     * any other key than "name" and "access" will be ignored.
     * <code>{.json}
     * {
     *     "topics": [{
     *         "name": "/text",
     *         "fileName": "haText.json"
     *    }]
     * }
     * </code>
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
     */
    void stop() final;

    /**
     * Update the display.
     * The scheduler will call this method periodically.
     *
     * @param[in] gfx   Display graphics interface
     */
    void update(YAGfx& gfx) final;

    /**
     * Get text.
     *
     * @return Formatted text
     */
    String getText() const;

    /**
     * Set text, which may contain format tags.
     *
     * @param[in] formatText    Text, which may contain format tags.
     * @param[in] storeFlag     Store the text persistent or not.
     */
    void setText(const String& formatText, bool storeFlag);

private:

    /**
     * Plugin topic, used for parameter exchange.
     */
    static const char* TOPIC_TEXT;

    /**
     * Filename for the plugin topic extra info, used for Home Assistant integration.
     */
    static const char*     TOPIC_TEXT_EXTRA_FILE_NAME;

    _JustTextPlugin::View  m_view;             /**< View with all widgets. */
    String                 m_formatTextStored; /**< It contains the format text, which is persistent stored. */
    mutable MutexRecursive m_mutex;            /**< Mutex to protect against concurrent access. */
    bool                   m_hasTopicChanged;  /**< Has the topic content changed? Used to notify the TopicHandlerService about changes. */

    /**
     * Get actual configuration in JSON.
     *
     * @param[out] cfg  Configuration
     */
    void getActualConfiguration(JsonObject& cfg) const;

    /**
     * Set actual configuration in JSON.
     * It will not be stored to configuration file.
     *
     * @param[in] cfg   Configuration
     *
     * @return If successful set, it will return true otherwise false.
     */
    bool setActualConfiguration(const JsonObjectConst& jsonCfg);

    /**
     * Get persistent configuration in JSON.
     *
     * @param[out] cfg  Configuration
     */
    void getConfiguration(JsonObject& jsonCfg) const final;

    /**
     * Set persistent configuration in JSON.
     *
     * @param[in] cfg   Configuration
     *
     * @return If successful set, it will return true otherwise false.
     */
    bool setConfiguration(const JsonObjectConst& jsonCfg) final;
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* JUSTTEXTPLUGIN_H */

/** @} */