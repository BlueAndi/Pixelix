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
 * @brief  Grab information via REST API plugin
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "GrabViaMqttPlugin.h"

#include <Logging.h>
#include <ArduinoJson.h>
#include <MqttService.h>

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
const char* GrabViaMqttPlugin::TOPIC_CONFIG = "/grabConfig";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void GrabViaMqttPlugin::getTopics(JsonArray& topics) const
{
    (void)topics.add(TOPIC_CONFIG);
}

bool GrabViaMqttPlugin::getTopic(const String& topic, JsonObject& value) const
{
    bool isSuccessful = false;

    if (0U != topic.equals(TOPIC_CONFIG))
    {
        getConfiguration(value);
        isSuccessful = true;
    }

    return isSuccessful;
}

bool GrabViaMqttPlugin::setTopic(const String& topic, const JsonObjectConst& value)
{
    bool isSuccessful = false;

    if (0U != topic.equals(TOPIC_CONFIG))
    {
        const size_t        JSON_DOC_SIZE           = 1024U;
        DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
        JsonObject          jsonCfg                 = jsonDoc.to<JsonObject>();
        JsonVariantConst    jsonPath                = value["path"];
        JsonVariantConst    jsonFilter              = value["filter"];
        JsonVariantConst    jsonIconPath            = value["iconPath"];
        JsonVariantConst    jsonFormat              = value["format"];
        JsonVariantConst    jsonMultiplier          = value["multiplier"];
        JsonVariantConst    jsonOffset              = value["offset"];

        /* The received configuration may not contain all single key/value pair.
         * Therefore read first the complete internal configuration and
         * overwrite them with the received ones.
         */
        getConfiguration(jsonCfg);

        /* Note:
         * Check only for the key/value pair availability.
         * The type check will follow in the setConfiguration().
         */

        if (false == jsonPath.isNull())
        {
            jsonCfg["path"] = jsonPath.as<String>();
            isSuccessful = true;
        }

        if (false == jsonFilter.isNull())
        {
            if (true == jsonFilter.is<JsonObjectConst>())
            {
                jsonCfg["filter"] = jsonFilter.as<JsonObjectConst>();
                isSuccessful = true;
            }
            else if (true == jsonFilter.is<String>())
            {
                const size_t            JSON_DOC_FILTER_SIZE    = 256U;
                DynamicJsonDocument     jsonDocFilter(JSON_DOC_FILTER_SIZE);
                DeserializationError    result = deserializeJson(jsonDocFilter, jsonFilter.as<String>());

                if (DeserializationError::Ok == result)
                {
                    jsonCfg["filter"] = jsonDocFilter.as<JsonObjectConst>();
                    isSuccessful = true;
                }
            }
            else
            {
                ;
            }
        }

        if (false == jsonIconPath.isNull())
        {
            jsonCfg["iconPath"] = jsonIconPath.as<String>();
            isSuccessful = true;
        }

        if (false == jsonFormat.isNull())
        {
            jsonCfg["format"] = jsonFormat.as<String>();
            isSuccessful = true;
        }

        if (false == jsonMultiplier.isNull())
        {
            jsonCfg["multiplier"] = jsonMultiplier.as<float>();
            isSuccessful = true;
        }

        if (false == jsonOffset.isNull())
        {
            jsonCfg["offset"] = jsonOffset.as<float>();
            isSuccessful = true;
        }

        if (true == isSuccessful)
        {
            JsonObjectConst jsonCfgConst = jsonCfg;

            isSuccessful = setConfiguration(jsonCfgConst);

            if (true == isSuccessful)
            {
                requestStoreToPersistentMemory();
            }
        }
    }

    return isSuccessful;
}

bool GrabViaMqttPlugin::hasTopicChanged(const String& topic)
{
    MutexGuard<MutexRecursive>  guard(m_mutex);
    bool                        hasTopicChanged = m_hasTopicChanged;

    /* Only a single topic, therefore its not necessary to check. */
    PLUGIN_NOT_USED(topic);

    m_hasTopicChanged = false;

    return hasTopicChanged;
}

