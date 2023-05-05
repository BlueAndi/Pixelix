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
 *
 * @addtogroup plugin
 *
 * @{
 */

#ifndef MQTT_API_TOPIC_HANDLER_H
#define MQTT_API_TOPIC_HANDLER_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <ITopicHandler.h>
#include <IPluginMaintenance.hpp>
#include <vector>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Adapts the MQTT service to the topic handler interface.
 */
class MqttApiTopicHandler : public ITopicHandler
{
public:

    /**
     * Construct the MQTT topic handler adapter.
     */
    MqttApiTopicHandler() :
        ITopicHandler(),
        m_hostname(),
        m_listOfTopicStates(),
        m_isMqttConnected(false)
    {
    }

    /**
     * Destroy the MQTT topic handler adapter.
     */
    ~MqttApiTopicHandler()
    {
        clearTopicStates();
    }

    /**
     * Register a single topic of the given plugin.
     * 
     * @param[in] plugin    The plugin which provides the topic.
     * @param[in] topic     The topic name.
     * @param[in] access    The topic accessibility.
     * @param[in] extra     Extra parameters, which depend on the topic handler.
     */
    void registerTopic(IPluginMaintenance* plugin, const String& topic, Access access, JsonObjectConst& extra) final;

    /**
     * Unregister the topic of the given plugin.
     * 
     * @param[in] plugin    The plugin which provides the topic.
     * @param[in] topic     The topic name.
     */
    void unregisterTopic(IPluginMaintenance* plugin, const String& topic) final;

    /**
     * Process the topic handler.
     */
    void process() final;

    /**
     * Notify that the topic has changed.
     * 
     * @param[in] plugin    The plugin which provides the topic.
     * @param[in] topic     The topic name.
     */
    void notify(IPluginMaintenance* plugin, const String& topic) final;

private:

    /** A topic state is published by a plugin. */
    struct TopicState
    {
        IPluginMaintenance* plugin;         /**< The plugin which provides the topic. */
        String              topic;          /**< The topic which provides its state. */
        String              topicUri;       /**< The topic MQTT URI without endpoint. */
        Access              access;         /**< The topic accessibility. */
        bool                isPublishReq;   /**< Is it required to publish the state? */

        /** Construct topic state. */
        TopicState() :
            plugin(nullptr),
            topic(),
            topicUri(),
            access(ACCESS_READ_WRITE),
            isPublishReq(false)
        {
        }
    };

    /** List of topic states. */
    typedef std::vector<TopicState*> ListOfTopicStates;

    /**
     * Max. file size in byte.
     */
    static const size_t MAX_FILE_SIZE   = 1024U;

    /** MQTT path endpoint for read access. */
    static const char*  MQTT_ENDPOINT_READ_ACCESS;

    /** MQTT path endpoint for write access. */
    static const char*  MQTT_ENDPOINT_WRITE_ACCESS;

    String              m_hostname;             /**< Hostname cache used for the base URI */
    ListOfTopicStates   m_listOfTopicStates;    /**< List of registered plugins. */
    bool                m_isMqttConnected;      /**< Is the MQTT connection to the broker established? */

    MqttApiTopicHandler(const MqttApiTopicHandler& adapter);
    MqttApiTopicHandler& operator=(const MqttApiTopicHandler& adapter);

    /**
     * Get plugin MQTT base URI to identify plugin by UID.
     *
     * @param[in] uid   Plugin UID
     *
     * @return Plugin MQTT API base URI
     */
    String getBaseUriByUid(uint16_t uid);

    /**
     * Get plugin MQTT base URI to identify plugin by alias name.
     *
     * @param[in] alias Plugin alias name
     *
     * @return Plugin MQTT API base URI
     */
    String getBaseUriByAlias(const String& alias);

    /**
     * Register a single topic of the given plugin.
     * 
     * @param[in] plugin    The plugin which provides the topic.
     * @param[in] topic     The topic name.
     * @param[in] access    The topic accessibility.
     * @param[in] extra     Extra parameters, which depend on the topic handler.
     * @param[in] baseUri   The REST API base URI which to use.
     */
    void registerTopic(IPluginMaintenance* plugin, const String& topic, Access access, JsonObjectConst& extra, const String& baseUri);

    /**
     * Write topic data.
     * 
     * @param[in] plugin    The plugin which relates to the topic.
     * @param[in] topicUri  The topic URI
     * @param[in] payload   The payload data.
     * @param[in] size      The payload size in byte.
     */
    void write(IPluginMaintenance* plugin, const String& topicUri, const uint8_t* payload, size_t size);

    /**
     * Unregister a single topic of the given plugin.
     * 
     * @param[in] plugin    The related plugin.
     * @param[in] topic     The topic.
     * @param[in] baseUri   The MQTT API base URI.
     */
    void unregisterTopic(IPluginMaintenance* plugin, const String& topic, const String& baseUri);

    /**
     * Publish plugin specific topic data.
     * 
     * @param[in] baseUri   The MQTT API base URI.
     * @param[in] plugin    The related plugin.
     * @param[in] topic     The topic.
     */
    void publish(const String& baseUri, IPluginMaintenance* plugin, const String& topic);

    /**
     * Clear all topic states.
     */
    void clearTopicStates();
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* MQTT_API_TOPIC_HANDLER_H */

/** @} */