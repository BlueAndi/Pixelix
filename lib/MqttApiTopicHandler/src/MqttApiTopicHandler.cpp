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

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void MqttApiTopicHandler::registerTopics(IPluginMaintenance* plugin)
{
    if (nullptr != plugin)
    {
        const size_t        JSON_DOC_SIZE   = 512U;
        DynamicJsonDocument topicsDoc(JSON_DOC_SIZE);
        JsonArray           topics          = topicsDoc.createNestedArray("topics");

        /* Get topics from plugin. */
        plugin->getTopics(topics);

        /* Handle each topic */
        if (0U < topics.size())
        {
            String baseUriByUid     = getBaseUriByUid(plugin->getUID());
            String baseUriByAlias;

            if (false == plugin->getAlias().isEmpty())
            {
                baseUriByAlias = getBaseUriByAlias(plugin->getAlias());
            }

            for (JsonVariantConst topic : topics)
            {
                registerTopic(baseUriByUid, plugin, topic.as<String>());

                if (false == baseUriByAlias.isEmpty())
                {
                    registerTopic(baseUriByAlias, plugin, topic.as<String>());
                }
            }
        }
    }
}

/**
 * Unregister all topics of the given plugin.
 * 
 * @param[in] plugin    The plugin, which topics to unregister.
 */
void MqttApiTopicHandler::unregisterTopics(IPluginMaintenance* plugin)
{
    if (nullptr != plugin)
    {
        const size_t        JSON_DOC_SIZE   = 512U;
        DynamicJsonDocument topicsDoc(JSON_DOC_SIZE);
        JsonArray           topics          = topicsDoc.createNestedArray("topics");

        /* Get topics from plugin. */
        plugin->getTopics(topics);

        /* Handle each topic */
        if (0U < topics.size())
        {
            String baseUriByUid     = getBaseUriByUid(plugin->getUID());
            String baseUriByAlias;

            if (false == plugin->getAlias().isEmpty())
            {
                baseUriByAlias = getBaseUriByAlias(plugin->getAlias());
            }

            for (JsonVariantConst topic : topics)
            {
                unregisterTopic(baseUriByUid, plugin, topic.as<String>());

                if (false == baseUriByAlias.isEmpty())
                {
                    unregisterTopic(baseUriByAlias, plugin, topic.as<String>());
                }
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
    
    baseUri += "display";
    baseUri += "/uid/";
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

    baseUri += "display";
    baseUri += "/alias/";
    baseUri += alias;

    return baseUri;
}

void MqttApiTopicHandler::registerTopic(const String& baseUri, IPluginMaintenance* plugin, const String& topic)
{
    if (nullptr != plugin)
    {
        String                      topicUri    = baseUri + topic;
        MqttService&                mqttService = MqttService::getInstance();
        MqttService::TopicCallback  callback    = [this, plugin, topic](const String& topicUri, const uint8_t* payload, size_t size) {
            if (0U != topicUri.endsWith(topic))
            {
                this->write(plugin, topic, payload, size);
            }
        };

        if (false == mqttService.subscribe(topicUri, callback))
        {
            LOG_WARNING("Couldn't subscribe %s.", topicUri.c_str());
        }
        else
        {
            LOG_INFO("[%s][%u] Register: %s", plugin->getName(), plugin->getUID(), topicUri.c_str());
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

void MqttApiTopicHandler::unregisterTopic(const String& baseUri, IPluginMaintenance* plugin, const String& topic)
{
    String          topicUri    = baseUri + topic;
    MqttService&    mqttService = MqttService::getInstance();

    LOG_INFO("[%s][%u] Unregister: %s", plugin->getName(), plugin->getUID(), topicUri.c_str());

    mqttService.unsubscribe(topicUri);
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
