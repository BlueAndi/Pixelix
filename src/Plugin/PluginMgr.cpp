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
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "PluginMgr.h"
#include "DisplayMgr.h"
#include "FileSystem.h"
#include "Plugin.hpp"
#include "JsonFile.h"

#include <Logging.h>
#include <ArduinoJson.h>
#include <Util.h>
#include <SettingsService.h>
#include <TopicHandlerService.h>

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

/* Initialize static members */
const char* PluginMgr::CONFIG_FILE_NAME         = "slotConfig.json";
const char* PluginMgr::MQTT_SPECIAL_CHARACTERS  = "+#*>$"; /* See MQTT specification */

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void PluginMgr::begin()
{
    SettingsService& settings = SettingsService::getInstance();

    if (false == settings.open(true))
    {
        m_deviceId = settings.getHostname().getDefault();
    }
    else
    {
        m_deviceId = settings.getHostname().getValue();

        settings.close();
    }

    createPluginConfigDirectory();
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
            TopicHandlerService::getInstance().unregisterTopics(m_deviceId, plugin);

            m_pluginFactory.destroyPlugin(plugin);
        }
    }

    return status;
}

bool PluginMgr::setPluginAliasName(IPluginMaintenance* plugin, const String& alias)
{
    bool isSuccessful = false;

    if ((nullptr != plugin) &&
        (plugin->getAlias() != alias) &&
        (true == isPluginAliasValid(alias)))
    {
        /* First remove current registered topics. */
        TopicHandlerService::getInstance().unregisterTopics(m_deviceId, plugin);

        /* Set new alias */
        plugin->setAlias(alias);

        /* Register web API, based on new alias. */
        TopicHandlerService::getInstance().registerTopics(m_deviceId, plugin);

        isSuccessful = true;
    }

    return isSuccessful;
}

void PluginMgr::unregisterAllPluginTopics()
{
    uint8_t maxSlots    = DisplayMgr::getInstance().getMaxSlots();
    uint8_t slotId      = 0U;

    for(slotId = 0U; slotId < maxSlots; ++slotId)
    {
        IPluginMaintenance* plugin = DisplayMgr::getInstance().getPluginInSlot(slotId);

        TopicHandlerService::getInstance().unregisterTopics(m_deviceId, plugin);
    }
}

bool PluginMgr::load()
{
    bool                isSuccessful            = true;
    JsonFile            jsonFile(FILESYSTEM);
    const size_t        JSON_DOC_SIZE           = 4096U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    String              fullConfigFileName      = PluginConfigFsHandler::CONFIG_PATH;

    fullConfigFileName += "/";
    fullConfigFileName += CONFIG_FILE_NAME;

    if (false == jsonFile.load(fullConfigFileName, jsonDoc))
    {
        LOG_WARNING("Failed to load file %s.", fullConfigFileName.c_str());
        isSuccessful = false;
    }
    else
    {
        JsonArray       jsonSlots   = jsonDoc["slotConfiguration"].as<JsonArray>();
        uint8_t         slotId      = 0;
        const uint8_t   MAX_SLOTS   = DisplayMgr::getInstance().getMaxSlots();

        checkJsonDocOverflow(jsonDoc, __LINE__);

        for(JsonObject jsonSlot: jsonSlots)
        {
            prepareSlotByConfiguration(slotId, jsonSlot);
            
            ++slotId;
            if (MAX_SLOTS <= slotId)
            {
                break;
            }
        }
    }

    return isSuccessful;
}

