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
#include <vector>

#include "HomeAssistantMqtt.h"

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
        m_listOfTopicStates(),
        m_isMqttConnected(false),
        m_haExtension()
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
     * Start the topic handler.
     */
    void start() final;

    /**
     * Stop the topic handler.
     */
    void stop() final;

    /**
     * Register the topic.
     * 
     * @param[in] deviceId      The device id which represents the physical device.
     * @param[in] entityId      The entity id which represents the entity of the device.
     * @param[in] topic         The topic name.
     * @param[in] extra         Extra parameters, which depend on the topic handler.
     * @param[in] getTopicFunc  Function to get the topic content.
     * @param[in] setTopicFunc  Function to set the topic content.
     * @param[in] uploadReqFunc Function used for requesting whether an file upload is allowed.
     */
    void registerTopic(const String& deviceId, const String& entityId, const String& topic, JsonObjectConst& extra, GetTopicFunc getTopicFunc, SetTopicFunc setTopicFunc, UploadReqFunc uploadReqFunc) final;

    /**
     * Unregister the topic.
     * 
     * @param[in] deviceId  The device id which represents the physical device.
     * @param[in] entityId  The entity id which represents the entity of the device.
     * @param[in] topic     The topic name.
     */
    void unregisterTopic(const String& deviceId, const String& entityId, const String& topic) final;

    /**
     * Process the topic handler.
     */
    void process() final;

    /**
     * Notify that the topic has changed.
     * 
     * @param[in] deviceId  The device id which represents the physical device.
     * @param[in] entityId  The entity id which represents the entity of the device.
     * @param[in] topic     The topic name.
     */
    void notify(const String& deviceId, const String& entityId, const String& topic) final;

private:

    /** A topic state is published by a plugin. */
    struct TopicState
    {
        String          deviceId;       /**< The device id. */
        String          entityId;       /**< The entity id. */
        String          topic;          /**< The topic which provides its state. */
        GetTopicFunc    getTopicFunc;   /**< Function used to get topic content. */
        SetTopicFunc    setTopicFunc;   /**< Function used to set topic content. */
        UploadReqFunc   uploadReqFunc;  /**< Function used to check whether a file upload is allowed. */
        bool            isPublishReq;   /**< Is it required to publish the state? */

        /** Construct topic state. */
        TopicState() :
            deviceId(),
            entityId(),
            topic(),
            getTopicFunc(nullptr),
            setTopicFunc(nullptr),
            uploadReqFunc(nullptr),
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

    ListOfTopicStates   m_listOfTopicStates;    /**< List of registered topic states. */
    bool                m_isMqttConnected;      /**< Is the MQTT connection to the broker established? */
    HomeAssistantMqtt   m_haExtension;          /**< Home Assistant extension */

    MqttApiTopicHandler(const MqttApiTopicHandler& adapter);
    MqttApiTopicHandler& operator=(const MqttApiTopicHandler& adapter);

    /**
     * Request to publish all topic states.
     */
    void requestToPublishAllTopicStates();

    /**
     * Publish topic states, which are requested.
     * 
     * Note: Need to be called continously and will only publish one info per
     *       call cycle.
     */
    void publishTopicStatesOnDemand();

    /**
     * Write topic data.
     * 
     * @param[in] deviceId      The device id which represents the physical device.
     * @param[in] entityId      The entity id which represents the entity of the device.
     * @param[in] topic         The topic name.
     * @param[in] payload       The payload data.
     * @param[in] size          The payload size in byte.
     * @param[in] setTopicFunc  Function to set the topic content.
     * @param[in] uploadReqFunc Function used for requesting whether an file upload is allowed.
     */
    void write(const String& deviceId, const String& entityId, const String& topic, const uint8_t* payload, size_t size, SetTopicFunc setTopicFunc, UploadReqFunc uploadReqFunc);

    /**
     * Publish topic data.
     * 
     * @param[in] deviceId      The device id which represents the physical device.
     * @param[in] entityId      The entity id which represents the entity of the device.
     * @param[in] topic         The topic name.
     * @param[in] getTopicFunc  Function to get the topic content.
     */
    void publish(const String& deviceId, const String& entityId, const String& topic, GetTopicFunc getTopicFunc);

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