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
#include <IPluginMaintenance.hpp>
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
        m_pluginTopicList()
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
    void process() final
    {
        /* Nothing to do. */
    }

    /**
     * Notify that the topic has changed.
     * 
     * @param[in] plugin    The plugin which provides the topic.
     * @param[in] topic     The topic name.
     */
    void notify(IPluginMaintenance* plugin, const String& topic) final
    {
        /* Nothing to do. */
    }

private:

    /**
     * Plugin topic relevant data.
     */
    struct PluginTopic
    {
        IPluginMaintenance*         plugin;         /**< The plugin which provides the topic. */
        String                      topic;          /**< The plugin topic. */
        AsyncCallbackWebHandler*    webHandler;     /**< Webhandler callback, necessary to remove it later again. */
        String                      uri;            /**< URI where the handler is registered. */
        bool                        isUploadError;  /**< If upload error happened, it will be true otherwise false. */
        String                      fullPath;       /**< Full path of uploaded file. If empty, there is no file available. */
        File                        fd;             /**< Upload file descriptor */
        Access                      access;         /**< Access to the topic (r, rw, w) */

        /**
         * Initialize the web handler data.
         */
        PluginTopic() :
            plugin(nullptr),
            topic(),
            webHandler(nullptr),
            uri(),
            isUploadError(false),
            fullPath(),
            fd(),
            access(ACCESS_READ_WRITE)
        {
        }
    };

    /**
     * List of plugin object data.
     */
    typedef std::vector<PluginTopic*> PluginTopicList;

    PluginTopicList m_pluginTopicList;  /**< List of plugin topics with webhandler data. */

    RestApiTopicHandler(const RestApiTopicHandler& adapter);
    RestApiTopicHandler& operator=(const RestApiTopicHandler& adapter);

    /**
     * Get plugin REST base URI to identify plugin by UID.
     *
     * @param[in] uid   Plugin UID
     *
     * @return Plugin REST API base URI
     */
    String getBaseUriByUid(uint16_t uid);

    /**
     * Get plugin REST base URI to identify plugin by alias name.
     *
     * @param[in] alias Plugin alias name
     *
     * @return Plugin REST API base URI
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
     * The web request handler handles all incoming HTTP requests for every plugin topic.
     * 
     * @param[in] request       The web request information from the client.
     * @param[in] pluginTopic   The related plugin topic data.
     */
    void webReqHandler(AsyncWebServerRequest *request, PluginTopic* pluginTopic);

    /**
     * File upload handler.
     *
     * @param[in] request           HTTP request.
     * @param[in] filename          Name of the uploaded file.
     * @param[in] index             Current file offset.
     * @param[in] data              Next data part of file, starting at offset.
     * @param[in] len               Data part size in byte.
     * @param[in] final             Is final packet or not.
     * @param[in] pluginTopic   The related plugin topic data.
     */
    void uploadHandler(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final, PluginTopic* pluginTopic);

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