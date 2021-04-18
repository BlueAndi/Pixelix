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
    m_pluginFactory.registerPlugin(name, createFunc);
    return;
}

IPluginMaintenance* PluginMgr::install(const String& name, uint8_t slotId)
{
    IPluginMaintenance* plugin = m_pluginFactory.createPlugin(name);

    if (nullptr != plugin)
    {
        if (false == install(plugin, slotId))
        {
            m_pluginFactory.destroyPlugin(plugin);
            plugin = nullptr;
        }
    }

    return plugin;
}

bool PluginMgr::uninstall(IPluginMaintenance* plugin)
{
    bool status = false;

    if (nullptr != plugin)
    {
        status = DisplayMgr::getInstance().uninstallPlugin(plugin);

        if (true == status)
        {
            plugin->unregisterWebInterface(MyWebServer::getInstance());
            m_pluginFactory.destroyPlugin(plugin);
        }
    }

    return status;
}

const char* PluginMgr::findFirst()
{
    return m_pluginFactory.findFirst();
}

const char* PluginMgr::findNext()
{
    return m_pluginFactory.findNext();
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
                            IPluginMaintenance* plugin = m_pluginFactory.createPlugin(name, uid);

                            if (nullptr == plugin)
                            {
                                LOG_ERROR("Couldn't create plugin %s (uid %u) in slot %u.", name.c_str(), uid, slotId);
                            }
                            else if (false == install(plugin, slotId))
                            {
                                LOG_WARNING("Couldn't install %s (uid %u) in slot %u.", name.c_str(), uid, slotId);

                                m_pluginFactory.destroyPlugin(plugin);
                                plugin = nullptr;
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

bool PluginMgr::install(IPluginMaintenance* plugin, uint8_t slotId)
{
    bool isSuccessful = false;

    if (nullptr != plugin)
    {
        if (DisplayMgr::SLOT_ID_INVALID == slotId)
        {
            isSuccessful = installToAutoSlot(plugin);
        }
        else
        {
            isSuccessful = installToSlot(plugin, slotId);
        }

        if (true == isSuccessful)
        {
            String baseUri = getRestApiBaseUri(plugin->getUID());

            plugin->registerWebInterface(MyWebServer::getInstance(), baseUri);
        }
    }

    return isSuccessful;
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
            status = true;
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
            status = true;
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