void GrabViaMqttPlugin::start(uint16_t width, uint16_t height)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_layoutLeft.setPosAndSize(0, 0, ICON_WIDTH, ICON_HEIGHT);
    (void)m_layoutLeft.addWidget(m_iconWidget);

    /* The text canvas is left aligned to the icon canvas and it spans over
     * the whole display height.
     */
    m_layoutRight.setPosAndSize(ICON_WIDTH, 0, width - ICON_WIDTH, height);
    (void)m_layoutRight.addWidget(m_textWidgetRight);

    /* If only text is used, it will span over the whole display. */
    m_layoutTextOnly.setPosAndSize(0, 0, width, height);
    (void)m_layoutTextOnly.addWidget(m_textWidgetTextOnly);

    /* Choose font. */
    m_textWidgetRight.setFont(Fonts::getFontByType(m_fontType));
    m_textWidgetTextOnly.setFont(Fonts::getFontByType(m_fontType));

    /* The text widget inside the text canvas is left aligned on x-axis and
     * aligned to the center of y-axis.
     */
    if (height > m_textWidgetRight.getFont().getHeight())
    {
        uint16_t diffY = height - m_textWidgetRight.getFont().getHeight();
        uint16_t offsY = diffY / 2U;

        m_textWidgetRight.move(0, offsY);
        m_textWidgetTextOnly.move(0, offsY);
    }

    /* Try to load configuration. If there is no configuration available, a default configuration
     * will be created.
     */
    if (false == loadConfiguration())
    {
        if (false == saveConfiguration())
        {
            LOG_WARNING("Failed to create initial configuration file %s.", getFullPathToConfiguration().c_str());
        }
    }
    else
    {
        /* Remember current timestamp to detect updates of the configuration in the
         * filesystem without using the plugin API.
         */
        updateTimestampLastUpdate();
    }

    if (false == m_iconPath.isEmpty())
    {
        (void)m_iconWidget.load(FILESYSTEM, m_iconPath);
    }

    m_cfgReloadTimer.start(CFG_RELOAD_PERIOD);

    subscribe();
}

void GrabViaMqttPlugin::stop()
{
    String                      configurationFilename = getFullPathToConfiguration();
    MutexGuard<MutexRecursive>  guard(m_mutex);

    m_cfgReloadTimer.stop();
    unsubscribe();

    if (false != FILESYSTEM.remove(configurationFilename))
    {
        LOG_INFO("File %s removed", configurationFilename.c_str());
    }
}

void GrabViaMqttPlugin::process(bool isConnected)
{
    MutexGuard<MutexRecursive>  guard(m_mutex);

    /* Configuration in persistent memory updated? */
    if ((true == m_cfgReloadTimer.isTimerRunning()) &&
        (true == m_cfgReloadTimer.isTimeout()))
    {
        if (true == isConfigurationUpdated())
        {
            m_reloadConfigReq = true;
        }

        m_cfgReloadTimer.restart();
    }

    if (true == m_storeConfigReq)
    {
        if (false == saveConfiguration())
        {
            LOG_WARNING("Failed to save configuration: %s", getFullPathToConfiguration().c_str());
        }

        m_storeConfigReq = false;
    }
    else if (true == m_reloadConfigReq)
    {
        LOG_INFO("Reload configuration: %s", getFullPathToConfiguration().c_str());

        if (true == loadConfiguration())
        {
            updateTimestampLastUpdate();
        }

        m_reloadConfigReq = false;
    }
    else
    {
        ;
    }
}

void GrabViaMqttPlugin::update(YAGfx& gfx)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    gfx.fillScreen(ColorDef::BLACK);

    /* If a icon is available, the icon/text layout will be used otherwise the text only layout. */
    if (false == m_iconPath.isEmpty())
    {
        m_layoutLeft.update(gfx);
        m_layoutRight.update(gfx);
    }
    else
    {
        m_layoutTextOnly.update(gfx);
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void GrabViaMqttPlugin::requestStoreToPersistentMemory()
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_storeConfigReq = true;
}

void GrabViaMqttPlugin::getConfiguration(JsonObject& jsonCfg) const
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    jsonCfg["path"]         = m_path;
    jsonCfg["filter"]       = m_filter;
    jsonCfg["iconPath"]     = m_iconPath;
    jsonCfg["format"]       = m_format;
    jsonCfg["multiplier"]   = m_multiplier;
    jsonCfg["offset"]       = m_offset;
}

