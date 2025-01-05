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
 * @brief  Home Assistant MQTT extension
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "HomeAssistantMqtt.h"
#include "Version.h"

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
const char* HomeAssistantMqtt::KEY_HA_DISCOVERY_PREFIX     = "ha_dp";

/* Initialize Home Assistant discovery prefix name. */
const char* HomeAssistantMqtt::NAME_HA_DISCOVERY_PREFIX    = "Home Assistant Discovery Prefix";

/* Initialize Home Assistant discovery prefix default value. */
const char* HomeAssistantMqtt::DEFAULT_HA_DISCOVERY_PREFIX = "homeassistant";

/* Initialize Home Assistant discovery enable flag key */
const char* HomeAssistantMqtt::KEY_HA_DISCOVERY_ENABLE     = "ha_ena";

/* Initialize Home Assistant discovery enable flag name */
const char* HomeAssistantMqtt::NAME_HA_DISCOVERY_ENABLE    = "Enable Home Assistant Discovery";

/* Initialize Home Assistant discovery enable flag default value */
const bool HomeAssistantMqtt::DEFAULT_HA_DISCOVERY_ENABLE  = false;

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
        m_haDiscoveryPrefix  = m_haDiscoveryPrefixSetting.getValue();
        m_haDiscoveryEnabled = m_haDiscoveryEnabledSetting.getValue();

        settings.close();
    }
}

void HomeAssistantMqtt::stop()
{
    SettingsService& settings = SettingsService::getInstance();

    settings.unregisterSetting(&m_haDiscoveryPrefixSetting);
    settings.unregisterSetting(&m_haDiscoveryEnabledSetting);
}

void HomeAssistantMqtt::process(bool isConnected)
{
    /* The Home Assistant discovery must be enabled. */
    if (true == m_haDiscoveryEnabled)
    {
        if (true == isConnected)
        {
            publishAutoDiscoveryInfosOnDemand();
        }
    }

    m_isConnected = isConnected;
}

