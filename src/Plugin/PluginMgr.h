/* MIT License
 *
 * Copyright (c) 2019 - 2021 Andreas Merkle Merkle <web@blue-andi.de>
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

#ifndef __PLUGINMGR_H__
#define __PLUGINMGR_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include "IPluginMaintenance.hpp"
#include "DisplayMgr.h"
#include "PluginFactory.h"

#include <LinkedList.hpp>

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
     * Register a plugin.
     *
     * @param[in] name          Plugin name
     * @param[in] createFunc    The plugin creation function.
     */
    void registerPlugin(const String& name, IPluginMaintenance::CreateFunc createFunc);

    /**
     * Install plugin.
     * If no slot id is given, the plugin will be installed in the next available slot.
     *
     * @param[in] name      Plugin name
     * @param[in] slotId    Optional slot id
     *
     * @return If successful, it will return a pointer to the plugin instance, otherwise nullptr.
     */
    IPluginMaintenance* install(const String& name, uint8_t slotId = DisplayMgr::SLOT_ID_INVALID);

    /**
     * Uninstall plugin.
     *
     * @param[in] plugin    Plugin, which to remove
     *
     * @return If successful uninstalled, it will return true otherwise false.
     */
    bool uninstall(IPluginMaintenance* plugin);

    /**
     * Find first plugin.
     *
     * @return If plugin found, it will return its name otherwise nullptr.
     */
    const char* findFirst();

    /**
     * Find next plugin.
     *
     * @return If plugin found, it will return its name otherwise nullptr.
     */
    const char* findNext();

    /**
     * Get plugin REST base URI.
     *
     * @param[in] uid   Plugin UID
     *
     * @return Plugin REST API base URI
     */
    String getRestApiBaseUri(uint16_t uid);

    /**
     * Load plugin installation from persistent memory.
     * It will automatically enable the installed plugins.
     * If a slot already contains a plugin, this slot won't change.
     */
    void load();

    /**
     * Save plugin installation to persistent memory.
     */
    void save();

private:

    PluginFactory   m_pluginFactory;    /**< The plugin factory with the plugin type registry. */

    /**
     * Constructs the plugin manager.
     */
    PluginMgr() :
        m_pluginFactory()
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
     * If configuration directory doesn't exists, it will be created.
     * Otherwise nothing happens.
     */
    void createPluginConfigDirectory();

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

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __PLUGINMGR_H__ */

/** @} */