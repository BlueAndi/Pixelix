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
const char* HomeAssistantMqtt::DEFAULT_HA_DISCOVERY_PREFIX  = "homeassistant";

/* Initialize Home Assistant discovery enable flag key */
const char* HomeAssistantMqtt::KEY_HA_DISCOVERY_ENABLE      = "ha_ena";

/* Initialize Home Assistant discovery enable flag name */
const char* HomeAssistantMqtt::NAME_HA_DISCOVERY_ENABLE     = "Enable Home Assistant Discovery";

/* Initialize Home Assistant discovery enable flag default value */
const bool  HomeAssistantMqtt::DEFAULT_HA_DISCOVERY_ENABLE  = false;

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
    else if (false == settings.registerSetting(&m_haDiscoveryEnabledSetting))
    {
        LOG_ERROR("Couldn't register HA discovery enable setting.");
    }
    else if (false == settings.open(true))
    {
        LOG_ERROR("Couldn't open settings.");
    }
    else
    {
        m_haDiscoveryPrefix     = m_haDiscoveryPrefixSetting.getValue();
        m_haDiscoveryEnabled    = m_haDiscoveryEnabledSetting.getValue();

        settings.close();
    }
}

void HomeAssistantMqtt::stop()
{
    SettingsService& settings = SettingsService::getInstance();

    settings.unregisterSetting(&m_haDiscoveryPrefixSetting);
    settings.unregisterSetting(&m_haDiscoveryEnabledSetting);

    clearMqttDiscoveryInfoList();
}

void HomeAssistantMqtt::process(bool isConnected)
{
    /* The Home Assistant discovery must be enabled. */
    if (true == m_haDiscoveryEnabled)
    {
        if (true == isConnected)
        {
            /* Connection to broker re-estiablished?
             * All automatic discovery info's need to be published again.
             */
            if (false == m_isConnected)
            {
                requestToPublishAllAutoDiscoveryInfos();
            }

            publishAutoDiscoveryInfosOnDemand();
        }
    }

    m_isConnected = isConnected;
}

void HomeAssistantMqtt::registerMqttDiscovery(const String& deviceId, const String& entityId, const String& stateTopic, const String& cmdTopic, const String& availabilityTopic, JsonObjectConst& extra)
{
    /* The Home Assistant discovery must be enabled and the prefix must be available, otherwise this
     * feature is disabled.
     */
    if ((true == m_haDiscoveryEnabled) &&
        (false == m_haDiscoveryPrefix.isEmpty()))
    {
        JsonVariantConst jsonHomeAssistant = extra["ha"];

        /* Configuration available? */
        if (false == jsonHomeAssistant.isNull())
        {
            JsonVariantConst jsonComponent = jsonHomeAssistant["component"];

            if (true == jsonComponent.is<String>())
            {
                MqttDiscoveryInfo* mqttDiscoveryInfo = new(std::nothrow) MqttDiscoveryInfo();

                if (nullptr != mqttDiscoveryInfo)
                {
                    mqttDiscoveryInfo->component        = jsonComponent.as<String>();
                    mqttDiscoveryInfo->nodeId           = deviceId;
                    mqttDiscoveryInfo->objectId         = getObjectId(entityId);
                    mqttDiscoveryInfo->discoveryDetails = jsonHomeAssistant["discovery"];

                    /* Readable topic? */
                    if (false == stateTopic.isEmpty())
                    {
                        mqttDiscoveryInfo->discoveryDetails["stat_t"] = stateTopic;
                    }

                    /* Writeable topic? */
                    if (false == cmdTopic.isEmpty())
                    {
                        mqttDiscoveryInfo->discoveryDetails["cmd_t"] = cmdTopic;
                    }

                    /* Availability? */
                    if (false == availabilityTopic.isEmpty())
                    {
                        mqttDiscoveryInfo->discoveryDetails["avty_t"] = availabilityTopic;
                    }

                    m_mqttDiscoveryInfoList.push_back(mqttDiscoveryInfo);
                }
            }
        }
    }
}

