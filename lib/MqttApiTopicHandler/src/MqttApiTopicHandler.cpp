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
 * @brief  MQTT API topic handler
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "MqttApiTopicHandler.h"
#include "FileSystem.h"

#include <Logging.h>
#include <MqttService.h>
#include <SettingsService.h>
#include <mbedtls/base64.h>

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

/* Initialize MQTT path endpoint for read access. */
const char* MqttApiTopicHandler::MQTT_ENDPOINT_READ_ACCESS  = "/state";

/* Initialize MQTT path endpoint for write access. */
const char* MqttApiTopicHandler::MQTT_ENDPOINT_WRITE_ACCESS = "/set";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void MqttApiTopicHandler::start()
{
    m_haExtension.start();
}

void MqttApiTopicHandler::stop()
{
    m_haExtension.stop();
}

void MqttApiTopicHandler::registerTopic(IPluginMaintenance* plugin, const String& topic, Access access, JsonObjectConst& extra)
{
    if ((nullptr != plugin) &&
        (false == topic.isEmpty()))
    {
        String baseUri;

        /* If plugin has no alias, use the plugin UID for the base URI otherwise use the alias. */
        if (false == plugin->getAlias().isEmpty())
        {
            baseUri = getBaseUriByAlias(plugin->getAlias());
        }
        else
        {
            baseUri = getBaseUriByUid(plugin->getUID());
        }

        registerTopic(plugin, topic, access, extra, baseUri);
    }
}

void MqttApiTopicHandler::unregisterTopic(IPluginMaintenance* plugin, const String& topic)
{
    if ((nullptr != plugin) &&
        (false == topic.isEmpty()))
    {
        String baseUri;

        /* If plugin has no alias, use the plugin UID for the base URI otherwise use the alias. */
        if (false == plugin->getAlias().isEmpty())
        {
            baseUri = getBaseUriByAlias(plugin->getAlias());
        }
        else
        {
            baseUri = getBaseUriByUid(plugin->getUID());
        }

        unregisterTopic(plugin, topic, baseUri);
    }
}

void MqttApiTopicHandler::process()
{
    MqttService&                mqttService     = MqttService::getInstance();
    bool                        publishAll      = false;
    ListOfTopicStates::iterator topicStateIt    = m_listOfTopicStates.begin();

    /* If connection to MQTT broker is the first time established or reconnected,
     * all topics will be published to be up-to-date.
     */
    if ((false == m_isMqttConnected) &&
        (MqttService::STATE_CONNECTED == mqttService.getState()))

    {
        m_isMqttConnected = true;
        
        /* Publish after connection establishment. */
        publishAll = true;
    }
    else if ((true == m_isMqttConnected) &&
             (MqttService::STATE_CONNECTED != mqttService.getState()))
    {
        m_isMqttConnected = false;
    }
    else
    {
        ;
    }

    /* If necessary, the topic state will be published. */
    while(m_listOfTopicStates.end() != topicStateIt)
    {
        TopicState* topicState = *topicStateIt;

        if ((nullptr != topicState) &&
            (nullptr != topicState->plugin) &&
            (
                (ACCESS_READ_ONLY == topicState->access) ||
                (ACCESS_READ_WRITE == topicState->access)
            ))
        {
            if ((true == publishAll) ||
                (true == topicState->isPublishReq))
            {
                publish(topicState->topicUri, topicState->plugin, topicState->topic);

                topicState->isPublishReq = false;
            }
        }

        ++topicStateIt;
    }

    /* Process Home Assistant extension. */
    m_haExtension.process(m_isMqttConnected);
}

