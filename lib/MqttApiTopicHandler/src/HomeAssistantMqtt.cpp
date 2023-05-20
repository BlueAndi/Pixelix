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
 * @brief  Home Assistant MQTT extension
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "HomeAssistantMqtt.h"

#include <SettingsService.h>
#include <Logging.h>
#include <MqttService.h>
#include <WiFi.h>

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/** Stringizing the value. */
#define Q(x) #x

/** Quote the given value to get a string literal. */
#define QUOTE(x) Q(x)

/******************************************************************************
 * Types and classes
 *****************************************************************************/

/******************************************************************************
 * Prototypes
 *****************************************************************************/

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/* Initialize Home Assistant discovery prefix key. */
const char* HomeAssistantMqtt::KEY_HA_DISCOVERY_PREFIX      = "ha_dp";

/* Initialize Home Assistant discovery prefix name. */
const char* HomeAssistantMqtt::NAME_HA_DISCOVERY_PREFIX     = "Home Assistant Discovery Prefix";

/* Initialize Home Assistant discovery prefix default value. */
const char* HomeAssistantMqtt::DEFAULT_HA_DISCOVERY_PREFIX  = "";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void HomeAssistantMqtt::start()
{
    SettingsService& settings = SettingsService::getInstance();

    if (false == settings.registerSetting(&m_haDiscoveryPrefixSetting))
    {
        LOG_ERROR("Couldn't register HA discovery prefix setting.");
    }
    else if (false == settings.open(true))
    {
        LOG_ERROR("Couldn't open settings.");
    }
    else
    {
        m_haDiscoveryPrefix = m_haDiscoveryPrefixSetting.getValue();

        settings.close();
    }
}

void HomeAssistantMqtt::stop()
{
    SettingsService& settings = SettingsService::getInstance();

    settings.unregisterSetting(&m_haDiscoveryPrefixSetting);

    clearMqttDiscoveryInfoList();
}

void HomeAssistantMqtt::process(bool isConnected)
{
    if (true == isConnected)
    {
        if (false == m_isConnected)
        {
            publishAllAutoDiscoveryInfo(true);
        }
        else
        {
            publishAllAutoDiscoveryInfo(false);
        }
    }

    m_isConnected = isConnected;
}

void HomeAssistantMqtt::registerMqttDiscovery(const String& nodeId, const String& objectId, const String& stateTopic, const String& cmdTopic, JsonObjectConst& extra)
{
    /* The Home Assistant discovery prefix must be available, otherwise this
     * feature is disabled.
     */
    if (false == m_haDiscoveryPrefix.isEmpty())
    {
        JsonVariantConst jsonHomeAssistant = extra["ha"];

        /* Configuration available? */
        if (false == jsonHomeAssistant.isNull())
        {
            JsonVariantConst    jsonComponent       = jsonHomeAssistant["component"];
            JsonVariantConst    jsonCommandTemplate = jsonHomeAssistant["commandTemplate"];
            JsonVariantConst    jsonValueTemplate   = jsonHomeAssistant["valueTemplate"];

            if (true == jsonComponent.is<String>())
            {
                MqttDiscoveryInfo* mqttDiscoveryInfo = new(std::nothrow) MqttDiscoveryInfo();

                if (nullptr != mqttDiscoveryInfo)
                {
                    mqttDiscoveryInfo->component    = jsonComponent.as<String>();
                    mqttDiscoveryInfo->nodeId       = nodeId;
                    mqttDiscoveryInfo->objectId     = objectId;
                    mqttDiscoveryInfo->stateTopic   = stateTopic;
                    mqttDiscoveryInfo->commandTopic = cmdTopic;

                    /* Command template is optional */
                    if (true == jsonCommandTemplate.is<String>())
                    {
                        mqttDiscoveryInfo->commandTemplate = jsonCommandTemplate.as<String>();
                    }

                    /* Value template is optional */
                    if (true == jsonValueTemplate.is<String>())
                    {
                        mqttDiscoveryInfo->valueTemplate = jsonValueTemplate.as<String>();
                    }

                    m_mqttDiscoveryInfoList.push_back(mqttDiscoveryInfo);
                }
            }
        }
    }
}

