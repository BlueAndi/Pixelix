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
 * @file   MultiIconPlugin.cpp
 * @brief  Multiple icon plugin
 * @author Yann Le Glaz <yann_le@web.de>
 *
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "MultiIconPlugin.h"

#include <Logging.h>
#include <Util.h>

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

/* Initialize slot control topic. */
const char* MultiIconPlugin::TOPIC_SLOT                    = "slot";

/* Initialize slot control Home Assistant discovery file name. */
const char* MultiIconPlugin::TOPIC_SLOT_EXTRA_HA_FILE_NAME = "/extra/multiIconPlugin.json";

/* Initialize slots control topic. */
const char* MultiIconPlugin::TOPIC_SLOTS                   = "slots";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

bool MultiIconPlugin::isEnabled() const
{
    bool isEnabled = m_isEnabled;

    /* The plugin shall only be scheduled if its enabled and at least one icon is set. */
    if (true == isEnabled)
    {
        uint8_t slotId;

        for (slotId = 0U; slotId < _MultiIconPlugin::View::MAX_ICON_SLOTS; ++slotId)
        {
            if (false == m_view.isIconSlotEmpty(slotId))
            {
                break;
            }
        }

        if (_MultiIconPlugin::View::MAX_ICON_SLOTS <= slotId)
        {
            isEnabled = false;
        }
    }

    return isEnabled;
}

void MultiIconPlugin::getTopics(JsonArray& topics) const
{
    uint8_t slotId;

    for (slotId = 0U; slotId < _MultiIconPlugin::View::MAX_ICON_SLOTS; ++slotId)
    {
        JsonObject jsonSlot     = topics.createNestedObject();

        jsonSlot["name"]        = String(TOPIC_SLOT) + "/" + slotId;
        jsonSlot["extra"]["ha"] = TOPIC_SLOT_EXTRA_HA_FILE_NAME;
    }

    (void)topics.add(TOPIC_SLOTS);
}

bool MultiIconPlugin::getTopic(const String& topic, JsonObject& value) const
{
    bool isSuccessful = false;

    /* Single slot requested? */
    if (true == topic.startsWith(String(TOPIC_SLOT) + "/"))
    {
        uint8_t slotId = _MultiIconPlugin::View::MAX_ICON_SLOTS;
        bool    status = getSlotIdFromTopic(slotId, topic);

        if ((true == status) &&
            (_MultiIconPlugin::View::MAX_ICON_SLOTS > slotId))
        {
            value["fileName"] = getIconFileName(slotId);

            isSuccessful      = true;
        }
    }
    /* All slots requested? */
    else if (true == topic.equals(TOPIC_SLOTS))
    {
        getConfiguration(value);
        isSuccessful = true;
    }
    else
    {
        ;
    }

    return isSuccessful;
}

bool MultiIconPlugin::setTopic(const String& topic, const JsonObjectConst& value)
{
    bool isSuccessful = false;

    if (true == topic.startsWith(String(TOPIC_SLOT) + "/"))
    {
        uint8_t slotId = _MultiIconPlugin::View::MAX_ICON_SLOTS;
        bool    status = getSlotIdFromTopic(slotId, topic);

        if ((true == status) &&
            (_MultiIconPlugin::View::MAX_ICON_SLOTS > slotId))
        {
            const size_t        JSON_DOC_SIZE = 512U;
            DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
            JsonObject          jsonCfg      = jsonDoc.to<JsonObject>();
            JsonVariantConst    jsonFileName = value["fileName"];

            /* The received configuration may not contain all single key/value pair.
             * Therefore read first the complete internal configuration and
             * overwrite them with the received ones.
             */
            getConfiguration(jsonCfg);

            if (false == jsonFileName.isNull())
            {
                jsonCfg["slots"][slotId] = jsonFileName.as<const char*>();
                isSuccessful             = true;
            }

            if (true == isSuccessful)
            {
                JsonObjectConst jsonCfgConst = jsonCfg;

                isSuccessful                 = setConfiguration(jsonCfgConst);

                if (true == isSuccessful)
                {
                    requestStoreToPersistentMemory();
                }
            }
        }
    }
    else if (true == topic.equals(TOPIC_SLOTS))
    {
        const size_t        JSON_DOC_SIZE = 512U;
        DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
        JsonObject          jsonCfg   = jsonDoc.to<JsonObject>();
        JsonVariantConst    jsonSlots = value["slots"];

        /* The received configuration may not contain all single key/value pair.
         * Therefore read first the complete internal configuration and
         * overwrite them with the received ones.
         */
        getConfiguration(jsonCfg);

        /* Note:
         * Check only for the key/value pair availability.
         * The type check will follow in the setConfiguration().
         */

        if (true == jsonSlots.is<JsonArrayConst>())
        {
            JsonArray jsonCfgSlots = jsonCfg.createNestedArray("slots");

            for (JsonVariantConst slot : jsonSlots.as<JsonArrayConst>())
            {
                const char* fileName = slot.as<const char*>();

                (void)jsonCfgSlots.add(fileName);
            }

            isSuccessful = true;
        }

        if (true == isSuccessful)
        {
            JsonObjectConst jsonCfgConst = jsonCfg;

            isSuccessful                 = setConfiguration(jsonCfgConst);

            if (true == isSuccessful)
            {
                requestStoreToPersistentMemory();
            }
        }
    }
    else
    {
        ;
    }

    return isSuccessful;
}