void PluginMgr::save()
{
    String              installation;
    uint8_t             slotId              = 0;
    const size_t        JSON_DOC_SIZE       = 4096U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    JsonArray           jsonSlots           = jsonDoc.createNestedArray("slotConfiguration");
    JsonFile            jsonFile(FILESYSTEM);
    String              fullConfigFileName  = PluginConfigFsHandler::CONFIG_PATH;

    fullConfigFileName += "/";
    fullConfigFileName += CONFIG_FILE_NAME;

    for(slotId = 0; slotId < DisplayMgr::getInstance().getMaxSlots(); ++slotId)
    {
        IPluginMaintenance* plugin      = DisplayMgr::getInstance().getPluginInSlot(slotId);
        JsonObject          jsonSlot    = jsonSlots.createNestedObject();

        if (nullptr == plugin)
        {
            jsonSlot["name"]        = "";
            jsonSlot["uid"]         = 0;
            jsonSlot["alias"]       = "";
            jsonSlot["fontType"]    = Fonts::fontTypeToStr(Fonts::FONT_TYPE_DEFAULT);
            jsonSlot["duration"]    = DisplayMgr::getInstance().getSlotDuration(slotId);
        }
        else
        {
            jsonSlot["name"]        = plugin->getName();
            jsonSlot["uid"]         = plugin->getUID();
            jsonSlot["alias"]       = plugin->getAlias();
            jsonSlot["fontType"]    = Fonts::fontTypeToStr(plugin->getFontType());
            jsonSlot["duration"]    = DisplayMgr::getInstance().getSlotDuration(slotId);
        }
    }

    checkJsonDocOverflow(jsonDoc, __LINE__);

    if (false == jsonFile.save(fullConfigFileName, jsonDoc))
    {
        LOG_ERROR("Couldn't save slot configuration.");
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/**
 * Check dynamic JSON document for overflow and log a corresponding message,
 * otherwise log its document size.
 * 
 * @param[in] jsonDoc   Dynamic JSON document, which to check.
 * @param[in] line      Line number where the document is handled in the module.
 */
void PluginMgr::checkJsonDocOverflow(const DynamicJsonDocument& jsonDoc, int line)
{
    if (true == jsonDoc.overflowed())
    {
        LOG_ERROR("JSON document @%d has less memory available.", line);
    }
    else
    {
        LOG_INFO("JSON document @%d size: %u", line, jsonDoc.memoryUsage());
    }
}

void PluginMgr::createPluginConfigDirectory()
{
    if (false == FILESYSTEM.exists(PluginConfigFsHandler::CONFIG_PATH))
    {
        if (false == FILESYSTEM.mkdir(PluginConfigFsHandler::CONFIG_PATH))
        {
            LOG_WARNING("Couldn't create directory: %s", PluginConfigFsHandler::CONFIG_PATH);
        }
    }
}

void PluginMgr::prepareSlotByConfiguration(uint8_t slotId, const JsonObject& jsonSlot)
{
    bool                isKeyValuePairMissing   = false;
    JsonVariantConst    jsonName                = jsonSlot["name"];
    JsonVariantConst    jsonUid                 = jsonSlot["uid"];
    JsonVariantConst    jsonAlias               = jsonSlot["alias"];
    JsonVariantConst    jsonFontType            = jsonSlot["fontType"];
    JsonVariantConst    jsonDuration            = jsonSlot["duration"];

    if (false == jsonName.is<String>())
    {
        LOG_WARNING("Slot %u: Name is missing.", slotId);
        isKeyValuePairMissing = true;
    }

    if (false == jsonUid.is<uint16_t>())
    {
        LOG_WARNING("Slot %u: UID is missing.", slotId);
        isKeyValuePairMissing = true;
    }

    if (false == jsonAlias.is<String>())
    {
        LOG_WARNING("Slot %u: Alias is missing.", slotId);
        isKeyValuePairMissing = true;
    }

    if (false == jsonFontType.is<String>())
    {
        LOG_WARNING("Slot %u: Font type is missing.", slotId);
        isKeyValuePairMissing = true;
    }

    if (false == jsonDuration.is<uint32_t>())
    {
        LOG_WARNING("Slot %u: Slot duration is missing.", slotId);
        isKeyValuePairMissing = true;
    }

    if (false == isKeyValuePairMissing)
    {
        String      name        = jsonName.as<String>();
        uint32_t    duration    = jsonDuration.as<uint32_t>();

        if (false == name.isEmpty())
        {
            IPluginMaintenance* plugin = DisplayMgr::getInstance().getPluginInSlot(slotId);

            /* If there is already a plugin installed, it may be a system plugin.
             * In this case skip it, otherwise continue.
             */
            if (nullptr == plugin)
            {
                uint16_t uid = jsonUid.as<uint16_t>();

                plugin = m_pluginFactory.createPlugin(name, uid);
            
                if (nullptr == plugin)
                {
                    LOG_ERROR("Couldn't create plugin %s (uid %u) in slot %u.", name.c_str(), uid, slotId);
                }
                else
                {
                    String          alias           = jsonAlias.as<String>();
                    String          filteredAlias   = filterPluginAlias(alias);
                    String          fontTypeStr     = jsonFontType.as<String>();
                    Fonts::FontType fontType        = Fonts::strToFontType(fontTypeStr.c_str());

                    plugin->setAlias(filteredAlias);
                    plugin->setFontType(fontType);

                    if (false == install(plugin, slotId))
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
            }
        }

        DisplayMgr::getInstance().setSlotDuration(slotId, duration);
    }
}

bool PluginMgr::install(IPluginMaintenance* plugin, uint8_t slotId)
{
    bool isSuccessful = false;

    if (nullptr != plugin)
    {
        if (SlotList::SLOT_ID_INVALID == slotId)
        {
            isSuccessful = installToAutoSlot(plugin);
        }
        else
        {
            isSuccessful = installToSlot(plugin, slotId);
        }

        if (true == isSuccessful)
        {
            TopicHandlerService::getInstance().registerTopics(m_deviceId, plugin);
        }
    }

    return isSuccessful;
}

bool PluginMgr::installToAutoSlot(IPluginMaintenance* plugin)
{
    bool status = false;

    if (nullptr != plugin)
    {
        if (SlotList::SLOT_ID_INVALID == DisplayMgr::getInstance().installPlugin(plugin))
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
        if (SlotList::SLOT_ID_INVALID == DisplayMgr::getInstance().installPlugin(plugin, slotId))
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

bool PluginMgr::isPluginAliasValid(const String& alias)
{
    const size_t    MQTT_SPECIAL_CHARACTERS_LEN = strlen(MQTT_SPECIAL_CHARACTERS);
    bool            isValid                     = true;
    size_t          idx                         = 0U;
    
    while((MQTT_SPECIAL_CHARACTERS_LEN > idx) && (true == isValid))
    {
        if (0 <= alias.indexOf(MQTT_SPECIAL_CHARACTERS[idx]))
        {
            isValid = false;
        }
        else
        {
            ++idx;
        }
    }

    return isValid;
}

String PluginMgr::filterPluginAlias(const String& alias)
{
    const size_t    MQTT_SPECIAL_CHARACTERS_LEN = strlen(MQTT_SPECIAL_CHARACTERS);
    size_t          idx                         = 0U;
    String          filteredPluginAlias         = alias;

    while(MQTT_SPECIAL_CHARACTERS_LEN > idx)
    {
        int pos = filteredPluginAlias.indexOf(MQTT_SPECIAL_CHARACTERS[idx]);

        while(0 <= pos)
        {
            filteredPluginAlias.remove(pos, 1U);
            pos = filteredPluginAlias.indexOf(MQTT_SPECIAL_CHARACTERS[idx]);
        }

        ++idx;
    }

    return filteredPluginAlias;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
