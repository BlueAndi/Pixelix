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

void RestApiTopicHandler::registerTopic(IPluginMaintenance* plugin, const String& topic, Access access, JsonObjectConst& extra)
{
    if ((nullptr != plugin) &&
        (false == topic.isEmpty()))
    {
        String baseUriByUid = getBaseUriByUid(plugin->getUID());

        registerTopic(plugin, topic, access, extra, baseUriByUid);

        if (false == plugin->getAlias().isEmpty())
        {
            String baseUriByAlias = getBaseUriByAlias(plugin->getAlias());

            registerTopic(plugin, topic, access, extra, baseUriByAlias);
        }
    }
}

void RestApiTopicHandler::unregisterTopic(IPluginMaintenance* plugin, const String& topic)
{
    if ((nullptr != plugin) &&
        (false == topic.isEmpty()))
    {
        PluginTopicList::iterator pluginTopicListIt = m_pluginTopicList.begin();

        while(m_pluginTopicList.end() != pluginTopicListIt)
        {
            PluginTopic* pluginTopic = *pluginTopicListIt;

            if ((plugin == pluginTopic->plugin) &&
                (topic == pluginTopic->topic))
            {
                if (false == MyWebServer::getInstance().removeHandler(pluginTopic->webHandler))
                {
                    LOG_WARNING("[%s][%u] Failed to unregister: %s", pluginTopic->plugin->getName(), pluginTopic->plugin->getUID(), pluginTopic->uri.c_str());
                }
                else
                {
                    LOG_INFO("[%s][%u] Unregistered: %s", pluginTopic->plugin->getName(), pluginTopic->plugin->getUID(), pluginTopic->uri.c_str());
                }

                pluginTopicListIt = m_pluginTopicList.erase(pluginTopicListIt);
                delete pluginTopic;
                pluginTopic = nullptr;
            }
            else
            {
                ++pluginTopicListIt;
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

void RestApiTopicHandler::registerTopic(IPluginMaintenance* plugin, const String& topic, Access access, JsonObjectConst& extra, const String& baseUri)
{
    if (nullptr != plugin)
    {
        PluginTopic* pluginTopic = new(std::nothrow) PluginTopic();

        if (nullptr != pluginTopic)
        {
            pluginTopic->plugin     = plugin;
            pluginTopic->topic      = topic;
            pluginTopic->access     = access;
            pluginTopic->uri        = baseUri + topic;
            pluginTopic->webHandler = &MyWebServer::getInstance().on(
                                        pluginTopic->uri.c_str(),
                                        HTTP_ANY,
                                        [this, pluginTopic](AsyncWebServerRequest *request)
                                        {
                                            this->webReqHandler(request, pluginTopic);
                                        },
                                        [this, pluginTopic](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final)
                                        {
                                            this->uploadHandler(request, filename, index, data, len, final, pluginTopic);
                                        });

            UTIL_NOT_USED(extra);

            m_pluginTopicList.push_back(pluginTopic);

            LOG_INFO("[%s][%u] Registered: %s", pluginTopic->plugin->getName(), pluginTopic->plugin->getUID(), pluginTopic->uri.c_str());
        }
    }
}

void RestApiTopicHandler::webReqHandler(AsyncWebServerRequest *request, PluginTopic* pluginTopic)
{
    String              content;
    const size_t        JSON_DOC_SIZE   = 1024U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    JsonObject          dataObj         = jsonDoc.createNestedObject("data");
    uint32_t            httpStatusCode  = HttpStatus::STATUS_CODE_OK;

    if ((nullptr == request) ||
        (nullptr == pluginTopic) ||
        (nullptr == pluginTopic->plugin))
    {
        return;
    }

    if ((HTTP_GET == request->method()) &&
        (
            (ACCESS_READ_ONLY == pluginTopic->access) ||
            (ACCESS_READ_WRITE == pluginTopic->access)
        ))
    {
        /* Topic data will be transported in the HTTP body as JSON. */
        if (false == pluginTopic->plugin->getTopic(pluginTopic->topic, dataObj))
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
    else if ((HTTP_POST == request->method())&&
        (
            (ACCESS_READ_WRITE == pluginTopic->access) ||
            (ACCESS_WRITE_ONLY == pluginTopic->access)
        ))
    {
        DynamicJsonDocument jsonDocPar(JSON_DOC_SIZE);
        
        /* Topic data is in the HTTP parameters and needs to be converted to JSON. */
        par2Json(jsonDocPar, request);

        /* Add uploaded file */
        if ((false == pluginTopic->isUploadError) &&
            (false == pluginTopic->fullPath.isEmpty()))
        {
            jsonDocPar["fullPath"] = pluginTopic->fullPath;
        }

        if (false == pluginTopic->plugin->setTopic(pluginTopic->topic, jsonDocPar.as<JsonObject>()))
        {
            RestUtil::prepareRspError(jsonDoc, "Requested topic not supported or invalid data.");

            jsonDoc.remove("data");

            /* If a file is available, it will be removed now. */
            if (false == pluginTopic->fullPath.isEmpty())
            {
                (void)FILESYSTEM.remove(pluginTopic->fullPath);
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
}

void RestApiTopicHandler::uploadHandler(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final, PluginTopic* pluginTopic)
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
            pluginTopic->isUploadError = true;
            pluginTopic->fullPath.clear();
        }
        else
        {
            LOG_INFO("Upload of %s (%d bytes) starts.", filename.c_str(), fileSize);
            pluginTopic->isUploadError = false;
            pluginTopic->fullPath.clear();

            /* Ask plugin, whether the upload is allowed or not. */
            if (false == pluginTopic->plugin->isUploadAccepted(pluginTopic->topic, filename, pluginTopic->fullPath))
            {
                LOG_WARNING("[%s][%u] Upload not supported.", pluginTopic->plugin->getName(), pluginTopic->plugin->getUID());
                pluginTopic->isUploadError = true;
                pluginTopic->fullPath.clear();
            }
            else
            {
                /* Create a new file and overwrite a existing one. */
                pluginTopic->fd = FILESYSTEM.open(pluginTopic->fullPath, "w");

                if (false == pluginTopic->fd)
                {
                    LOG_ERROR("Couldn't create file: %s", pluginTopic->fullPath.c_str());
                    pluginTopic->isUploadError = true;
                    pluginTopic->fullPath.clear();
                }
            }
        }
    }

    if (false == pluginTopic->isUploadError)
    {
        /* If file is open, write data to it. */
        if (true == pluginTopic->fd)
        {
            if (len != pluginTopic->fd.write(data, len))
            {
                LOG_ERROR("Less data written, upload aborted.");
                pluginTopic->isUploadError = true;
                pluginTopic->fullPath.clear();
                pluginTopic->fd.close();
            }
        }

        /* Upload finished? */
        if (true == final)
        {
            LOG_INFO("Upload of %s finished.", filename.c_str());

            pluginTopic->fd.close();
        }
    }
}

void RestApiTopicHandler::par2Json(JsonDocument& jsonDocPar, AsyncWebServerRequest *request)
{
    size_t idx = 0U;

    /* Add arguments:
     * - key=value              --> { "key": "value" }
     * - key.subKey=value       --> { "key": { "subKey": "value "} }
     * - key._0_=value          --> { "key": [ "value" ] }
     * - key._0_.subKey=value   --> { "key": [ "subKey": "value" ] }
     * 
     * Note: Only the patterns above are supported, but not a higher
     *       nesting level.
     */
    for(idx = 0U; idx < request->args(); ++idx)
    {
        const String&   keyPattern  = request->argName(idx);
        const String&   value 	    = request->arg(idx);
        int             dotIdx      = keyPattern.indexOf(".");

        /* No "."  in the key pattern means: key=value */
        if (0 > dotIdx)
        {
            jsonDocPar[keyPattern] = value;
        }
        /* No "_" after the "." means: key.subKey=value */
        else if ('_' != keyPattern[dotIdx + 1U])
        {
            String  key     = keyPattern.substring(0, dotIdx);
            String  subKey  = keyPattern.substring(dotIdx + 1U);

            jsonDocPar[key][subKey] = value;
        }
        /* Its an array. */
        else
        {
            String  key     = keyPattern.substring(0, dotIdx);
            int     dot2Idx = keyPattern.lastIndexOf(".");

            /* No additional "." means: key._0_=value */
            if (dotIdx == dot2Idx)
            {
                String  strArrayIdx = keyPattern.substring(dotIdx + 1U);

                /* Remove "_" at the front and the end. */
                strArrayIdx.remove(0U, 1U);
                strArrayIdx.remove(strArrayIdx.length() - 1U);

                jsonDocPar[key][strArrayIdx.toInt()] = value;
            }
            /* Additional "." means: key._0_.subKey=value */
            else
            {
                String  strArrayIdx = keyPattern.substring(dotIdx + 1U);
                String  subKey      = keyPattern.substring(dot2Idx + 1U);

                /* Remove "_" at the front and the end. */
                strArrayIdx.remove(0U, 1U);
                strArrayIdx.remove(strArrayIdx.length() - 1U);

                jsonDocPar[key][strArrayIdx.toInt()][subKey] = value;
            }
        }
    }
}

void RestApiTopicHandler::clearPluginTopics()
{
    PluginTopicList::iterator pluginTopicListIt = m_pluginTopicList.begin();

    while(m_pluginTopicList.end() != pluginTopicListIt)
    {
        PluginTopic* pluginTopic = *pluginTopicListIt;

        (void)MyWebServer::getInstance().removeHandler(pluginTopic->webHandler);

        pluginTopicListIt = m_pluginTopicList.erase(pluginTopicListIt);
        delete pluginTopic;
        pluginTopic = nullptr;
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
