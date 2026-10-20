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
 * @file   HomeAssistantMqtt.cpp
 * @brief  Home Assistant MQTT extension
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "HomeAssistantMqtt.h"

#include <JsonEscape.hpp>
#include "Version.h"

#include <SettingsService.h>
#include <Logging.h>
#include <MqttService.h>
#include <WiFi.h>
#include <JsonFile.h>
#include <FileSystem.h>
#include <Board.h>
#include <Util.h>
#include <string.h>

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

/* Initialize Home Assistant extra info key. */
const char* HomeAssistantMqtt::KEY_EXTRA_INFO_HA           = "ha";

/* Initialize Home Assistant discovery prefix key. */
const char* HomeAssistantMqtt::KEY_HA_DISCOVERY_PREFIX     = "ha_dp";

/* Initialize Home Assistant discovery prefix name. */
const char* HomeAssistantMqtt::NAME_HA_DISCOVERY_PREFIX    = "Home Assistant Discovery Prefix";

/* Initialize Home Assistant discovery prefix default value. */
const char* HomeAssistantMqtt::DEFAULT_HA_DISCOVERY_PREFIX = "homeassistant";

/* Initialize Home Assistant discovery enable flag key */
const char* HomeAssistantMqtt::KEY_HA_DISCOVERY_ENABLE     = "ha_ena";

/* Initialize Home Assistant discovery enable flag name */
const char* HomeAssistantMqtt::NAME_HA_DISCOVERY_ENABLE    = "Enable Home Assistant MQTT Discovery";

/* Initialize Home Assistant discovery enable flag default value */
const bool HomeAssistantMqtt::DEFAULT_HA_DISCOVERY_ENABLE  = false;

/* Initialize Home Assistant birth topic endpoint. */
const char* HomeAssistantMqtt::HA_BIRTH_TOPIC_ENDPOINT     = "/status";

/* Initialize Home Assistant birth payload. */
const char* HomeAssistantMqtt::HA_BIRTH_PAYLOAD_ONLINE     = "online";

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

    unsubscribeBirthTopic();

    settings.unregisterSetting(&m_haDiscoveryPrefixSetting);
    settings.unregisterSetting(&m_haDiscoveryEnabledSetting);
}

void HomeAssistantMqtt::process(bool isConnected)
{
    /* The Home Assistant discovery must be enabled. */
    if ((true == m_haDiscoveryEnabled) &&
        (true == isConnected))
    {
        subscribeBirthTopic();

        /* Home Assistant reported itself as online again?
         *
         * The discovery infos are published retained and therefore the broker
         * delivers them on its own. The topic states are not retained, they
         * would stay unknown until the next change. Therefore everything is
         * published again.
         */
        if (true == m_isReqToRepublishAll)
        {
            m_isReqToRepublishAll = false;

            LOG_INFO("Home Assistant is online, publish discovery infos and states again.");

            requestToPublishAllDiscoveryInfos();

            if (nullptr != m_republishStatesFunc)
            {
                m_republishStatesFunc();
            }
        }

        publishAutoDiscoveryInfosOnDemand();
    }
}

void HomeAssistantMqtt::setRepublishStatesFunc(RepublishStatesFunc republishStatesFunc)
{
    m_republishStatesFunc = republishStatesFunc;
}

