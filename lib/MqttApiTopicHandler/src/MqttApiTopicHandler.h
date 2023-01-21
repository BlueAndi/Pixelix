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

#ifndef __MQTT_API_TOPIC_HANDLER_H__
#define __MQTT_API_TOPIC_HANDLER_H__

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
        m_hostname()
    {
    }

    /**
     * Destroy the MQTT topic handler adapter.
     */
    ~MqttApiTopicHandler()
    {
    }

    /**
     * Register all topics of the given plugin.
     * 
     * @param[in] plugin    The plugin, which topics shall be registered.
     */
    void registerTopics(IPluginMaintenance* plugin) final;

    /**
     * Unregister all topics of the given plugin.
     * 
     * @param[in] plugin    The plugin, which topics to unregister.
     */
    void unregisterTopics(IPluginMaintenance* plugin) final;

private:

    /**
     * Max. file size in byte.
     */
    static const size_t MAX_FILE_SIZE   = 1024U;

    String  m_hostname; /**< Hostname cache used for the base URI */

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
     * @param[in] baseUri   The MQTT API base URI.
     * @param[in] plugin    The related plugin.
     * @param[in] topic     The topic.
     */
    void registerTopic(const String& baseUri, IPluginMaintenance* plugin, const String& topic);

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
     * @param[in] baseUri   The MQTT API base URI.
     * @param[in] plugin    The related plugin.
     * @param[in] topic     The topic.
     */
    void unregisterTopic(const String& baseUri, IPluginMaintenance* plugin, const String& topic);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __MQTT_API_TOPIC_HANDLER_H__ */

/** @} */