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
 *
 * @addtogroup TOPIC_HANDLER_SERVICE
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

    /** Function prototype to check whether the topic content changed since last time. */
    typedef std::function<bool(const String& topic)> HasChangedFunc;

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
     * It will start all topic handlers.
     *
     * @return If successful started, it will return true otherwise false.
     */
    bool start() final;

    /**
     * Stop the service.
     * It will stop all topic handlers.
     * Topics are NOT unregistered. If necessary, this must be done before.
     */
    void stop() final;

    /**
     * Process the service.
     */
    void process() final;

    /**
     * Register all topics of the given plugin.
     *
     * @param[in] deviceId  The device id which represents the physical device.
     * @param[in] entityId  The entity id which represents the entity of the device. May be empty.
     * @param[in] plugin    The plugin, which topics shall be registered.
     */
    void registerTopics(const String& deviceId, const String& entityId, IPluginMaintenance* plugin);

    /**
     * Unregister all topics of the given plugin.
     *
     * If the purge flag is set, the topic handler will purge the topics like they never existed.
     * If the topics will be registered again, they will be treated as new topics.
     *
     * @param[in] deviceId  The device id which represents the physical device.
     * @param[in] entityId  The entity id which represents the entity of the device.
     * @param[in] plugin    The plugin, which topics to unregister.
     * @param[in] purge     If true, the topic handler will purge the topics like they never existed.
     */
    void unregisterTopics(const String& deviceId, const String& entityId, IPluginMaintenance* plugin, bool purge = false);

    /**
     * Register a topic.
     *
     * @param[in] deviceId          The device id which represents the physical device.
     * @param[in] entityId          The entity id which represents the entity of the device.
     * @param[in] topic             The topic which to register.
     * @param[in] jsonExtra         Extra JSON parameters for concrete topic handlers, which are pushed through.
     * @param[in] getTopicFunc      Function which is called to read the topic.
     * @param[in] hasChangedFunc    Function which is periodically called to check whether the topic has changed.
     * @param[in] setTopicFunc      Function which is called to set the topic.
     * @param[in] uploadReqFunc     Function which is called to accept a file upload or not.
     */
    void registerTopic(const String& deviceId, const String& entityId, const String& topic, JsonObjectConst& jsonExtra, ITopicHandler::GetTopicFunc getTopicFunc, HasChangedFunc hasChangedFunc, ITopicHandler::SetTopicFunc setTopicFunc, ITopicHandler::UploadReqFunc uploadReqFunc);

    /**
     * Register a topic.
     *
     * @param[in] deviceId          The device id which represents the physical device.
     * @param[in] entityId          The entity id which represents the entity of the device.
     * @param[in] topic             The topic which to register.
     * @param[in] extraFileName     Name of the file with extra JSON parameters for concrete topic handlers, which are pushed through.
     * @param[in] getTopicFunc      Function which is called to read the topic.
     * @param[in] hasChangedFunc    Function which is periodically called to check whether the topic has changed.
     * @param[in] setTopicFunc      Function which is called to set the topic.
     * @param[in] uploadReqFunc     Function which is called to accept a file upload or not.
     */
    void registerTopic(const String& deviceId, const String& entityId, const String& topic, const char* extraFileName, ITopicHandler::GetTopicFunc getTopicFunc, HasChangedFunc hasChangedFunc, ITopicHandler::SetTopicFunc setTopicFunc, ITopicHandler::UploadReqFunc uploadReqFunc);

    /**
     * Unregister a topic.
     *
     * @param[in] deviceId  The device id which represents the physical device.
     * @param[in] entityId  The entity id which represents the entity of the device.
     * @param[in] topic     The topic which to unregister.
     * @param[in] purge     If true, the topic handler will purge the topics like they never existed.
     */
    void unregisterTopic(const String& deviceId, const String& entityId, const String& topic, bool purge = false);

