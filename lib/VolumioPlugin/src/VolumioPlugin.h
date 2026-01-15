/* MIT License
 *
 * Copyright (c) 2019 - 2026 Andreas Merkle <web@blue-andi.de>
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
 * @file   VolumioPlugin.h
 * @brief  VOLUMIO plugin
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup PLUGIN
 *
 * @{
 */

#ifndef VOLUMIOPLUGIN_H
#define VOLUMIOPLUGIN_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "./internal/View.h"

#include <stdint.h>
#include <PluginWithConfig.hpp>
#include <Mutex.hpp>
#include <FileSystem.h>
#include <RestService.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Shows the current state of VOLUMIO and the artist/title of the played music.
 * If the VOLUMIO server is offline, the plugin gets automatically disabled,
 * otherwise enabled.
 */
class VolumioPlugin : public PluginWithConfig
{
public:

    /**
     * Constructs the plugin.
     *
     * @param[in] name  Plugin name (must exist over lifetime)
     * @param[in] uid   Unique id
     */
    VolumioPlugin(const char* name, uint16_t uid) :
        PluginWithConfig(name, uid, FILESYSTEM),
        m_view(),
        m_volumioHost("volumio.fritz.box"),
        m_urlIcon(),
        m_urlText(),
        m_requestTimer(),
        m_offlineTimer(),
        m_mutex(),
        m_lastSeekValue(0U),
        m_pos(0U),
        m_state(STATE_UNKNOWN),
        m_hasTopicChanged(false),
        m_dynamicRestId(RestService::INVALID_REST_ID),
        m_isAllowedToSend(true)
    {
        (void)m_mutex.create();
    }

    /**
     * Destroys the plugin.
     */
    ~VolumioPlugin()
    {
        m_mutex.destroy();
    }

    /**
     * Plugin creation method, used to register on the plugin manager.
     *
     * @param[in] name  Plugin name (must exist over lifetime)
     * @param[in] uid   Unique id
     *
     * @return If successful, it will return the pointer to the plugin instance, otherwise nullptr.
     */
    static IPluginMaintenance* create(const char* name, uint16_t uid)
    {
        return new (std::nothrow) VolumioPlugin(name, uid);
    }

    /**
     * Get plugin topics, which can be get/set via different communication
     * interfaces like REST, websocket, MQTT, etc.
     *
     * Example:
     * <code>{.json}
     * {
     *     "topics": [
     *         "text"
     *     ]
     * }
     * </code>
     *
     * By default a topic is readable and writeable.
     * This can be set explicit with the "access" key with the following possible
     * values:
     * - Only readable: "r"
     * - Only writeable: "w"
     * - Readable and writeable: "rw"
     *
     * Example:
     * <code>{.json}
     * {
     *     "topics": [{
     *         "name": "text",
     *         "access": "r"
     *     }]
     * }
     * </code>
     *
     * Homeassistant MQTT discovery support can be added with the "ha" JSON object inside
     * the "extra" JSON object.
     * <code>{.json}
     * {
     *     "topics": [{
     *         "name": "text",
     *         "extra": {
     *             "ha": {
     *                 ... everything here will be used for MQTT discovery ...
     *             }
     *         }
     *     }]
     * }
     * </code>
     *
     * Extra information can be loaded from a file too. This is useful for complex
     * configurations and to keep program memory usage low.
     * <code>{.json}
     * {
     *     "topics": [{
     *         "name": "text",
     *         "extra": "extra.json"
     *    }]
     * }
     * </code>
     *
     * @param[out] topics   Topis in JSON format
     */
    void getTopics(JsonArray& topics) const final;

    /**
     * Get a topic data.
     * Note, currently only JSON format is supported.
     *
     * @param[in]   topic   The topic which data shall be retrieved.
     * @param[out]  value   The topic value in JSON format.
     *
     * @return If successful it will return true otherwise false.
     */
    bool getTopic(const String& topic, JsonObject& value) const final;

    /**
     * Set a topic data.
     * Note, currently only JSON format is supported.
     *
     * @param[in]   topic   The topic which data shall be retrieved.
     * @param[in]   value   The topic value in JSON format.
     *
     * @return If successful it will return true otherwise false.
     */
    bool setTopic(const String& topic, const JsonObjectConst& value) final;

    /**
     * Is the topic content changed since last time?
     * Every readable volatile topic shall support this. Otherwise the topic
     * handlers might not be able to provide updated information.
     *
     * @param[in] topic The topic which to check.
     *
     * @return If the topic content changed since last time, it will return true otherwise false.
     */
    bool hasTopicChanged(const String& topic) final;

    /**
     * Start the plugin. This is called only once during plugin lifetime.
     * It can be used as deferred initialization (after the constructor)
     * and provides the canvas size.
     *
     * If your display layout depends on canvas or font size, calculate it
     * here.
     *
     * Overwrite it if your plugin needs to know that it was installed.
     *
     * @param[in] width     Display width in pixel
     * @param[in] height    Display height in pixel
     */
    void start(uint16_t width, uint16_t height) final;

