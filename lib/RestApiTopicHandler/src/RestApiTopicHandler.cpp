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
 * @brief  REST API topic handler
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "RestApiTopicHandler.h"
#include "FileSystem.h"
#include "MyWebServer.h"
#include "HttpStatus.h"
#include "RestApi.h"
#include "RestUtil.h"

#include <Logging.h>
#include <Util.h>

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

void RestApiTopicHandler::registerTopics(IPluginMaintenance* plugin)
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
            PluginObjData*  metaData = new(std::nothrow) PluginObjData();

            if (nullptr != metaData)
            {
                String baseUriByUid     = getBaseUriByUid(plugin->getUID());
                String baseUriByAlias;

                if (false == plugin->getAlias().isEmpty())
                {
                    baseUriByAlias = getBaseUriByAlias(plugin->getAlias());
                }

                metaData->plugin = plugin;

                for (JsonVariantConst topic : topics)
                {
                    registerTopic(baseUriByUid, metaData, topic.as<String>());

                    if (false == baseUriByAlias.isEmpty())
                    {
                        registerTopic(baseUriByAlias, metaData, topic.as<String>());
                    }
                }

                m_pluginMeta.push_back(metaData);
            }
        }
    }
}

void RestApiTopicHandler::unregisterTopics(IPluginMaintenance* plugin)
{
    if (nullptr != plugin)
    {
        PluginObjDataList::iterator pluginMetaIt = m_pluginMeta.begin();

        /* Walk through plugin meta and remove every topic.
         * At the end, destroy the meta information.
         */
        while(m_pluginMeta.end() != pluginMetaIt)
        {
            PluginObjData* pluginMeta = *pluginMetaIt;

            if (plugin == pluginMeta->plugin)
            {
                WebHandlerDataList::iterator webHandlerDataIt = pluginMeta->webHandlers.begin();

                while(pluginMeta->webHandlers.end() != webHandlerDataIt)
                {
                    WebHandlerData* webHandlerData = *webHandlerDataIt;

                    if (nullptr != webHandlerData->webHandler)
                    {
                        LOG_INFO("[%s][%u] Unregister: %s", pluginMeta->plugin->getName(), pluginMeta->plugin->getUID(), webHandlerData->uri.c_str());

                        if (false == MyWebServer::getInstance().removeHandler(webHandlerData->webHandler))
                        {
                            LOG_WARNING("Couldn't remove handler %s.", webHandlerData->uri.c_str());
                        }

                        webHandlerDataIt = pluginMeta->webHandlers.erase(webHandlerDataIt);
                        delete webHandlerData;
                    }
                    else
                    {
                        ++webHandlerDataIt;
                    }
                }

                pluginMetaIt = m_pluginMeta.erase(pluginMetaIt);
                delete pluginMeta;
            }
            else
            {
                ++pluginMetaIt;
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

String RestApiTopicHandler::getBaseUriByUid(uint16_t uid)
{
    String  baseUri = RestApi::BASE_URI;
    baseUri += "/display";
    baseUri += "/uid/";
    baseUri += uid;

    return baseUri;
}

String RestApiTopicHandler::getBaseUriByAlias(const String& alias)
{
    String  baseUri = RestApi::BASE_URI;
    baseUri += "/display";
    baseUri += "/alias/";
    baseUri += alias;

    return baseUri;
}

void RestApiTopicHandler::registerTopic(const String& baseUri, PluginObjData* metaData, const String& topic)
{
    if ((nullptr != metaData) &&
        (nullptr != metaData->plugin))
    {
        WebHandlerData* webHandlerData = new(std::nothrow) WebHandlerData;

        if (nullptr == webHandlerData)
        {
            LOG_ERROR("[%s][%u] Couldn't allocate web handler data.", metaData->plugin->getName(), metaData->plugin->getUID());
        }
        else
        {
            String              topicUri    = baseUri + topic;
            IPluginMaintenance* plugin      = metaData->plugin;

            LOG_INFO("[%s][%u] Register: %s", metaData->plugin->getName(), metaData->plugin->getUID(), topicUri.c_str());

            webHandlerData->webHandler  = &MyWebServer::getInstance().on(
                                            topicUri.c_str(),
                                            HTTP_ANY,
                                            [this, plugin, topic, webHandlerData](AsyncWebServerRequest *request)
                                            {
                                                this->webReqHandler(request, plugin, topic, webHandlerData);
                                            },
                                            [this, plugin, topic, webHandlerData](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final)
                                            {
                                                this->uploadHandler(request, filename, index, data, len, final, plugin, topic, webHandlerData);
                                            });
            webHandlerData->uri         = topicUri;

            metaData->webHandlers.push_back(webHandlerData);
        }
    }
}

void RestApiTopicHandler::webReqHandler(AsyncWebServerRequest *request, IPluginMaintenance* plugin, const String& topic, WebHandlerData* webHandlerData)
{
    String              content;
    const size_t        JSON_DOC_SIZE   = 1024U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    JsonObject          dataObj         = jsonDoc.createNestedObject("data");
    uint32_t            httpStatusCode  = HttpStatus::STATUS_CODE_OK;

    if ((nullptr == request) ||
        (nullptr == plugin) ||
        (nullptr == webHandlerData))
    {
        return;
    }

    if (HTTP_GET == request->method())
    {
        if (false == plugin->getTopic(topic, dataObj))
        {
            RestUtil::prepareRspError(jsonDoc, "Requested topic not supported.");

            jsonDoc.remove("data");

            httpStatusCode = HttpStatus::STATUS_CODE_NOT_FOUND;
        }
        else
        {
            jsonDoc["status"]   = "ok";
            httpStatusCode      = HttpStatus::STATUS_CODE_OK;
        }
    }
    else if (HTTP_POST == request->method())
    {
        DynamicJsonDocument jsonDocPar(JSON_DOC_SIZE);
        size_t              idx = 0U;

        /* Add arguments */
        for(idx = 0U; idx < request->args(); ++idx)
        {
            jsonDocPar[request->argName(idx)] = request->arg(idx);
        }

        /* Add uploaded file */
        if ((false == webHandlerData->isUploadError) &&
            (false == webHandlerData->fullPath.isEmpty()))
        {
            jsonDocPar["fullPath"] = webHandlerData->fullPath;
        }

        if (false == plugin->setTopic(topic, jsonDocPar.as<JsonObject>()))
        {
            RestUtil::prepareRspError(jsonDoc, "Requested topic not supported or invalid data.");

            jsonDoc.remove("data");

            /* If a file is available, it will be removed now. */
            if (false == webHandlerData->fullPath.isEmpty())
            {
                (void)FILESYSTEM.remove(webHandlerData->fullPath);
            }

            httpStatusCode = HttpStatus::STATUS_CODE_NOT_FOUND;
        }
        else
        {
            jsonDoc["status"]   = "ok";
            httpStatusCode      = HttpStatus::STATUS_CODE_OK;
        }
    }
    else
    {
        RestUtil::prepareRspErrorHttpMethodNotSupported(jsonDoc);

        jsonDoc.remove("data");

        httpStatusCode = HttpStatus::STATUS_CODE_NOT_FOUND;
    }

    RestUtil::sendJsonRsp(request, jsonDoc, httpStatusCode);

    return;
}

void RestApiTopicHandler::uploadHandler(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final, IPluginMaintenance* plugin, const String& topic, WebHandlerData* webHandlerData)
{
    /* Begin of upload? */
    if (0 == index)
    {
        AsyncWebHeader* headerXFileSize = request->getHeader("X-File-Size");
        size_t          fileSize        = request->contentLength();
        size_t          fileSystemSpace = FILESYSTEM.totalBytes() - FILESYSTEM.usedBytes();

        /* File size available? */
        if (nullptr != headerXFileSize)
        {
            uint32_t u32FileSize = 0U;

            if (true == Util::strToUInt32(headerXFileSize->value(), u32FileSize))
            {
                fileSize = u32FileSize;
            }
        }

        if (fileSystemSpace <= fileSize)
        {
            LOG_WARNING("Upload of %s aborted. Not enough space.", filename.c_str());
            webHandlerData->isUploadError = true;
            webHandlerData->fullPath.clear();
        }
        else
        {
            LOG_INFO("Upload of %s (%d bytes) starts.", filename.c_str(), fileSize);
            webHandlerData->isUploadError = false;
            webHandlerData->fullPath.clear();

            /* Ask plugin, whether the upload is allowed or not. */
            if (false == plugin->isUploadAccepted(topic, filename, webHandlerData->fullPath))
            {
                LOG_WARNING("[%s][%u] Upload not supported.", plugin->getName(), plugin->getUID());
                webHandlerData->isUploadError = true;
                webHandlerData->fullPath.clear();
            }
            else
            {
                /* Create a new file and overwrite a existing one. */
                webHandlerData->fd = FILESYSTEM.open(webHandlerData->fullPath, "w");

                if (false == webHandlerData->fd)
                {
                    LOG_ERROR("Couldn't create file: %s", webHandlerData->fullPath.c_str());
                    webHandlerData->isUploadError = true;
                    webHandlerData->fullPath.clear();
                }
            }
        }
    }

    if (false == webHandlerData->isUploadError)
    {
        /* If file is open, write data to it. */
        if (true == webHandlerData->fd)
        {
            if (len != webHandlerData->fd.write(data, len))
            {
                LOG_ERROR("Less data written, upload aborted.");
                webHandlerData->isUploadError = true;
                webHandlerData->fullPath.clear();
                webHandlerData->fd.close();
            }
        }

        /* Upload finished? */
        if (true == final)
        {
            LOG_INFO("Upload of %s finished.", filename.c_str());

            webHandlerData->fd.close();
        }
    }

    return;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