void HomeAssistantMqtt::registerMqttDiscovery(const String& deviceId, const String& entityId, const String& topic, const String& mqttStateTopic, const String& mqttCmdTopic, const String& mqttAvailabilityTopic, JsonObjectConst& jsonExtra)
{
    /* The Home Assistant discovery must be enabled and the prefix must be available, otherwise this
     * feature is disabled.
     */
    if ((true == m_haDiscoveryEnabled) &&
        (false == m_haDiscoveryPrefix.isEmpty()))
    {
        JsonVariantConst jsonHomeAssistant = jsonExtra[KEY_EXTRA_INFO_HA];

        /* Is the extra info not relevant? */
        if (true == jsonHomeAssistant.isNull())
        {
            /* Skip it, because it seems to be not Home Assistant relevant. */
            ;
        }
        /* Filename for the HA discovery info available? */
        else if (true == jsonHomeAssistant.is<String>())
        {
            const char* discoveryInfoFileName = jsonHomeAssistant.as<const char*>();

            /* Is the discovery info file name not empty? */
            if ('\0' != discoveryInfoFileName[0])
            {
                MqttDiscoveryInfo* mqttDiscoveryInfo = new (std::nothrow) MqttDiscoveryInfo();

                if (nullptr != mqttDiscoveryInfo)
                {
                    mqttDiscoveryInfo->deviceId              = deviceId;              /* Required for Home Assistant node-id generation. */
                    mqttDiscoveryInfo->entityId              = entityId;              /* Required for Home Assistant object-id generation. */
                    mqttDiscoveryInfo->topic                 = topic;                 /* Required for Home Assistant object-id generation. */
                    mqttDiscoveryInfo->mqttStateTopic        = mqttStateTopic;        /* Required for the Home Assistant MQTT discovery configuration. */
                    mqttDiscoveryInfo->mqttCmdTopic          = mqttCmdTopic;          /* Required for the Home Assistant MQTT discovery configuration. */
                    mqttDiscoveryInfo->mqttAvailabilityTopic = mqttAvailabilityTopic; /* Required for the Home Assistant MQTT discovery configuration. */
                    mqttDiscoveryInfo->discoveryInfoFileName = discoveryInfoFileName; /* Required for the Home Assistant MQTT discovery configuration. */
                    mqttDiscoveryInfo->isReqToPublish        = true;                  /* Publish in next process cycle. */

                    m_mqttDiscoveryInfoList.push_back(mqttDiscoveryInfo);
                }
            }
        }
        /* Invalid discovery info. */
        else
        {
            /* Skip. */
            LOG_ERROR("HA extra info invalid.");
        }
    }
}