    /**
     * Stop the plugin. This is called only once during plugin lifetime.
     * It can be used as a first clean-up, before the plugin will be destroyed.
     *
     * Overwrite it if your plugin needs to know that it will be uninstalled.
     */
    void stop() final;

    /**
     * Process the plugin.
     * Overwrite it if your plugin has cyclic stuff to do without being in a
     * active slot.
     *
     * @param[in] isConnected   The network connection status. If network
     *                          connection is established, it will be true otherwise false.
     */
    void process(bool isConnected) final;

    /**
     * Update the display.
     * The scheduler will call this method periodically.
     *
     * @param[in] gfx   Display graphics interface
     */
    void update(YAGfx& gfx) final;

private:

    /**
     * The different Volumio player states.
     */
    enum VolumioState
    {
        STATE_UNKNOWN = 0, /**< Unknown state */
        STATE_STOP,        /**< Volumio player is stopped */
        STATE_PLAY,        /**< Volumio player plays */
        STATE_PAUSE        /**< Volumio player is paused */
    };

    /**
     * Icon width in pixels.
     */
    static const uint16_t ICON_WIDTH  = 8U;

    /**
     * Icon height in pixels.
     */
    static const uint16_t ICON_HEIGHT = 8U;

    /**
     * Plugin topic, used to read/write the configuration.
     */
    static const char* TOPIC_CONFIG;

    /**
     * Period in ms for requesting data from server.
     * This is used in case the last request to the server was successful.
     * The period is shorter than the UPDATE_PERIOD_SHORT, because if the music
     * changes, the display shall be updated more or less immediately.
     */
    static const uint32_t UPDATE_PERIOD       = SIMPLE_TIMER_SECONDS(2U);

    /**
     * Short period in ms for requesting data from server.
     * This is used in case the request to the server failed.
     */
    static const uint32_t UPDATE_PERIOD_SHORT = SIMPLE_TIMER_SECONDS(10U);

    /**
     * Period in ms after which the plugin gets automatically disabled if no new
     * data is available.
     */
    static const uint32_t  OFFLINE_PERIOD     = SIMPLE_TIMER_SECONDS(60U);

    _VolumioPlugin::View   m_view;            /**< View with all widgets. */
    String                 m_volumioHost;     /**< Host address of the VOLUMIO server. */
    String                 m_urlIcon;         /**< REST API URL for updating the icon */
    String                 m_urlText;         /**< REST API URL for updating the text */
    SimpleTimer            m_requestTimer;    /**< Timer used for cyclic request of new data. */
    SimpleTimer            m_offlineTimer;    /**< Timer used for offline detection. */
    mutable MutexRecursive m_mutex;           /**< Mutex to protect against concurrent access. */
    uint32_t               m_lastSeekValue;   /**< Last seek value, retrieved from VOLUMIO. Used to cross-check the provided status. */
    uint8_t                m_pos;             /**< Current music position in percent. */
    VolumioState           m_state;           /**< Volumio player state */
    bool                   m_hasTopicChanged; /**< Has the topic content changed? */
    uint32_t               m_dynamicRestId;   /**< Used to identify plugin when interacting with RestService. Id changes with every request. */
    bool                   m_isAllowedToSend; /**< Is allowed to send REST-Api request? */

    /**
     * Get configuration in JSON.
     *
     * @param[out] jsonCfg   Configuration
     */
    void getConfiguration(JsonObject& jsonCfg) const final;

    /**
     * Set configuration in JSON.
     *
     * @param[in] jsonCfg   Configuration
     *
     * @return If successful set, it will return true otherwise false.
     */
    bool setConfiguration(const JsonObjectConst& jsonCfg) final;

    /**
     * Change Volumio player state.
     * Depended on the new state, the corresponding bitmap icon is enabled.
     *
     * @param[in] state Current player state
     */
    void changeState(VolumioState state);

    /**
     * Request new data.
     *
     * @return If successful it will return true otherwise false.
     */
    bool startHttpRequest(void);

    /**
     * Handle asynchronous web response from the server.
     * This will be called in LwIP context! Don't modify any member here directly!
     *
     * @param[in] payload     Payload of the web response
     * @param[in] payloadSize Size of the payload
     * @param[out] jsonDoc    DynamicJsonDocument used to store result in.
     *
     * @return If successful it will return true otherwise false.
     */
    bool preProcessAsyncWebResponse(const char* payload, size_t payloadSize, DynamicJsonDocument& jsonDoc);

    /**
     * Handle a web response from the server.
     *
     * @param[in] jsonDoc   Web response as JSON document
     */
    void handleWebResponse(const DynamicJsonDocument& jsonDoc);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* VOLUMIOPLUGIN_H */

/** @} */