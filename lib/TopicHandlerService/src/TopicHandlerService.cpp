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
 * @brief  Topic handler service
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "TopicHandlerService.h"
#include "TopicHandlers.h"

#include <Logging.h>

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

    LOG_INFO("Topic handler service started.");

    return true;
}

void TopicHandlerService::stop()
{
    m_onChangeTimer.stop();

    stopAllHandlers();

    LOG_INFO("Topic handler service stopped.");
}

void TopicHandlerService::process()
{
    processAllHandlers();

    if ((true == m_onChangeTimer.isTimerRunning()) &&
        (true == m_onChangeTimer.isTimeout()))
    {
        processOnChange();
    }
}

void TopicHandlerService::registerTopics(const String& deviceId, IPluginMaintenance* plugin)
{
    if ((false == deviceId.isEmpty()) &&
        (nullptr != plugin))
    {
        const size_t        JSON_DOC_SIZE   = 1024U;
        DynamicJsonDocument topicsDoc(JSON_DOC_SIZE);
        JsonArray           jsonTopics      = topicsDoc.createNestedArray("topics");

        /* Get topics from plugin. */
        plugin->getTopics(jsonTopics);

        if (true == topicsDoc.overflowed())
        {
            LOG_ERROR("JSON document has less memory available.");
        }

        /* Handle each topic */
        if (0U < jsonTopics.size())
        {
            for (JsonVariantConst jsonTopic : jsonTopics)
            {
                String                          topicName;
                JsonObjectConst                 extra;
                String                          topicAccess     = DEFAULT_ACCESS;
                ITopicHandler::GetTopicFunc     getTopicFunc    = nullptr;
                ITopicHandler::SetTopicFunc     setTopicFunc    = nullptr;
                ITopicHandler::UploadReqFunc    uploadReqFunc   = nullptr;

                /* Topic specific parameter available? */
                if (true == jsonTopic.is<JsonObjectConst>())
                {
                    JsonVariantConst    jsonTopicName   = jsonTopic["name"];
                    JsonVariantConst    jsonTopicAccess = jsonTopic["access"];

                    if (true == jsonTopicName.is<String>())
                    {
                        topicName = jsonTopicName.as<String>();
                    }

                    if (true == jsonTopicAccess.is<String>())
                    {
                        topicAccess = jsonTopicAccess.as<String>();
                    }

                    extra = jsonTopic;
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
                    
                    /* Register plugin topic with plugin UID as entity id. */
                    registerTopic(deviceId, getEntityIdByPluginUid(plugin->getUID()), topicName, extra, getTopicFunc, nullptr, setTopicFunc, uploadReqFunc);

                    /* Register plugin topic with plugin alias as entity id (if possible). */
                    if (false == plugin->getAlias().isEmpty())
                    {
                        registerTopic(deviceId, getEntityIdByPluginAlias(plugin->getAlias()), topicName, extra, getTopicFunc, nullptr, setTopicFunc, uploadReqFunc);
                    }

                    addToPluginMetaDataList(deviceId, plugin, topicName);
                }
            }
        }
    }
}

void TopicHandlerService::unregisterTopics(const String& deviceId, IPluginMaintenance* plugin)
{
    if ((false == deviceId.isEmpty()) &&
        (nullptr != plugin))
    {
        const size_t        JSON_DOC_SIZE   = 512U;
        DynamicJsonDocument topicsDoc(JSON_DOC_SIZE);
        JsonArray           jsonTopics      = topicsDoc.createNestedArray("topics");

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
                    /* Unregister plugin topic with plugin UID as entity id. */
                    unregisterTopic(deviceId, getEntityIdByPluginUid(plugin->getUID()), topicName);

                    /* Unregister plugin topic with plugin UID as entity id (if possible). */
                    if (false == plugin->getAlias().isEmpty())
                    {
                        unregisterTopic(deviceId, getEntityIdByPluginAlias(plugin->getAlias()), topicName);
                    }

                    removeFromPluginMetaDataList(deviceId, plugin);
                }
            }
        }
    }
}

