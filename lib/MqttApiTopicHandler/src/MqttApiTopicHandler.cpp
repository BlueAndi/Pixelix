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

void MqttApiTopicHandler::registerTopic(const String& deviceId, const String& entityId, const String& topic, JsonObjectConst& extra, GetTopicFunc getTopicFunc, SetTopicFunc setTopicFunc, UploadReqFunc uploadReqFunc)
{
    if ((false == deviceId.isEmpty()) &&
        (false == entityId.isEmpty()) &&
        (false == topic.isEmpty()))
    {
        String      mqttTopicNameBase   = deviceId + "/" + entityId + topic;
        TopicState* topicState          = new(std::nothrow) TopicState();

        LOG_INFO("Register: %s", mqttTopicNameBase.c_str());

        if (nullptr != topicState)
        {
            String  topicUriReadable;
            String  topicUriWriteable;

            topicState->deviceId        = deviceId;
            topicState->entityId        = entityId;
            topicState->topic           = topic;
            topicState->getTopicFunc    = getTopicFunc;
            topicState->setTopicFunc    = setTopicFunc;
            topicState->uploadReqFunc   = uploadReqFunc;
            topicState->isPublishReq    = false;

            /* Is the topic readable? */
            if (nullptr != getTopicFunc)
            {
                topicUriReadable = mqttTopicNameBase + MQTT_ENDPOINT_READ_ACCESS;

                /* Publish initially. */
                topicState->isPublishReq = true;
            }

            /* Is the topic writeable? */
            if (nullptr != setTopicFunc)
            {
                MqttService&                mqttService = MqttService::getInstance();
                MqttService::TopicCallback  setCallback =
                    [this, topicState](const String& mqttTopic, const uint8_t* payload, size_t size)
                    {
                        if (0U != mqttTopic.endsWith(topicState->topic + MQTT_ENDPOINT_WRITE_ACCESS))
                        {
                            this->write(topicState->deviceId, topicState->entityId, topicState->topic, payload, size, topicState->setTopicFunc, topicState->uploadReqFunc);
                        }
                    };

                topicUriWriteable = mqttTopicNameBase + MQTT_ENDPOINT_WRITE_ACCESS;

                LOG_INFO("Subscribe: %s", topicUriWriteable.c_str());
                if (false == mqttService.subscribe(topicUriWriteable, setCallback))
                {
                    LOG_WARNING("Couldn't subscribe %s.", topicUriWriteable.c_str());
                }
            }

            /* Handle Home Assistant extension */
            {
                String willTopic = deviceId + "/status";

                m_haExtension.registerMqttDiscovery(deviceId, entityId, topicUriReadable, topicUriWriteable, willTopic, extra);
            }

            m_listOfTopicStates.push_back(topicState);
        }
    }
}

