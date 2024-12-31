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
 * @brief  Plugin base
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup PLUGIN
 *
 * @{
 */

#ifndef PLUGIN_HPP
#define PLUGIN_HPP

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "IPluginMaintenance.hpp"

#include <stdint.h>
#include <YAGfx.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/** Use it to mark not used function parameters. */
#define PLUGIN_NOT_USED(__var)  (void)(__var)

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * A base plugin can be plugged into a display slot and will shown.
 * Overwrite the methods in a derived class to extend its functionality.
 */
class Plugin : public IPluginMaintenance
{
public:

    /**
     * Destroys the plugin.
     */
    virtual ~Plugin()
    {
    }

    /**
     * Set the slot interface, which the plugin can used to request information
     * from the slot, it is plugged in.
     *
     * @param[in] slotInterf    Slot interface
     */
    void setSlot(const ISlotPlugin* slotInterf) override
    {
        PLUGIN_NOT_USED(slotInterf);
    }

    /**
     * Get unique id.
     *
     * @return Unique id
     */
    uint16_t getUID() const final
    {
        return m_uid;
    }

    /**
     * Set instance alias name, which is more user friendly than the UID.
     * 
     * @param[in] alias Plugin instance alias name
     */
    void setAlias(const String& alias) final
    {
        m_alias = alias;
    }

    /**
     * Get instance alias name.
     * 
     * @return Plugin instance alias name
     */
    String getAlias() const final
    {
        return m_alias;
    }

    /**
     * Get font type.
     * 
     * @return The font type the plugin uses.
     */
    Fonts::FontType getFontType() const override
    {
        return Fonts::FONT_TYPE_DEFAULT;
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
    void setFontType(Fonts::FontType fontType) override
    {
        PLUGIN_NOT_USED(fontType);
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
    void getTopics(JsonArray& topics) const override
    {
        PLUGIN_NOT_USED(topics);
    }

    /**
     * Get a topic data.
     * Note, currently only JSON format is supported.
     * 
     * @param[in]   topic   The topic which data shall be retrieved.
     * @param[out]  value   The topic value in JSON format.
     * 
     * @return If successful it will return true otherwise false.
     */
    bool getTopic(const String& topic, JsonObject& value) const override
    {
        PLUGIN_NOT_USED(topic);
        PLUGIN_NOT_USED(value);

        return false;
    }

    /**
     * Set a topic data.
     * Note, currently only JSON format is supported.
     * 
     * @param[in]   topic   The topic which data shall be retrieved.
     * @param[in]   value   The topic value in JSON format.
     * 
     * @return If successful it will return true otherwise false.
     */
    bool setTopic(const String& topic, const JsonObjectConst& value) override
    {
        PLUGIN_NOT_USED(topic);
        PLUGIN_NOT_USED(value);

        return false;
    }

    /**
     * Has the topic content changed since last time?
     * Every readable volatile topic shall support this. Otherwise the topic
     * handlers might not be able to provide updated information.
     * 
     * @param[in] topic The topic which to check.
     * 
     * @return If the topic content changed since last time, it will return true otherwise false.
     */
    bool hasTopicChanged(const String& topic) override
    {
        PLUGIN_NOT_USED(topic);
        return false;
    }

    /**
     * Is a upload request accepted or rejected?
     * 
     * @param[in] topic         The topic which the upload belongs to.
     * @param[in] srcFilename   Name of the file, which will be uploaded if accepted.
     * @param[in] dstFilename   The destination filename, after storing the uploaded file.
     * 
     * @return If accepted it will return true otherwise false.
     */
    bool isUploadAccepted(const String& topic, const String& srcFilename, String& dstFilename) override
    {
        PLUGIN_NOT_USED(topic);
        PLUGIN_NOT_USED(srcFilename);
        PLUGIN_NOT_USED(dstFilename);
        return false;
    }

    /**
     * Get the plugin name.
     *
     * @return Name of the plugin.
     */
    const char* getName() const override
    {
        return m_name;
    }

    /**
     * Is plugin enabled or not?
     *
     * @return If plugin is enabled, it will return true otherwise false.
     */
    bool isEnabled() const override
    {
        return m_isEnabled;
    }

    /**
     * Enable plugin.
     * Only a enabled plugin will be scheduled.
     */
    void enable() override
    {
        m_isEnabled = true;
    }

    /**
     * Disable plugin.
     * A disabled plugin won't be scheduled in the next cycle.
     * Note, calling this doesn't abort a active phase.
     */
    void disable() override
    {
        m_isEnabled = false;
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
    void start(uint16_t width, uint16_t height) override
    {
        PLUGIN_NOT_USED(width);
        PLUGIN_NOT_USED(height);
    }

    /**
     * Stop the plugin. This is called only once during plugin lifetime.
     * It can be used as a first clean-up, before the plugin will be destroyed.
     * 
     * Overwrite it if your plugin needs to know that it will be uninstalled.
     */
    void stop() override
    {
    }

    /**
     * Process the plugin.
     * Overwrite it if your plugin has cyclic stuff to do without being in a
     * active slot.
     * 
     * @param[in] isConnected   The network connection status. If network
     *                          connection is established, it will be true otherwise false.
     */
    void process(bool isConnected) override
    {
        PLUGIN_NOT_USED(isConnected);
    }

    /**
     * This method will be called in case the plugin is set active, which means
     * it will be shown on the display in the next step.
     * Overwrite it if your plugin needs to know this.
     *
     * @param[in] gfx   Display graphics interface
     */
    void active(YAGfx& gfx) override
    {
        PLUGIN_NOT_USED(gfx);
    }

    /**
     * This method will be called in case the plugin is set inactive, which means
     * it won't be shown on the display anymore.
     * Overwrite it if your plugin needs to know this.
     */
    void inactive() override
    {
    }

    /**
     * Generate the full path for any plugin instance specific kind of configuration
     * file.
     * 
     * @param[in] uid       The plugin UID
     * @param[in] extension Full path filename
     *
     * @return Full path
     */
    static String generateFullPath(uint16_t uid, const String& extension)
    {
        return String(CONFIG_PATH) + "/" + uid + extension;
    }

    /**
     * Path where plugin specific configuration files shall be stored.
     */
    static constexpr const char* CONFIG_PATH = "/configuration";

protected:

    bool    m_isEnabled;    /**< Plugin is enabled or disabled */

    /**
     * Constructs the plugin.
     * It is disabled by default.
     *
     * @param[in] name  Plugin name (must exist over lifetime)
     * @param[in] uid   Unique id
     */
    Plugin(const char* name, uint16_t uid) :
        m_isEnabled(false),
        m_uid(uid),
        m_alias(),
        m_name(name)
    {
    }

private:

    const uint16_t  m_uid;      /**< Unique id */
    String          m_alias;    /**< Alias name */
    const char*     m_name;     /**< Plugin name */

    Plugin();
    Plugin(const Plugin& plugin);
    Plugin& operator=(const Plugin& plugin);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* PLUGIN_HPP */

/** @} */