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

void RestApiTopicHandler::registerTopic(const String& deviceId, const String& entityId, const String& topic, JsonObjectConst& extra, GetTopicFunc getTopicFunc, SetTopicFunc setTopicFunc, UploadReqFunc uploadReqFunc)
{
    if ((false == deviceId.isEmpty()) &&
        (false == entityId.isEmpty()) &&
        (false == topic.isEmpty()))
    {
        TopicMetaData* topicMetaData = new(std::nothrow) TopicMetaData();

        if (nullptr != topicMetaData)
        {
            String                      baseUri     = getBaseUri(entityId);
            ArRequestHandlerFunction    onRequest   =
                                            [this, topicMetaData](AsyncWebServerRequest *request)
                                            {
                                                this->webReqHandler(request, topicMetaData);
                                            };
            ArUploadHandlerFunction     onUpload    =
                                            [this, topicMetaData](AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final)
                                            {
                                                this->uploadHandler(request, filename, index, data, len, final, topicMetaData);
                                            };

            topicMetaData->deviceId         = deviceId;
            topicMetaData->entityId         = entityId;
            topicMetaData->topic            = topic;
            topicMetaData->getTopicFunc     = getTopicFunc;
            topicMetaData->setTopicFunc     = setTopicFunc;
            topicMetaData->uploadReqFunc    = uploadReqFunc;
            topicMetaData->uri              = baseUri + topic;
            topicMetaData->webHandler       = &MyWebServer::getInstance().on(topicMetaData->uri.c_str(), HTTP_ANY, onRequest, onUpload);

            UTIL_NOT_USED(extra);

            LOG_INFO("Register: %s", topicMetaData->uri.c_str());

            m_listOfTopicMetaData.push_back(topicMetaData);
        }
    }
}