void HomeAssistantMqtt::unregisterMqttDiscovery(const String& deviceId, const String& entityId, const String& stateTopic, const String& cmdTopic)
{
    /* The Home Assistant discovery must be enabled and the prefix must be available, otherwise this
     * feature is disabled.
     */
    if ((true == m_haDiscoveryEnabled) &&
        (false == m_haDiscoveryPrefix.isEmpty()))
    {
        ListOfMqttDiscoveryInfo::iterator   listOfMqttDiscoveryInfoIt   = m_mqttDiscoveryInfoList.begin();
        String                              objectId                    = getObjectId(entityId);

        while(m_mqttDiscoveryInfoList.end() != listOfMqttDiscoveryInfoIt)
        {
            MqttDiscoveryInfo*  mqttDiscoveryInfo   = *listOfMqttDiscoveryInfoIt;
            bool                found               = false;
            
            if ((nullptr != mqttDiscoveryInfo) &&
                (deviceId == mqttDiscoveryInfo->nodeId) &&
                (objectId == mqttDiscoveryInfo->objectId))
            {
                JsonVariantConst    jsonStateTopic  = mqttDiscoveryInfo->discoveryDetails["stat_t"];
                JsonVariantConst    jsonCmdTopic    = mqttDiscoveryInfo->discoveryDetails["cmd_t"];
                
                /* Topic only readable? */
                if ((false == jsonStateTopic.isNull()) &&
                    (true == jsonCmdTopic.isNull()))
                {
                    if (stateTopic == jsonStateTopic.as<String>())
                    {
                        found = true;
                    }
                }
                /* Topic only writeable? */
                else if ((true == jsonStateTopic.isNull()) &&
                         (false == jsonCmdTopic.isNull()))
                {
                    if (cmdTopic == jsonCmdTopic.as<String>())
                    {
                        found = true;
                    }
                }
                /* Topic is read- and writeable? */
                else if ((false == jsonStateTopic.isNull()) &&
                         (false == jsonCmdTopic.isNull()))
                {
                    if ((stateTopic == jsonStateTopic.as<String>()) &&
                        (cmdTopic == jsonCmdTopic.as<String>()))
                    {
                        found = true;
                    }
                }
                else
                {
                    ;
                }
            }

            if (false == found)
            {
                ++listOfMqttDiscoveryInfoIt;
            }
            else
            {
                MqttService&    mqttService = MqttService::getInstance();
                String          mqttTopic;

                getConfigTopic(mqttTopic, mqttDiscoveryInfo->component, mqttDiscoveryInfo->nodeId, mqttDiscoveryInfo->objectId);

                /* Purge discovery info. */
                if (false == mqttService.publish(mqttTopic, ""))
                {
                    LOG_WARNING("Failed to purge HA discovery info of %s.", mqttDiscoveryInfo->objectId.c_str());
                }
                else
                {
                    LOG_INFO("HA discovery info of %s purged.", mqttDiscoveryInfo->objectId.c_str());
                }

                listOfMqttDiscoveryInfoIt = m_mqttDiscoveryInfoList.erase(listOfMqttDiscoveryInfoIt);

                delete mqttDiscoveryInfo;
                mqttDiscoveryInfo = nullptr;
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

String HomeAssistantMqtt::getObjectId(const String& entityId)
{
    String objectId = entityId;

    /* Home Assistant MQTT discovery doesn't allow '/' and '.' in the object id. */
    objectId.replace('/', '_');
    objectId.replace('.', '_');

    return objectId;
}

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
    const size_t            JSON_DOC_SIZE               = 1024U;
    DynamicJsonDocument     jsonDoc(JSON_DOC_SIZE);
    MqttService&            mqttService                 = MqttService::getInstance();
    String                  mqttTopic;
    String                  discoveryInfo;
    JsonObjectConstIterator discoveryDetailsIt          = mqttDiscoveryInfo.discoveryDetails.as<JsonObjectConst>().begin();

    getConfigTopic(mqttTopic, mqttDiscoveryInfo.component, mqttDiscoveryInfo.nodeId, mqttDiscoveryInfo.objectId);

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

    while(discoveryDetailsIt != mqttDiscoveryInfo.discoveryDetails.as<JsonObjectConst>().end())
    {
        jsonDoc[discoveryDetailsIt->key()] = discoveryDetailsIt->value();

        ++discoveryDetailsIt;
    }

    /* Send the JSON as string. */
    if (0U < serializeJson(jsonDoc, discoveryInfo))
    {
        if (false == mqttService.publish(mqttTopic, discoveryInfo))
        {
            LOG_WARNING("Failed to provide HA discovery info of %s.", mqttDiscoveryInfo.objectId.c_str());
        }
        else
        {
            LOG_INFO("HA discovery info of %s published.", mqttDiscoveryInfo.objectId.c_str());
        }
    }
}

void HomeAssistantMqtt::requestToPublishAllAutoDiscoveryInfos()
{
    ListOfMqttDiscoveryInfo::iterator listOfMqttDiscoveryInfoIt = m_mqttDiscoveryInfoList.begin();

    while(m_mqttDiscoveryInfoList.end() != listOfMqttDiscoveryInfoIt)
    {
        MqttDiscoveryInfo*  mqttDiscoveryInfo   = *listOfMqttDiscoveryInfoIt;

        if (nullptr != mqttDiscoveryInfo)
        {
            mqttDiscoveryInfo->isReqToPublish = true;
        }

        ++listOfMqttDiscoveryInfoIt;
    }
}

void HomeAssistantMqtt::publishAutoDiscoveryInfosOnDemand()
{
    ListOfMqttDiscoveryInfo::iterator listOfMqttDiscoveryInfoIt = m_mqttDiscoveryInfoList.begin();

    while(m_mqttDiscoveryInfoList.end() != listOfMqttDiscoveryInfoIt)
    {
        MqttDiscoveryInfo*  mqttDiscoveryInfo   = *listOfMqttDiscoveryInfoIt;

        if (nullptr != mqttDiscoveryInfo)
        {
            if (true == mqttDiscoveryInfo->isReqToPublish)
            {
                publishAutoDiscoveryInfo(*mqttDiscoveryInfo);

                mqttDiscoveryInfo->isReqToPublish = false;

                /* Continue with next call cycle. */
                break;
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