bool GrabViaMqttPlugin::setConfiguration(JsonObjectConst& jsonCfg)
{
    bool                status          = false;
    JsonVariantConst    jsonPath        = jsonCfg["path"];
    JsonVariantConst    jsonFilter      = jsonCfg["filter"];
    JsonVariantConst    jsonIconPath    = jsonCfg["iconPath"];
    JsonVariantConst    jsonFormat      = jsonCfg["format"];
    JsonVariantConst    jsonMultiplier  = jsonCfg["multiplier"];
    JsonVariantConst    jsonOffset      = jsonCfg["offset"];

    if (false == jsonPath.is<String>())
    {
        LOG_WARNING("JSON path not found or invalid type.");
    }
    else if (false == jsonFilter.is<JsonObjectConst>())
    {
        LOG_WARNING("JSON filter not found or invalid type.");
    }
    else if (false == jsonIconPath.is<String>())
    {
        LOG_WARNING("JSON icon path not found or invalid type.");
    }
    else if (false == jsonFormat.is<String>())
    {
        LOG_WARNING("JSON format not found or invalid type.");
    }
    else if (false == jsonMultiplier.is<float>())
    {
        LOG_WARNING("JSON multiplier not found or invalid type.");
    }
    else if (false == jsonOffset.is<float>())
    {
        LOG_WARNING("JSON offset not found or invalid type.");
    }
    else
    {
        bool                        reqInit = false;
        bool                        reqIcon = false;
        MutexGuard<MutexRecursive>  guard(m_mutex);

        if (m_path != jsonPath.as<String>())
        {
            unsubscribe();
            reqInit = true;
        }

        if (m_iconPath != jsonIconPath.as<String>())
        {
            reqIcon = true;
        }

        m_path          = jsonPath.as<String>();
        m_filter        = jsonFilter.as<JsonObjectConst>();
        m_iconPath      = jsonIconPath.as<String>();
        m_format        = jsonFormat.as<String>();
        m_multiplier    = jsonMultiplier.as<float>();
        m_offset        = jsonOffset.as<float>();

        if (true == reqInit)
        {
            subscribe();
        }

        /* Load icon immediately */
        if (true == reqIcon)
        {
            if (true == m_iconPath.endsWith(".sprite"))
            {
                String textureFileName = m_iconPath;

                textureFileName.replace(".sprite", ".bmp");

                if (false == m_iconWidget.loadSpriteSheet(FILESYSTEM, m_iconPath, textureFileName))
                {
                    LOG_WARNING("Failed to load animation %s / %s.", m_iconPath.c_str(), textureFileName.c_str());
                }
            }
            else if (true == m_iconPath.endsWith(".bmp"))
            {
                if (false == m_iconWidget.load(FILESYSTEM, m_iconPath))
                {
                    LOG_WARNING("Failed to load bitmap %s.", m_iconPath.c_str());
                }
            }
            else
            {
                m_iconWidget.clear(ColorDef::BLACK);
            }
        }

        m_hasTopicChanged = true;

        status = true;
    }

    return status;
}

void GrabViaMqttPlugin::getJsonValueByFilter(JsonObjectConst src, JsonObjectConst filter, JsonVariantConst& value)
{
    for (JsonPairConst pair : filter)
    {
        if (true == pair.value().is<JsonObjectConst>())
        {
            getJsonValueByFilter(src[pair.key()], filter[pair.key()], value);
        }
        else
        {
            value = src[pair.key()];
        }

        /* Break immediately as its assumed that the filter only contains one
         * single object.
         */
        break;
    }
}

void GrabViaMqttPlugin::subscribe()
{
    MqttService& mqttService = MqttService::getInstance();

    if (false == m_path.isEmpty())
    {
        (void)mqttService.subscribe(m_path,
                                    [this](const String& topic, const uint8_t* payload, size_t size)
                                    {
                                        this->mqttTopicCallback(topic, payload, size);
                                    });
    }
}

void GrabViaMqttPlugin::unsubscribe()
{
    MqttService& mqttService = MqttService::getInstance();

    if (false == m_path.isEmpty())
    {
        mqttService.unsubscribe(m_path);
    }
}

void GrabViaMqttPlugin::mqttTopicCallback(const String& topic, const uint8_t* payload, size_t size)
{
    const size_t            JSON_DOC_SIZE   = 1024U;
    DynamicJsonDocument     jsonDoc(JSON_DOC_SIZE);
    DeserializationError    error           = deserializeJson(jsonDoc, payload, size);

    if (DeserializationError::Ok != error)
    {
        LOG_WARNING("MQTT payload contains invalid JSON.");
    }
    else
    {
        JsonVariantConst jsonValue;

        getJsonValueByFilter(jsonDoc.as<JsonObjectConst>(), m_filter.as<JsonObjectConst>(), jsonValue);

        /* Is it a number? */
        if ((true == jsonValue.is<float>()) &&
            (0 > m_format.indexOf("%s"))) /* Prevent mistake which may cause a LoadProhibited core panic by snprintf. */
        {
            const size_t    BUFFER_SIZE = 128U;
            char            buffer[BUFFER_SIZE];
            float           value = jsonValue.as<float>();

            value *= m_multiplier;
            value += m_offset;

            (void)snprintf(buffer, sizeof(buffer), m_format.c_str(), value);

            m_textWidgetRight.setFormatStr(buffer);
            m_textWidgetTextOnly.setFormatStr(buffer);
        }
        /* Is it a string? */
        else if (true == jsonValue.is<String>())
        {
            const size_t    BUFFER_SIZE = 40U;
            char            buffer[BUFFER_SIZE];

            (void)snprintf(buffer, sizeof(buffer), m_format.c_str(), jsonValue.as<String>().c_str());

            m_textWidgetRight.setFormatStr(buffer);
            m_textWidgetTextOnly.setFormatStr(buffer);
        }
        else
        {
            m_textWidgetRight.setFormatStr("\\calign-");
            m_textWidgetTextOnly.setFormatStr("\\calign-");
        }
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
