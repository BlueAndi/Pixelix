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
 * @brief  JustText plugin
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "JustTextPlugin.h"

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
const char* JustTextPlugin::TOPIC_TEXT                    = "text";
const char* JustTextPlugin::TOPIC_TEXT_EXTRA_HA_FILE_NAME = "/extra/justTextPlugin.json";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

bool JustTextPlugin::isEnabled() const
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

void JustTextPlugin::getTopics(JsonArray& topics) const
{
    JsonObject jsonText     = topics.createNestedObject();

    /* The topic contains Home Assistant support of the MQTT discovery
     * (https://www.home-assistant.io/integrations/mqtt). See the configured
     * JSON file.
     *
     * The used icon is from MaterialDesignIcons.com (namespace: mdi).
     */
    jsonText["name"]        = TOPIC_TEXT;
    jsonText["extra"]["ha"] = TOPIC_TEXT_EXTRA_HA_FILE_NAME;
}

bool JustTextPlugin::getTopic(const String& topic, JsonObject& value) const
{
    bool isSuccessful = false;

    if (true == topic.equals(TOPIC_TEXT))
    {
        getActualConfiguration(value);
        isSuccessful = true;
    }

    return isSuccessful;
}

bool JustTextPlugin::setTopic(const String& topic, const JsonObjectConst& value)
{
    bool isSuccessful = false;

    if (true == topic.equals(TOPIC_TEXT))
    {
        bool                storeFlag     = false;
        const size_t        JSON_DOC_SIZE = 512U;
        DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
        JsonObject          jsonCfg       = jsonDoc.to<JsonObject>();
        JsonVariantConst    jsonText      = value["text"];
        JsonVariantConst    jsonStoreFlag = value["storeFlag"];

        /* The received configuration may not contain all single key/value pair.
         * Therefore read first the complete internal configuration and
         * overwrite them with the received ones.
         */
        getActualConfiguration(jsonCfg);

        /* Note:
         * Check only for the key/value pair availability.
         * The type check will follow in the setConfiguration().
         */

        if (false == jsonText.isNull())
        {
            jsonCfg["text"] = jsonText.as<String>();
            isSuccessful    = true;
        }

        /* Note: The store flag is not part of the stored configuration, its just
         * used by the user to force that the text is stored persistent. By default
         * text is not stored to avoid too many flash write cycles.
         */
        if (false == jsonStoreFlag.isNull())
        {
            if (true == jsonStoreFlag.is<String>())
            {
                storeFlag    = jsonStoreFlag.as<String>().equalsIgnoreCase("true");
                isSuccessful = true;
            }
            else if (true == jsonStoreFlag.is<bool>())
            {
                storeFlag    = jsonStoreFlag.as<bool>();
                isSuccessful = true;
            }
            else
            {
                ;
            }
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

bool JustTextPlugin::hasTopicChanged(const String& topic)
{
    MutexGuard<MutexRecursive> guard(m_mutex);
    bool                       hasTopicChanged = m_hasTopicChanged;

    /* Only a single topic, therefore its not necessary to check. */
    PLUGIN_NOT_USED(topic);

    m_hasTopicChanged = false;

    return hasTopicChanged;
}

void JustTextPlugin::start(uint16_t width, uint16_t height)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_view.init(width, height);

    PluginWithConfig::start(width, height);

    m_view.setFormatText(m_formatTextStored);
}

void JustTextPlugin::stop()
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    PluginWithConfig::stop();
}

void JustTextPlugin::update(YAGfx& gfx)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_view.update(gfx);
}

String JustTextPlugin::getText() const
{
    MutexGuard<MutexRecursive> guard(m_mutex);
    String                     formattedText = m_view.getFormatText();

    return formattedText;
}

void JustTextPlugin::setText(const String& formatText, bool storeFlag)
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

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void JustTextPlugin::getActualConfiguration(JsonObject& jsonCfg) const
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    jsonCfg["text"] = m_view.getFormatText();
}

bool JustTextPlugin::setActualConfiguration(const JsonObjectConst& jsonCfg)
{
    bool             status   = false;
    JsonVariantConst jsonText = jsonCfg["text"];

    if (false == jsonText.is<String>())
    {
        LOG_WARNING("JSON text not found or invalid type.");
    }
    else
    {
        MutexGuard<MutexRecursive> guard(m_mutex);
        String                     newFormatText = jsonText.as<String>();

        if (m_view.getFormatText() != newFormatText)
        {
            m_view.setFormatText(newFormatText);

            m_hasTopicChanged = true;
        }

        status = true;
    }

    return status;
}

void JustTextPlugin::getConfiguration(JsonObject& jsonCfg) const
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    jsonCfg["text"] = m_formatTextStored;
}

bool JustTextPlugin::setConfiguration(const JsonObjectConst& jsonCfg)
{
    bool status = setActualConfiguration(jsonCfg);

    if (true == status)
    {
        m_formatTextStored = m_view.getFormatText();
    }

    return status;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