void TopicHandlerService::registerTopic(const String& deviceId, const String& entityId, const String& topic, JsonObjectConst& extra, ITopicHandler::GetTopicFunc getTopicFunc, HasChangedFunc hasChangedFunc, ITopicHandler::SetTopicFunc setTopicFunc, ITopicHandler::UploadReqFunc uploadReqFunc)
{
    if ((false == deviceId.isEmpty()) &&
        (false == entityId.isEmpty()) &&
        (false == topic.isEmpty()))
    {
        bool    isReadAccess    = false;
        bool    isWriteAccess   = false;

        /* Determine the kind of accessability. */
        if (nullptr != getTopicFunc)
        {
            isReadAccess = true;
        }

        if ((nullptr != setTopicFunc) &&
            (nullptr != uploadReqFunc))
        {
            isWriteAccess = true;
        }

        if ((true == isReadAccess) ||
            (true == isWriteAccess))
        {
            uint8_t         idx                 = 0U;
            uint8_t         count               = 0U;
            ITopicHandler** topicHandlerList    = TopicHandlers::getList(count);

            /* Register topic by every known topic handler. */
            while(count > idx)
            {
                ITopicHandler* handler = topicHandlerList[idx];

                if (nullptr != handler)
                {
                    handler->registerTopic(deviceId, entityId, topic, extra, getTopicFunc, setTopicFunc, uploadReqFunc);
                }

                ++idx;
            }

            /* Store every readable topic in a list for automatic publishing on topic change,
             * except topics from plugins. They will be considered separately.
             */
            if ((true == isReadAccess) &&
                (nullptr != hasChangedFunc))
            {
                addToTopicMetaDataList(deviceId, entityId, topic, hasChangedFunc);
            }
        }
    }
}

