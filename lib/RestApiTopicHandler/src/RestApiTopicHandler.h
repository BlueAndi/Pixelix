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
 *
 * @addtogroup plugin
 *
 * @{
 */

#ifndef REST_API_TOPIC_HANDLER_H
#define REST_API_TOPIC_HANDLER_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <ITopicHandler.h>
#include <vector>
#include <ESPAsyncWebServer.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Adapts the REST API web service to the topic handler interface.
 */
class RestApiTopicHandler : public ITopicHandler
{
public:

    /**
     * Construct the REST API web topic handler adapter.
     */
    RestApiTopicHandler() :
        ITopicHandler(),
        m_listOfTopicMetaData()
    {
    }

    /**
     * Destroy the REST API web topic handler adapter.
     */
    ~RestApiTopicHandler()
    {
        clearPluginTopics();
    }

    /**
     * Start the topic handler.
     */
    void start() final
    {
        /* Nothing to do. */
    }

    /**
     * Stop the topic handler.
     */
    void stop() final
    {
        /* Nothing to do. */
    }

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
    void process() final
    {
        /* Nothing to do. */
    }

    /**
     * Notify that the topic has changed.
     * 
     * @param[in] deviceId  The device id which represents the physical device.
     * @param[in] entityId  The entity id which represents the entity of the device.
     * @param[in] topic     The topic name.
     */
    void notify(const String& deviceId, const String& entityId, const String& topic) final
    {
        /* Nothing to do. */
        (void)deviceId;
        (void)entityId;
        (void)topic;
    }

private:

    /**
     * Topic meta data.
     */
    struct TopicMetaData
    {
        String                      deviceId;       /**< The device id. */
        String                      entityId;       /**< The entity id. */
        String                      topic;          /**< The plugin topic. */
        GetTopicFunc                getTopicFunc;   /**< Function used to get topic content. */
        SetTopicFunc                setTopicFunc;   /**< Function used to set topic content. */
        UploadReqFunc               uploadReqFunc;  /**< Function used to check whether a file upload is allowed. */
        AsyncCallbackWebHandler*    webHandler;     /**< Webhandler callback, necessary to remove it later again. */
        String                      uri;            /**< URI where the handler is registered. */
        bool                        isUploadError;  /**< If upload error happened, it will be true otherwise false. */
        String                      fullPath;       /**< Full path of uploaded file. If empty, there is no file available. */

        /**
         * Initialize topic meta data.
         */
        TopicMetaData() :
            deviceId(),
            entityId(),
            topic(),
            getTopicFunc(nullptr),
            setTopicFunc(nullptr),
            uploadReqFunc(nullptr),
            webHandler(nullptr),
            uri(),
            isUploadError(false),
            fullPath()
        {
        }
    };

    /**
     * List of topic meta data.
     */
    typedef std::vector<TopicMetaData*> ListOfTopicMetaData;

    ListOfTopicMetaData m_listOfTopicMetaData;  /**< List of topic meta data. */

    RestApiTopicHandler(const RestApiTopicHandler& adapter);
    RestApiTopicHandler& operator=(const RestApiTopicHandler& adapter);

    /**
     * Get plugin REST base URI to identify plugin.
     *
     * @param[in] entityId  The entity id which represents the entity of the device.
     *
     * @return Plugin REST API base URI
     */
    String getBaseUri(const String& entityId);

    /**
     * The web request handler handles all incoming HTTP requests for every plugin topic.
     * 
     * @param[in] request       The web request information from the client.
     * @param[in] topicMetaData The related topic meta data.
     */
    void webReqHandler(AsyncWebServerRequest *request, TopicMetaData* topicMetaData);

    /**
     * File upload handler.
     *
     * @param[in] request       HTTP request.
     * @param[in] filename      Name of the uploaded file.
     * @param[in] index         Current file offset.
     * @param[in] data          Next data part of file, starting at offset.
     * @param[in] len           Data part size in byte.
     * @param[in] final         Is final packet or not.
     * @param[in] topicMetaData The related topic meta data.
     */
    void uploadHandler(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final, TopicMetaData* topicMetaData);

    /**
     * Convert HTTP parameters to JSON format.
     * 
     * @param[in, out]  jsonDocPar  JSON document which the parameters will be transfered to.
     * @param[in]       request     HTTP request with parameters
     */
    void par2Json(JsonDocument& jsonDocPar, AsyncWebServerRequest *request);

    /**
     * Clear plugin topics.
     */
    void clearPluginTopics();
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* REST_API_TOPIC_HANDLER_H */

/** @} */