private:

    /** Default topic accessibility. */
    static const char* DEFAULT_ACCESS;

    /** Period in ms to check for changed topics. */
    static const uint32_t ON_CHANGE_PERIOD    = 500U;

    /**
     * The update counter forces an topic update, independent whether the topic changed.
     * This ensures that the topic content is updated periodically and that in case e.g.
     * Home Assistant restarted, it will receive the topic content.
     * 
     * The update counter is decremented every ON_CHANGE_PERIOD.
     */
    static const uint8_t UPDATE_COUNTER_VALUE = 20U;

    /**
     * Topic meta data, used for automatic publishing.
     */
    struct TopicMetaData
    {
        String              deviceId;       /**< Id of the device this data is related to. */
        String              entityId;       /**< Id of the entity this data is related to. */
        IPluginMaintenance* plugin;         /**< Plugin this data is related to. */
        String              topic;          /**< The topic this data is related to. */
        HasChangedFunc      hasChangedFunc; /**< Function to check whether the topic content has changed. */

        /**
         * Construct topic meta data instance.
         */
        TopicMetaData() :
            deviceId(),
            entityId(),
            plugin(nullptr),
            topic(),
            hasChangedFunc(nullptr)
        {
        }
    };

    /**
     * List of topic meta data.
     */
    typedef std::vector<TopicMetaData*> TopicMetaDataList;

    /**
     * Plugin meta data, used for automatic publishing.
     */
    struct PluginMetaData
    {
        IPluginMaintenance* plugin; /**< Plugin */
        String              topic;  /**< Topic */
        uint8_t             count;  /**< Number of topic registrations */

        /**
         * Construct plugin meta data instance.
         */
        PluginMetaData() :
            plugin(nullptr),
            topic(),
            count(0U)
        {
        }
    };

    /**
     * List of plugins which have at least one topic registered.
     */
    typedef std::vector<PluginMetaData*> PluginMetaDataList;

    bool                                 m_isStarted;          /**< Is the service started? */
    TopicMetaDataList                    m_topicMetaDataList;  /**< List of readable topics and the required meta data. */
    PluginMetaDataList                   m_pluginMetaDataList; /**< List of plugins which have at least one topic registered. */
    SimpleTimer                          m_onChangeTimer;      /**< Timer for on change processing period. */
    uint8_t                              m_updateCounter;      /**< If counter is 0, a topic content will be updated indepdendent its changed. */

    /**
     * Constructs the service instance.
     */
    TopicHandlerService() :
        IService(),
        m_isStarted(false),
        m_topicMetaDataList(),
        m_pluginMetaDataList(),
        m_onChangeTimer(),
        m_updateCounter(UPDATE_COUNTER_VALUE)
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
     * Generates the access functions depended on the plugin accessibility.
     *
     * @param[in]   plugin          The plugin which to consider.
     * @param[in]   strAccess       Topic accessibility as string (r, rw, w).
     * @param[out]  getTopicFunc    Function to get the topic content.
     * @param[out]  setTopicFunc    Function to set the topic content.
     * @param[out]  uploadReqFunc   Function used for requesting whether an file upload is allowed.
     */
    void strToAccess(IPluginMaintenance* plugin, const String& strAccess, ITopicHandler::GetTopicFunc& getTopicFunc, ITopicHandler::SetTopicFunc& setTopicFunc, ITopicHandler::UploadReqFunc& uploadReqFunc) const;

    /**
     * Add topic meta data to list of automatic publishing on change.
     *
     * @param[in] deviceId          The device id which represents the physical device.
     * @param[in] entityId          The entity id which represents the entity of the device.
     * @param[in] plugin            The plugin, which is related to the topic.
     * @param[in] topic             The topic name.
     * @param[in] hasChangedFunc    Function to retrieve whether the topic changes since last time.
     */
    void addToTopicMetaDataList(const String& deviceId, const String& entityId, IPluginMaintenance* plugin, const String& topic, HasChangedFunc hasChangedFunc);

    /**
     * Remove topic meta data from list of automatic publishing on change.
     *
     * @param[in] deviceId  The device id which represents the physical device.
     * @param[in] entityId  The entity id which represents the entity of the device.
     * @param[in] topic     The topic name.
     */
    void removeFromTopicMetaDataList(const String& deviceId, const String& entityId, const String& topic);

    /**
     * Add a plugin to the list of plugins, which have at least one topic registered.
     * If a plugin is already in the list with the same topic, nothing will be done.
     *
     * @param[in] plugin    The plugin, which is related to the topic.
     * @param[in] topic     The topic name.
     */
    void addToPluginList(IPluginMaintenance* plugin, const String& topic);

    /**
     * Remove a plugin from the list of plugins, which have at least one topic registered.
     *
     * @param[in] plugin    The plugin, which is related to the topic.
     * @param[in] topic     The topic name.
     */
    void removeFromPluginList(IPluginMaintenance* plugin, const String& topic);

    /**
     * Process all topics to check which one has changed.
     * For every changed one, notify the handlers about.
     * 
     * @param[in] forceUpdate  If true, the topic content will be updated independent its changed.
     */
    void processOnChange(bool forceUpdate);

    /**
     * Process all plugin topics to check which one has changed.
     * For every changed one, notify the handlers about.
     * 
     * @param[in] forceUpdate  If true, the topic content will be updated independent its changed.
     */
    void processPluginsOnChange(bool forceUpdate);

    /**
     * Process all not plugin related topics to check which one has changed.
     * For every changed one, notify the handlers about.
     * 
     * @param[in] forceUpdate  If true, the topic content will be updated independent its changed.
     */
    void processOthersOnChange(bool forceUpdate);

    /**
     * Start all topic handlers.
     */
    void startAllHandlers();

    /**
     * Stop all topic handlers.
     */
    void stopAllHandlers();

    /**
     * Process all topic handlers.
     */
    void processAllHandlers();

    /**
     * Notify all topic handlers about changed topic.
     *
     * @param[in] deviceId  The device id which represents the physical device.
     * @param[in] entityId  The entity id which represents the entity of the device.
     * @param[in] topic     The topic name.
     */
    void notifyAllHandlers(const String& deviceId, const String& entityId, const String& topic);
};

/******************************************************************************
 * Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* TOPIC_HANDLER_SERVICE_H */

/** @} */