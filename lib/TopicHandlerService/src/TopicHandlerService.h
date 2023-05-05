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
 * 
 * @addtogroup service
 *
 * @{
 */

#ifndef TOPIC_HANDLER_SERVICE_H
#define TOPIC_HANDLER_SERVICE_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <IService.hpp>
#include <ITopicHandler.h>
#include <IPluginMaintenance.hpp>
#include <SimpleTimer.hpp>
#include <vector>

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * The topic handler service manages all topic handlers.
 */
class TopicHandlerService : public IService
{
public:

    /**
     * Get the topic handler service instance.
     * 
     * @return Topic handler service instance
     */
    static TopicHandlerService& getInstance()
    {
        static TopicHandlerService instance; /* idiom */

        return instance;
    }

    /**
     * Start the service.
     */
    bool start() final;

    /**
     * Stop the service.
     */
    void stop() final;

    /**
     * Process the service.
     */
    void process() final;

    /**
     * Register all topics of the given plugin.
     * 
     * @param[in] plugin    The plugin, which topics shall be registered.
     */
    void registerTopics(IPluginMaintenance* plugin);

    /**
     * Unregister all topics of the given plugin.
     * 
     * @param[in] plugin    The plugin, which topics to unregister.
     */
    void unregisterTopics(IPluginMaintenance* plugin);

private:

    /** Default topic accessibility. */
    static const ITopicHandler::Access  DEFAULT_ACCESS      = ITopicHandler::ACCESS_READ_WRITE;

    /** Period in ms to check for changed topics. */
    static const uint32_t               ON_CHANGE_PERIOD    = 500U;

    /**
     * Plugin topic data, used for automatic publishing.
     */
    struct PluginTopic
    {
        IPluginMaintenance* plugin; /**< The plugin */
        String              topic;  /**< The plugin topic */

        /**
         * Construct plugin topic data instance.
         */
        PluginTopic() :
            plugin(nullptr),
            topic()
        {
        }
    };

    /**
     * List of plugin topics.
     */
    typedef std::vector<PluginTopic*>   PluginTopicList;

    PluginTopicList m_pluginTopicList;  /**< List of readable plugin topics. */
    SimpleTimer     m_onChangeTimer;    /**< Timer for on change processing period. */

    /**
     * Constructs the service instance.
     */
    TopicHandlerService() :
        IService(),
        m_pluginTopicList(),
        m_onChangeTimer()
    {
    }

    /**
     * Destroys the service instance.
     */
    ~TopicHandlerService()
    {
        /* Never called. */
    }

    /* An instance shall not be copied. */
    TopicHandlerService(const TopicHandlerService& service);
    TopicHandlerService& operator=(const TopicHandlerService& service);

    /**
     * Converts the topic access in string form to the enumeration form.
     * 
     * @param[in] strAccess Topic accessibility as string (r, rw, w)
     * 
     * @return Topic accessibility
     */
   ITopicHandler::Access strToAccess(const String& strAccess) const;

    /**
     * Register a single topic of the given plugin.
     * 
     * @param[in] plugin    The plugin which provides the topic.
     * @param[in] topic     The topic name.
     * @param[in] access    The topic accessibility.
     * @param[in] extra     Extra parameters, which depend on the topic handler.
     */
    void registerTopic(IPluginMaintenance* plugin, const String& topic, ITopicHandler::Access access, JsonObjectConst& extra);

    /**
     * Unregister the topic of the given plugin.
     * 
     * @param[in] plugin    The plugin which provides the topic.
     * @param[in] topic     The topic name.
     */
    void unregisterTopic(IPluginMaintenance* plugin, const String& topic);

    /**
     * Add plugin topic to list of automatic publishing on change.
     * 
     * @param[in] plugin    The plugin which provides the topic.
     * @param[in] topic     The topic name.
     */
    void addToList(IPluginMaintenance* plugin, const String& topic);

    /**
     * Remove plugin topic from list of automatic publishing on change.
     * 
     * @param[in] plugin    The plugin which provides the topic.
     * @param[in] topic     The topic name.
     */
    void removeFromList(IPluginMaintenance* plugin, const String& topic);

    /**
     * Process all plugin topics to check which one has changed.
     * For every changed one, notify the handlers about.
     */
    void processOnChange();

    /**
     * Notify all topic handlers about changed topic.
     * 
     * @param[in] plugin    The plugin which provides the topic.
     * @param[in] topic     The topic name.
     */
    void notifyAllHandlers(IPluginMaintenance* plugin, const String& topic);
};

/******************************************************************************
 * Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* TOPIC_HANDLER_SERVICE_H */

/** @} */