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
 * @brief  Icon, text and lamp plugin
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "IconTextLampPlugin.h"

#include <FileSystem.h>
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

/* Initialize plugin topic. */
const char* IconTextLampPlugin::TOPIC_TEXT      = "/iconText";

/* Initialize plugin topic. */
const char* IconTextLampPlugin::TOPIC_LAMPS     = "/lamps";

/* Initialize plugin topic. */
const char* IconTextLampPlugin::TOPIC_LAMP      = "/lamp";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void IconTextLampPlugin::getTopics(JsonArray& topics) const
{
    uint8_t     lampId      = 0U;
    JsonObject  jsonText    = topics.createNestedObject();
    JsonObject  jsonLamps   = topics.createNestedObject();

    jsonText["name"]    = TOPIC_TEXT;

    /* Home Assistant support of MQTT discovery (https://www.home-assistant.io/integrations/mqtt) */
    jsonText["ha"]["component"]             = "text";                           /* MQTT integration */
    jsonText["ha"]["discovery"]["name"]     = "MQTT text";                      /* Application that is the origin the discovered MQTT. */
    jsonText["ha"]["discovery"]["cmd_tpl"]  = "{\"text\": \"{{ value }}\" }";   /* Command template */
    jsonText["ha"]["discovery"]["val_tpl"]  = "{{ value_json.text }}";          /* Value template */
    jsonText["ha"]["discovery"]["ic"]       = "mdi:form-textbox";               /* Icon (MaterialDesignIcons.com) */

    jsonLamps["name"]   = TOPIC_LAMPS;
    jsonLamps["access"] = "r"; /* Only read access allowed. */

    for(lampId = 0U; lampId < _IconTextLampPlugin::View::MAX_LAMPS; ++lampId)
    {
        (void)topics.add(String(TOPIC_LAMP) + "/" + lampId);
    }
}

bool IconTextLampPlugin::getTopic(const String& topic, JsonObject& value) const
{
    bool isSuccessful = false;

    if (true == topic.equals(TOPIC_TEXT))
    {
        getActualConfiguration(value);
        isSuccessful = true;
    }
    else if (true == topic.equals(TOPIC_LAMPS))
    {
        JsonArray   lampArray   = value.createNestedArray("lamps");
        uint8_t     lampId      = 0U;

        for(lampId = 0U; lampId < _IconTextLampPlugin::View::MAX_LAMPS; ++lampId)
        {
            bool        lampOnState = getLamp(lampId);
            JsonObject  lampObj     = lampArray.createNestedObject();

            lampObj["id"]       = lampId;
            lampObj["state"]    = (false == lampOnState) ? String("off") : String("on");
        }

        isSuccessful = true;
    }
    else if (true == topic.startsWith(String(TOPIC_LAMP) + "/"))
    {
        uint32_t    indexBeginLampId    = topic.lastIndexOf("/") + 1U;
        String      lampIdStr           = topic.substring(indexBeginLampId);
        uint8_t     lampId              = _IconTextLampPlugin::View::MAX_LAMPS;
        bool        status              = Util::strToUInt8(lampIdStr, lampId);

        if ((true == status) &&
            (_IconTextLampPlugin::View::MAX_LAMPS > lampId))
        {
            bool    lampState       = getLamp(lampId);
            String  lampStateStr    = (false == lampState) ? "off" : "on";

            value["id"]     = lampId;
            value["state"]  = lampStateStr;

            isSuccessful = true;
        }
    }
    else
    {
        ;
    }

    return isSuccessful;
}