void MqttApiTopicHandler::unregisterTopic(const String& deviceId, const String& entityId, const String& topic)
{
    if ((false == deviceId.isEmpty()) &&
        (false == entityId.isEmpty()) &&
        (false == topic.isEmpty()))
    {
        String                      mqttTopicNameBase   = deviceId + "/" + entityId + topic;
        MqttService&                mqttService         = MqttService::getInstance();
        ListOfTopicStates::iterator topicStateIt        = m_listOfTopicStates.begin();

        LOG_INFO("Unregister: %s", mqttTopicNameBase.c_str());

        while(m_listOfTopicStates.end() != topicStateIt)
        {
            TopicState* topicState = *topicStateIt;

            if ((nullptr != topicState) &&
                (deviceId == topicState->deviceId) &&
                (entityId == topicState->entityId) &&
                (topic == topicState->topic))
            {
                String topicUriReadable;
                String topicUriWriteable;

                if (nullptr != topicState->getTopicFunc)
                {
                    topicUriReadable = mqttTopicNameBase + MQTT_ENDPOINT_READ_ACCESS;

                    /* Purge topic */
                    if (false == mqttService.publish(topicUriReadable, ""))
                    {
                        LOG_WARNING("Failed to purge: %s", topicUriReadable.c_str());
                    }
                    else
                    {
                        LOG_INFO("Purged: %s", topicUriReadable.c_str());
                    }
                }
                
                if (nullptr != topicState->setTopicFunc)
                {
                    topicUriWriteable = mqttTopicNameBase + MQTT_ENDPOINT_WRITE_ACCESS;

                    LOG_INFO("Unsubscribe: %s", topicUriWriteable.c_str());

                    mqttService.unsubscribe(topicUriWriteable);
                }

                /* Handle Home Assistant extension */
                m_haExtension.unregisterMqttDiscovery(deviceId, entityId, topicUriReadable, topicUriWriteable);

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

void MqttApiTopicHandler::process()
{
    MqttService& mqttService = MqttService::getInstance();
    
    /* If connection to MQTT broker is the first time established or reconnected,
     * all topics will be published to be up-to-date.
     */
    if ((false == m_isMqttConnected) &&
        (MqttService::STATE_CONNECTED == mqttService.getState()))

    {
        m_isMqttConnected = true;
        
        /* Publish after connection establishment. */
        requestToPublishAllTopicStates();
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

    if (true == m_isMqttConnected)
    {
        /* If necessary, a topic state will be published.
         *
         * Don't publish all of them at once, only one per process cycle.
         * This has the advantage to detect lost MQTT connection, because remember
         * its cooperative! As long as the MQTT service is not called, no update
         * about the connection status will appear.
         */
        publishTopicStatesOnDemand();
    }

    /* Process Home Assistant extension. */
    m_haExtension.process(m_isMqttConnected);
}

void MqttApiTopicHandler::notify(const String& deviceId, const String& entityId, const String& topic)
{
    if ((false == deviceId.isEmpty()) &&
        (false == entityId.isEmpty()) &&
        (false == topic.isEmpty()))
    {
        ListOfTopicStates::iterator topicStateIt = m_listOfTopicStates.begin();

        while(m_listOfTopicStates.end() != topicStateIt)
        {
            TopicState* topicState = *topicStateIt;

            if ((nullptr != topicState) &&
                (deviceId == topicState->deviceId) &&
                (entityId == topicState->entityId) &&
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

void MqttApiTopicHandler::requestToPublishAllTopicStates()
{
    ListOfTopicStates::iterator topicStateIt = m_listOfTopicStates.begin();

    /* Set the publish request flag for all topic states. */
    while(m_listOfTopicStates.end() != topicStateIt)
    {
        TopicState* topicState = *topicStateIt;

        if ((nullptr != topicState) &&
            (false == topicState->deviceId.isEmpty()) &&
            (false == topicState->entityId.isEmpty()) &&
            (nullptr != topicState->getTopicFunc))
        {
            topicState->isPublishReq = true;
        }

        ++topicStateIt;
    }
}

void MqttApiTopicHandler::publishTopicStatesOnDemand()
{
    ListOfTopicStates::iterator topicStateIt = m_listOfTopicStates.begin();

    while(m_listOfTopicStates.end() != topicStateIt)
    {
        TopicState* topicState = *topicStateIt;

        if ((nullptr != topicState) &&
            (false == topicState->deviceId.isEmpty()) &&
            (false == topicState->entityId.isEmpty()) &&
            (nullptr != topicState->getTopicFunc))
        {
            if (true == topicState->isPublishReq)
            {
                publish(topicState->deviceId, topicState->entityId, topicState->topic, topicState->getTopicFunc);

                topicState->isPublishReq = false;

                /* Continue next process cycle. */
                break;
            }
        }

        ++topicStateIt;
    }
}

void MqttApiTopicHandler::write(const String& deviceId, const String& entityId, const String& topic, const uint8_t* payload, size_t size, SetTopicFunc setTopicFunc, UploadReqFunc uploadReqFunc)
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
        JsonVariantConst    jsonFileName   = jsonDoc["fileName"];
        JsonVariantConst    jsonFileBase64 = jsonDoc["file"];

        /* File transfer? */
        if ((true == jsonFileName.is<String>()) &&
            (true == jsonFileBase64.is<String>()))
        {
            String dstFullPath;

            /* Ask plugin, whether the upload is allowed or not. */
            if ((nullptr == uploadReqFunc) ||
                (false == uploadReqFunc(topic, jsonFileName.as<String>(), dstFullPath)))
            {
                LOG_WARNING("Upload not supported  by %s.", entityId.c_str());
            }
            else
            {
                String  fileBase64  = jsonFileBase64.as<String>();
                size_t  fileSize    = 0U;
                int32_t decodeRet   = mbedtls_base64_decode(nullptr, 0U, &fileSize, reinterpret_cast<const unsigned char*>(fileBase64.c_str()), fileBase64.length());

                if (MBEDTLS_ERR_BASE64_INVALID_CHARACTER == decodeRet)
                {
                    LOG_WARNING("File encoding contains invalid character.");
                }
                else if ((MAX_FILE_SIZE < fileSize) ||
                         (0U == fileSize))
                {
                    LOG_WARNING("File size %u not supported.", fileSize);
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
                            LOG_WARNING("File decode error: %d", decodeRet);
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

        if (false == setTopicFunc(topic, jsonDoc.as<JsonObjectConst>()))
        {
            LOG_WARNING("Payload rejected by %s.", entityId.c_str());
        }
    }
}

void MqttApiTopicHandler::publish(const String& deviceId, const String& entityId, const String& topic, GetTopicFunc getTopicFunc)
{
    if (nullptr != getTopicFunc)
    {
        const size_t        JSON_DOC_SIZE       = 1024U;
        DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
        JsonObject          jsonObj             = jsonDoc.createNestedObject("data");
        String              mqttTopicNameBase   = deviceId + "/" + entityId + topic;

        if (true == getTopicFunc(topic, jsonObj))
        {
            String topicContent;

            if (0U < serializeJson(jsonDoc["data"], topicContent))
            {
                MqttService&    mqttService     = MqttService::getInstance();
                String          topicStateUri   = mqttTopicNameBase + MQTT_ENDPOINT_READ_ACCESS;

                if (false == mqttService.publish(topicStateUri, topicContent))
                {
                    LOG_WARNING("Couldn't publish %s.", topicStateUri.c_str());
                }
                else
                {
                    LOG_INFO("Published: %s", topicStateUri.c_str());
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
            if (nullptr != topicState->setTopicFunc)
            {
                String mqttTopicNameBase    = topicState->deviceId + "/" + topicState->entityId + topicState->topic;
                String topicStateUri        = mqttTopicNameBase + MQTT_ENDPOINT_WRITE_ACCESS;

                mqttService.unsubscribe(topicStateUri);
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
