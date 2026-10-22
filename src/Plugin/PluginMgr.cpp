/* MIT License
 *
 * Copyright (c) 2019 - 2026 Andreas Merkle <web@blue-andi.de>
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
 * @file   PluginMgr.cpp
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
#include <PsramJsonDocument.hpp>
#include <Util.h>
#include <PluginAlias.hpp>
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
const char* PluginMgr::CONFIG_FILE_NAME = "slotConfig.json";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void PluginMgr::begin()
{
    SettingsService& settings = SettingsService::getInstance();

    m_deviceId                = settings.getHostname().getValueFromRam();

    createPluginConfigDirectory();
}

IPluginMaintenance* PluginMgr::install(const char* name, uint8_t slotId, const String& alias)
{
    IPluginMaintenance* plugin = m_pluginFactory.createPlugin(name);

    if (nullptr != plugin)
    {
        plugin->setAlias(determineAlias(name, alias, plugin));

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
            unregisterTopics(m_deviceId, plugin, true);

            m_pluginFactory.destroyPlugin(plugin);
        }
    }

    return status;
}

PluginAlias::Result PluginMgr::setPluginAliasName(IPluginMaintenance* plugin, const String& alias)
{
    PluginAlias::Result result    = PluginAlias::RESULT_INVALID;
    String              candidate = PluginAlias::sanitize(alias);

    if (nullptr == plugin)
    {
        /* Guard path: nothing to rename. */
        ;
    }
    else if (false == PluginAlias::isValid(candidate))
    {
        /* Guard path: result is already RESULT_INVALID. */
        ;
    }
    else if (true == m_pluginFactory.isAliasInUse(candidate, plugin))
    {
        result = PluginAlias::RESULT_IN_USE;
    }
    else if (candidate == plugin->getAlias())
    {
        /* No change, so no topic churn and no Home Assistant entity churn. */
        result = PluginAlias::RESULT_OK;
    }
    else
    {
        /* First remove the currently registered topics, so the old Home Assistant
         * entity is purged before the new one is announced.
         */
        unregisterTopics(m_deviceId, plugin, true);

        /* Set new alias */
        plugin->setAlias(candidate);

        /* Register web API, based on new alias. */
        registerTopics(m_deviceId, plugin);

        result = PluginAlias::RESULT_OK;
    }

    return result;
}

void PluginMgr::unregisterAllPluginTopics()
{
    uint8_t maxSlots = DisplayMgr::getInstance().getMaxSlots();
    uint8_t slotId   = 0U;

    for (slotId = 0U; slotId < maxSlots; ++slotId)
    {
        IPluginMaintenance* plugin = DisplayMgr::getInstance().getPluginInSlot(slotId);

        unregisterTopics(m_deviceId, plugin, false);
    }
}

bool PluginMgr::load()
{
    bool              isSuccessful = true;
    JsonFile          jsonFile(FILESYSTEM);
    const size_t      JSON_DOC_SIZE = 4096U;
    PsramJsonDocument jsonDoc(JSON_DOC_SIZE);
    String            fullConfigFileName  = Plugin::CONFIG_PATH;

    fullConfigFileName                   += "/";
    fullConfigFileName                   += CONFIG_FILE_NAME;

    if (false == jsonFile.load(fullConfigFileName, jsonDoc))
    {
        LOG_WARNING("Failed to load file %s.", fullConfigFileName.c_str());
        isSuccessful = false;
    }
    else if (true == jsonDoc.overflowed())
    {
        LOG_ERROR("JSON document size exceeded.");
        isSuccessful = false;
    }
    else
    {
        JsonArray     jsonSlots = jsonDoc["slotConfiguration"].as<JsonArray>();
        uint8_t       slotId    = 0;
        const uint8_t MAX_SLOTS = DisplayMgr::getInstance().getMaxSlots();

        m_isSlotConfigRepaired  = false;

        for (JsonObject jsonSlot : jsonSlots)
        {
            prepareSlotByConfiguration(slotId, jsonSlot);

            ++slotId;
            if (MAX_SLOTS <= slotId)
            {
                break;
            }
        }

        /* Keep the repaired aliases, so the next power cycle doesn't repair again
         * and the user sees the effective names.
         */
        if (true == m_isSlotConfigRepaired)
        {
            LOG_INFO("Slot configuration repaired, saving it.");
            save();
        }
    }

    return isSuccessful;
}

