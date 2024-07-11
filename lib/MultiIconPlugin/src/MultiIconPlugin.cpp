/* MIT License
 *
 * Copyright (c) 2019 - 2024 Andreas Merkle <web@blue-andi.de>
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

#include <FileSystem.h>
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

/* Initialize bitmap control topic. */
const char* MultiIconPlugin::TOPIC_BITMAP   = "/bitmap";

/* Initialize slot control topic. */
const char* MultiIconPlugin::TOPIC_SLOT     = "/slot";

/* Initialize slots topic. */
const char* MultiIconPlugin::TOPIC_SLOTS    = "/slots";

/* Initialize file extension for temporary files. */
const char* MultiIconPlugin::FILE_EXT_TMP   = ".tmp";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void MultiIconPlugin::getTopics(JsonArray& topics) const
{
    uint8_t slotId = 0U;
    uint8_t iconId = 0U;

    for(slotId = 0U; slotId < _MultiIconPlugin::View::MAX_ICON_SLOTS; ++slotId)
    {
        JsonObject jsonAnimation = topics.createNestedObject();

        jsonAnimation["name"] = String(TOPIC_SLOT) + "/" + slotId;

        for(iconId = 0U; iconId < MAX_ICONS_PER_SLOT; ++iconId)
        {
            JsonObject jsonIcon = topics.createNestedObject();

            jsonIcon["name"]    = String(TOPIC_BITMAP) + "/" + slotId + "/" + iconId;
            jsonIcon["access"]  = "w"; /* Only icon upload is supported. */
        }
    }

    topics.add(TOPIC_SLOTS);
}