void HomeAssistantMqtt::unregisterMqttDiscovery(const String& nodeId, const String& objectId, const String& stateTopic, const String& cmdTopic)
{
    /* The Home Assistant discovery prefix must be available, otherwise this
     * feature is disabled.
     */
    if (false == m_haDiscoveryPrefix.isEmpty())
    {
        ListOfMqttDiscoveryInfo::iterator listOfMqttDiscoveryInfoIt = m_mqttDiscoveryInfoList.begin();

        while(m_mqttDiscoveryInfoList.end() != listOfMqttDiscoveryInfoIt)
        {
            MqttDiscoveryInfo* mqttDiscoveryInfo = *listOfMqttDiscoveryInfoIt;

            if ((nullptr != mqttDiscoveryInfo) &&
                (nodeId == mqttDiscoveryInfo->nodeId) &&
                (objectId == mqttDiscoveryInfo->objectId) &&
                (stateTopic == mqttDiscoveryInfo->stateTopic) &&
                (cmdTopic == mqttDiscoveryInfo->commandTopic))
            {
                MqttService&    mqttService = MqttService::getInstance();
                String          mqttTopic;

                getConfigTopic(mqttTopic, mqttDiscoveryInfo->component, mqttDiscoveryInfo->nodeId, mqttDiscoveryInfo->objectId);

                /* Purge discovery info. */
                if (false == mqttService.publish(mqttTopic, ""))
                {
                    LOG_WARNING("[%s] Failed to purge HA discovery info.", mqttDiscoveryInfo->objectId.c_str());
                }
                else
                {
                    LOG_INFO("[%s] HA discovery info purged.", mqttDiscoveryInfo->objectId.c_str());
                }

                listOfMqttDiscoveryInfoIt = m_mqttDiscoveryInfoList.erase(listOfMqttDiscoveryInfoIt);

                delete mqttDiscoveryInfo;
                mqttDiscoveryInfo = nullptr;
            }
            else
            {
                ++listOfMqttDiscoveryInfoIt;
            }
        }
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void HomeAssistantMqtt::clearMqttDiscoveryInfoList()
{
    ListOfMqttDiscoveryInfo::iterator listOfMqttDiscoveryInfoIt = m_mqttDiscoveryInfoList.begin();

    while(m_mqttDiscoveryInfoList.end() != listOfMqttDiscoveryInfoIt)
    {
        MqttDiscoveryInfo* mqttDiscoveryInfo = *listOfMqttDiscoveryInfoIt;

        listOfMqttDiscoveryInfoIt = m_mqttDiscoveryInfoList.erase(listOfMqttDiscoveryInfoIt);

        if (nullptr != mqttDiscoveryInfo)
        {
            delete mqttDiscoveryInfo;
            mqttDiscoveryInfo = nullptr;
        }
    }
}

void HomeAssistantMqtt::getConfigTopic(String& haConfigTopic, const String& component, const String& nodeId, const String& objectId)
{
    haConfigTopic  = m_haDiscoveryPrefix;
    haConfigTopic += "/";
    haConfigTopic += component;
    haConfigTopic += "/";
    haConfigTopic += nodeId;
    haConfigTopic += "/";
    haConfigTopic += objectId;
    haConfigTopic += "/config";
}

void HomeAssistantMqtt::publishAutoDiscoveryInfo(MqttDiscoveryInfo& mqttDiscoveryInfo)
{
    const size_t        JSON_DOC_SIZE               = 1024U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    MqttService&        mqttService                 = MqttService::getInstance();
    String              mqttTopic;
    String              discoveryInfo;

    getConfigTopic(mqttTopic, mqttDiscoveryInfo.component, mqttDiscoveryInfo.nodeId, mqttDiscoveryInfo.objectId);

    /* Entity name */
    jsonDoc["name"]                 = "MQTT text";
    /* The object id (object_id) is used to generate the entity id. */
    jsonDoc["obj_id"]               = mqttDiscoveryInfo.objectId;
    /* The unique id (unique_id) identifies the device and its entity. */
    jsonDoc["uniq_id"]              = mqttDiscoveryInfo.nodeId + "/" + mqttDiscoveryInfo.objectId;
    /* Device identifier */
    jsonDoc["dev"]["identifiers"]   = WiFi.macAddress();
    /* URL to configuration of the device (configuration_url). */
    jsonDoc["dev"]["cu"]            = String("http://") + WiFi.localIP().toString();
    /* Name of the device. */
    jsonDoc["dev"]["name"]          = mqttDiscoveryInfo.nodeId;
    /* Device model name (model) */
    jsonDoc["dev"]["mdl"]           = "Pixelix";
    /* Manufacturer (manufacturer) */
    jsonDoc["dev"]["mf"]            = "BlueAndi & Friends";
    /* SW version of the device (sw_version) */
    jsonDoc["dev"]["sw"]            = QUOTE(SW_VERSION);

    /* Readable topic? */
    if (false == mqttDiscoveryInfo.stateTopic.isEmpty())
    {
        jsonDoc["stat_t"] = mqttDiscoveryInfo.stateTopic;

        if (false == mqttDiscoveryInfo.valueTemplate.isEmpty())
        {
            jsonDoc["val_tpl"] = mqttDiscoveryInfo.valueTemplate;
        }
    }

    /* Writeable topic? */
    if (false == mqttDiscoveryInfo.commandTopic.isEmpty())
    {
        jsonDoc["cmd_t"] = mqttDiscoveryInfo.commandTopic;

        if (false == mqttDiscoveryInfo.commandTemplate.isEmpty())
        {
            jsonDoc["cmd_tpl"] = mqttDiscoveryInfo.commandTemplate;
        }
    }

    /* Send the JSON as string. */
    if (0U < serializeJson(jsonDoc, discoveryInfo))
    {
        if (false == mqttService.publish(mqttTopic, discoveryInfo))
        {
            LOG_WARNING("[%s] Failed to provide HA discovery info.", mqttDiscoveryInfo.objectId.c_str());
        }
        else
        {
            LOG_INFO("[%s] HA discovery info published.", mqttDiscoveryInfo.objectId.c_str());
        }
    }

    mqttDiscoveryInfo.isReqToPublish = false;
}

void HomeAssistantMqtt::publishAllAutoDiscoveryInfo(bool force)
{
    ListOfMqttDiscoveryInfo::iterator listOfMqttDiscoveryInfoIt = m_mqttDiscoveryInfoList.begin();

    while(m_mqttDiscoveryInfoList.end() != listOfMqttDiscoveryInfoIt)
    {
        MqttDiscoveryInfo*  mqttDiscoveryInfo   = *listOfMqttDiscoveryInfoIt;

        if (nullptr != mqttDiscoveryInfo)
        {
            if ((true == force) ||
                (true == mqttDiscoveryInfo->isReqToPublish))
            {
                publishAutoDiscoveryInfo(*mqttDiscoveryInfo);
            }
        }

        ++listOfMqttDiscoveryInfoIt;
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
