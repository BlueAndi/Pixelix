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
 * @brief  Topic handler service
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "TopicHandlerService.h"
#include "TopicHandlers.h"

#include <Logging.h>
#include <FileSystem.h>
#include <JsonFile.h>

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

/* Initialize static values. */
const char* TopicHandlerService::DEFAULT_ACCESS = "rw";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

bool TopicHandlerService::start()
{
    startAllHandlers();

    m_onChangeTimer.start(ON_CHANGE_PERIOD);
    m_isStarted = true;

    LOG_INFO("Topic handler service started.");

    return true;
}

void TopicHandlerService::stop()
{
    m_isStarted = false;
    m_onChangeTimer.stop();

    stopAllHandlers();

    LOG_INFO("Topic handler service stopped.");
}

void TopicHandlerService::process()
{
    if (true == m_isStarted)
    {
        processAllHandlers();

        if ((true == m_onChangeTimer.isTimerRunning()) &&
            (true == m_onChangeTimer.isTimeout()))
        {
            processOnChange();
        }
    }
}

void TopicHandlerService::registerTopics(const String& deviceId, const String& entityId, IPluginMaintenance* plugin)
{
    if ((false == deviceId.isEmpty()) &&
        (nullptr != plugin))
    {
        const size_t        JSON_DOC_SIZE = 1024U;
        DynamicJsonDocument topicsDoc(JSON_DOC_SIZE);
        JsonArray           jsonTopics = topicsDoc.createNestedArray("topics");

        /* Get topics from plugin. */
        plugin->getTopics(jsonTopics);

        if (true == topicsDoc.overflowed())
        {
            LOG_ERROR("JSON document has less memory available.");
        }
        /* Handle each topic */
        else if (0U < jsonTopics.size())
        {
            for (JsonVariantConst jsonTopic : jsonTopics)
            {
                String                       topicName;
                JsonObjectConst              jsonExtra;
                String                       extraFileName;
                String                       topicAccess   = DEFAULT_ACCESS;
                ITopicHandler::GetTopicFunc  getTopicFunc  = nullptr;
                ITopicHandler::SetTopicFunc  setTopicFunc  = nullptr;
                ITopicHandler::UploadReqFunc uploadReqFunc = nullptr;

                /* Topic specific parameter available? */
                if (true == jsonTopic.is<JsonObjectConst>())
                {
                    JsonVariantConst jsonTopicName   = jsonTopic["name"];
                    JsonVariantConst jsonTopicAccess = jsonTopic["access"];
                    JsonVariantConst jsonExtraVar    = jsonTopic["extra"];

                    if (true == jsonTopicName.is<String>())
                    {
                        topicName = jsonTopicName.as<String>();
                    }

                    if (true == jsonTopicAccess.is<String>())
                    {
                        topicAccess = jsonTopicAccess.as<String>();
                    }

                    if (true == jsonExtraVar.is<JsonObjectConst>())
                    {
                        jsonExtra = jsonExtraVar.as<JsonObjectConst>();
                    }
                    else if (true == jsonExtraVar.is<String>())
                    {
                        extraFileName = jsonExtraVar.as<String>();
                    }
                    else
                    {
                        /* Skip */
                        ;
                    }
                }
                /* Only topic name is available */
                else if (true == jsonTopic.is<String>())
                {
                    topicName = jsonTopic.as<String>();
                }
                else
                {
                    /* Skip */
                    ;
                }

                if (false == topicName.isEmpty())
                {
                    strToAccess(plugin, topicAccess, getTopicFunc, setTopicFunc, uploadReqFunc);

                    if (true == extraFileName.isEmpty())
                    {
                        registerTopic(deviceId, entityId, topicName, jsonExtra, getTopicFunc, nullptr, setTopicFunc, uploadReqFunc);
                    }
                    else
                    {
                        registerTopic(deviceId, entityId, topicName, extraFileName.c_str(), getTopicFunc, nullptr, setTopicFunc, uploadReqFunc);
                    }

                    addToTopicMetaDataList(deviceId, entityId, plugin, topicName, nullptr);
                }
            }
        }
    }
}

void TopicHandlerService::unregisterTopics(const String& deviceId, const String& entityId, IPluginMaintenance* plugin, bool purge)
{
    if ((false == deviceId.isEmpty()) &&
        (nullptr != plugin))
    {
        const size_t        JSON_DOC_SIZE = 512U;
        DynamicJsonDocument topicsDoc(JSON_DOC_SIZE);
        JsonArray           jsonTopics = topicsDoc.createNestedArray("topics");

        /* Get topics from plugin. */
        plugin->getTopics(jsonTopics);

        /* Handle each topic */
        if (0U < jsonTopics.size())
        {
            for (JsonVariantConst jsonTopic : jsonTopics)
            {
                String topicName;

                /* Topic specific parameter available? */
                if (true == jsonTopic.is<JsonObjectConst>())
                {
                    JsonVariantConst jsonTopicName = jsonTopic["name"];

                    if (true == jsonTopicName.is<String>())
                    {
                        topicName = jsonTopicName.as<String>();
                    }
                }
                /* Only topic name is available */
                else if (true == jsonTopic.is<String>())
                {
                    topicName = jsonTopic.as<String>();
                }
                else
                {
                    /* Skip */
                    ;
                }

                if (false == topicName.isEmpty())
                {
                    unregisterTopic(deviceId, entityId, topicName, purge);

                    removeFromTopicMetaDataList(deviceId, entityId, topicName);
                }
            }
        }
    }
}

