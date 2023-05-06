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
            JsonVariantConst jsonComponent = jsonHomeAssistant["component"];

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

void HomeAssistantMqtt::publishAutoDiscoveryInfo()
{
    const size_t                        JSON_DOC_SIZE               = 1024U;
    DynamicJsonDocument                 jsonDoc(JSON_DOC_SIZE);
    ListOfMqttDiscoveryInfo::iterator   listOfMqttDiscoveryInfoIt   = m_mqttDiscoveryInfoList.begin();
    MqttService&                        mqttService                 = MqttService::getInstance();

    while(m_mqttDiscoveryInfoList.end() != listOfMqttDiscoveryInfoIt)
    {
        MqttDiscoveryInfo*  mqttDiscoveryInfo   = *listOfMqttDiscoveryInfoIt;
        String              mqttTopic           = m_haDiscoveryPrefix;
        String              discoveryInfo;

        mqttTopic += "/";
        mqttTopic += mqttDiscoveryInfo->component;
        mqttTopic += "/";
        mqttTopic += mqttDiscoveryInfo->nodeId;
        mqttTopic += "/";
        mqttTopic += mqttDiscoveryInfo->objectId;
        mqttTopic += "/config";

        jsonDoc.clear();

        jsonDoc["name"]         = mqttDiscoveryInfo->nodeId;
        jsonDoc["device_class"] = "display";

        if (false == mqttDiscoveryInfo->stateTopic.isEmpty())
        {
            jsonDoc["state_topic"] = mqttDiscoveryInfo->stateTopic;
        }

        if (false == mqttDiscoveryInfo->commandTopic.isEmpty())
        {
            jsonDoc["command_topic"] = mqttDiscoveryInfo->commandTopic;
        }

        if (0U < serializeJson(jsonDoc, discoveryInfo))
        {
            if (false == mqttService.publish(mqttTopic, discoveryInfo))
            {
                LOG_WARNING("[%s] Failed to provide HA discovery info.", mqttDiscoveryInfo->objectId.c_str());
            }
            else
            {
                LOG_INFO("[%s] HA discovery info published.", mqttDiscoveryInfo->objectId.c_str());
            }
        }

        ++listOfMqttDiscoveryInfoIt;
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

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
