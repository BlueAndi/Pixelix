/* MIT License
 *
 * Copyright (c) 2019 - 2025 Andreas Merkle <web@blue-andi.de>
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

void MultiIconPlugin::getTopics(JsonArray& topics) const
{
    uint8_t slotId;

    for (slotId = 0U; slotId < _MultiIconPlugin::View::MAX_ICON_SLOTS; ++slotId)
    {
        JsonObject jsonSlot     = topics.createNestedObject();

        jsonSlot["name"]        = String(TOPIC_SLOT) + "/" + slotId;
        jsonSlot["extra"]["ha"] = TOPIC_SLOT_EXTRA_HA_FILE_NAME;
    }

    topics.add(TOPIC_SLOTS);
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
            value["slotId"] = slotId;
            value["fileId"] = getIconFileId(slotId);

            isSuccessful    = true;
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
            JsonObject          jsonCfg    = jsonDoc.to<JsonObject>();
            JsonVariantConst    jsonFileId = value["fileId"];

            /* The received configuration may not contain all single key/value pair.
             * Therefore read first the complete internal configuration and
             * overwrite them with the received ones.
             */
            getConfiguration(jsonCfg);

            if (false == jsonFileId.isNull())
            {
                jsonCfg["slots"][slotId] = jsonFileId.as<FileMgrService::FileId>();
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
                FileMgrService::FileId fileId = slot.as<String>().toInt();

                jsonCfgSlots.add(fileId);
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

        if (FileMgrService::FILE_ID_INVALID != iconSlot.fileId)
        {
            String iconFullPath;

            if (false == FileMgrService::getInstance().getFileFullPathById(iconFullPath, iconSlot.fileId))
            {
                LOG_WARNING("Unknown file id %u.", iconSlot.fileId);
            }
            else if (false == m_view.loadIcon(slotId, iconFullPath))
            {
                LOG_ERROR("Icon not found: %s", iconFullPath.c_str());
            }
            else
            {
                ;
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

uint8_t MultiIconPlugin::getIconFileId(uint8_t slotId) const
{
    FileMgrService::FileId     fileId = FileMgrService::FILE_ID_INVALID;
    MutexGuard<MutexRecursive> guard(m_mutex);

    if (_MultiIconPlugin::View::MAX_ICON_SLOTS > slotId)
    {
        fileId = m_slots[slotId].fileId;
    }

    return fileId;
}

bool MultiIconPlugin::loadIcon(uint8_t slotId, FileMgrService::FileId fileId)
{
    bool isSuccessful = false;

    if (_MultiIconPlugin::View::MAX_ICON_SLOTS > slotId)
    {
        MutexGuard<MutexRecursive> guard(m_mutex);
        IconSlot&                  iconSlot = m_slots[slotId];
        String                     iconFullPath;

        iconSlot.fileId         = fileId;
        iconSlot.hasSlotChanged = true;
        m_hasTopicSlotsChanged  = true;

        if (FileMgrService::FILE_ID_INVALID == iconSlot.fileId)
        {
            m_view.clearIcon(slotId);
        }
        else if (false == FileMgrService::getInstance().getFileFullPathById(iconFullPath, iconSlot.fileId))
        {
            LOG_WARNING("Unknown file id %u.", iconSlot.fileId);
            m_view.clearIcon(slotId);
        }
        else if (false == m_view.loadIcon(slotId, iconFullPath))
        {
            LOG_ERROR("Icon not found: %s", iconFullPath.c_str());
        }
        else
        {
            ;
        }

        isSuccessful = true;
    }

    return isSuccessful;
}

void MultiIconPlugin::clearIcon(uint8_t slotId)
{
    if (_MultiIconPlugin::View::MAX_ICON_SLOTS > slotId)
    {
        MutexGuard<MutexRecursive> guard(m_mutex);
        IconSlot&                  iconSlot = m_slots[slotId];

        iconSlot.fileId                     = FileMgrService::FILE_ID_INVALID;
        iconSlot.hasSlotChanged             = true;
        m_hasTopicSlotsChanged              = true;

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
        jsonSlots.add(m_slots[slotId].fileId);
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
            if (true == jsonSlot.is<FileMgrService::FileId>())
            {
                FileMgrService::FileId fileId = jsonSlot.as<FileMgrService::FileId>();

                if (fileId != m_slots[slotId].fileId)
                {
                    (void)loadIcon(slotId, fileId);
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