bool IconTextLampPlugin::setTopic(const String& topic, const JsonObjectConst& value)
{
    bool isSuccessful = false;

    if (true == topic.equals(TOPIC_TEXT))
    {
        bool                storeFlag               = false;
        const size_t        JSON_DOC_SIZE           = 512U;
        DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
        JsonObject          jsonCfg                 = jsonDoc.to<JsonObject>();
        JsonVariantConst    jsonIconFileId          = value["iconFileId"];
        JsonVariantConst    jsonText                = value["text"];
        JsonVariantConst    jsonStoreFlag           = value["storeFlag"];
    
        /* The received configuration may not contain all single key/value pair.
         * Therefore read first the complete internal configuration and
         * overwrite them with the received ones.
         */
        getActualConfiguration(jsonCfg);

        /* Note:
         * Check only for the key/value pair availability.
         * The type check will follow in the setConfiguration().
         */

        if (false == jsonIconFileId.isNull())
        {
            jsonCfg["iconFileId"] = jsonIconFileId.as<FileMgrService::FileId>();
            isSuccessful = true;
        }
        
        if (false == jsonText.isNull())
        {
            jsonCfg["text"] = jsonText.as<String>();
            isSuccessful = true;
        }

        /* Note: The store flag is not part of the stored configuration, its just
         * used by the user to force that the text is stored persistent. By default
         * text is not stored to avoid too many flash write cycles.
         */
        if (false == jsonStoreFlag.isNull())
        {
            storeFlag = jsonStoreFlag.as<bool>();
            isSuccessful = true;
        }

        if (true == isSuccessful)
        {
            JsonObjectConst jsonCfgConst = jsonCfg;

            if (false == storeFlag)
            {
                isSuccessful = setActualConfiguration(jsonCfgConst);
            }
            else
            {
                isSuccessful = setConfiguration(jsonCfgConst);
                requestStoreToPersistentMemory();
            }
        }
    }
    else if (true == topic.startsWith(String(TOPIC_LAMP) + "/"))
    {
        uint32_t            indexBeginLampId    = topic.lastIndexOf("/") + 1U;
        String              lampIdStr           = topic.substring(indexBeginLampId);
        uint8_t             lampId              = _IconTextLampPlugin::View::MAX_LAMPS;
        bool                status              = Util::strToUInt8(lampIdStr, lampId);
        JsonVariantConst    jsonSet             = value["state"];

        if ((true == status) &&
            (_IconTextLampPlugin::View::MAX_LAMPS > lampId) &&
            (false == jsonSet.isNull()))
        {
            String state = jsonSet.as<String>();

            if (state == "off")
            {
                setLamp(lampId, false);
                isSuccessful = true;
            }
            else if (state == "on")
            {
                setLamp(lampId, true);
                isSuccessful = true;
            }
            else
            {
                ;
            }
        }
    }
    else
    {
        ;
    }

    return isSuccessful;
}

bool IconTextLampPlugin::hasTopicChanged(const String& topic)
{
    bool hasTopicChanged = false;

    if (true == topic.equals(TOPIC_TEXT))
    {
        MutexGuard<MutexRecursive> guard(m_mutex);

        hasTopicChanged = m_hasTopicTextChanged;
        m_hasTopicTextChanged = false;
    }
    else if (true == topic.equals(TOPIC_LAMPS))
    {
        MutexGuard<MutexRecursive> guard(m_mutex);

        hasTopicChanged = m_hasTopicLampsChanged;
        m_hasTopicLampsChanged = false;
    }
    else if (true == topic.startsWith(String(TOPIC_LAMP) + "/"))
    {
        uint32_t    indexBeginLampId    = topic.lastIndexOf("/") + 1U;
        String      lampIdStr           = topic.substring(indexBeginLampId);
        uint8_t     lampId              = _IconTextLampPlugin::View::MAX_LAMPS;
        bool        status              = Util::strToUInt8(lampIdStr, lampId);

        if ((true == status) &&
            (_IconTextLampPlugin::View::MAX_LAMPS > lampId))
        {
            MutexGuard<MutexRecursive> guard(m_mutex);
            
            hasTopicChanged = m_hasTopicLampChanged[lampId];
            m_hasTopicLampChanged[lampId] = false;
        }
    }
    else
    {
        ;
    }

    return hasTopicChanged;
}

void IconTextLampPlugin::start(uint16_t width, uint16_t height)
{
    String                      iconFullPath;
    MutexGuard<MutexRecursive>  guard(m_mutex);

    m_view.init(width, height);

    PluginWithConfig::start(width, height);

    m_view.setFormatText(m_formatTextStored);

    if (FileMgrService::FILE_ID_INVALID != m_iconFileId)
    {
        if (false == FileMgrService::getInstance().getFileFullPathById(iconFullPath, m_iconFileId))
        {
            LOG_WARNING("Unknown file id %u.", m_iconFileId);
        }
        else if (false == m_view.loadIcon(iconFullPath))
        {
            LOG_ERROR("Icon not found: %s", iconFullPath.c_str());
        }
        else
        {
            ;
        }
    }
}

void IconTextLampPlugin::stop()
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    PluginWithConfig::stop();
}

void IconTextLampPlugin::update(YAGfx& gfx)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_view.update(gfx);
}

String IconTextLampPlugin::getText() const
{
    MutexGuard<MutexRecursive>  guard(m_mutex);
    String                      formattedText   = m_view.getFormatText();

    return formattedText;
}

void IconTextLampPlugin::setText(const String& formatText, bool storeFlag)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    if (m_view.getFormatText() != formatText)
    {
        m_view.setFormatText(formatText);

        if (true == storeFlag)
        {
            m_formatTextStored = formatText;
            requestStoreToPersistentMemory();
        }

        m_hasTopicTextChanged = true;
    }
}