bool MultiIconPlugin::getTopic(const String& topic, JsonObject& value) const
{
    bool isSuccessful = false;

    if (0U != topic.startsWith(String(TOPIC_SLOT) + "/"))
    {
        uint8_t slotId  = _MultiIconPlugin::View::MAX_ICON_SLOTS;
        bool    status  = getSlotIdFromTopic(slotId, topic);

        if ((true == status) &&
            (_MultiIconPlugin::View::MAX_ICON_SLOTS > slotId))
        {
            uint8_t     iconIdx;
            uint8_t     activeIconId    = getActiveIconId(slotId);
            JsonArray   jsonIcons       = value.createNestedArray("icons");

            value["slotId"]         = slotId;
            value["activeIconId"]   = activeIconId;

            for (iconIdx = 0U; iconIdx < MAX_ICONS_PER_SLOT; ++iconIdx)
            {
                String      fullPath;
                JsonObject  jsonIcon    = jsonIcons.createNestedObject();

                (void)getIconFilePath(slotId, iconIdx, fullPath);

                jsonIcon["iconId"]      = iconIdx;
                jsonIcon["fullPath"]    = fullPath;
            }

            isSuccessful = true;
        }
    }
    else if (0U != topic.equals(TOPIC_SLOTS))
    {
        value["slots"] = _MultiIconPlugin::View::MAX_ICON_SLOTS;

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

    if (0U != topic.startsWith(String(TOPIC_BITMAP) + "/"))
    {
        uint8_t slotId  = _MultiIconPlugin::View::MAX_ICON_SLOTS;
        uint8_t iconId  = MAX_ICONS_PER_SLOT;
        bool    status  = getSlotIdAndIconIdFromTopic(slotId, iconId, topic);

        if ((true == status) &&
            (_MultiIconPlugin::View::MAX_ICON_SLOTS > slotId) &&
            (MAX_ICONS_PER_SLOT > iconId))
        {
            JsonVariantConst jsonIconPath = value["fullPath"];

            /* File upload? */
            if (false == jsonIconPath.isNull())
            {
                String iconFullPath = jsonIconPath.as<String>();

                /* Clear always the icon indpended whether its requested by user.
                 * In case of an uploaded new icon, clearing will close the image
                 * file and makes it possible to overwrite the file.
                 */
                clearIcon(slotId, iconId);

                if (false == iconFullPath.isEmpty())
                {
                    /* Rename uploaded icon by removing the file extension for temporary files. */
                    String iconFullPathWithoutTmp = iconFullPath.substring(0, iconFullPath.length() - strlen(FILE_EXT_TMP));

                    FILESYSTEM.rename(iconFullPath, iconFullPathWithoutTmp);

                    isSuccessful = setIconFilePath(slotId, iconId, iconFullPathWithoutTmp);
                }
                else
                {
                    isSuccessful = true;
                }
            }
        }
    }
    else if (0U != topic.startsWith(String(TOPIC_SLOT) + "/"))
    {
        uint8_t slotId  = _MultiIconPlugin::View::MAX_ICON_SLOTS;
        bool    status  = getSlotIdFromTopic(slotId, topic);

        if ((true == status) &&
            (_MultiIconPlugin::View::MAX_ICON_SLOTS > slotId))
        {
            JsonVariantConst    jsonActiveIconId    = value["activeIconId"];
            JsonVariantConst    jsonIcons           = value["icons"];

            if (false == jsonActiveIconId.isNull())
            {
                uint8_t activeIconId = jsonActiveIconId.as<uint8_t>();

                if (true == setActiveIconId(slotId, activeIconId))
                {
                    isSuccessful = true;
                }
            }

            if ((false == jsonIcons.isNull()) &&
                (true == jsonIcons.is<JsonArrayConst>()))
            {
                uint8_t iconIdx;

                for (iconIdx = 0U; iconIdx < MAX_ICONS_PER_SLOT; ++iconIdx)
                {
                    String              fullPath;
                    JsonVariantConst    jsonObject  = jsonIcons[iconIdx];

                    if (false == jsonObject.isNull())
                    {
                        JsonVariantConst    jsonIconId      = jsonObject["iconId"];
                        JsonVariantConst    jsonFullPath    = jsonObject["fullPath"];

                        if ((false == jsonIconId.isNull()) &&
                            (false == jsonFullPath.isNull()))
                        {
                            uint8_t iconId      = jsonIconId.as<uint8_t>();
                            String  fullPath    = jsonFullPath.as<String>();

                            isSuccessful = setIconFilePath(slotId, iconId, fullPath);
                        }
                    }
                }
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

    if (0U != topic.startsWith(String(TOPIC_SLOT) + "/"))
    {
        uint8_t slotId  = MAX_ICONS_PER_SLOT;
        bool    status  = getSlotIdFromTopic(slotId, topic);

        if ((true == status) &&
            (_MultiIconPlugin::View::MAX_ICON_SLOTS > slotId))
        {
            MutexGuard<MutexRecursive> guard(m_mutex);

            hasTopicChanged                 = m_slots[slotId].hasSlotChanged;
            m_slots[slotId].hasSlotChanged  = false;
        }
    }

    return hasTopicChanged;
}

bool MultiIconPlugin::isUploadAccepted(const String& topic, const String& srcFilename, String& dstFilename)
{
    bool isAccepted = false;

    if (0U != topic.startsWith(String(TOPIC_BITMAP) + "/"))
    {
        uint8_t slotId  = _MultiIconPlugin::View::MAX_ICON_SLOTS;
        uint8_t iconId  = MAX_ICONS_PER_SLOT;
        bool    status  = getSlotIdAndIconIdFromTopic(slotId, iconId, topic);

        if ((true == status) &&
            (_MultiIconPlugin::View::MAX_ICON_SLOTS > slotId) &&
            (MAX_ICONS_PER_SLOT > iconId))
        {
            if (0U != srcFilename.endsWith(BitmapWidget::FILE_EXT_BITMAP))
            {
                dstFilename = getFileName(slotId, iconId, BitmapWidget::FILE_EXT_BITMAP);
                isAccepted  = true;
            }
            else if (0U != srcFilename.endsWith(BitmapWidget::FILE_EXT_GIF))
            {
                dstFilename = getFileName(slotId, iconId, BitmapWidget::FILE_EXT_GIF);
                isAccepted  = true;
            }
            else
            {
                ;
            }

            if (true == isAccepted)
            {
                /* If a GIF image is loaded, the file is kept open and can not be overwritten.
                * Therefore store it first with the additional extension for temporary files.
                * It will be renamed then in the setTopic() method if upload is successful.
                */
                dstFilename += FILE_EXT_TMP;
            }
        }
    }

    return isAccepted;
}

void MultiIconPlugin::start(uint16_t width, uint16_t height)
{
    uint8_t                     slotId      = 0U;
    uint8_t                     iconId      = 0U;
    MutexGuard<MutexRecursive>  guard(m_mutex);

    m_view.init(width, height);

    /* Scan the filesystem for the icon images. */
    for(slotId = 0U; slotId < _MultiIconPlugin::View::MAX_ICON_SLOTS; ++slotId)
    { 
        IconSlot&   iconSlot = m_slots[slotId];

        for(iconId = 0U; iconId < MAX_ICONS_PER_SLOT; ++iconId)
        {
            String fullPath = getFileName(slotId, iconId, BitmapWidget::FILE_EXT_GIF);

            if (true == FILESYSTEM.exists(fullPath))
            {
                iconSlot.icons[iconId]  = fullPath;
                iconSlot.hasSlotChanged = true;

                LOG_INFO("Found %s.", fullPath.c_str());
            }
            else
            {
                fullPath = getFileName(slotId, iconId, BitmapWidget::FILE_EXT_BITMAP);

                if (true == FILESYSTEM.exists(fullPath))
                {
                    iconSlot.icons[iconId]  = fullPath;
                    iconSlot.hasSlotChanged = true;

                    LOG_INFO("Found %s.", fullPath.c_str());
                }
                else
                {
                    iconSlot.icons[iconId].clear();
                }
            }
        }

        if (false == iconSlot.icons[iconSlot.activeIconId].isEmpty())
        {
            (void)m_view.loadIcon(slotId, iconSlot.icons[iconSlot.activeIconId]);
        }
    }
}

void MultiIconPlugin::stop()
{
    uint8_t                     slotId      = 0U;
    uint8_t                     iconId      = 0U;
    MutexGuard<MutexRecursive>  guard(m_mutex);

    /* Scan the filesystem for the icon images. */
    for(slotId = 0U; slotId < _MultiIconPlugin::View::MAX_ICON_SLOTS; ++slotId)
    { 
        IconSlot&   iconSlot = m_slots[slotId];

        for(iconId = 0U; iconId < MAX_ICONS_PER_SLOT; ++iconId)
        {
            String fullPathGif = getFileName(slotId, iconId, BitmapWidget::FILE_EXT_GIF);
            String fullPathBmp = getFileName(slotId, iconId, BitmapWidget::FILE_EXT_BITMAP);

            if (false != FILESYSTEM.remove(fullPathGif))
            {
                LOG_INFO("File %s removed", fullPathGif.c_str());
            }

            if (false != FILESYSTEM.remove(fullPathBmp))
            {
                LOG_INFO("File %s removed", fullPathGif.c_str());
            }
        }
    }
}

void MultiIconPlugin::update(YAGfx& gfx)
{
    MutexGuard<MutexRecursive>  guard(m_mutex);

    m_view.update(gfx);
}

uint8_t MultiIconPlugin::getActiveIconId(uint8_t slotId) const
{
    uint8_t                     activeIconId    = 0U;
    MutexGuard<MutexRecursive>  guard(m_mutex);

    if (_MultiIconPlugin::View::MAX_ICON_SLOTS > slotId)
    {
        activeIconId = m_slots[slotId].activeIconId;
    }
    
    return activeIconId;
}

bool MultiIconPlugin::setActiveIconId(uint8_t slotId, uint8_t iconId)
{
    bool                        isSuccessful    = false;
    MutexGuard<MutexRecursive>  guard(m_mutex);

    if ((_MultiIconPlugin::View::MAX_ICON_SLOTS > slotId) &&
        (MAX_ICONS_PER_SLOT > iconId))
    {
        IconSlot& iconSlot = m_slots[slotId];

        if (iconSlot.activeIconId != iconId)
        {
            iconSlot.activeIconId   = iconId;
            iconSlot.hasSlotChanged = true;

            if (true == iconSlot.icons[iconId].isEmpty())
            {
                m_view.clearIcon(slotId);
                isSuccessful = true;
            }
            else if (true == m_view.loadIcon(iconId, iconSlot.icons[iconId]))
            {
                isSuccessful = true;
            }
        }
        else
        {
            isSuccessful = true;
        }
    }

    return isSuccessful;
}

void MultiIconPlugin::clearIcon(uint8_t slotId, uint8_t iconId)
{
    if ((_MultiIconPlugin::View::MAX_ICON_SLOTS > slotId) &&
        (MAX_ICONS_PER_SLOT > iconId))
    {
        MutexGuard<MutexRecursive>  guard(m_mutex);
        IconSlot&                   iconSlot = m_slots[slotId];

        if (false == iconSlot.icons[iconId].isEmpty())
        {
            if (iconSlot.activeIconId == iconId)
            {
                /* Clear icon first in the view (will close file). */
                m_view.clearIcon(slotId);
            }

            /* Remove icon from filesystem. */
            (void)FILESYSTEM.remove(iconSlot.icons[iconId]);

            /* Clear the path to the icon. */
            iconSlot.icons[iconId].clear();
            iconSlot.hasSlotChanged = true;
        }
    }
}

bool MultiIconPlugin::getIconFilePath(uint8_t slotId, uint8_t iconId, String& fullPath) const
{
    bool isSuccessful = false;

    if ((_MultiIconPlugin::View::MAX_ICON_SLOTS > slotId) &&
        (MAX_ICONS_PER_SLOT > iconId))
    {
        MutexGuard<MutexRecursive> guard(m_mutex);

        fullPath = m_slots[slotId].icons[iconId];
    }

    return isSuccessful;
}

bool MultiIconPlugin::setIconFilePath(uint8_t slotId, uint8_t iconId, const String& fullPath)
{
    bool isSuccessful = false;

    if ((_MultiIconPlugin::View::MAX_ICON_SLOTS > slotId) &&
        (MAX_ICONS_PER_SLOT > iconId))
    {
        MutexGuard<MutexRecursive>  guard(m_mutex);
        IconSlot&                   iconSlot = m_slots[slotId];

        if (iconSlot.icons[iconId] != fullPath)
        {
            iconSlot.icons[iconId]  = fullPath;
            iconSlot.hasSlotChanged = true;
        }

        /* If the active icon path changed or not, always reload the icon.
         * Because the user might only uploaded a new icon with the same path,
         * as the previous icon.
         */
        if (iconSlot.activeIconId == iconId)
        {
            /* If a empty icon file path is set, the icon will be cleared. */
            if (true == iconSlot.icons[iconId].isEmpty())
            {
                m_view.clearIcon(slotId);
                isSuccessful = true;
            }
            /* If a icon file path is set, the icon will be loaded. */
            else if (true == m_view.loadIcon(slotId, iconSlot.icons[iconId]))
            {
                isSuccessful = true;
            }
            /* Failed to load the icon. */
            else
            {
                ;
            }
        }
        else
        {
            isSuccessful = true;
        }
    }

    return isSuccessful;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

String MultiIconPlugin::getFileName(uint8_t slotId, uint8_t iconId, const String& ext) const
{
    return generateFullPath(getUID(), "_" + String(slotId) + "_" + String(iconId) + ext);
}

bool MultiIconPlugin::getSlotIdFromTopic(uint8_t& slotId, const String& topic) const
{
    bool    isSuccessful        = false;
    int32_t indexBeginSlotId    = topic.lastIndexOf("/");

    if (0 <= indexBeginSlotId)
    {
        String  slotIdStr       = topic.substring(indexBeginSlotId + 1);
        uint8_t slotIdTmp;
        bool    statusSlotId    = Util::strToUInt8(slotIdStr, slotIdTmp);

        if (true == statusSlotId)
        {
            slotId          = slotIdTmp;
            isSuccessful    = true;
        }
    }

    return isSuccessful;
}

bool MultiIconPlugin::getSlotIdAndIconIdFromTopic(uint8_t& slotId, uint8_t& iconId, const String& topic) const
{
    bool    isSuccessful        = false;
    int32_t indexBeginIconId    = topic.lastIndexOf("/");

    if (0 <= indexBeginIconId)
    {
        int32_t indexBeginSlotId = topic.lastIndexOf("/", indexBeginIconId - 1);

        if (0 <= indexBeginSlotId)
        {
            String  iconIdStr       = topic.substring(indexBeginIconId + 1);
            String  slotIdStr       = topic.substring(indexBeginSlotId + 1, indexBeginIconId);
            uint8_t iconIdTmp;
            uint8_t slotIdTmp;
            bool    statusIconId    = Util::strToUInt8(iconIdStr, iconIdTmp);
            bool    statusSlotId    = Util::strToUInt8(slotIdStr, slotIdTmp);

            if ((true == statusIconId) &&
                (true == statusSlotId))
            {
                slotId          = slotIdTmp;
                iconId          = iconIdTmp;
                isSuccessful    = true;
            }
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
