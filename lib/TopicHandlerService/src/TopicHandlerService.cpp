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

/******************************************************************************
 * Public Methods
 *****************************************************************************/

bool TopicHandlerService::start()
{
    bool isSuccessful = true;

    startAllHandlers();

    m_onChangeTimer.start(ON_CHANGE_PERIOD);

    if (false == isSuccessful)
    {
        stop();
    }
    else
    {
        LOG_INFO("Topic handler service started.");
    }

    return isSuccessful;
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

void TopicHandlerService::registerTopics(IPluginMaintenance* plugin)
{
    if (nullptr != plugin)
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
                String                  topicName;
                ITopicHandler::Access   topicAccess = DEFAULT_ACCESS;
                JsonObjectConst         extra;

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
                        topicAccess = strToAccess(jsonTopicAccess.as<String>());
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
                
                registerTopic(plugin, topicName, topicAccess, extra);
            }
        }
    }
}

void TopicHandlerService::unregisterTopics(IPluginMaintenance* plugin)
{
    if (nullptr != plugin)
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
                
                unregisterTopic(plugin, topicName);
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

void TopicHandlerService::registerTopic(IPluginMaintenance* plugin, const String& topic, ITopicHandler::Access access, JsonObjectConst& extra)
{
    if ((nullptr != plugin) &&
        (false == topic.isEmpty()))
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
                handler->registerTopic(plugin, topic, access, extra);
            }

            ++idx;
        }

        /* Store every readable topic in a list for automatic publishing
         * on topic change.
         */
        if ((ITopicHandler::ACCESS_READ_ONLY == access) ||
            (ITopicHandler::ACCESS_READ_WRITE == access))
        {
            addToList(plugin, topic);
        }
    }
}

void TopicHandlerService::unregisterTopic(IPluginMaintenance* plugin, const String& topic)
{
    if ((nullptr != plugin) &&
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
                handler->unregisterTopic(plugin, topic);
            }

            ++idx;
        }

        /* If topic is stored for automatic publishing, it will be removed. */
        removeFromList(plugin, topic);
    }
}

ITopicHandler::Access TopicHandlerService::strToAccess(const String& strAccess) const
{
    ITopicHandler::Access access = ITopicHandler::ACCESS_READ_ONLY;

    if (true == strAccess.equalsIgnoreCase("rw"))
    {
        access = ITopicHandler::ACCESS_READ_WRITE;
    }
    else if (true == strAccess.equalsIgnoreCase("w"))
    {
        access = ITopicHandler::ACCESS_WRITE_ONLY;
    }
    else
    {
        /* Keep init value. */
        ;
    }

    return access;
}

void TopicHandlerService::addToList(IPluginMaintenance* plugin, const String& topic)
{
    if ((nullptr != plugin) &&
        (false == topic.isEmpty()))
    {
        PluginTopic* pluginTopic = new(std::nothrow) PluginTopic();

        if (nullptr != pluginTopic)
        {
            pluginTopic->plugin = plugin;
            pluginTopic->topic  = topic;

            m_pluginTopicList.push_back(pluginTopic);
        }
    }
}

void TopicHandlerService::removeFromList(IPluginMaintenance* plugin, const String& topic)
{
    if ((nullptr != plugin) &&
        (false == topic.isEmpty()))
    {
        PluginTopicList::iterator pluginTopicListIt = m_pluginTopicList.begin();

        while(m_pluginTopicList.end() != pluginTopicListIt)
        {
            PluginTopic* pluginTopic = *pluginTopicListIt;

            if ((nullptr != pluginTopic) &&
                (plugin == pluginTopic->plugin) &&
                (topic == pluginTopic->topic))
            {
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

void TopicHandlerService::processOnChange()
{
    PluginTopicList::iterator pluginTopicListIt = m_pluginTopicList.begin();

    while(m_pluginTopicList.end() != pluginTopicListIt)
    {
        PluginTopic* pluginTopic = *pluginTopicListIt;

        if ((nullptr != pluginTopic) &&
            (true == pluginTopic->plugin->hasTopicChanged(pluginTopic->topic)))
        {
            notifyAllHandlers(pluginTopic->plugin, pluginTopic->topic);
        }

        ++pluginTopicListIt;
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

void TopicHandlerService::notifyAllHandlers(IPluginMaintenance* plugin, const String& topic)
{
    if ((nullptr != plugin) &&
        (false == topic.isEmpty()))
    {
        uint8_t         idx                 = 0U;
        uint8_t         count               = 0U;
        ITopicHandler** topicHandlerList    = TopicHandlers::getList(count);

        while(count > idx)
        {
            ITopicHandler* handler = topicHandlerList[idx];

            if (nullptr != handler)
            {
                handler->notify(plugin, topic);
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
