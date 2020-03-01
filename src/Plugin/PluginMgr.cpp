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
#include "Settings.h"

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

void PluginMgr::registerPlugin(const String& name, IPluginMaintenance::CreateFunc createFunc)
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

IPluginMaintenance* PluginMgr::install(const String& name, uint8_t slotId)
{
    IPluginMaintenance* plugin  = nullptr;
    PluginRegEntry*     entry   = nullptr;

    if (true == m_registry.selectFirstElement())
    {
        bool isFound = false;

        /* Find plugin in the registry */
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

        /* Plugin found? */
        if ((true == isFound) &&
            (nullptr != entry))
        {
            plugin = entry->createFunc(entry->name, generateUID());

            if (DisplayMgr::SLOT_ID_INVALID == slotId)
            {
                if (false == installToAutoSlot(plugin))
                {
                    delete plugin;
                    plugin = nullptr;
                }
            }
            else
            {
                if (false == installToSlot(plugin, slotId))
                {
                    delete plugin;
                    plugin = nullptr;
                }
            }
        }
    }

    return plugin;
}

bool PluginMgr::uninstall(IPluginMaintenance* plugin)
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

String PluginMgr::getRestApiBaseUri(uint16_t uid)
{
    String  baseUri = RestApi::BASE_URI;
    baseUri += "/display";
    baseUri += "/uid/";
    baseUri += uid;

    return baseUri;
}

void PluginMgr::load()
{
    Settings& settings = Settings::getInstance();

    if (false == settings.open(true))
    {
        LOG_WARNING("Couldn't open filesystem.");
    }
    else
    {
        String installation = settings.getPluginInstallation().getValue();

        if (false == installation.isEmpty())
        {
            uint8_t slotId  = 0;
            uint8_t index   = 0;
            String  pluginName;

            while('\0' != installation[index])
            {
                if (DELIMITER == installation[index])
                {
                    if (false == pluginName.isEmpty())
                    {
                        IPluginMaintenance* plugin = install(pluginName, slotId);

                        if (nullptr != plugin)
                        {
                            plugin->enable();
                        }
                    }

                    pluginName.clear();
                    ++slotId;
                }
                else
                {
                    pluginName += installation[index];
                }

                ++index;
            }

            if (false == pluginName.isEmpty())
            {
                IPluginMaintenance* plugin = install(pluginName, slotId);

                if (nullptr != plugin)
                {
                    plugin->enable();
                }
            }
        }

        settings.close();
    }
}

void PluginMgr::save()
{
    String              installation;
    uint8_t             slotId      = 0;
    IPluginMaintenance* plugin      = nullptr;
    Settings&           settings    = Settings::getInstance();

    for(slotId = 0; slotId < DisplayMgr::MAX_SLOTS; ++slotId)
    {
        plugin = DisplayMgr::getInstance().getPluginInSlot(slotId);

        if (0 < slotId)
        {
            installation += DELIMITER;
        }

        if (nullptr != plugin)
        {
            installation += plugin->getName();
        }
    }

    if (false == settings.open(false))
    {
        LOG_WARNING("Couldn't open filesystem.");
    }
    else
    {
        settings.getPluginInstallation().setValue(installation);
        settings.close();
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

bool PluginMgr::installToAutoSlot(IPluginMaintenance* plugin)
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
                String baseUri = getRestApiBaseUri(plugin->getUID());

                plugin->registerWebInterface(MyWebServer::getInstance(), baseUri);

                status = true;
            }
        }
    }

    return status;
}

bool PluginMgr::installToSlot(IPluginMaintenance* plugin, uint8_t slotId)
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
                String baseUri = getRestApiBaseUri(plugin->getUID());

                plugin->registerWebInterface(MyWebServer::getInstance(), baseUri);

                status = true;
            }
        }
    }

    return status;
}

uint16_t PluginMgr::generateUID()
{
    uint16_t    uid;
    bool        isFound;

    do
    {
        isFound = false;
        uid     = random(UINT16_MAX);

        /* Ensure that UID is really unique. */
        if (true == m_plugins.selectFirstElement())
        {
            IPluginMaintenance* plugin = *m_plugins.current();

            while((false == isFound) && (nullptr != plugin))
            {
                if (uid == plugin->getUID())
                {
                    isFound = true;
                }
                else if (false == m_plugins.next())
                {
                    plugin = nullptr;
                }
                else
                {
                    plugin = *m_plugins.current();
                }
            }
        }
    }
    while(true == isFound);

    return uid;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