void PluginMgr::save()
{
    String            installation;
    uint8_t           slotId        = 0;
    const size_t      JSON_DOC_SIZE = 4096U;
    PsramJsonDocument jsonDoc(JSON_DOC_SIZE);
    JsonArray         jsonSlots = jsonDoc.createNestedArray("slotConfiguration");
    JsonFile          jsonFile(FILESYSTEM);
    String            fullConfigFileName  = Plugin::CONFIG_PATH;
    DisplayMgr&       displayMgr          = DisplayMgr::getInstance();

    fullConfigFileName                   += "/";
    fullConfigFileName                   += CONFIG_FILE_NAME;

    for (slotId = 0; slotId < displayMgr.getMaxSlots(); ++slotId)
    {
        DisplayMgr::SlotConfig config;
        JsonObject             jsonSlot = jsonSlots.createNestedObject();

        (void)displayMgr.getSlotConfig(slotId, config);

        jsonSlot["name"]       = config.name;
        jsonSlot["uid"]        = config.uid;
        jsonSlot["alias"]      = config.alias;
        jsonSlot["fontType"]   = Fonts::fontTypeToStr(config.fontType);
        jsonSlot["duration"]   = config.duration;
        jsonSlot["isSticky"]   = config.isSticky;
        jsonSlot["isDisabled"] = config.isDisabled;
    }

    if (true == jsonDoc.overflowed())
    {
        LOG_ERROR("JSON document size exceeded.");
    }
    else if (false == jsonFile.save(fullConfigFileName, jsonDoc))
    {
        LOG_ERROR("Couldn't save slot configuration.");
    }
    else
    {
        /* Nothing to do. */
        ;
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

void PluginMgr::prepareSlotByConfiguration(uint8_t slotId, const JsonObject& jsonSlot)
{
    JsonVariantConst jsonName       = jsonSlot["name"];
    JsonVariantConst jsonUid        = jsonSlot["uid"];
    JsonVariantConst jsonAlias      = jsonSlot["alias"];
    JsonVariantConst jsonFontType   = jsonSlot["fontType"];
    JsonVariantConst jsonDuration   = jsonSlot["duration"];
    JsonVariantConst jsonIsSticky   = jsonSlot["isSticky"];
    JsonVariantConst jsonIsDisabled = jsonSlot["isDisabled"];

    if (false == jsonName.is<String>())
    {
        LOG_ERROR("Slot %u: Name is missing.", slotId);
    }
    else if (false == jsonUid.is<uint16_t>())
    {
        LOG_ERROR("Slot %u: UID is missing.", slotId);
    }
    else if (false == jsonAlias.is<String>())
    {
        LOG_ERROR("Slot %u: Alias is missing.", slotId);
    }
    else if (false == jsonFontType.is<String>())
    {
        LOG_ERROR("Slot %u: Font type is missing.", slotId);
    }
    else if (false == jsonDuration.is<uint32_t>())
    {
        LOG_ERROR("Slot %u: Slot duration is missing.", slotId);
    }
    else
    {
        DisplayMgr& displayMgr = DisplayMgr::getInstance();
        const char* name       = jsonName.as<const char*>();
        uint32_t    duration   = jsonDuration.as<uint32_t>();
        bool        isSticky   = false;
        bool        isDisabled = false;

        /* Optional */
        if (false == jsonIsSticky.is<bool>())
        {
            LOG_WARNING("Slot %u: Is sticky flag is missing.", slotId);
        }
        else
        {
            isSticky = jsonIsSticky.as<bool>();
        }

        if (false == jsonIsDisabled.is<bool>())
        {
            LOG_WARNING("Slot %u: Is disabled flag is missing.", slotId);
        }
        else
        {
            isDisabled = jsonIsDisabled.as<bool>();
        }

        /* Name available? */
        if ('\0' != name[0])
        {
            IPluginMaintenance* plugin = DisplayMgr::getInstance().getPluginInSlot(slotId);

            /* If there is already a plugin installed, it may be a system plugin.
             * In this case skip it, otherwise continue.
             */
            if (nullptr == plugin)
            {
                uint16_t uid = jsonUid.as<uint16_t>();

                plugin       = m_pluginFactory.createPlugin(name, uid);

                if (nullptr == plugin)
                {
                    LOG_ERROR("Couldn't create plugin %s (uid %u) in slot %u.", name, uid, slotId);
                }
                else
                {
                    String          requestedAlias = jsonAlias.as<const char*>();
                    String          alias          = determineAlias(name, requestedAlias, plugin);
                    String          fontTypeStr    = jsonFontType.as<const char*>();
                    Fonts::FontType fontType       = Fonts::strToFontType(fontTypeStr.c_str());

                    if (alias != requestedAlias)
                    {
                        LOG_WARNING("Slot %u: Alias %s replaced by %s.", slotId, requestedAlias.c_str(), alias.c_str());
                        m_isSlotConfigRepaired = true;
                    }

                    plugin->setAlias(alias);
                    plugin->setFontType(fontType);

                    if (false == install(plugin, slotId))
                    {
                        LOG_WARNING("Couldn't install %s (uid %u) in slot %u.", name, uid, slotId);

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

        displayMgr.setSlotDuration(slotId, duration);

        if (true == isSticky)
        {
            displayMgr.setSlotSticky(slotId);
        }

        if (false == isDisabled)
        {
            displayMgr.enableSlot(slotId);
        }
        else
        {
            displayMgr.disableSlot(slotId);
        }
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
            registerTopics(m_deviceId, plugin);
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

String PluginMgr::findFreeAlias(const char* pluginTypeName) const
{
    String  alias;
    uint8_t index = 1U;

    while ((MAX_ALIAS_SEARCH >= index) && (true == alias.isEmpty()))
    {
        String candidate = PluginAlias::makeDefault(pluginTypeName, index);

        if (false == m_pluginFactory.isAliasInUse(candidate, nullptr))
        {
            alias = candidate;
        }
        else
        {
            ++index;
        }
    }

    /* Fallback: the search is bounded, but there are far less slots than
     * MAX_ALIAS_SEARCH. Reaching this means the plugin list is inconsistent.
     * Use the last candidate instead of returning an empty alias, which would
     * break the topic registration.
     */
    if (true == alias.isEmpty())
    {
        LOG_ERROR("No free alias for %s found.", pluginTypeName);
        alias = PluginAlias::makeDefault(pluginTypeName, MAX_ALIAS_SEARCH);
    }

    return alias;
}

String PluginMgr::determineAlias(const char* pluginTypeName, const String& requestedAlias, const IPluginMaintenance* plugin) const
{
    String alias = PluginAlias::sanitize(requestedAlias);

    if (false == PluginAlias::isValid(alias))
    {
        alias = findFreeAlias(pluginTypeName);
    }
    else if (true == m_pluginFactory.isAliasInUse(alias, plugin))
    {
        LOG_WARNING("Alias %s is already in use.", alias.c_str());
        alias = findFreeAlias(pluginTypeName);
    }
    else
    {
        /* Requested alias is valid and free. */
        ;
    }

    return alias;
}

String PluginMgr::getEntityId(const IPluginMaintenance* plugin) const
{
    String entityId;

    if (nullptr != plugin)
    {
        entityId  = "display/plugin/";
        entityId += plugin->getAlias();
    }

    return entityId;
}

void PluginMgr::registerTopics(const String& deviceId, IPluginMaintenance* plugin)
{
    if (nullptr != plugin)
    {
        String entityId = getEntityId(plugin);

        TopicHandlerService::getInstance().registerTopics(deviceId, entityId.c_str(), plugin);
    }
}

void PluginMgr::unregisterTopics(const String& deviceId, IPluginMaintenance* plugin, bool purge)
{
    if (nullptr != plugin)
    {
        String entityId = getEntityId(plugin);

        TopicHandlerService::getInstance().unregisterTopics(deviceId, entityId.c_str(), plugin, purge);
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
