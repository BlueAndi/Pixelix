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
        m_pluginMeta()
    {
    }

    /**
     * Destroy the REST API web topic handler adapter.
     */
    ~RestApiTopicHandler()
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
     * Web handler data, which is necessary for the webserver handling.
     */
    struct WebHandlerData
    {
        AsyncCallbackWebHandler*    webHandler;     /**< Webhandler callback, necessary to remove it later again. */
        String                      uri;            /**< URI where the handler is registered. */
        bool                        isUploadError;  /**< If upload error happened, it will be true otherwise false. */
        String                      fullPath;       /**< Full path of uploaded file. If empty, there is no file available. */
        File                        fd;             /**< Upload file descriptor */

        /**
         * Initialize the web handler data.
         */
        WebHandlerData() :
            webHandler(nullptr),
            uri(),
            isUploadError(false),
            fullPath(),
            fd()
        {
        }
    };

    /**
     * List of web handler data.
     */
    typedef std::vector<WebHandlerData*>    WebHandlerDataList;

    /**
     * Plugin object specific data, used for plugin management.
     */
    struct PluginObjData
    {
        IPluginMaintenance* plugin;         /**< Plugin object, where this data record belongs to. */
        WebHandlerDataList  webHandlers;    /**< Web handler data of the plugin, necessary to remove it later again. */

        /**
         * Initializes the plugin object data.
         */
        PluginObjData() :
            plugin(nullptr),
            webHandlers()
        {
        }
    };

    /**
     * List of plugin object data.
     */
    typedef std::vector<PluginObjData*> PluginObjDataList;

    PluginObjDataList m_pluginMeta; /**< Plugin object management information. */

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
     * @param[in] baseUri   The REST API base URI.
     * @param[in] metaData  The plugin meta data, which shall be handled.
     * @param[in] topic     The topic.
     */
    void registerTopic(const String& baseUri, PluginObjData* metaData, const String& topic);

    /**
     * The web request handler handles all incoming HTTP requests for every plugin topic.
     * 
     * @param[in] request           The web request information from the client.
     * @param[in] plugin            The responsible plugin, which is related to the request.
     * @param[in] topic             The topic, which is requested.
     * @param[in] webHandlerData    Plugin web handler data, which is related to this request.
     */
    void webReqHandler(AsyncWebServerRequest *request, IPluginMaintenance* plugin, const String& topic, WebHandlerData* webHandlerData);

    /**
     * File upload handler.
     *
     * @param[in] request           HTTP request.
     * @param[in] filename          Name of the uploaded file.
     * @param[in] index             Current file offset.
     * @param[in] data              Next data part of file, starting at offset.
     * @param[in] len               Data part size in byte.
     * @param[in] final             Is final packet or not.
     * @param[in] plugin            The responsible plugin, which is related to the upload.
     * @param[in] topic             The topic, which is requested.
     * @param[in] webHandlerData    Plugin web handler data, which is related to this upload.
     */
    void uploadHandler(AsyncWebServerRequest *request, const String& filename, size_t index, uint8_t *data, size_t len, bool final, IPluginMaintenance* plugin, const String& topic, WebHandlerData* webHandlerData);

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* REST_API_TOPIC_HANDLER_H */

/** @} */