void HomeAssistantMqtt::registerMqttDiscovery(const String& deviceId, const String& entityId, const String& topic, const String& stateTopic, const String& cmdTopic, const String& availabilityTopic, JsonObjectConst& extra)
{
    /* The Home Assistant discovery must be enabled and the prefix must be available, otherwise this
     * feature is disabled.
     */
    if ((true == m_haDiscoveryEnabled) &&
        (false == m_haDiscoveryPrefix.isEmpty()))
    {
        JsonVariantConst jsonHomeAssistant = extra["ha"];

        /* Configuration available? */
        if (true == jsonHomeAssistant.is<JsonObjectConst>())
        {
            JsonVariantConst jsonComponent = jsonHomeAssistant["component"];
            JsonVariantConst jsonDiscovery = jsonHomeAssistant["discovery"];

            if ((false == jsonComponent.is<String>()) ||
                (false == jsonDiscovery.is<JsonObjectConst>()))
            {
                LOG_WARNING("Invalid Home Assistant MQTT discovery configuration.");
            }
            else
            {
                MqttDiscoveryInfo* mqttDiscoveryInfo = new (std::nothrow) MqttDiscoveryInfo();

                if (nullptr != mqttDiscoveryInfo)
                {
                    mqttDiscoveryInfo->component = jsonComponent.as<String>();
                    mqttDiscoveryInfo->nodeId    = getNodeId(deviceId);
                    mqttDiscoveryInfo->objectId  = getObjectId(entityId, topic);
                    mqttDiscoveryInfo->discoveryDetails.set(jsonDiscovery); /* Deep copy, because discovery details are handled in different context. */

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

void HomeAssistantMqtt::unregisterMqttDiscovery(const String& deviceId, const String& entityId, const String& topic, const String& stateTopic, const String& cmdTopic)
{
    /* The Home Assistant discovery must be enabled and the prefix must be available, otherwise this
     * feature is disabled.
     */
    if ((true == m_haDiscoveryEnabled) &&
        (false == m_haDiscoveryPrefix.isEmpty()))
    {
        ListOfMqttDiscoveryInfo::iterator listOfMqttDiscoveryInfoIt = m_mqttDiscoveryInfoList.begin();
        String                            objectId                  = getObjectId(entityId, topic);

        while (m_mqttDiscoveryInfoList.end() != listOfMqttDiscoveryInfoIt)
        {
            MqttDiscoveryInfo* mqttDiscoveryInfo = *listOfMqttDiscoveryInfoIt;
            bool               found             = false;

            if ((nullptr != mqttDiscoveryInfo) &&
                (deviceId == mqttDiscoveryInfo->nodeId) &&
                (objectId == mqttDiscoveryInfo->objectId))
            {
                JsonVariantConst jsonStateTopic = mqttDiscoveryInfo->discoveryDetails["stat_t"];
                JsonVariantConst jsonCmdTopic   = mqttDiscoveryInfo->discoveryDetails["cmd_t"];

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
                MqttService& mqttService = MqttService::getInstance();
                String       mqttTopic;

                getConfigTopic(mqttTopic, mqttDiscoveryInfo->component, mqttDiscoveryInfo->nodeId, mqttDiscoveryInfo->objectId);

                /* Purge retained discovery info. */
                if (false == mqttService.publish(mqttTopic, "", true))
                {
                    LOG_WARNING("Failed to purge HA discovery info of %s.", mqttDiscoveryInfo->objectId.c_str());
                }
                /* Successful purged. */
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

String HomeAssistantMqtt::getNodeId(const String& deviceId)
{
    String nodeId = deviceId;

    /* Home Assistant MQTT discovery doesn't allow '/' and '.' in the node id.
     * See https://www.home-assistant.io/integrations/mqtt#discovery-messages
     */
    nodeId.replace('/', '_');
    nodeId.replace('.', '_');

    return nodeId;
}

String HomeAssistantMqtt::getObjectId(const String& entityId, const String& topic)
{
    String objectId;

    if (false == entityId.isEmpty())
    {
        objectId = entityId + "/" + topic;
    }
    else
    {
        objectId = topic;
    }

    /* Home Assistant MQTT discovery doesn't allow '/' and '.' in the object id.
     * See https://www.home-assistant.io/integrations/mqtt#discovery-messages
     */
    objectId.replace('/', '_');
    objectId.replace('.', '_');

    return objectId;
}

void HomeAssistantMqtt::clearMqttDiscoveryInfoList()
{
    ListOfMqttDiscoveryInfo::iterator listOfMqttDiscoveryInfoIt = m_mqttDiscoveryInfoList.begin();

    while (m_mqttDiscoveryInfoList.end() != listOfMqttDiscoveryInfoIt)
    {
        MqttDiscoveryInfo* mqttDiscoveryInfo = *listOfMqttDiscoveryInfoIt;

        listOfMqttDiscoveryInfoIt            = m_mqttDiscoveryInfoList.erase(listOfMqttDiscoveryInfoIt);

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
    const size_t            JSON_DOC_SIZE = 2048U;
    DynamicJsonDocument     jsonDoc(JSON_DOC_SIZE);
    MqttService&            mqttService = MqttService::getInstance();
    String                  mqttTopic;
    String                  discoveryInfo;
    JsonObjectConstIterator discoveryDetailsIt = mqttDiscoveryInfo.discoveryDetails.as<JsonObjectConst>().begin();

    getConfigTopic(mqttTopic, mqttDiscoveryInfo.component, mqttDiscoveryInfo.nodeId, mqttDiscoveryInfo.objectId);

    /* The object id (object_id) is used to generate the entity id. */
    jsonDoc["obj_id"]      = mqttDiscoveryInfo.objectId;
    /* The unique id (unique_id) identifies the device and its entity. */
    jsonDoc["uniq_id"]     = mqttDiscoveryInfo.nodeId + "/" + mqttDiscoveryInfo.objectId;
    /* Device identifier */
    jsonDoc["dev"]["ids"]  = WiFi.macAddress();
    /* URL to configuration of the device (configuration_url). */
    jsonDoc["dev"]["cu"]   = String("http://") + WiFi.localIP().toString();
    /* Name of the device. */
    jsonDoc["dev"]["name"] = mqttDiscoveryInfo.nodeId;
    /* Device model name (model) */
    jsonDoc["dev"]["mdl"]  = "Pixelix";
    /* Manufacturer (manufacturer) */
    jsonDoc["dev"]["mf"]   = "BlueAndi & Friends";
    /* SW version of the device (sw_version) */
    jsonDoc["dev"]["sw"]   = Version::getSoftwareVersion();
    /* HW version is used for the target name (hw_version). */
    jsonDoc["dev"]["hw"]   = Version::getTargetName();
    /* Origin name */
    jsonDoc["o"]["name"]   = "Pixelix";
    /* Origin URL */
    jsonDoc["o"]["url"]    = "https://github.com/BlueAndi/Pixelix";

    while (discoveryDetailsIt != mqttDiscoveryInfo.discoveryDetails.as<JsonObjectConst>().end())
    {
        jsonDoc[discoveryDetailsIt->key()] = discoveryDetailsIt->value();

        ++discoveryDetailsIt;
    }

    /* Send the JSON as string. */
    if (0U < serializeJson(jsonDoc, discoveryInfo))
    {
        /* Publish retained to ensure that HomeAssistant will recognize the device entity. */
        if (false == mqttService.publish(mqttTopic, discoveryInfo, true))
        {
            LOG_WARNING("Failed to provide HA discovery info of %s.", mqttDiscoveryInfo.objectId.c_str());
        }
        else
        {
            LOG_INFO("HA discovery info of %s published.", mqttDiscoveryInfo.objectId.c_str());
        }
    }
}

void HomeAssistantMqtt::publishAutoDiscoveryInfosOnDemand()
{
    ListOfMqttDiscoveryInfo::iterator listOfMqttDiscoveryInfoIt = m_mqttDiscoveryInfoList.begin();

    while (m_mqttDiscoveryInfoList.end() != listOfMqttDiscoveryInfoIt)
    {
        MqttDiscoveryInfo* mqttDiscoveryInfo = *listOfMqttDiscoveryInfoIt;

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
