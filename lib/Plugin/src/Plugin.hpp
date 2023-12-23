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
 * @brief  Plugin interface
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup plugin
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
#include <JsonFile.h>
#include <ArduinoJson.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/** Use it to mark not used function parameters. */
#define PLUGIN_NOT_USED(__var)  (void)(__var)

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

protected:

    bool    m_isEnabled;    /**< Plugin is enabled or disabled */

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

private:

    const uint16_t  m_uid;      /**< Unique id */
    String          m_alias;    /**< Alias name */
    String          m_name;     /**< Plugin name */

    Plugin();
    Plugin(const Plugin& plugin);
    Plugin& operator=(const Plugin& plugin);
};

/**
 * This class handles the plugin configuration file in the filesystem.
 * Use it for loading and saving the configuration, as well as checking
 * whether the configuration file was changed without the handlers
 * knowledge.
 */
class PluginConfigFsHandler
{
public:

    /**
     * Destroy the plugin configuration filesystem handler.
     */
    ~PluginConfigFsHandler()
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
     * Get full path (path + filename) to plugin instance specific configuration
     * in JSON format.
     * 
     * @return Full path to configuration file
     */
    String getFullPathToConfiguration() const
    {
        return generateFullPath(m_uid, ".json");
    }

    /**
     * Path where plugin specific configuration files shall be stored.
     */
    static constexpr const char*    CONFIG_PATH = "/configuration";

protected:

    const uint16_t  m_uid;                          /**< Unique id */
    FS&             m_fs;                           /**< Filesystem used to load and save the configuration file. */
    time_t          m_timestampOfLastFileUpdate;    /**< Configuration in persistent memory written the last time (unix timeformat). */

    PluginConfigFsHandler();
    PluginConfigFsHandler(const PluginConfigFsHandler& handler);
    PluginConfigFsHandler& operator=(const PluginConfigFsHandler& handler);

    /**
     * Construct the plugin configuration filesystem handler.
     * 
     * @param[in] uid   The plugin UID.
     * @param[in] fs    The filesystem where to load and save the configuration file.
     */
    PluginConfigFsHandler(uint16_t uid, FS& fs) :
        m_uid(uid),
        m_fs(fs),
        m_timestampOfLastFileUpdate(0U)
    {
    }

    /**
     * Get configuration in JSON.
     * 
     * @param[out] cfg  Configuration
     */
    virtual void getConfiguration(JsonObject& cfg) const = 0;

    /**
     * Set configuration in JSON.
     * 
     * @param[in] cfg   Configuration
     * 
     * @return If successful set, it will return true otherwise false.
     */
    virtual bool setConfiguration(JsonObjectConst& cfg) = 0;

    /**
     * Update the internal timestamp of the last configuration file update.
     */
    void updateTimestampLastUpdate()
    {
        m_timestampOfLastFileUpdate = getLastConfigurationUpdate();
    }

    /**
     * Get timestamp of the last configuration update in the persistent memory.
     * 
     * @return Timestamp in unix time format.
     */
    time_t getLastConfigurationUpdate() const
    {
        String  configurationFilename   = getFullPathToConfiguration();
        File    fd                      = m_fs.open(configurationFilename, "r");
        time_t  timestamp               = 0U;

        if (true == fd)
        {
            timestamp = fd.getLastWrite();
            fd.close();
        }

        return timestamp;
    }

    /**
     * Is the configuration in persistent memory updated without using the
     * plugin API?
     * 
     * @return If updated, it will return true otherwise false.
     */
    bool isConfigurationUpdated() const
    {
        bool    isConfigurationUpdated      = false;
        time_t  timestampOfLastFileUpdate   = getLastConfigurationUpdate();
        
        if (timestampOfLastFileUpdate != m_timestampOfLastFileUpdate)
        {
            isConfigurationUpdated = true;
        }

        return isConfigurationUpdated;
    }

    /**
     * Saves current configuration to JSON file.
     * 
     * @return If successful saved, it will return true otherwise false.
     */
    bool saveConfiguration()
    {
        bool                status                  = true;
        JsonFile            jsonFile(m_fs);
        const size_t        JSON_DOC_SIZE           = 1024U;
        DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
        JsonObject          jsonRootObject          = jsonDoc.to<JsonObject>();
        String              configurationFilename   = getFullPathToConfiguration();

        getConfiguration(jsonRootObject);
        
        if (false == jsonFile.save(configurationFilename, jsonDoc))
        {
            status = false;
        }
        else
        {
            m_timestampOfLastFileUpdate = getLastConfigurationUpdate();
        }

        return status;
    }

    /**
     * Load configuration from JSON file.
     * 
     * @return If successful loaded, it will return true otherwise false.
     */
    bool loadConfiguration()
    {
        bool                status                  = true;
        JsonFile            jsonFile(m_fs);
        const size_t        JSON_DOC_SIZE           = 1024U;
        DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
        JsonObjectConst     jsonRootObject          = jsonDoc.to<JsonObject>();
        String              configurationFilename   = getFullPathToConfiguration();

        if (false == jsonFile.load(configurationFilename, jsonDoc))
        {
            status = false;
        }
        else
        {
            status = setConfiguration(jsonRootObject);
        }

        return status;
    }

private:

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* PLUGIN_HPP */

/** @} */