void RestApiTopicHandler::unregisterTopic(const String& deviceId, const String& entityId, const String& topic)
{
    if ((false == deviceId.isEmpty()) &&
        (false == entityId.isEmpty()) &&
        (false == topic.isEmpty()))
    {
        ListOfTopicMetaData::iterator topicMetaDataIt = m_listOfTopicMetaData.begin();

        while(m_listOfTopicMetaData.end() != topicMetaDataIt)
        {
            TopicMetaData* topicMetaData = *topicMetaDataIt;

            if ((nullptr != topicMetaData) &&
                (deviceId == topicMetaData->deviceId) &&
                (entityId == topicMetaData->entityId) &&
                (topic == topicMetaData->topic))
            {
                if (false == MyWebServer::getInstance().removeHandler(topicMetaData->webHandler))
                {
                    LOG_WARNING("Failed to unregister: %s", topicMetaData->uri.c_str());
                }
                else
                {
                    LOG_INFO("Unregister: %s", topicMetaData->uri.c_str());
                }

                topicMetaDataIt = m_listOfTopicMetaData.erase(topicMetaDataIt);
                
                delete topicMetaData;
                topicMetaData = nullptr;
            }
            else
            {
                ++topicMetaDataIt;
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

String RestApiTopicHandler::getBaseUri(const String& entityId)
{
    String  baseUri = RestApi::BASE_URI;
    baseUri += "/";
    baseUri += entityId;

    return baseUri;
}

void RestApiTopicHandler::webReqHandler(AsyncWebServerRequest *request, TopicMetaData* topicMetaData)
{
    String              content;
    const size_t        JSON_DOC_SIZE   = 2048U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    JsonObject          dataObj         = jsonDoc.createNestedObject("data");
    uint32_t            httpStatusCode  = HttpStatus::STATUS_CODE_OK;

    if ((nullptr == request) ||
        (nullptr == topicMetaData))
    {
        return;
    }

    if ((HTTP_GET == request->method()) &&
        (nullptr != topicMetaData->getTopicFunc))
    {
        /* Topic data will be transported in the HTTP body as JSON. */
        if (false == topicMetaData->getTopicFunc(topicMetaData->topic, dataObj))
        {
            LOG_WARNING("Topic \"%s\" not supported by %s.", topicMetaData->topic.c_str(), topicMetaData->entityId.c_str());

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
    else if ((HTTP_POST == request->method()) &&
             (nullptr != topicMetaData->setTopicFunc))
    {
        DynamicJsonDocument jsonDocPar(JSON_DOC_SIZE);
        JsonObjectConst     jsonValue;
        
        /* Topic data is in the HTTP parameters and needs to be converted to JSON. */
        par2Json(jsonDocPar, request);

        /* Add uploaded file */
        if ((false == topicMetaData->isUploadError) &&
            (false == topicMetaData->fullPath.isEmpty()))
        {
            jsonDocPar["fullPath"] = topicMetaData->fullPath;
        }

        jsonValue = jsonDocPar.as<JsonObjectConst>(); /* Assign after par2Json conversion! Otherwise there will be a empty object. */
        if (false == topicMetaData->setTopicFunc(topicMetaData->topic, jsonValue))
        {
            LOG_WARNING("Topic \"%s\" not supported by %s or invalid data.", topicMetaData->topic.c_str(), topicMetaData->entityId.c_str());

            RestUtil::prepareRspError(jsonDoc, "Requested topic not supported or invalid data.");

            jsonDoc.remove("data");

            /* If a file is available, it will be removed now. */
            if (false == topicMetaData->fullPath.isEmpty())
            {
                (void)FILESYSTEM.remove(topicMetaData->fullPath);
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

void RestApiTopicHandler::uploadHandler(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final, TopicMetaData* topicMetaData)
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
            topicMetaData->isUploadError = true;
            topicMetaData->fullPath.clear();
        }
        else
        {
            LOG_INFO("Upload of %s (%d bytes) starts.", filename.c_str(), fileSize);
            topicMetaData->isUploadError = false;
            topicMetaData->fullPath.clear();

            /* Ask plugin, whether the upload is allowed or not. */
            if ((nullptr == topicMetaData->uploadReqFunc) ||
                (false == topicMetaData->uploadReqFunc(topicMetaData->topic, filename, topicMetaData->fullPath)))
            {
                LOG_WARNING("Upload not supported by %s.", topicMetaData->entityId.c_str());
                topicMetaData->isUploadError = true;
                topicMetaData->fullPath.clear();
            }
            else
            {
                /* Create a new file and overwrite a existing one. */
                request->_tempFile = FILESYSTEM.open(topicMetaData->fullPath, "w");

                if (false == request->_tempFile)
                {
                    LOG_ERROR("Couldn't create file: %s", topicMetaData->fullPath.c_str());
                    topicMetaData->isUploadError = true;
                    topicMetaData->fullPath.clear();
                }
            }
        }
    }

    if (false == topicMetaData->isUploadError)
    {
        /* If file is open, write data to it. */
        if (true == request->_tempFile)
        {
            if (len != request->_tempFile.write(data, len))
            {
                LOG_ERROR("Less data written, upload aborted.");
                topicMetaData->isUploadError = true;
                topicMetaData->fullPath.clear();
                request->_tempFile.close();
            }
        }

        /* Upload finished? */
        if (true == final)
        {
            LOG_INFO("Upload of %s finished.", filename.c_str());

            request->_tempFile.close();
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
    ListOfTopicMetaData::iterator topicMetaDataIt = m_listOfTopicMetaData.begin();

    while(m_listOfTopicMetaData.end() != topicMetaDataIt)
    {
        TopicMetaData* topicMetaData = *topicMetaDataIt;

        if (nullptr != topicMetaData)
        {
            (void)MyWebServer::getInstance().removeHandler(topicMetaData->webHandler);

            topicMetaDataIt = m_listOfTopicMetaData.erase(topicMetaDataIt);

            delete topicMetaData;
            topicMetaData = nullptr;
        }
        else
        {
            ++topicMetaDataIt;
        }
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