void TopicHandlerService::registerTopic(const String& deviceId, const String& entityId, const String& topic, JsonObjectConst& jsonExtra, ITopicHandler::GetTopicFunc getTopicFunc, HasChangedFunc hasChangedFunc, ITopicHandler::SetTopicFunc setTopicFunc, ITopicHandler::UploadReqFunc uploadReqFunc)
{
    if ((false == deviceId.isEmpty()) &&
        (false == entityId.isEmpty()) &&
        (false == topic.isEmpty()))
    {
        bool isReadAccess  = false;
        bool isWriteAccess = false;

        /* Determine the kind of accessability. */
        if (nullptr != getTopicFunc)
        {
            isReadAccess = true;
        }

        if ((nullptr != setTopicFunc) ||
            (nullptr != uploadReqFunc))
        {
            isWriteAccess = true;
        }

        if ((true == isReadAccess) ||
            (true == isWriteAccess))
        {
            uint8_t         idx              = 0U;
            uint8_t         count            = 0U;
            ITopicHandler** topicHandlerList = TopicHandlers::getList(count);

            /* Register topic by every known topic handler. */
            while (count > idx)
            {
                ITopicHandler* handler = topicHandlerList[idx];

                if (nullptr != handler)
                {
                    handler->registerTopic(deviceId, entityId, topic, jsonExtra, getTopicFunc, setTopicFunc, uploadReqFunc);
                }

                ++idx;
            }

            /* Store every readable topic in a list for automatic publishing on topic change,
             * except topics from plugins. They will be considered separately.
             */
            if ((true == isReadAccess) &&
                (nullptr != hasChangedFunc))
            {
                addToTopicMetaDataList(deviceId, entityId, nullptr, topic, hasChangedFunc);
            }
        }
    }
}

void TopicHandlerService::registerTopic(const String& deviceId, const String& entityId, const String& topic, const char* extraFileName, ITopicHandler::GetTopicFunc getTopicFunc, HasChangedFunc hasChangedFunc, ITopicHandler::SetTopicFunc setTopicFunc, ITopicHandler::UploadReqFunc uploadReqFunc)
{
    const size_t        JSON_DOC_SIZE = 1024U;
    DynamicJsonDocument jsonDocExtra(JSON_DOC_SIZE);
    JsonObjectConst     jsonExtra;

    if (nullptr != extraFileName)
    {
        JsonFile jsonFile(FILESYSTEM);

        LOG_INFO("Load extra info from file: %s", extraFileName);

        if (false == jsonFile.load(extraFileName, jsonDocExtra))
        {
            LOG_WARNING("Failed to load extra info from file: %s", extraFileName);
        }
        else
        {
            jsonExtra = jsonDocExtra.as<JsonObject>();
        }
    }

    registerTopic(deviceId, entityId, topic, jsonExtra, getTopicFunc, hasChangedFunc, setTopicFunc, uploadReqFunc);
}