bool MultiIconPlugin::hasTopicChanged(const String& topic)
{
    bool hasTopicChanged = false;

    if (true == topic.startsWith(String(TOPIC_SLOT) + "/"))
    {
        uint8_t slotId = _MultiIconPlugin::View::MAX_ICON_SLOTS;
        bool    status = getSlotIdFromTopic(slotId, topic);

        if ((true == status) &&
            (_MultiIconPlugin::View::MAX_ICON_SLOTS > slotId))
        {
            MutexGuard<MutexRecursive> guard(m_mutex);

            hasTopicChanged                = m_slots[slotId].hasSlotChanged;
            m_slots[slotId].hasSlotChanged = false;
        }
    }
    else if (true == topic.equals(TOPIC_SLOTS))
    {
        MutexGuard<MutexRecursive> guard(m_mutex);

        hasTopicChanged        = m_hasTopicSlotsChanged;
        m_hasTopicSlotsChanged = false;
    }
    else
    {
        ;
    }

    return hasTopicChanged;
}

void MultiIconPlugin::start(uint16_t width, uint16_t height)
{
    uint8_t                    slotId;
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_view.init(width, height);

    PluginWithConfig::start(width, height);

    for (slotId = 0U; slotId < _MultiIconPlugin::View::MAX_ICON_SLOTS; ++slotId)
    {
        IconSlot& iconSlot = m_slots[slotId];

        if (false == iconSlot.fileName.isEmpty())
        {
            if (false == m_view.loadIcon(slotId, iconSlot.fileName))
            {
                LOG_ERROR("Icon not found: %s", iconSlot.fileName.c_str());
            }
        }
    }
}

void MultiIconPlugin::stop()
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    PluginWithConfig::stop();
}

void MultiIconPlugin::update(YAGfx& gfx)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_view.update(gfx);
}

String MultiIconPlugin::getIconFileName(uint8_t slotId) const
{
    String                     fileName;
    MutexGuard<MutexRecursive> guard(m_mutex);

    if (_MultiIconPlugin::View::MAX_ICON_SLOTS > slotId)
    {
        fileName = m_slots[slotId].fileName;
    }

    return fileName;
}

bool MultiIconPlugin::loadIcon(uint8_t slotId, const String& fileName)
{
    bool isSuccessful = false;

    if (_MultiIconPlugin::View::MAX_ICON_SLOTS > slotId)
    {
        MutexGuard<MutexRecursive> guard(m_mutex);
        IconSlot&                  iconSlot = m_slots[slotId];

        iconSlot.fileName                   = fileName;
        iconSlot.hasSlotChanged             = true;
        m_hasTopicSlotsChanged              = true;

        if (true == iconSlot.fileName.isEmpty())
        {
            m_view.clearIcon(slotId);

            isSuccessful = true;
        }
        else
        {
            isSuccessful = m_view.loadIcon(slotId, iconSlot.fileName);

            if (false == isSuccessful)
            {
                LOG_ERROR("Icon not found: %s", iconSlot.fileName.c_str());
            }
        }
    }

    return isSuccessful;
}

void MultiIconPlugin::clearIcon(uint8_t slotId)
{
    if (_MultiIconPlugin::View::MAX_ICON_SLOTS > slotId)
    {
        MutexGuard<MutexRecursive> guard(m_mutex);
        IconSlot&                  iconSlot = m_slots[slotId];

        iconSlot.fileName.clear();
        iconSlot.hasSlotChanged = true;
        m_hasTopicSlotsChanged  = true;

        m_view.clearIcon(slotId);
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void MultiIconPlugin::getConfiguration(JsonObject& jsonCfg) const
{
    MutexGuard<MutexRecursive> guard(m_mutex);
    JsonArray                  jsonSlots = jsonCfg.createNestedArray("slots");
    uint8_t                    slotId;

    for (slotId = 0U; slotId < _MultiIconPlugin::View::MAX_ICON_SLOTS; ++slotId)
    {
        (void)jsonSlots.add(m_slots[slotId].fileName);
    }
}

bool MultiIconPlugin::setConfiguration(const JsonObjectConst& jsonCfg)
{
    bool             status    = false;
    JsonVariantConst jsonSlots = jsonCfg["slots"];

    if (false == jsonSlots.is<JsonArrayConst>())
    {
        LOG_WARNING("Slots not found or invalid type.");
    }
    else
    {
        MutexGuard<MutexRecursive> guard(m_mutex);
        uint8_t                    slotId = 0U;

        for (JsonVariantConst jsonSlot : jsonSlots.as<JsonArrayConst>())
        {
            if (true == jsonSlot.is<String>())
            {
                const String iconFileName = jsonSlot.as<const char*>();

                if (m_slots[slotId].fileName != iconFileName)
                {
                    (void)loadIcon(slotId, iconFileName);
                }

                ++slotId;
                if (_MultiIconPlugin::View::MAX_ICON_SLOTS <= slotId)
                {
                    break;
                }
            }
        }

        status = true;
    }

    return status;
}

bool MultiIconPlugin::getSlotIdFromTopic(uint8_t& slotId, const String& topic) const
{
    bool    isSuccessful     = false;
    int32_t indexBeginSlotId = topic.lastIndexOf("/");

    if (0 <= indexBeginSlotId)
    {
        String  slotIdStr = topic.substring(indexBeginSlotId + 1);
        uint8_t slotIdTmp;
        bool    statusSlotId = Util::strToUInt8(slotIdStr, slotIdTmp);

        if (true == statusSlotId)
        {
            slotId       = slotIdTmp;
            isSuccessful = true;
        }
    }

    return isSuccessful;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
