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
     * @param[in] deviceId  The device id which represents the physical device.
     * @param[in] plugin    The plugin, which topics shall be registered.
     */
    void registerTopics(const String& deviceId, IPluginMaintenance* plugin);

    /**
     * Unregister all topics of the given plugin.
     * 
     * @param[in] deviceId  The device id which represents the physical device.
     * @param[in] plugin    The plugin, which topics to unregister.
     */
    void unregisterTopics(const String& deviceId, IPluginMaintenance* plugin);

    /**
     * Register a topic.
     * 
     * @param[in] deviceId          The device id which represents the physical device.
     * @param[in] entityId          The entity id which represents the entity of the device.
     * @param[in] topic             The topic which to register.
     * @param[in] extra             Extra JSON parameters for concrete topic handlers, which are pushed through.
     * @param[in] getTopicFunc      Function which is called to read the topic.
     * @param[in] hasChangedFunc    Function which is periodically called to check whether the topic has changed.
     * @param[in] setTopicFunc      Function which is called to set the topic.
     * @param[in] uploadReqFunc     Function which is called to accept a file upload or not.
     */
    void registerTopic(const String& deviceId, const String& entityId, const String& topic, JsonObjectConst& extra, ITopicHandler::GetTopicFunc getTopicFunc, HasChangedFunc hasChangedFunc, ITopicHandler::SetTopicFunc setTopicFunc, ITopicHandler::UploadReqFunc uploadReqFunc);

    /**
     * Unregister a topic.
     * 
     * @param[in] deviceId  The device id which represents the physical device.
     * @param[in] entityId  The entity id which represents the entity of the device.
     * @param[in] topic     The topic which to unregister.
     */
    void unregisterTopic(const String& deviceId, const String& entityId, const String& topic);

private:

    /** Default topic accessibility. */
    static const char*      DEFAULT_ACCESS;

    /** Period in ms to check for changed topics. */
    static const uint32_t   ON_CHANGE_PERIOD    = 500U;

    /**
     * Topic meta data, used for automatic publishing.
     */
    struct TopicMetaData
    {
        String          deviceId;       /**< Id of the device this data is related to. */
        String          entityId;       /**< Id of the entity this data is related to. */
        String          topic;          /**< The topic this data is related to. */
        HasChangedFunc  hasChangedFunc; /**< Function to check whether the topic content has changed. */

        /**
         * Construct topic meta data instance.
         */
        TopicMetaData() :
            deviceId(),
            entityId(),
            topic(),
            hasChangedFunc(nullptr)
        {
        }
    };

    /**
     * List of topic meta data.
     */
    typedef std::vector<TopicMetaData*>   TopicMetaDataList;

    /**
     * Plugin meta data, used to determine whether publishing
     * of a topic shall take place or not.
     */
    struct PluginMetaData
    {
        String              deviceId;   /**< Id of the device this data is related to. */
        IPluginMaintenance* plugin;     /**< Plugin which topics are handled. */
        String              topic;      /**< The topic this data is related to. */

        /**
         * Construct topic meta data instance.
         */
        PluginMetaData() :
            plugin(nullptr),
            topic()
        {
        }
    };

    /**
     * List of plugin meta data.
     */
    typedef std::vector<PluginMetaData*>    PluginMetaDataList;

    TopicMetaDataList   m_topicMetaDataList;    /**< List of readable topics and the required meta data. */
    PluginMetaDataList  m_pluginMetaDataList;   /**< List of plugins, which topics are handled. */
    SimpleTimer         m_onChangeTimer;        /**< Timer for on change processing period. */

    /**
     * Constructs the service instance.
     */
    TopicHandlerService() :
        IService(),
        m_topicMetaDataList(),
        m_pluginMetaDataList(),
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
     * Get the entity id by plugin UID.
     * 
     * @param[in] uid   Plugin UID
     * 
     * @return Entity id
     */
    String getEntityIdByPluginUid(uint16_t uid);

    /**
     * Get the entity id by plugin alias.
     * 
     * @param[in] alias Plugin alias
     * 
     * @return Entity id
     */
    String getEntityIdByPluginAlias(const String& alias);

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
     * @param[in] topic             The topic name.
     * @param[in] hasChangedFunc    Function to retrieve whether the topic changes since last time.
     */
    void addToTopicMetaDataList(const String& deviceId, const String& entityId, const String& topic, HasChangedFunc hasChangedFunc);

    /**
     * Remove topic meta data from list of automatic publishing on change.
     * 
     * @param[in] deviceId  The device id which represents the physical device.
     * @param[in] entityId  The entity id which represents the entity of the device.
     * @param[in] topic     The topic name.
     */
    void removeFromTopicMetaDataList(const String& deviceId, const String& entityId, const String& topic);

    /**
     * Add plugin meta data to list of automatic publishing on change.
     * 
     * @param[in] deviceId  The device id which represents the physical device.
     * @param[in] plugin    The related plugin.
     * @param[in] topic     The topic name.
     */
    void addToPluginMetaDataList(const String& deviceId, IPluginMaintenance* plugin, const String& topic);

    /**
     * Remove plugin meta data from list of automatic publishing on change.
     * 
     * @param[in] deviceId  The device id which represents the physical device.
     * @param[in] plugin    The related plugin.
     */
    void removeFromPluginMetaDataList(const String& deviceId, IPluginMaintenance* plugin);

    /**
     * Process all plugin topics to check which one has changed.
     * For every changed one, notify the handlers about.
     */
    void processOnChange();

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

#endif  /* TOPIC_HANDLER_SERVICE_H */

/** @} */