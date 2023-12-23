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
 * @brief  Plugin manager
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup plugin
 *
 * @{
 */

#ifndef PLUGINMGR_H
#define PLUGINMGR_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include "SlotList.h"
#include "PluginFactory.h"

#include <IPluginMaintenance.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * The plugin manager installs a plugin in a display slot and register its web pages.
 * Or uninstalls a plugin and unregister its web pages.
 */
class PluginMgr
{
public:

    /**
     * Get instance of plugin manager.
     *
     * @return Plugin manager instance
     */
    static PluginMgr& getInstance()
    {
        static PluginMgr instance; /* singleton idiom to force initialization in the first usage. */

        return instance;
    }

    /**
     * The plugin manager prepares everything for the plugins.
     * Call this once before any other method is used.
     */
    void begin();

    /**
     * Install plugin.
     * If no slot id is given, the plugin will be installed in the next available slot.
     *
     * @param[in] name      Plugin name
     * @param[in] slotId    Optional slot id
     *
     * @return If successful, it will return a pointer to the plugin instance, otherwise nullptr.
     */
    IPluginMaintenance* install(const String& name, uint8_t slotId = SlotList::SLOT_ID_INVALID);

    /**
     * Uninstall plugin.
     *
     * @param[in] plugin    Plugin, which to remove
     *
     * @return If successful uninstalled, it will return true otherwise false.
     */
    bool uninstall(IPluginMaintenance* plugin);

    /**
     * Set the alias name of a plugin.
     * If the plugin has registered a topic handler, the corresponding URIs will be updated.
     * 
     * @param[in] plugin    Plugin which to assign the alias name
     * @param[in] alias     Plugin alias name
     * @return If successful, it will return true otherwise false.
     */
    bool setPluginAliasName(IPluginMaintenance* plugin, const String& alias);

    /**
     * Unregister all plugin topics from the topic handler service.
     * The plugins will still be installed, but won't get any update from outside.
     */
    void unregisterAllPluginTopics();

    /**
     * Load plugin installation from persistent memory.
     * It will automatically enable the installed plugins.
     * 
     * If a slot already contains a plugin, this slot won't change.
     * If loading fails, no plugin at all will be installed.
     * 
     * @return If successful, it will return true otherwise false.
     */
    bool load();

    /**
     * Save plugin installation to persistent memory.
     */
    void save();

    /**
     * Filename of slot configuration.
     */
    static const char*  CONFIG_FILE_NAME;

private:

    /** MQTT special characters, which shall not be part of a plugin alias. */
    static const char*  MQTT_SPECIAL_CHARACTERS;

    PluginFactory   m_pluginFactory;    /**< The plugin factory with the plugin type registry. */
    String          m_deviceId;         /**< Device id, used for topic registration. */

    /**
     * Constructs the plugin manager.
     */
    PluginMgr() :
        m_pluginFactory(),
        m_deviceId()
    {
    }

    /**
     * Destroys the plugin manager.
     */
    ~PluginMgr()
    {
        /* Will never be called. */
    }

    PluginMgr(const PluginMgr& fab);
    PluginMgr& operator=(const PluginMgr& fab);

    /**
     * Check dynamic JSON document for overflow and log a corresponding message,
     * otherwise log its document size.
     * 
     * @param[in] jsonDoc   Dynamic JSON document, which to check.
     * @param[in] line      Line number where the document is handled in the module.
     */
    void checkJsonDocOverflow(const DynamicJsonDocument& jsonDoc, int line);

    /**
     * If configuration directory doesn't exists, it will be created.
     * Otherwise nothing happens.
     */
    void createPluginConfigDirectory();

    /**
     * Prepares a slot according to the given configuration.
     * 
     * @param[in]   slotId      The ID of the slot.
     * @param[in]   jsonSlot    Slot configuration
     */
    void prepareSlotByConfiguration(uint8_t slotId, const JsonObject& jsonSlot);

    /**
     * Install plugin.
     * If no slot id is given, the plugin will be installed in the next available slot.
     *
     * @param[in] plugin    The plugin
     * @param[in] slotId    Slot id
     *
     * @return If successful, it will return a pointer to the plugin instance, otherwise nullptr.
     */
    bool install(IPluginMaintenance* plugin, uint8_t slotId);

    /**
     * Install plugin to any available display slot.
     *
     * @param[in] plugin    Plugin, which to install
     *
     * @return If successful installed, it will return true otherwise false.
     */
    bool installToAutoSlot(IPluginMaintenance* plugin);

    /**
     * Install plugin to a specific display slot.
     *
     * @param[in] plugin    Plugin, which to install
     * @param[in] slotId    Id of the slot, where to install the plugin
     *
     * @return If successful installed, it will return true otherwise false.
     */
    bool installToSlot(IPluginMaintenance* plugin, uint8_t slotId);

    /**
     * Checks whether the alias is valid. It will check for not compliant
     * special characters.
     * 
     * @param[in] alias Plugin alias
     * 
     * @return If plugin alias is valid, it will return true otherwise false.
     */
    bool isPluginAliasValid(const String& alias);

    /**
     * Filters not allowed characters out of the plugin alias.
     * 
     * @param[in] alias Plugin alias
     * 
     * @return Filtered plugin alias
     */
    String filterPluginAlias(const String& alias);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* PLUGINMGR_H */

/** @} */