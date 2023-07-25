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
 * @brief  Gets Chicago Bus Tracker info
 * @author Queen K Juul <queenkjuul69@gmail.com>
 * @addtogroup plugin
 *
 * @{
 */

#ifndef ChicagoBusTrackerPlugin_H
#define ChicagoBusTrackerPlugin_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "AsyncHttpClient.h"
#include "Plugin.hpp"

#include <WidgetGroup.h>
#include <BitmapWidget.h>
#include <stdint.h>
#include <TextWidget.h>
#include <SimpleTimer.hpp>
#include <TaskProxy.hpp>
#include <Mutex.hpp>
#include <FileSystem.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

// Gets upcoming bus arrivals for a given route, stop, and direction
class ChicagoBusTrackerPlugin : public Plugin, private PluginConfigFsHandler
{
public:
    /**
     * Constructs the plugin.
     *
     * @param[in] name  Plugin name
     * @param[in] uid   Unique id
     */
    ChicagoBusTrackerPlugin(const String &name, uint16_t uid) : Plugin(name, uid),
                                                            PluginConfigFsHandler(uid, FILESYSTEM),
                                                            m_fontType(Fonts::FONT_TYPE_DEFAULT),
                                                            m_arrivalsCanvas(),
                                                            m_routeCanvas(),
                                                            m_arrivalsTextWidget("\\calign?"),
                                                            m_routeTextWidget("\\calign?"),
                                                            m_relevantResponsePart(" NO DATA "),
                                                            m_routeResponsePart(" - "),
                                                            m_displayColor("\\#FF5500"),
                                                            m_delayColor("\\#FD1000"),
                                                            m_dueColor("\\#00AF02"),
                                                            m_client(),
                                                            m_mutex(),
                                                            m_requestTimer(),
                                                            m_taskProxy(),
                                                            m_rte("55"),
                                                            m_dir("Eastbound"),
                                                            m_stpid("10507"),
                                                            m_orig(true),
                                                            m_dest(true),
                                                            m_two(),
                                                            m_apiKeyFilename("/configuration/chicago_bus_key.json"),
                                                            m_cfgReloadTimer(),
                                                            m_slotInterf(nullptr),
                                                            m_reloadConfigReq(false),
                                                            m_storeConfigReq(false),
                                                            m_hasTopicChanged(false)
    {
        (void)m_mutex.create();
    }

    /**
     * Destroys the plugin.
     */
    ~ChicagoBusTrackerPlugin()
    {
        m_client.regOnResponse(nullptr);
        m_client.regOnClosed(nullptr);
        m_client.regOnError(nullptr);

        /* Abort any pending TCP request to avoid getting a callback after the
         * object is destroyed.
         */
        m_client.end();

        clearQueue();

        m_mutex.destroy();
    }

    /**
     * Plugin creation method, used to register on the plugin manager.
     *
     * @param[in] name  Plugin name
     * @param[in] uid   Unique id
     *
     * @return If successful, it will return the pointer to the plugin instance, otherwise nullptr.
     */
    static IPluginMaintenance *create(const String &name, uint16_t uid)
    {
        return new ChicagoBusTrackerPlugin(name, uid);
    }

    /**
     * Get font type.
     *
     * @return The font type the plugin uses.
     */
    Fonts::FontType getFontType() const final
    {
        return m_fontType;
    }

    /**
     * Set font type.
     * The plugin may skip the font type in case it gets conflicts with the layout.
     *
     * A font type change will only be considered if it is set before the start()
     * method is called!
     *
     * @param[in] fontType  The font type which the plugin shall use.
     */
    void setFontType(Fonts::FontType fontType) final
    {
        m_fontType = fontType;

        return;
    }

    /**
     * Get plugin topics, which can be get/set via different communication
     * interfaces like REST, websocket, MQTT, etc.
     *
     * Example:
     * {
     *     "topics": [
     *         "/text"
     *     ]
     * }
     *
     * By default a topic is readable and writeable.
     * This can be set explicit with the "access" key with the following possible
     * values:
     * - Only readable: "r"
     * - Only writeable: "w"
     * - Readable and writeable: "rw"
     *
     * Example:
     * {
     *     "topics": [{
     *         "name": "/text",
     *         "access": "r"
     *     }]
     * }
     *
     * @param[out] topics   Topis in JSON format
     */
    void getTopics(JsonArray &topics) const final;

    /**
     * Get a topic data.
     * Note, currently only JSON format is supported.
     *
     * @param[in]   topic   The topic which data shall be retrieved.
     * @param[out]  value   The topic value in JSON format.
     *
     * @return If successful it will return true otherwise false.
     */
    bool getTopic(const String &topic, JsonObject &value) const final;

    /**
     * Set a topic data.
     * Note, currently only JSON format is supported.
     *
     * @param[in]   topic   The topic which data shall be retrieved.
     * @param[in]   value   The topic value in JSON format.
     *
     * @return If successful it will return true otherwise false.
     */
    bool setTopic(const String &topic, const JsonObject &value) final;

    /**
     * Is the topic content changed since last time?
     * Every readable volatile topic shall support this. Otherwise the topic
     * handlers might not be able to provide updated information.
     *
     * @param[in] topic The topic which to check.
     *
     * @return If the topic content changed since last time, it will return true otherwise false.
     */
    bool hasTopicChanged(const String &topic) final;

    /**
     * Set the slot interface, which the plugin can used to request information
     * from the slot, it is plugged in.
     *
     * @param[in] slotInterf    Slot interface
     */
    void setSlot(const ISlotPlugin *slotInterf) final;

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
    void update(YAGfx &gfx) final;

    /**
     * Get CTA Bus Tracker API key.
     *
     * @return CTA Bus Tracker API key
     */
    String getApiKey();

private:
    /**
     * Icon width in pixels.
     */
    static const int16_t RTE_SECTION_WIDTH = 12;

    /**
     * Icon height in pixels.
     */
    static const int16_t RTE_SECTION_HEIGHT = 8;

    static const char *CHICAGO_BUS_BASE_URI;

    static const char *TOPIC_CONFIG;

    static const char *TOPIC_ROUTES;

    static const char *TOPIC_DIRS;

    static const char *TOPIC_STOPS;

    /**
     * Period between arrival prediction updates.
     * API is updated every 1 minute, we go every 2 min to keep 
     * within the daily API limits when fetching multiple stops.
     */
    static const uint32_t UPDATE_PERIOD = SIMPLE_TIMER_MINUTES(2U);

    /**
     * 30 second timer used for scheduling config reloads and keeping
     * arrivals requests from colloding with web config requests
     */
    static const uint32_t UPDATE_PERIOD_SHORT = SIMPLE_TIMER_SECONDS(30U);

    /**
     * The configuration in the persistent memory shall be cyclic loaded.
     * This mechanism ensure that manual changes in the file are considered.
     * This is the reload period in ms.
     */
    static const uint32_t CFG_RELOAD_PERIOD = SIMPLE_TIMER_SECONDS(30U);

    Fonts::FontType m_fontType;      /**< Font type which shall be used if there is no conflict with the layout. */
    WidgetGroup m_arrivalsCanvas;    /**< Canvas used for the text widget. */
    WidgetGroup m_routeCanvas;       /**< Canvas used for the bitmap widget. */
    TextWidget m_arrivalsTextWidget; /**< Text widget, used for showing the text. */
    TextWidget m_routeTextWidget;    /**< Text widget, used for showing the text. */
    String m_relevantResponsePart;   /**< String used for the relevant part of the HTTP response. */
    String m_routeResponsePart;      /**< String returned as route we're tracking */
    String m_displayColor;           /**< String format prefix for main color */
    String m_delayColor;             /**< String format prefix for delayed status */
    String m_dueColor;               /**< String format prefix for bus due status */
    String m_apiKey;                 /**< CTA Bus Tracker API Key */
    String m_rte;                    /**< CTA Bus route ID (e.g. 81, 151, X49)*/
    String m_dir;                    /**< CTA-defined route direction (e.g. Southbound) */
    String m_stpid;                  /**< CTA-defined stop ID (numeric) */
    bool m_orig;                     /**< option to show origin (selected stop name) */
    bool m_dest;                     /**< option to show destination (end of line in chosen direction) */
    bool m_two;                      /**< option to show next 2 arrivals instead of just one */
    String m_apiKeyFilename;         /**< filename for API key shared across all ChicagoBusTrackerPlugin instances */
    AsyncHttpClient m_client;        /**< Asynchronous HTTP client. */
    mutable MutexRecursive m_mutex;  /**< Mutex to protect against concurrent access. */
    SimpleTimer m_requestTimer;      /**< Timer is used for cyclic bus times http request. */
    SimpleTimer m_cfgReloadTimer;    /**< Timer to check for updated configs e.g. updated via browser/REST */
    const ISlotPlugin *m_slotInterf; /**< interface for pixelix slot binding */
    bool m_storeConfigReq;
    bool m_reloadConfigReq;
    bool m_hasTopicChanged;

    /**
     * Defines the message types, which are necessary for HTTP client/server handling.
     */
    enum MsgType
    {
        MSG_TYPE_INVALID = 0, /**< Invalid message type. */
        MSG_TYPE_RSP          /**< A response, caused by a previous request. */
    };

    /**
     * A message for HTTP client/server handling.
     */
    struct Msg
    {
        MsgType type;             /**< Message type */
        DynamicJsonDocument *rsp; /**< Response, only valid if message type is a response. */

        /**
         * Constructs a message.
         */
        Msg() : type(MSG_TYPE_INVALID),
                rsp(nullptr)
        {
        }
    };

    /**
     * Task proxy used to decouple server responses, which happen in a different task context.
     */
    TaskProxy<Msg, 2U, 0U> m_taskProxy;

    /**
     * Request to store configuration to persistent memory.
     */
    void requestStoreToPersistentMemory();

    void getRoutes(JsonObject &rtes);

    void getDirections(JsonObject &dirs);

    void getStops(JsonObject &stops);

    /**
     * Get configuration in JSON.
     *
     * @param[out] cfg  Configuration
     */
    void getConfiguration(JsonObject &cfg) const final;

    /**
     * Set configuration in JSON.
     *
     * @param[in] cfg   Configuration
     *
     * @return If successful set, it will return true otherwise false.
     */
    bool setConfiguration(JsonObjectConst &cfg) final;

    /**
     * Request new data.
     *
     * @return If successful it will return true otherwise false.
     */
    bool startHttpRequest(void);

    /**
     * Register callback function on response reception.
     */
    void initHttpClient(void);

    /**
     * Handle a web response from the server.
     *
     * @param[in] jsonDoc   Web response as JSON document
     */
    void handleWebResponse(DynamicJsonDocument &jsonDoc);

    /**
     * Clear the task proxy queue.
     */
    void clearQueue();
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* ChicagoBusTrackerPlugin_H */

/** @} */
