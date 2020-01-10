/* MIT License
 *
 * Copyright (c) 2019 - 2020 Andreas Merkle <web@blue-andi.de>
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
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "PluginMgr.h"
#include "DisplayMgr.h"
#include "MyWebServer.h"
#include "RestApi.h"

#include <Logging.h>

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and classes
 *****************************************************************************/

/******************************************************************************
 * Prototypes
 *****************************************************************************/

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/* Initialize plugin manager */
PluginMgr   PluginMgr::m_instance;

/******************************************************************************
 * Public Methods
 *****************************************************************************/

SysMsgPlugin* PluginMgr::installSysMsgPlugin()
{
    SysMsgPlugin*   plugin = new SysMsgPlugin();

    if (nullptr != plugin)
    {
        if (false == install(plugin))
        {
            delete plugin;
            plugin = nullptr;
        }
    }

    return  plugin;
}

JustTextPlugin* PluginMgr::installJustTextPlugin()
{
    JustTextPlugin* plugin = new JustTextPlugin();

    if (nullptr != plugin)
    {
        if (false == install(plugin))
        {
            delete plugin;
            plugin = nullptr;
        }
    }

    return  plugin;
}

FirePlugin* PluginMgr::installFirePlugin()
{
    FirePlugin* plugin = new FirePlugin();

    if (nullptr != plugin)
    {
        if (false == install(plugin))
        {
            delete plugin;
            plugin = nullptr;
        }
    }

    return  plugin;
}

IconTextPlugin* PluginMgr::installIconTextPlugin()
{
    IconTextPlugin* plugin = new IconTextPlugin();

    if (nullptr != plugin)
    {
        if (false == install(plugin))
        {
            delete plugin;
            plugin = nullptr;
        }
    }

    return  plugin;
}

IconTextLampPlugin* PluginMgr::installIconTextLampPlugin()
{
    IconTextLampPlugin* plugin = new IconTextLampPlugin();

    if (nullptr != plugin)
    {
        if (false == install(plugin))
        {
            delete plugin;
            plugin = nullptr;
        }
    }

    return  plugin;
}

GameOfLifePlugin* PluginMgr::installGameOfLifePlugin()
{
    GameOfLifePlugin* plugin = new GameOfLifePlugin();

    if (nullptr != plugin)
    {
        if (false == install(plugin))
        {
            delete plugin;
            plugin = nullptr;
        }
    }

    return  plugin;
}

void PluginMgr::uninstall(Plugin* plugin)
{
    if (nullptr != plugin)
    {
        if (false == m_plugins.find(plugin))
        {
            LOG_WARNING("Plugin 0x%X (%s) not found in list.", plugin, plugin->getName());
        }
        else
        {
            plugin->unregisterWebInterface(MyWebServer::getInstance());
            DisplayMgr::getInstance().uninstallPlugin(plugin);

            m_plugins.removeSelected();
        }
        
    }

    return;
}

String PluginMgr::getRestApiBaseUri(uint8_t slotId)
{
    String  baseUri = RestApi::BASE_URI;
    baseUri += "/display";

    if (DisplayMgr::MAX_SLOTS > slotId)
    {
        baseUri += "/slot/";
        baseUri += slotId;
    }

    return baseUri;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

bool PluginMgr::install(Plugin* plugin)
{
    bool status = false;

    if (nullptr != plugin)
    {
        if (DisplayMgr::SLOT_ID_INVALID == DisplayMgr::getInstance().installPlugin(plugin))
        {
            LOG_ERROR("Couldn't install plugin %s.", plugin->getName());
        }
        else
        {
            if (false == m_plugins.append(plugin))
            {
                LOG_ERROR("Couldn't append plugin %s.", plugin->getName());

                DisplayMgr::getInstance().uninstallPlugin(plugin);
            }
            else
            {
                String baseUri = getRestApiBaseUri(plugin->getSlotId());

                plugin->registerWebInterface(MyWebServer::getInstance(), baseUri);

                status = true;
            }
        }
    }

    return status;
}

bool PluginMgr::installToSlot(Plugin* plugin, uint8_t slotId)
{
    bool status = false;

    if (nullptr != plugin)
    {
        if (DisplayMgr::SLOT_ID_INVALID == DisplayMgr::getInstance().installPlugin(plugin, slotId))
        {
            LOG_ERROR("Couldn't install plugin %s to slot %u.", plugin->getName(), slotId);
        }
        else
        {
            if (false == m_plugins.append(plugin))
            {
                LOG_ERROR("Couldn't append plugin %s.", plugin->getName());
                
                DisplayMgr::getInstance().uninstallPlugin(plugin);
            }
            else
            {
                String baseUri = getRestApiBaseUri(plugin->getSlotId());

                plugin->registerWebInterface(MyWebServer::getInstance(), baseUri);

                status = true;
            }
        }
    }

    return status;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
