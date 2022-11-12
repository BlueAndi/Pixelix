/* MIT License
 *
 * Copyright (c) 2019 - 2022 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Plugin interface
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup plugin
 *
 * @{
 */

#ifndef __PLUGIN_HPP__
#define __PLUGIN_HPP__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <YAGfx.h>
#include <Util.h>
#include "IPluginMaintenance.hpp"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * A plugin can be plugged into a display slot and will shown.
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
        UTIL_NOT_USED(slotInterf);
        return;
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
        return;
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
        UTIL_NOT_USED(fontType);
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
    void getTopics(JsonArray& topics) const override
    {
        UTIL_NOT_USED(topics);
        return;
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
        UTIL_NOT_USED(topic);
        UTIL_NOT_USED(value);

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
    bool setTopic(const String& topic, const JsonObject& value) override
    {
        UTIL_NOT_USED(topic);
        UTIL_NOT_USED(value);

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
        UTIL_NOT_USED(topic);
        UTIL_NOT_USED(srcFilename);
        UTIL_NOT_USED(dstFilename);
        return false;
    }

    /**
     * Get the plugin name.
     *
     * @return Name of the plugin.
     */
    const char* getName() const override
    {
        return m_name.c_str();
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
        return;
    }

    /**
     * Disable plugin.
     * A disabled plugin won't be scheduled in the next cycle.
     * Note, calling this doesn't abort a active phase.
     */
    void disable() override
    {
        m_isEnabled = false;
        return;
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
        UTIL_NOT_USED(width);
        UTIL_NOT_USED(height);
        return;
    }

    /**
     * Stop the plugin. This is called only once during plugin lifetime.
     * It can be used as a first clean-up, before the plugin will be destroyed.
     * 
     * Overwrite it if your plugin needs to know that it will be uninstalled.
     */
    void stop() override
    {
        return;
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
        UTIL_NOT_USED(isConnected);
        return;
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
        UTIL_NOT_USED(gfx);
        return;
    }

    /**
     * This method will be called in case the plugin is set inactive, which means
     * it won't be shown on the display anymore.
     * Overwrite it if your plugin needs to know this.
     */
    void inactive() override
    {
        return;
    }

    /**
     * Path where plugin specific configuration files shall be stored.
     */
    static constexpr const char*    CONFIG_PATH = "/configuration";

protected:

    bool        m_isEnabled;    /**< Plugin is enabled or disabled */

    /**
     * Constructs the plugin.
     * It is disabled by default.
     *
     * @param[in] name  Plugin name
     * @param[in] uid   Unique id
     */
    Plugin(const String& name, uint16_t uid) :
        m_isEnabled(false),
        m_uid(uid),
        m_alias(),
        m_name(name)
    {
    }

    /**
     * Generate the full path for any plugin instance specific kind of configuration
     * file.
     * 
     * @param[in] extension Full path filename
     *
     * @return Full path
     */
    String generateFullPath(const String& extension) const
    {
        return String(CONFIG_PATH) + "/" + getUID() + extension;
    }

    /**
     * Get full path (path + filename) to plugin instance specific configuration
     * in JSON format.
     * 
     * @return Full path to configuration file
     */
    String getFullPathToConfiguration() const
    {
        return generateFullPath(".json");
    }

private:

    uint16_t    m_uid;          /**< Unique id */
    String      m_alias;        /**< Alias name */
    String      m_name;         /**< Plugin name */

    Plugin();
    Plugin(const Plugin& plugin);
    Plugin& operator=(const Plugin& plugin);

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __PLUGIN_HPP__ */

/** @} */