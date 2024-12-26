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
 * @brief  Icon and text plugin
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "IconTextPlugin.h"

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
const char* IconTextPlugin::TOPIC_TEXT = "/iconText";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

bool IconTextPlugin::isEnabled() const
{
    bool isEnabled = false;

    /* The plugin shall only be scheduled if its enabled and text is set. */
    if ((true == m_isEnabled) &&
        (false == m_view.getText().isEmpty()))
    {
        isEnabled = true;
    }

    return isEnabled;
}

void IconTextPlugin::getTopics(JsonArray& topics) const
{
    JsonObject  jsonText    = topics.createNestedObject();

    jsonText["name"]    = TOPIC_TEXT;

    /* Home Assistant support of MQTT discovery (https://www.home-assistant.io/integrations/mqtt) */
    jsonText["ha"]["component"]             = "text";                           /* MQTT integration */
    jsonText["ha"]["discovery"]["name"]     = "MQTT text";                      /* Application that is the origin the discovered MQTT. */
    jsonText["ha"]["discovery"]["cmd_tpl"]  = "{\"text\": \"{{ value }}\" }";   /* Command template */
    jsonText["ha"]["discovery"]["val_tpl"]  = "{{ value_json.text }}";          /* Value template */
    jsonText["ha"]["discovery"]["ic"]       = "mdi:form-textbox";               /* Icon (MaterialDesignIcons.com) */
}

bool IconTextPlugin::getTopic(const String& topic, JsonObject& value) const
{
    bool isSuccessful = false;

    if (true == topic.equals(TOPIC_TEXT))
    {
        getActualConfiguration(value);
        isSuccessful = true;
    }

    return isSuccessful;
}

bool IconTextPlugin::setTopic(const String& topic, const JsonObjectConst& value)
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

    return isSuccessful;
}

bool IconTextPlugin::hasTopicChanged(const String& topic)
{
    MutexGuard<MutexRecursive>  guard(m_mutex);
    bool                        hasTopicChanged = m_hasTopicChanged;

    /* Only a single topic, therefore its not necessary to check. */
    PLUGIN_NOT_USED(topic);

    m_hasTopicChanged = false;

    return hasTopicChanged;
}

void IconTextPlugin::start(uint16_t width, uint16_t height)
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

void IconTextPlugin::stop()
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    PluginWithConfig::stop();
}

void IconTextPlugin::update(YAGfx& gfx)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_view.update(gfx);
}

String IconTextPlugin::getText() const
{
    MutexGuard<MutexRecursive>  guard(m_mutex);
    String                      formattedText   = m_view.getFormatText();

    return formattedText;
}

void IconTextPlugin::setText(const String& formatText, bool storeFlag)
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

        m_hasTopicChanged = true;
    }
}

bool IconTextPlugin::loadIcon(FileMgrService::FileId fileId, bool storeFlag)
{
    bool                        isSuccessful    = false;
    String                      iconFullPath;
    MutexGuard<MutexRecursive>  guard(m_mutex);

    if (m_iconFileId != fileId)
    {
        m_iconFileId        = fileId;
        m_hasTopicChanged   = true;

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

void IconTextPlugin::clearIcon(bool storeFlag)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    if (FileMgrService::FILE_ID_INVALID != m_iconFileId)
    {
        /* Clear icon first in the view (will close file). */
        m_view.clearIcon();

        m_iconFileId        = FileMgrService::FILE_ID_INVALID;
        m_hasTopicChanged   = true;

        if (true == storeFlag)
        {
            m_iconFileIdStored = m_iconFileId;
            requestStoreToPersistentMemory();
        }
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void IconTextPlugin::getActualConfiguration(JsonObject& jsonCfg) const
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    jsonCfg["iconFileId"]   = m_iconFileId;
    jsonCfg["text"]         = m_view.getFormatText();
}

bool IconTextPlugin::setActualConfiguration(const JsonObjectConst& jsonCfg)
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

            m_hasTopicChanged = true;
        }

        if (m_view.getFormatText() != newFormatText)
        {
            m_view.setFormatText(newFormatText);

            m_hasTopicChanged = true;
        }

        status = true;
    }

    return status;
}

void IconTextPlugin::getConfiguration(JsonObject& jsonCfg) const
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    jsonCfg["iconFileId"]   = m_iconFileIdStored;
    jsonCfg["text"]         = m_formatTextStored;
}

bool IconTextPlugin::setConfiguration(const JsonObjectConst& jsonCfg)
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