void TopicHandlerService::unregisterTopic(const String& deviceId, const String& entityId, const String& topic, bool purge)
{
    if ((false == deviceId.isEmpty()) &&
        (false == entityId.isEmpty()) &&
        (false == topic.isEmpty()))
    {
        uint8_t         idx              = 0U;
        uint8_t         count            = 0U;
        ITopicHandler** topicHandlerList = TopicHandlers::getList(count);

        /* Unregister topic by every known topic handler. */
        while (count > idx)
        {
            ITopicHandler* handler = topicHandlerList[idx];

            if (nullptr != handler)
            {
                handler->unregisterTopic(deviceId, entityId, topic, purge);
            }

            ++idx;
        }

        /* If topic is stored for automatic publishing, it will be removed. */
        removeFromTopicMetaDataList(deviceId, entityId, topic);
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void TopicHandlerService::strToAccess(IPluginMaintenance* plugin, const String& strAccess, ITopicHandler::GetTopicFunc& getTopicFunc, ITopicHandler::SetTopicFunc& setTopicFunc, ITopicHandler::UploadReqFunc& uploadReqFunc) const
{
    if (nullptr != plugin)
    {
        bool isReadAccess  = false;
        bool isWriteAccess = false;

        if (true == strAccess.equalsIgnoreCase("rw"))
        {
            /* Read/Write access */
            isReadAccess  = true;
            isWriteAccess = true;
        }
        else if (true == strAccess.equalsIgnoreCase("w"))
        {
            /* Write only access */
            isWriteAccess = true;
        }
        else
        {
            /* Read only access */
            isReadAccess = true;
        }

        if (true == isReadAccess)
        {
            getTopicFunc = [plugin](const String& topic, JsonObject& value) -> bool {
                LOG_INFO("Get %s of plugin %u.", topic.c_str(), plugin->getUID());
                return plugin->getTopic(topic, value);
            };
        }

        if (true == isWriteAccess)
        {
            setTopicFunc = [plugin](const String& topic, const JsonObjectConst& value) -> bool {
                LOG_INFO("Set %s of plugin %u.", topic.c_str(), plugin->getUID());
                return plugin->setTopic(topic, value);
            };

            uploadReqFunc = [plugin](const String& topic, const String& srcFilename, String& dstFilename) -> bool {
                return plugin->isUploadAccepted(topic, srcFilename, dstFilename);
            };
        }
    }
}

void TopicHandlerService::addToTopicMetaDataList(const String& deviceId, const String& entityId, IPluginMaintenance* plugin, const String& topic, HasChangedFunc hasChangedFunc)
{
    if ((false == deviceId.isEmpty()) &&
        (false == entityId.isEmpty()) &&
        (false == topic.isEmpty()))
    {
        TopicMetaData* topicMetaData = new (std::nothrow) TopicMetaData();

        if (nullptr != topicMetaData)
        {
            topicMetaData->deviceId       = deviceId;
            topicMetaData->entityId       = entityId;
            topicMetaData->plugin         = plugin;
            topicMetaData->topic          = topic;
            topicMetaData->hasChangedFunc = hasChangedFunc;

            m_topicMetaDataList.push_back(topicMetaData);
        }
    }
}

void TopicHandlerService::removeFromTopicMetaDataList(const String& deviceId, const String& entityId, const String& topic)
{
    TopicMetaDataList::iterator topicMetaDataListIt = m_topicMetaDataList.begin();

    while (m_topicMetaDataList.end() != topicMetaDataListIt)
    {
        TopicMetaData* topicMetaData = *topicMetaDataListIt;

        if ((nullptr != topicMetaData) &&
            (deviceId == topicMetaData->deviceId) &&
            (entityId == topicMetaData->entityId) &&
            (topic == topicMetaData->topic))
        {
            topicMetaDataListIt = m_topicMetaDataList.erase(topicMetaDataListIt);

            delete topicMetaData;
            topicMetaData = nullptr;
        }
        else
        {
            ++topicMetaDataListIt;
        }
    }
}

void TopicHandlerService::processOnChange()
{
    TopicMetaDataList::iterator topicMetaDataListIt = m_topicMetaDataList.begin();

    /** Proces all topics which are independent from plugins. */
    while (m_topicMetaDataList.end() != topicMetaDataListIt)
    {
        TopicMetaData* topicMetaData = *topicMetaDataListIt;

        if (nullptr != topicMetaData)
        {
            bool hasTopicChanged = false;

            if (nullptr != topicMetaData->plugin)
            {
                hasTopicChanged = topicMetaData->plugin->hasTopicChanged(topicMetaData->topic);
            }
            else if (nullptr != topicMetaData->hasChangedFunc)
            {
                hasTopicChanged = topicMetaData->hasChangedFunc(topicMetaData->topic);
            }
            else
            {
                /* Skip */
                ;
            }

            if (true == hasTopicChanged)
            {
                notifyAllHandlers(topicMetaData->deviceId, topicMetaData->entityId, topicMetaData->topic);
            }
        }

        ++topicMetaDataListIt;
    }
}

void TopicHandlerService::startAllHandlers()
{
    uint8_t         idx              = 0U;
    uint8_t         count            = 0U;
    ITopicHandler** topicHandlerList = TopicHandlers::getList(count);

    while (count > idx)
    {
        ITopicHandler* handler = topicHandlerList[idx];

        if (nullptr != handler)
        {
            handler->start();
        }

        ++idx;
    }
}

void TopicHandlerService::stopAllHandlers()
{
    uint8_t         idx              = 0U;
    uint8_t         count            = 0U;
    ITopicHandler** topicHandlerList = TopicHandlers::getList(count);

    while (count > idx)
    {
        ITopicHandler* handler = topicHandlerList[idx];

        if (nullptr != handler)
        {
            handler->stop();
        }

        ++idx;
    }
}

void TopicHandlerService::processAllHandlers()
{
    uint8_t         idx              = 0U;
    uint8_t         count            = 0U;
    ITopicHandler** topicHandlerList = TopicHandlers::getList(count);

    while (count > idx)
    {
        ITopicHandler* handler = topicHandlerList[idx];

        if (nullptr != handler)
        {
            handler->process();
        }

        ++idx;
    }
}

void TopicHandlerService::notifyAllHandlers(const String& deviceId, const String& entityId, const String& topic)
{
    if (false == topic.isEmpty())
    {
        uint8_t         idx              = 0U;
        uint8_t         count            = 0U;
        ITopicHandler** topicHandlerList = TopicHandlers::getList(count);

        while (count > idx)
        {
            ITopicHandler* handler = topicHandlerList[idx];

            if (nullptr != handler)
            {
                handler->notify(deviceId, entityId, topic);
            }

            ++idx;
        }
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