void MqttApiTopicHandler::notify(IPluginMaintenance* plugin, const String& topic)
{
    if ((nullptr != plugin) &&
        (false == topic.isEmpty()))
    {
        ListOfTopicStates::iterator topicStateIt = m_listOfTopicStates.begin();

        while(m_listOfTopicStates.end() != topicStateIt)
        {
            TopicState* topicState = *topicStateIt;

            if ((nullptr != topicState) &&
                (plugin == topicState->plugin) &&
                (topic == topicState->topic))
            {
                topicState->isPublishReq = true;
            }

            ++topicStateIt;
        }
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

String MqttApiTopicHandler::getBaseUriByUid(uint16_t uid)
{
    String  baseUri;

    if (true == m_hostname.isEmpty())
    {
        SettingsService& settingsService = SettingsService::getInstance();

        if (false == settingsService.open(true))
        {
            m_hostname = settingsService.getHostname().getDefault();
        }
        else
        {
            m_hostname = settingsService.getHostname().getValue();
            settingsService.close();
        }
    }

    if (false == m_hostname.isEmpty())
    {
        baseUri += m_hostname;
        baseUri += "/";
    }
    
    baseUri += "uid/";
    baseUri += uid;

    return baseUri;
}

String MqttApiTopicHandler::getBaseUriByAlias(const String& alias)
{
    String  baseUri;

    if (true == m_hostname.isEmpty())
    {
        SettingsService& settingsService = SettingsService::getInstance();

        if (false == settingsService.open(true))
        {
            m_hostname = settingsService.getHostname().getDefault();
        }
        else
        {
            m_hostname = settingsService.getHostname().getValue();
            settingsService.close();
        }
    }

    if (false == m_hostname.isEmpty())
    {
        baseUri += m_hostname;
        baseUri += "/";
    }

    baseUri += "alias/";
    baseUri += alias;

    return baseUri;
}

void MqttApiTopicHandler::registerTopic(IPluginMaintenance* plugin, const String& topic, Access access, JsonObjectConst& extra, const String& baseUri)
{
    if (nullptr != plugin)
    {
        String      topicUri    = baseUri + topic;
        TopicState* topicState  = new(std::nothrow) TopicState();

        LOG_INFO("[%s][%u] Register: %s", plugin->getName(), plugin->getUID(), topicUri.c_str());

        if (nullptr != topicState)
        {
            String  topicUriReadable;
            String  topicUriWriteable;

            topicState->plugin          = plugin;
            topicState->topic           = topic;
            topicState->access          = access;
            topicState->topicUri        = topicUri;
            topicState->isPublishReq    = false;

            /* Is the topic readable? */
            if ((ACCESS_READ_ONLY == access) ||
                (ACCESS_READ_WRITE == access))
            {
                topicUriReadable = topicUri + MQTT_ENDPOINT_READ_ACCESS;

                /* Publish initially. */
                topicState->isPublishReq = true;
            }

            /* Is the topic writeable? */
            if ((ACCESS_READ_WRITE == access) ||
                (ACCESS_WRITE_ONLY == access))
            {
                MqttService&                mqttService = MqttService::getInstance();
                MqttService::TopicCallback  setCallback = [this, plugin, topic](const String& topicUri, const uint8_t* payload, size_t size) {
                    if (0U != topicUri.endsWith(topic + MQTT_ENDPOINT_WRITE_ACCESS))
                    {
                        this->write(plugin, topic, payload, size);
                    }
                };

                topicUriWriteable = topicUri + MQTT_ENDPOINT_WRITE_ACCESS;

                if (false == mqttService.subscribe(topicUriWriteable, setCallback))
                {
                    LOG_WARNING("Couldn't subscribe %s.", topicUriWriteable.c_str());
                }
                else
                {
                    LOG_INFO("[%u] Subscribed: %s", plugin->getUID(), topicUriWriteable.c_str());
                }
            }

            /* Handle Home Assistant extension */
            {
                int dividerIdx  = baseUri.lastIndexOf("/");

                if (0 <= dividerIdx)
                {
                    String  haObjectId   = baseUri.substring(dividerIdx + 1);
                    String  willTopic    = m_hostname + "/status";

                    m_haExtension.registerMqttDiscovery(m_hostname, haObjectId, topicUriReadable, topicUriWriteable, willTopic, extra);
                }
            }

            m_listOfTopicStates.push_back(topicState);
        }
    }
}

void MqttApiTopicHandler::write(IPluginMaintenance* plugin, const String& topic, const uint8_t* payload, size_t size)
{
    const size_t            JSON_DOC_SIZE   = 1024U;
    DynamicJsonDocument     jsonDoc(JSON_DOC_SIZE);
    DeserializationError    error           = deserializeJson(jsonDoc, payload, size);

    if (DeserializationError::Ok != error)
    {
        LOG_WARNING("Received invalid payload.");
    }
    else
    {
        JsonVariantConst jsonFileName   = jsonDoc["fileName"];
        JsonVariantConst jsonFileBase64 = jsonDoc["file"];

        /* File transfer? */
        if ((true == jsonFileName.is<String>()) &&
            (true == jsonFileBase64.is<String>()))
        {
            String dstFullPath;

            /* Ask plugin, whether the upload is allowed or not. */
            if (false == plugin->isUploadAccepted(topic, jsonFileName.as<String>(), dstFullPath))
            {
                LOG_WARNING("[%s][%u] Upload not supported.", plugin->getName(), plugin->getUID());
            }
            else
            {
                String  fileBase64  = jsonFileBase64.as<String>();
                size_t  fileSize    = 0U;
                int32_t decodeRet   = mbedtls_base64_decode(nullptr, 0U, &fileSize, reinterpret_cast<const unsigned char*>(fileBase64.c_str()), fileBase64.length());

                if (MBEDTLS_ERR_BASE64_INVALID_CHARACTER == decodeRet)
                {
                    LOG_WARNING("[%s][%u] File encoding contains invalid character.", plugin->getName(), plugin->getUID(), fileSize);
                }
                else if ((MAX_FILE_SIZE < fileSize) ||
                         (0U == fileSize))
                {
                    LOG_WARNING("[%s][%u] File size %u not supported.", plugin->getName(), plugin->getUID(), fileSize);
                }
                else
                {
                    uint8_t* buffer = new(std::nothrow) uint8_t[fileSize];

                    if (nullptr != buffer)
                    {
                        File fd;

                        decodeRet = mbedtls_base64_decode(buffer, fileSize, &fileSize, reinterpret_cast<const unsigned char*>(fileBase64.c_str()), fileBase64.length());

                        if (0U != decodeRet)
                        {
                            LOG_WARNING("[%s][%u] File decode error: %d", plugin->getName(), plugin->getUID(), decodeRet);
                        }
                        else
                        {
                            /* Create a new file and overwrite a existing one. */
                            fd = FILESYSTEM.open(dstFullPath, "w");

                            if (false == fd)
                            {
                                LOG_ERROR("Couldn't create file: %s", dstFullPath.c_str());
                            }
                            else
                            {
                                (void)fd.write(buffer, fileSize);
                                fd.close();

                                jsonDoc["fullPath"] = dstFullPath;
                            }
                        }

                        delete[] buffer;
                    }
                }

                jsonDoc.remove("fileName");
                jsonDoc.remove("file");
            }
        }

        if (false == plugin->setTopic(topic, jsonDoc.as<JsonObject>()))
        {
            LOG_WARNING("Plugin %u rejected payload.", plugin->getUID());
        }
    }
}

void MqttApiTopicHandler::unregisterTopic(IPluginMaintenance* plugin, const String& topic, const String& baseUri)
{
    if (nullptr != plugin)
    {
        String                      topicUri        = baseUri + topic;
        MqttService&                mqttService     = MqttService::getInstance();
        ListOfTopicStates::iterator topicStateIt    = m_listOfTopicStates.begin();

        LOG_INFO("[%s][%u] Unregister: %s", plugin->getName(), plugin->getUID(), topicUri.c_str());

        while(m_listOfTopicStates.end() != topicStateIt)
        {
            TopicState* topicState = *topicStateIt;

            if ((nullptr != topicState) &&
                (plugin == topicState->plugin) &&
                (topic == topicState->topic))
            {
                String topicUriReadable;
                String topicUriWriteable;

                if ((ACCESS_READ_ONLY == topicState->access) ||
                    (ACCESS_READ_WRITE == topicState->access))
                {
                    topicUriReadable = topicUri + MQTT_ENDPOINT_READ_ACCESS;

                    /* Purge topic */
                    if (false == mqttService.publish(topicUriReadable, ""))
                    {
                        LOG_WARNING("[%u] Failed to purge: %s", plugin->getUID(), topicUriReadable.c_str());
                    }
                    else
                    {
                        LOG_INFO("[%u] Purged: %s", plugin->getUID(), topicUriReadable.c_str());
                    }
                }
                
                if ((ACCESS_READ_WRITE == topicState->access) ||
                    (ACCESS_WRITE_ONLY == topicState->access))
                {
                    topicUriWriteable = topicUri + MQTT_ENDPOINT_WRITE_ACCESS;

                    mqttService.unsubscribe(topicUriWriteable);

                    LOG_INFO("[%u] Unsubscribed: %s", plugin->getUID(), topicUriWriteable.c_str());
                }

                /* Handle Home Assistant extension */
                {
                    /* The object id is the last directory of the base URI.
                    * Its the plugin UID or the plugin alias.
                    */
                    int dividerIdx  = baseUri.lastIndexOf("/");

                    if (0 <= dividerIdx)
                    {
                        String haObjectId = baseUri.substring(dividerIdx + 1);

                        m_haExtension.unregisterMqttDiscovery(m_hostname, haObjectId, topicUriReadable, topicUriWriteable);
                    }
                }

                topicStateIt = m_listOfTopicStates.erase(topicStateIt);

                delete topicState;
                topicState = nullptr;
            }
            else
            {
                ++topicStateIt;
            }
        }
    }
}

void MqttApiTopicHandler::publish(const String& baseUri, IPluginMaintenance* plugin, const String& topic)
{
    if (nullptr != plugin)
    {
        const size_t        JSON_DOC_SIZE   = 1024U;
        DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
        JsonObject          jsonObj         = jsonDoc.createNestedObject("data");

        if (true == plugin->getTopic(topic, jsonObj))
        {
            String topicContent;

            if (0U < serializeJson(jsonDoc["data"], topicContent))
            {
                MqttService&    mqttService     = MqttService::getInstance();
                String          topicStateUri   = baseUri + MQTT_ENDPOINT_READ_ACCESS;

                if (false == mqttService.publish(topicStateUri, topicContent))
                {
                    LOG_WARNING("Couldn't publish %s.", topicStateUri.c_str());
                }
                else
                {
                    LOG_INFO("[%s][%u] Published: %s", plugin->getName(), plugin->getUID(), topicStateUri.c_str());
                }
            }
        }
    }
}

void MqttApiTopicHandler::clearTopicStates()
{
    MqttService&                mqttService     = MqttService::getInstance();
    ListOfTopicStates::iterator topicStateIt    = m_listOfTopicStates.begin();

    while(m_listOfTopicStates.end() != topicStateIt)
    {
        TopicState* topicState = *topicStateIt;

        if (nullptr != topicState)
        {
            if ((ACCESS_READ_WRITE == topicState->access) ||
                (ACCESS_WRITE_ONLY == topicState->access))
            {
                String topicUri = topicState->topicUri + MQTT_ENDPOINT_WRITE_ACCESS;

                mqttService.unsubscribe(topicUri);
            }

            topicStateIt = m_listOfTopicStates.erase(topicStateIt);

            delete topicState;
            topicState = nullptr;
        }
        else
        {
            ++topicStateIt;
        }
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
