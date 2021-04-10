/* MIT License
 *
 * Copyright (c) 2019 - 2021 Andreas Merkle <web@blue-andi.de>
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
#include "FileSystem.h"
#include "Plugin.hpp"

#include <Logging.h>
#include <ArduinoJson.h>

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

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void PluginMgr::begin()
{
    createPluginConfigDirectory();
}

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
    return install(name, generateUID(), slotId);
}

bool PluginMgr::uninstall(IPluginMaintenance* plugin)
{
    bool status = false;

    if (nullptr != plugin)
    {
        DLinkedListIterator<IPluginMaintenance*> it(m_plugins);

        if (false == it.find(plugin))
        {
            LOG_WARNING("Plugin 0x%X (%s) not found in list.", plugin, plugin->getName());
        }
        else
        {
            status = DisplayMgr::getInstance().uninstallPlugin(plugin);

            if (true == status)
            {
                plugin->unregisterWebInterface(MyWebServer::getInstance());
                it.remove();
            }
        }
    }

    return status;
}

const char* PluginMgr::findFirst()
{
    const char* name = nullptr;

    if (true == m_registryIter.first())
    {
        name = (*m_registryIter.current())->name.c_str();
    }

    return name;
}

const char* PluginMgr::findNext()
{
    const char* name = nullptr;

    if (true == m_registryIter.next())
    {
        name = (*m_registryIter.current())->name.c_str();
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

        if (true == installation.isEmpty())
        {
            LOG_WARNING("Plugin installation is empty.");
        }
        else
        {
            const size_t            JSON_DOC_SIZE   = 1024U;
            DynamicJsonDocument     jsonDoc(JSON_DOC_SIZE);
            DeserializationError    error           = deserializeJson(jsonDoc, installation);

            if (true == jsonDoc.overflowed())
            {
                LOG_ERROR("JSON document has less memory available.");
            }
            else
            {
                LOG_INFO("JSON document size: %u", jsonDoc.memoryUsage());
            }

            if (DeserializationError::Ok != error.code())
            {
                LOG_WARNING("JSON deserialization failed: %s", error.c_str());
            }
            else if (false == jsonDoc["slots"].is<JsonArray>())
            {
                LOG_WARNING("Invalid JSON format.");
            }
            else
            {
                JsonArray   jsonSlots   = jsonDoc["slots"].as<JsonArray>();
                uint8_t     slotId      = 0;

                for(JsonObject jsonSlot: jsonSlots)
                {
                    if ((true == jsonSlot["name"].is<String>()) &&
                        (true == jsonSlot["uid"].is<uint16_t>()))
                    {
                        String      name    = jsonSlot["name"].as<String>();
                        uint16_t    uid     = jsonSlot["uid"].as<uint16_t>();

                        if (false == name.isEmpty())
                        {
                            IPluginMaintenance* plugin = install(name, uid, slotId);

                            if (nullptr == plugin)
                            {
                                LOG_WARNING("Couldn't install %s (uid %u) in slot %u.", name.c_str(), uid, slotId);
                            }
                            else
                            {
                                plugin->enable();
                            }
                        }

                        ++slotId;
                        if (DisplayMgr::getInstance().getMaxSlots() <= slotId)
                        {
                            break;
                        }
                    }
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
    Settings&           settings    = Settings::getInstance();
    const size_t        JSON_DOC_SIZE   = 1024U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    JsonArray           jsonSlots   = jsonDoc.createNestedArray("slots");

    for(slotId = 0; slotId < DisplayMgr::getInstance().getMaxSlots(); ++slotId)
    {
        IPluginMaintenance* plugin      = DisplayMgr::getInstance().getPluginInSlot(slotId);
        JsonObject          jsonSlot    = jsonSlots.createNestedObject();

        if (nullptr == plugin)
        {
            jsonSlot["name"]    = "";
            jsonSlot["uid"]     = 0;
        }
        else
        {
            jsonSlot["name"]    = plugin->getName();
            jsonSlot["uid"]     = plugin->getUID();
        }
    }

    if (true == jsonDoc.overflowed())
    {
        LOG_ERROR("JSON document has less memory available.");
    }
    else
    {
        LOG_INFO("JSON document size: %u", jsonDoc.memoryUsage());
    }

    if (false == settings.open(false))
    {
        LOG_WARNING("Couldn't open filesystem.");
    }
    else
    {
        (void)serializeJson(jsonDoc, installation);

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

void PluginMgr::createPluginConfigDirectory()
{
    if (false == FILESYSTEM.exists(Plugin::CONFIG_PATH))
    {
        if (false == FILESYSTEM.mkdir(Plugin::CONFIG_PATH))
        {
            LOG_WARNING("Couldn't create directory: %s", Plugin::CONFIG_PATH);
        }
    }
}

IPluginMaintenance* PluginMgr::install(const String& name, uint16_t uid, uint8_t slotId)
{
    IPluginMaintenance*                     plugin  = nullptr;
    PluginRegEntry*                         entry   = nullptr;
    DLinkedListIterator<PluginRegEntry*>    it(m_registry);

    if (true == it.first())
    {
        bool isFound = false;

        /* Find plugin in the registry */
        entry = *it.current();

        while((false == isFound) && (nullptr != entry))
        {
            if (name == entry->name)
            {
                isFound = true;
            }
            else if (false == it.next())
            {
                entry = nullptr;
            }
            else
            {
                entry = *it.current();
            }
        }

        /* Plugin found? */
        if ((true == isFound) &&
            (nullptr != entry))
        {
            plugin = entry->createFunc(entry->name, uid);

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
    uint16_t                                        uid;
    bool                                            isFound;
    DLinkedListConstIterator<IPluginMaintenance*>   it(m_plugins);

    do
    {
        isFound = false;
        uid     = random(UINT16_MAX);

        /* Ensure that UID is really unique. */
        if (true == it.first())
        {
            const IPluginMaintenance* plugin = *it.current();

            while((false == isFound) && (nullptr != plugin))
            {
                if (uid == plugin->getUID())
                {
                    isFound = true;
                }
                else if (false == it.next())
                {
                    plugin = nullptr;
                }
                else
                {
                    plugin = *it.current();
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