void HomeAssistantMqtt::unregisterMqttDiscovery(const String& deviceId, const String& entityId, const String& topic)
{
    /* The Home Assistant discovery must be enabled and the prefix must be available, otherwise this
     * feature is disabled.
     */
    if ((true == m_haDiscoveryEnabled) &&
        (false == m_haDiscoveryPrefix.isEmpty()))
    {
        MqttService&                      mqttService               = MqttService::getInstance();
        ListOfMqttDiscoveryInfo::iterator listOfMqttDiscoveryInfoIt = m_mqttDiscoveryInfoList.begin();

        while (m_mqttDiscoveryInfoList.end() != listOfMqttDiscoveryInfoIt)
        {
            MqttDiscoveryInfo* mqttDiscoveryInfo = *listOfMqttDiscoveryInfoIt;

            /* Registration found? */
            if ((nullptr != mqttDiscoveryInfo) &&
                (deviceId == mqttDiscoveryInfo->deviceId) &&
                (entityId == mqttDiscoveryInfo->entityId) &&
                (topic == mqttDiscoveryInfo->topic))
            {
                /* Without any component the discovery info was never published,
                 * e.g. because the discovery info file is invalid or the MQTT
                 * connection was not established yet. There is nothing to purge
                 * and a empty component would result in a invalid topic.
                 */
                if (true == mqttDiscoveryInfo->components.isEmpty())
                {
                    LOG_INFO("No HA discovery published for %s, nothing to purge.", topic.c_str());
                }
                else
                {
                    uint8_t componentCount = getComponentCount(mqttDiscoveryInfo->components);
                    uint8_t idx            = 0U;

                    for (idx = 0U; idx < componentCount; ++idx)
                    {
                        int8_t discoveryEntityIndex = (1U == componentCount) ? -1 : idx;
                        String objectId             = getObjectId(entityId, topic, discoveryEntityIndex);
                        String component;
                        String mqttDiscoveryInfoTopic;

                        getComponentByIndex(component, mqttDiscoveryInfo->components, idx);
                        getConfigTopic(mqttDiscoveryInfoTopic, component, mqttDiscoveryInfo->deviceId, mqttDiscoveryInfo->entityId, mqttDiscoveryInfo->topic, discoveryEntityIndex);

                        LOG_DEBUG("Component: %s", component.c_str());

                        /* Purge retained discovery info. */
                        if (false == mqttService.publish(MqttService::PRIMARY_MQTT_INST, mqttDiscoveryInfoTopic, "", true))
                        {
                            LOG_WARNING("Failed to purge HA discovery of %s.", objectId.c_str());
                        }
                        /* Successful purged. */
                        else
                        {
                            LOG_INFO("Purged HA discovery of %s.", objectId.c_str());
                        }
                    }
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

String HomeAssistantMqtt::getBirthTopic() const
{
    return m_haDiscoveryPrefix + HA_BIRTH_TOPIC_ENDPOINT;
}

void HomeAssistantMqtt::subscribeBirthTopic()
{
    /* Subscribe only once. The MQTT service subscribes the topic again after
     * a broker reconnect on its own.
     */
    if ((false == m_isBirthTopicSubscribed) &&
        (false == m_haDiscoveryPrefix.isEmpty()))
    {
        MqttService&             mqttService = MqttService::getInstance();
        String                   birthTopic  = getBirthTopic();
        MqttTypes::TopicCallback birthCallback =
            [this](const String& mqttTopic, const uint8_t* payload, size_t size) {
                const size_t ONLINE_PAYLOAD_LENGTH = strlen(HA_BIRTH_PAYLOAD_ONLINE);

                UTIL_NOT_USED(mqttTopic);

                if ((nullptr != payload) &&
                    (ONLINE_PAYLOAD_LENGTH == size) &&
                    (0 == memcmp(payload, HA_BIRTH_PAYLOAD_ONLINE, ONLINE_PAYLOAD_LENGTH)))
                {
                    /* Handled in the process() method, because publishing is
                     * throttled there to one message per process cycle.
                     */
                    m_isReqToRepublishAll = true;
                }
            };

        if (false == mqttService.subscribe(MqttService::PRIMARY_MQTT_INST, birthTopic, birthCallback))
        {
            LOG_WARNING("Couldn't subscribe %s.", birthTopic.c_str());
        }
        else
        {
            m_isBirthTopicSubscribed = true;

            LOG_INFO("Subscribe: %s", birthTopic.c_str());
        }
    }
}

void HomeAssistantMqtt::unsubscribeBirthTopic()
{
    if (true == m_isBirthTopicSubscribed)
    {
        MqttService& mqttService = MqttService::getInstance();

        mqttService.unsubscribe(MqttService::PRIMARY_MQTT_INST, getBirthTopic());

        m_isBirthTopicSubscribed = false;
    }
}

void HomeAssistantMqtt::requestToPublishAllDiscoveryInfos()
{
    ListOfMqttDiscoveryInfo::iterator listOfMqttDiscoveryInfoIt = m_mqttDiscoveryInfoList.begin();

    while (m_mqttDiscoveryInfoList.end() != listOfMqttDiscoveryInfoIt)
    {
        MqttDiscoveryInfo* mqttDiscoveryInfo = *listOfMqttDiscoveryInfoIt;

        if (nullptr != mqttDiscoveryInfo)
        {
            mqttDiscoveryInfo->isReqToPublish = true;
        }

        ++listOfMqttDiscoveryInfoIt;
    }
}

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

String HomeAssistantMqtt::getObjectId(const String& entityId, const String& topic, int8_t discoveryEntityIndex)
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

    if (0 <= discoveryEntityIndex)
    {
        objectId += "/";
        objectId += discoveryEntityIndex;
    }

    /* Home Assistant MQTT discovery doesn't allow '/' and '.' in the object id.
     * See https://www.home-assistant.io/integrations/mqtt#discovery-messages
     */
    objectId.replace('/', '_');
    objectId.replace('.', '_');

    return objectId;
}

String HomeAssistantMqtt::getUniqueId(const String& nodeId, const String& objectId)
{
    return nodeId + "/" + objectId;
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

void HomeAssistantMqtt::getConfigTopic(String& haConfigTopic, const String& component, const String& deviceId, const String& entityId, const String& topic, int8_t discoveryEntityIndex)
{
    String nodeId    = getNodeId(deviceId);
    String objectId  = getObjectId(entityId, topic, discoveryEntityIndex);

    haConfigTopic    = m_haDiscoveryPrefix;
    haConfigTopic   += "/";
    haConfigTopic   += component;
    haConfigTopic   += "/";
    haConfigTopic   += nodeId;
    haConfigTopic   += "/";
    haConfigTopic   += objectId;
    haConfigTopic   += "/config";
}

void HomeAssistantMqtt::addDeviceInfo(JsonDocument& jsonDoc, MqttDiscoveryInfo& mqttDiscoveryInfo)
{
    String nodeId = getNodeId(mqttDiscoveryInfo.deviceId);
    String chipIdStr;

    Board::getInstance().getSystemDrv().getChipId(chipIdStr);

    /* Device identifier */
    jsonDoc["dev"]["ids"]  = chipIdStr;
    /* URL to configuration of the device (configuration_url).
     *
     * The discovery info is published retained, therefore the IP address is not
     * used. It would become stale after the device got another one. The host
     * name is resolved via mDNS, which the device announces anyway.
     */
    jsonDoc["dev"]["cu"]   = String("http://") + mqttDiscoveryInfo.deviceId + ".local";
    /* Name of the device. */
    jsonDoc["dev"]["name"] = nodeId;
    /* Device model name (model) */
    jsonDoc["dev"]["mdl"]  = "Pixelix";
    /* Manufacturer (manufacturer) */
    jsonDoc["dev"]["mf"]   = "BlueAndi & Friends";
    /* SW version of the device (sw_version) */
    jsonDoc["dev"]["sw"]   = Version::getSoftwareVersion();
    /* HW version is used for the target name (hw_version). */
    jsonDoc["dev"]["hw"]   = Version::getTargetName();

    /* Network connection of the device (connections). Home Assistant expects
     * a list of [type, identifier] pairs. The MAC address lets Home Assistant
     * recognize the device as the same one which the Pixelix integration
     * reports, so both are merged to a single device.
     */
    if (WIFI_MODE_NULL != WiFi.getMode())
    {
        JsonObject devObj      = jsonDoc["dev"];
        JsonArray  connections = devObj.createNestedArray("cns");
        JsonArray  connection  = connections.createNestedArray();

        connection.add("mac");
        connection.add(WiFi.macAddress());
    }
}

void HomeAssistantMqtt::addOriginInfo(JsonDocument& jsonDoc)
{
    /* Origin name */
    jsonDoc["o"]["name"] = "Pixelix";
    /* Origin URL */
    jsonDoc["o"]["url"]  = "https://github.com/BlueAndi/Pixelix";
}

bool HomeAssistantMqtt::loadDiscoveryInfo(JsonDocument& jsonDoc, const String& fileName)
{
    JsonFile jsonFile(FILESYSTEM);
    bool     isSuccessful = false;

    /* Read the discovery info from file. */
    if (false == jsonFile.load(fileName, jsonDoc))
    {
        LOG_ERROR("Failed to load discovery info from file: %s", fileName.c_str());
    }
    else if (false == jsonDoc.is<JsonArrayConst>())
    {
        LOG_ERROR("Discovery info shall be an array.");
    }
    else
    {
        JsonArrayConst discoveryInfoArray      = jsonDoc.as<JsonArrayConst>();
        size_t         discoveryInfoArrayCount = discoveryInfoArray.size();
        size_t         idx                     = 0U;

        isSuccessful                           = true;

        while ((idx < discoveryInfoArrayCount) && (true == isSuccessful))
        {
            JsonVariantConst discoveryInfoVar = discoveryInfoArray[idx];

            if (false == discoveryInfoVar.is<JsonObjectConst>())
            {
                isSuccessful = false;
            }
            else
            {
                JsonObjectConst  discoveryInfo = discoveryInfoVar.as<JsonObjectConst>();
                JsonVariantConst jsonComponent = discoveryInfo["component"];
                JsonVariantConst jsonDiscovery = discoveryInfo["discovery"];

                if ((false == jsonComponent.is<String>()) ||
                    (false == jsonDiscovery.is<JsonObjectConst>()))
                {
                    isSuccessful = false;
                }
                else
                {
                    String component = jsonComponent.as<const char*>();

                    /* Component shall not be empty and
                     * shall not contain a comma, because it is used as delimiter.
                     */
                    if ((true == component.isEmpty()) ||
                        (0 <= component.indexOf(',')))
                    {
                        isSuccessful = false;
                    }
                }
            }

            if (false == isSuccessful)
            {
                LOG_ERROR("Discovery info element %u invalid.", idx);
            }
            else
            {
                ++idx;
            }
        }
    }

    return isSuccessful;
}

void HomeAssistantMqtt::publishAutoDiscoveryInfo(MqttDiscoveryInfo& mqttDiscoveryInfo)
{
    const size_t        JSON_DISCOVERY_INFO_DOC_SIZE = 1024U;
    DynamicJsonDocument jsonDiscoveryInfoDoc(JSON_DISCOVERY_INFO_DOC_SIZE);

    /* Read the discovery info from file. */
    if (false == loadDiscoveryInfo(jsonDiscoveryInfoDoc, mqttDiscoveryInfo.discoveryInfoFileName))
    {
        /* Skip it. */
        ;
    }
    else
    {
        MqttService&        mqttService   = MqttService::getInstance();
        const size_t        JSON_DOC_SIZE = 2048U;
        DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
        JsonArrayConst      jsonDiscoveryInfoArray  = jsonDiscoveryInfoDoc.as<JsonArrayConst>();
        size_t              discoveryInfoArrayCount = jsonDiscoveryInfoArray.size();
        String              nodeId                  = getNodeId(mqttDiscoveryInfo.deviceId);
        size_t              idx;

        addDeviceInfo(jsonDoc, mqttDiscoveryInfo);
        addOriginInfo(jsonDoc);

        /* Readable topic? */
        if (false == mqttDiscoveryInfo.mqttStateTopic.isEmpty())
        {
            jsonDoc["stat_t"] = mqttDiscoveryInfo.mqttStateTopic;
        }

        /* Writeable topic? */
        if (false == mqttDiscoveryInfo.mqttCmdTopic.isEmpty())
        {
            jsonDoc["cmd_t"] = mqttDiscoveryInfo.mqttCmdTopic;
        }

        /* Availability? */
        if (false == mqttDiscoveryInfo.mqttAvailabilityTopic.isEmpty())
        {
            jsonDoc["avty_t"] = mqttDiscoveryInfo.mqttAvailabilityTopic;
        }

        /* The component list is rebuilt, because this method is called again
         * after Home Assistant reported itself as online. Otherwise the
         * components would be appended a second time.
         */
        mqttDiscoveryInfo.components.clear();

        for (idx = 0U; idx < discoveryInfoArrayCount; ++idx)
        {
            JsonObjectConst discoveryInfo        = jsonDiscoveryInfoArray[idx];
            String          component            = discoveryInfo["component"].as<const char*>();
            JsonObjectConst jsonDiscovery        = discoveryInfo["discovery"];
            int8_t          discoveryEntityIndex = (1U == discoveryInfoArrayCount) ? -1 : idx;
            String          objectId             = getObjectId(mqttDiscoveryInfo.entityId, mqttDiscoveryInfo.topic, discoveryEntityIndex);
            String          uniqueId             = getUniqueId(nodeId, objectId);
            String          mqttDiscoveryTopic;
            String          mqttDiscoveryContent;

            getConfigTopic(mqttDiscoveryTopic, component, mqttDiscoveryInfo.deviceId, mqttDiscoveryInfo.entityId, mqttDiscoveryInfo.topic, discoveryEntityIndex);

            /* The Home Assistant object id (object_id) is used to generate the Home Assistant entity id. */
            jsonDoc["obj_id"]  = objectId;
            /* The Home Assistant unique id (unique_id) identifies the device and its Home Assistant entity.
             * It shall be unique in the Home Assistant entity domain.
             */
            jsonDoc["uniq_id"] = uniqueId;

            /* Copy all discovery details. */
            for (JsonObjectConst::iterator discoveryDetailsIt = jsonDiscovery.begin(); discoveryDetailsIt != jsonDiscovery.end(); ++discoveryDetailsIt)
            {
                /* Replace %ALIAS% in the discovery details with the actual alias. */
                if (0 == strcmp(discoveryDetailsIt->key().c_str(), "name"))
                {
                    /* The alias is the last part of the plugin entity id: display/plugin/<alias> */
                    String discoveryDetailValue = discoveryDetailsIt->value().as<const char*>();
                    String alias                = mqttDiscoveryInfo.entityId;
                    int    lastSlashIndex       = alias.lastIndexOf('/');

                    if (lastSlashIndex >= 0)
                    {
                        alias = alias.substring(lastSlashIndex + 1);
                    }
                    discoveryDetailValue.replace("%ALIAS%", alias);

                    jsonDoc[discoveryDetailsIt->key()] = discoveryDetailValue;
                }
                else
                {
                    jsonDoc[discoveryDetailsIt->key()] = discoveryDetailsIt->value();
                }
            }

            /* Does the discovery info fit into the JSON document? Otherwise it
             * would be silently truncated and the published content would be
             * invalid JSON.
             */
            if (true == jsonDoc.overflowed())
            {
                LOG_ERROR("HA discovery info of %s needs more than %u bytes.", objectId.c_str(), JSON_DOC_SIZE);
            }
            /* Send the JSON as string. */
            else if (0U < serializeJson(jsonDoc, mqttDiscoveryContent))
            {
                /* Escape control characters, otherwise the content would not be
                 * valid JSON anymore, see JsonEscape.hpp.
                 */
                JsonEscape::escapeControlCharacters(mqttDiscoveryContent);

                /* Publish retained to ensure that HomeAssistant will recognize the device entity. */
                if (false == mqttService.publish(MqttService::PRIMARY_MQTT_INST, mqttDiscoveryTopic, mqttDiscoveryContent.c_str(), true))
                {
                    LOG_WARNING("Failed to provide HA discovery info of %s.", objectId.c_str());
                }
                else
                {
                    LOG_INFO("HA discovery info of %s published.", objectId.c_str());
                }
            }
            else
            {
                /* Serialization failed, nothing to publish. */
                ;
            }

            /* Remember the component for unregistration later. */
            if (0U < idx)
            {
                mqttDiscoveryInfo.components += ",";
            }
            mqttDiscoveryInfo.components += component;
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

uint8_t HomeAssistantMqtt::getComponentCount(const String& components)
{
    uint8_t  count  = 1U;
    uint32_t length = components.length();

    for (size_t idx = 0U; idx < length; ++idx)
    {
        if (',' == components[idx])
        {
            ++count;
        }
    }

    return count;
}

void HomeAssistantMqtt::getComponentByIndex(String& component, const String& components, uint8_t idx)
{
    int32_t startIdx = 0;
    int32_t endIdx   = components.indexOf(',');
    uint8_t count    = 1U;

    while ((count <= idx) && (0 <= endIdx))
    {
        startIdx = endIdx + 1;
        endIdx   = components.indexOf(',', startIdx);

        ++count;
    }

    /* The loop above stops at the last component, therefore a index which
     * addresses it or anything behind it is out of bounce.
     */
    if (count <= idx)
    {
        component.clear();
    }
    else if (0 > endIdx)
    {
        component = components.substring(startIdx);
    }
    else
    {
        component = components.substring(startIdx, endIdx);
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
