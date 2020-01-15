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

void PluginMgr::registerPlugin(const String& name, Plugin::CreateFunc createFunc)
{
    PluginRegEntry* entry = new PluginRegEntry();

    if (nullptr != entry)
    {
        entry->name         = name;
        entry->createFunc   = createFunc;

        if (false == m_registry.append(entry))
        {
            LOG_ERROR("Couldn't add %s to registry.", name.c_str());

            delete entry;
            entry = nullptr;
        }
        else
        {
            LOG_INFO("Plugin %s registered.", name.c_str());
        }
    }
    else
    {
        LOG_ERROR("Couldn't add %s to registry.", name.c_str());
    }

    return;
}

Plugin* PluginMgr::install(const String& name)
{
    Plugin*         plugin  = nullptr;
    PluginRegEntry* entry   = nullptr;

    if (true == m_registry.selectFirstElement())
    {
        bool isFound = false;

        entry = *m_registry.current();

        while((false == isFound) && (nullptr != entry))
        {
            if (name == entry->name)
            {
                isFound = true;
            }
            else if (false == m_registry.next())
            {
                entry = nullptr;
            }
            else
            {
                entry = *m_registry.current();
            }
        }

        if ((true == isFound) &&
            (nullptr != entry))
        {
            plugin = entry->createFunc(entry->name);
            if (false == installToAutoSlot(plugin))
            {
                delete plugin;
                plugin = nullptr;
            }
        }
    }

    return plugin;
}

bool PluginMgr::uninstall(Plugin* plugin)
{
    bool status = false;

    if (nullptr != plugin)
    {
        if (false == m_plugins.find(plugin))
        {
            LOG_WARNING("Plugin 0x%X (%s) not found in list.", plugin, plugin->getName());
        }
        else
        {
            status = DisplayMgr::getInstance().uninstallPlugin(plugin);

            if (true == status)
            {
                plugin->unregisterWebInterface(MyWebServer::getInstance());
                m_plugins.removeSelected();
            }
        }
    }

    return status;
}

const char* PluginMgr::findFirst()
{
    const char* name = nullptr;

    if (true == m_registry.selectFirstElement())
    {
        name = (*m_registry.current())->name.c_str();
    }

    return name;
}

const char* PluginMgr::findNext()
{
    const char* name = nullptr;

    if (true == m_registry.next())
    {
        name = (*m_registry.current())->name.c_str();
    }

    return name;
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

bool PluginMgr::installToAutoSlot(Plugin* plugin)
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

                (void)DisplayMgr::getInstance().uninstallPlugin(plugin);
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

                (void)DisplayMgr::getInstance().uninstallPlugin(plugin);
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