bool IconTextLampPlugin::loadIcon(FileMgrService::FileId fileId, bool storeFlag)
{
    bool                        isSuccessful    = false;
    String                      iconFullPath;
    MutexGuard<MutexRecursive>  guard(m_mutex);

    if (m_iconFileId != fileId)
    {
        m_iconFileId            = fileId;
        m_hasTopicTextChanged   = true;

        if (true == storeFlag)
        {
            m_iconFileIdStored = m_iconFileId;
            requestStoreToPersistentMemory();
        }
    }

    if (FileMgrService::FILE_ID_INVALID == m_iconFileId)
    {
        m_view.clearIcon();
    }
    else if (false == FileMgrService::getInstance().getFileFullPathById(iconFullPath, m_iconFileId))
    {
        LOG_WARNING("Unknown file id %u.", m_iconFileId);
        m_view.clearIcon();
    }
    else
    {
        /* Load the icon always again, as the path might be the same, but
         * the icon file changed.
         */
        isSuccessful = m_view.loadIcon(iconFullPath);
    }
    
    return isSuccessful;
}

void IconTextLampPlugin::clearIcon(bool storeFlag)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    if (FileMgrService::FILE_ID_INVALID != m_iconFileId)
    {
        /* Clear icon first in the view (will close file). */
        m_view.clearIcon();

        m_iconFileId            = FileMgrService::FILE_ID_INVALID;
        m_hasTopicTextChanged   = true;

        if (true == storeFlag)
        {
            m_iconFileIdStored = m_iconFileId;
            requestStoreToPersistentMemory();
        }
    }
}

bool IconTextLampPlugin::getLamp(uint8_t lampId) const
{
    MutexGuard<MutexRecursive>  guard(m_mutex);
    bool                        lampState = m_view.getLamp(lampId);

    return lampState;
}

void IconTextLampPlugin::setLamp(uint8_t lampId, bool state)
{
    if (_IconTextLampPlugin::View::MAX_LAMPS > lampId)
    {
        MutexGuard<MutexRecursive> guard(m_mutex);

        if (state != m_view.getLamp(lampId))
        {
            m_view.setLamp(lampId, state);

            m_hasTopicLampsChanged = true;
            m_hasTopicLampChanged[lampId] = true;
        }
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void IconTextLampPlugin::getActualConfiguration(JsonObject& jsonCfg) const
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    jsonCfg["iconFileId"]   = m_iconFileId;
    jsonCfg["text"]         = m_view.getFormatText();
}

bool IconTextLampPlugin::setActualConfiguration(const JsonObjectConst& jsonCfg)
{
    bool             status         = false;
    JsonVariantConst jsonIconFileId = jsonCfg["iconFileId"];
    JsonVariantConst jsonText       = jsonCfg["text"];

    if (false == jsonIconFileId.is<FileMgrService::FileId>())
    {
        LOG_WARNING("JSON icon file id not found or invalid type.");
    }
    else if (false == jsonText.is<String>())
    {
        LOG_WARNING("JSON text not found or invalid type.");
    }
    else
    {
        MutexGuard<MutexRecursive>  guard(m_mutex);
        FileMgrService::FileId      newIconFileId   = jsonIconFileId.as<FileMgrService::FileId>();
        String                      newFormatText   = jsonText.as<String>();

        if (m_iconFileId != newIconFileId)
        {
            m_iconFileId = newIconFileId;

            if (FileMgrService::FILE_ID_INVALID == m_iconFileId)
            {
                m_view.clearIcon();
            }
            else
            {
                String iconFullPath;

                if (false == FileMgrService::getInstance().getFileFullPathById(iconFullPath, m_iconFileId))
                {
                    LOG_WARNING("Unknown file id %u.", m_iconFileId);
                    m_view.clearIcon();
                }
                else
                {
                    if (false == m_view.loadIcon(iconFullPath))
                    {
                        LOG_WARNING("Couldn't load icon: %s", iconFullPath.c_str());
                    }
                }
            }

            m_hasTopicTextChanged = true;
        }

        if (m_view.getFormatText() != newFormatText)
        {
            m_view.setFormatText(newFormatText);

            m_hasTopicTextChanged = true;
        }

        status = true;
    }

    return status;
}

void IconTextLampPlugin::getConfiguration(JsonObject& jsonCfg) const
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    jsonCfg["iconFileId"]   = m_iconFileIdStored;
    jsonCfg["text"]         = m_formatTextStored;
}

bool IconTextLampPlugin::setConfiguration(const JsonObjectConst& jsonCfg)
{
    bool status = setActualConfiguration(jsonCfg);

    if (true == status)
    {
        m_iconFileIdStored  = m_iconFileId;
        m_formatTextStored  = m_view.getFormatText();
    }

    return status;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
