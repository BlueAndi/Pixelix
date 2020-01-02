/* MIT License
 *
 * Copyright (c) 2019 Andreas Merkle <web@blue-andi.de>
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
#include "Plugin.hpp"
#include "SysMsgPlugin.h"
#include "JustTextPlugin.h"

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
    static PluginMgr& getInstance(void)
    {
        return m_instance;
    }

    /**
     * Install SysMsgPlugin plugin.
     * 
     * @return If successful installed, it will return true otherwise false.
     */
    SysMsgPlugin* installSysMsgPlugin(void);

    /**
     * Install JustText plugin.
     * 
     * @return If successful installed, it will return true otherwise false.
     */
    JustTextPlugin* installJustTextPlugin(void);

    /**
     * Uninstall plugin.
     *
     * @param[in] plugin    Plugin, which to remove
     */
    void uninstall(Plugin* plugin);

    /**
     * Get plugin REST base URI.
     * 
     * @param[in] slotId    Slot id of installed plugin.
     * 
     * @return Plugin REST API base URI
     */
    String getRestApiBaseUri(uint8_t slotId);

private:

    static PluginMgr        m_instance; /**< Plugin manager instance */

    DLinkedList<Plugin*>    m_plugins;  /**< List with all installed plugins */

    /**
     * Constructs the plugin manager.
     */
    PluginMgr() :
        m_plugins()
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
     * Install plugin to any available display slot.
     *
     * @param[in] plugin    Plugin, which to install
     * 
     * @return If successful installed, it will return true otherwise false.
     */
    bool install(Plugin* plugin);

    /**
     * Install plugin to a specific display slot.
     *
     * @param[in] plugin    Plugin, which to install
     * @param[in] slotId    Id of the slot, where to install the plugin
     * 
     * @return If successful installed, it will return true otherwise false.
     */
    bool installToSlot(Plugin* plugin, uint8_t slotId);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __PLUGINMGR_H__ */

/** @} */