void TopicHandlerService::unregisterTopic(const String& deviceId, const String& entityId, const String& topic)
{
    if ((false == deviceId.isEmpty()) &&
        (false == entityId.isEmpty()) &&
        (false == topic.isEmpty()))
    {
        uint8_t         idx                 = 0U;
        uint8_t         count               = 0U;
        ITopicHandler** topicHandlerList    = TopicHandlers::getList(count);

        /* Unregister topic by every known topic handler. */
        while(count > idx)
        {
            ITopicHandler* handler = topicHandlerList[idx];

            if (nullptr != handler)
            {
                handler->unregisterTopic(deviceId, entityId, topic);
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

String TopicHandlerService::getEntityIdByPluginUid(uint16_t uid)
{
    return String("display/uid/") + uid;
}

String TopicHandlerService::getEntityIdByPluginAlias(const String& alias)
{
    return String("display/alias/") + alias;
}

void TopicHandlerService::strToAccess(IPluginMaintenance* plugin, const String& strAccess, ITopicHandler::GetTopicFunc& getTopicFunc, ITopicHandler::SetTopicFunc& setTopicFunc, ITopicHandler::UploadReqFunc& uploadReqFunc) const
{
    if (nullptr != plugin)
    {
        bool isReadAccess = false;
        bool isWriteAccess = false;

        if (true == strAccess.equalsIgnoreCase("rw"))
        {
            /* Read/Write access */
            isReadAccess    = true;
            isWriteAccess   = true;
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
            getTopicFunc =      [plugin](const String& topic, JsonObject& value) -> bool
                                {
                                    LOG_INFO("Get %s of plugin %u.", topic.c_str(), plugin->getUID());
                                    return plugin->getTopic(topic, value);
                                };
        }

        if (true == isWriteAccess)
        {
            setTopicFunc  =     [plugin](const String& topic, const JsonObjectConst& value) -> bool
                                {
                                    LOG_INFO("Set %s of plugin %u.", topic.c_str(), plugin->getUID());
                                    return plugin->setTopic(topic, value);
                                };

            uploadReqFunc =     [plugin](const String& topic, const String& srcFilename, String& dstFilename) -> bool
                                {
                                    return plugin->isUploadAccepted(topic, srcFilename, dstFilename);
                                };
        }
    }
}

void TopicHandlerService::addToTopicMetaDataList(const String& deviceId, const String& entityId, const String& topic, HasChangedFunc hasChangedFunc)
{
    if ((false == deviceId.isEmpty()) &&
        (false == entityId.isEmpty()) &&
        (false == topic.isEmpty()) &&
        (nullptr != hasChangedFunc))
    {
        TopicMetaData* topicMetaData = new(std::nothrow) TopicMetaData();

        if (nullptr != topicMetaData)
        {
            topicMetaData->deviceId         = deviceId;
            topicMetaData->entityId         = entityId;
            topicMetaData->topic            = topic;
            topicMetaData->hasChangedFunc   = hasChangedFunc;

            m_topicMetaDataList.push_back(topicMetaData);
        }
    }
}

void TopicHandlerService::removeFromTopicMetaDataList(const String& deviceId, const String& entityId, const String& topic)
{
    TopicMetaDataList::iterator topicMetaDataListIt = m_topicMetaDataList.begin();

    while(m_topicMetaDataList.end() != topicMetaDataListIt)
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

void TopicHandlerService::addToPluginMetaDataList(const String& deviceId, IPluginMaintenance* plugin, const String& topic)
{
    if ((false == deviceId.isEmpty()) &&
        (nullptr != plugin) &&
        (false == topic.isEmpty()))
    {
        PluginMetaData* pluginMetaData = new(std::nothrow) PluginMetaData();

        if (nullptr != pluginMetaData)
        {
            pluginMetaData->deviceId    = deviceId;
            pluginMetaData->plugin      = plugin;
            pluginMetaData->topic       = topic;

            m_pluginMetaDataList.push_back(pluginMetaData);
        }
    }
}

void TopicHandlerService::removeFromPluginMetaDataList(const String& deviceId, IPluginMaintenance* plugin)
{
    PluginMetaDataList::iterator pluginMetaDataListIt = m_pluginMetaDataList.begin();

    while(m_pluginMetaDataList.end() != pluginMetaDataListIt)
    {
        PluginMetaData* pluginMetaData = *pluginMetaDataListIt;

        if ((nullptr != pluginMetaData) &&
            (deviceId == pluginMetaData->deviceId) &&
            (plugin == pluginMetaData->plugin))
        {
            pluginMetaDataListIt = m_pluginMetaDataList.erase(pluginMetaDataListIt);
            
            delete pluginMetaData;
            pluginMetaData = nullptr;
        }
        else
        {
            ++pluginMetaDataListIt;
        }
    }
}

void TopicHandlerService::processOnChange()
{
    PluginMetaDataList::iterator    pluginMetaDataListIt    = m_pluginMetaDataList.begin();
    TopicMetaDataList::iterator     topicMetaDataListIt     = m_topicMetaDataList.begin();
    
    /** Process all plugin related topics. */
    while(m_pluginMetaDataList.end() != pluginMetaDataListIt)
    {
        PluginMetaData* pluginMetaData = *pluginMetaDataListIt;

        if ((nullptr != pluginMetaData) &&
            (nullptr != pluginMetaData->plugin) &&
            (true == pluginMetaData->plugin->hasTopicChanged(pluginMetaData->topic)))
        {
            notifyAllHandlers(pluginMetaData->deviceId, getEntityIdByPluginUid(pluginMetaData->plugin->getUID()), pluginMetaData->topic);

            if (false == pluginMetaData->plugin->getAlias().isEmpty())
            {
                notifyAllHandlers(pluginMetaData->deviceId, getEntityIdByPluginAlias(pluginMetaData->plugin->getAlias()), pluginMetaData->topic);
            }
        }

        ++pluginMetaDataListIt;
    }

    /** Proces all topics which are independent from plugins. */
    while(m_topicMetaDataList.end() != topicMetaDataListIt)
    {
        TopicMetaData* topicMetaData = *topicMetaDataListIt;

        if ((nullptr != topicMetaData) &&
            (nullptr != topicMetaData->hasChangedFunc) &&
            (true == topicMetaData->hasChangedFunc(topicMetaData->topic)))
        {
            notifyAllHandlers(topicMetaData->deviceId, topicMetaData->entityId, topicMetaData->topic);
        }

        ++topicMetaDataListIt;
    }
}

void TopicHandlerService::startAllHandlers()
{
    uint8_t         idx                 = 0U;
    uint8_t         count               = 0U;
    ITopicHandler** topicHandlerList    = TopicHandlers::getList(count);

    while(count > idx)
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
    uint8_t         idx                 = 0U;
    uint8_t         count               = 0U;
    ITopicHandler** topicHandlerList    = TopicHandlers::getList(count);

    while(count > idx)
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
    uint8_t         idx                 = 0U;
    uint8_t         count               = 0U;
    ITopicHandler** topicHandlerList    = TopicHandlers::getList(count);

    while(count > idx)
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
        uint8_t         idx                 = 0U;
        uint8_t         count               = 0U;
        ITopicHandler** topicHandlerList    = TopicHandlers::getList(count);

        while(count > idx)
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
