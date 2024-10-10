/* MIT License
 *
 * Copyright (c) 2019 - 2024 Andreas Merkle <web@blue-andi.de>
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
 * @brief  OpenWeather plugin
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup plugin
 *
 * @{
 */

#ifndef OPENWEATHERPLUGIN_H
#define OPENWEATHERPLUGIN_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "./internal/View.h"
#include "IOpenWeatherCurrent.h"
#include "IOpenWeatherForecast.h"

#include <stdint.h>
#include <PluginWithConfig.hpp>
#include <AsyncHttpClient.h>
#include <TaskProxy.hpp>
#include <Mutex.hpp>
#include <FileSystem.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Shows weather informations provided by OpenWeather: https://openweathermap.org/
 */
class OpenWeatherPlugin : public PluginWithConfig
{
public:

    /**
     * The supported OpenWeather sources.
     */
    enum OpenWeatherSource
    {
        OPENWEATHER_SOURCE_CURRENT_FORECAST = 0,    /**< Current/Forecast weather data */
        OPENWEATHER_SOURCE_ONE_CALL_30,             /**< OpenWeather One-Call API v3.0 */
    };

    /**
     * Constructs the plugin.
     *
     * @param[in] name  Plugin name (must exist over lifetime)
     * @param[in] uid   Unique id
     */
    OpenWeatherPlugin(const char* name, uint16_t uid) :
        PluginWithConfig(name, uid, FILESYSTEM),
        m_view(),
        m_sourceId(OPENWEATHER_SOURCE_ONE_CALL_30),
        m_updatePeriod(UPDATE_PERIOD),
        m_sourceCurrent(nullptr),
        m_sourceForecast(nullptr),
        m_configurationFilename(),
        m_client(),
        m_weatherReqStatus(WEATHER_REQUEST_STATUS_IDLE),
        m_requestTimer(),
        m_mutex(),
        m_isConnectionError(false),
        m_slotInterf(nullptr),
        m_hasTopicChanged(false),
        m_taskProxy()
    {
        (void)m_mutex.create();
        createOpenWeatherCurrentSource(m_sourceId); /* Default */

        if (true == _OpenWeatherPlugin::View::isWeatherForecastSupported())
        {
            createOpenWeatherForecastSource(m_sourceId); /* Default */
        }
    }

    /**
     * Destroys the plugin.
     */
    ~OpenWeatherPlugin()
    {
        m_client.regOnResponse(nullptr);
        m_client.regOnClosed(nullptr);
        m_client.regOnError(nullptr);

        /* Abort any pending TCP request to avoid getting a callback after the
         * object is destroyed.
         */
        m_client.end();
        
        clearQueue();
        destroyOpenWeatherCurrentSource();

        if (true == _OpenWeatherPlugin::View::isWeatherForecastSupported())
        {
            destroyOpenWeatherForecastSource();
        }
        
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
        return new(std::nothrow)OpenWeatherPlugin(name, uid);
    }

    /**
     * Get font type.
     * 
     * @return The font type the plugin uses.
     */
    Fonts::FontType getFontType() const final
    {
        return m_view.getFontType();
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
        m_view.setFontType(fontType);
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
     * Set the slot interface, which the plugin can used to request information
     * from the slot, it is plugged in.
     *
     * @param[in] slotInterf    Slot interface
     */
    void setSlot(const ISlotPlugin* slotInterf) final;

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
     * This method will be called in case the plugin is set active, which means
     * it will be shown on the display in the next step.
     *
     * @param[in] gfx   Display graphics interface
     */
    void active(YAGfx& gfx) final;

    /**
     * This method will be called in case the plugin is set inactive, which means
     * it won't be shown on the display anymore.
     */
    void inactive() final;

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

    /** Weather request status */
    enum WeatherRequestStatus
    {
        WEATHER_REQUEST_STATUS_IDLE = 0,        /**< No weather request is running. */
        WEATHER_REQUEST_STATUS_CURRENT_PENDING, /**< Current weather request is pending. */
        WEATHER_REQUEST_STATUS_FORECAST_REQ,    /**< Forecast weather request is requested. */
        WEATHER_REQUEST_STATUS_FORECAST_PENDING /**< Forecast weather request is pending. */
    };

    /**
     * OpenWeather API base URI
     */
    static const char*      OPEN_WEATHER_BASE_URI;

    /**
     * Plugin topic, used to read/write the configuration.
     */
    static const char*      TOPIC_CONFIG;

    /**
     * Period in ms for requesting data from server.
     * This is used in case the last request to the server was successful.
     * 
     * Note, the OpenWeather recommendation is no more than once in 10 minutes.
     */
    static const uint32_t   UPDATE_PERIOD           = SIMPLE_TIMER_MINUTES(10U);

    /**
     * Short period in ms for requesting data from server.
     * This is used in case the request to the server failed.
     */
    static const uint32_t   UPDATE_PERIOD_SHORT     = SIMPLE_TIMER_SECONDS(10U);

    /** Time for duration tick period in ms */
    static const uint32_t   DURATION_TICK_PERIOD    = SIMPLE_TIMER_SECONDS(1U);

    _OpenWeatherPlugin::View    m_view;                     /**< View with all widgets. */
    OpenWeatherSource           m_sourceId;                 /**< OpenWeather source id. */
    uint32_t                    m_updatePeriod;             /**< Period in ms for requesting data from server. This is used in case the last request to the server was successful. */
    IOpenWeatherCurrent*        m_sourceCurrent;            /**< OpenWeather source to use to retrieve current weather information. */
    IOpenWeatherForecast*       m_sourceForecast;           /**< OpenWeather source to use to retrieve forecast weather information. */
    String                      m_configurationFilename;    /**< String used for specifying the configuration filename. */
    AsyncHttpClient             m_client;                   /**< Asynchronous HTTP client. */
    WeatherRequestStatus        m_weatherReqStatus;         /**< The weather request status. */
    SimpleTimer                 m_requestTimer;             /**< Timer used for cyclic request of new data. */
    mutable MutexRecursive      m_mutex;                    /**< Mutex to protect against concurrent access. */
    bool                        m_isConnectionError;        /**< Is connection error happened? */
    const ISlotPlugin*          m_slotInterf;               /**< Slot interface */
    bool                        m_hasTopicChanged;          /**< Has the topic content changed? */

    /**
     * Defines the message types, which are necessary for HTTP client/server handling.
     */
    enum MsgType
    {
        MSG_TYPE_INVALID = 0,   /**< Invalid message type. */
        MSG_TYPE_RSP,           /**< A response, caused by a previous request. */
        MSG_TYPE_CONN_CLOSED,   /**< The connection is closed. */
        MSG_TYPE_CONN_ERROR     /**< A connection error happened. */
    };

    /**
     * A message for HTTP client/server handling.
     */
    struct Msg
    {
        MsgType                 type;   /**< Message type */
        DynamicJsonDocument*    rsp;    /**< Response, only valid if message type is a response. */

        /**
         * Constructs a message.
         */
        Msg() :
            type(MSG_TYPE_INVALID),
            rsp(nullptr)
        {
        }
    }; 

    /**
     * Task proxy used to decouple server responses, which happen in a different task context.
     */
    TaskProxy<Msg, 2U, 0U> m_taskProxy;

    /**
     * Create OpenWeather current source according to id.
     * 
     * @param[in] id    OpenWeather source id
     */
    void createOpenWeatherCurrentSource(OpenWeatherSource id);

    /**
     * Create OpenWeather forecast source according to id.
     * 
     * @param[in] id    OpenWeather source id
     */
    void createOpenWeatherForecastSource(OpenWeatherSource id);

    /**
     * Destroy OpenWeather current source.
     */
    void destroyOpenWeatherCurrentSource();

    /**
     * Destroy OpenWeather forecast source.
     */
    void destroyOpenWeatherForecastSource();

    /**
     * Get configuration in JSON.
     * 
     * @param[out] cfg  Configuration
     */
    void getConfiguration(JsonObject& jsonCfg) const final;

    /**
     * Set configuration in JSON.
     * 
     * @param[in] cfg   Configuration
     * 
     * @return If successful set, it will return true otherwise false.
     */
    bool setConfiguration(const JsonObjectConst& jsonCfg) final;

    /**
     * Updates the text and icon, which to be displayed.
     *
     * @param[in] force Force update.
     */
    void updateDisplay(bool force);

    /**
     * Request new data.
     * 
     * @param[in] source    Request source
     * 
     * @return If successful it will return true otherwise false.
     */
    bool startHttpRequest(const IOpenWeatherGeneric* source);

    /**
     * Register callback function on response reception.
     */
    void initHttpClient(void);

    /**
     * Handle asynchronous web response from the server.
     * This will be called in LwIP context! Don't modify any member here directly!
     * 
     * @param[in] jsonDoc   Web response as JSON document
     */
    void handleAsyncWebResponse(const HttpResponse& rsp);

    /**
     * Handle a web response from the server.
     * 
     * @param[in] jsonDoc   Web response as JSON document
     */
    void handleWebResponse(const DynamicJsonDocument& jsonDoc);

    /**
     * Clear the task proxy queue.
     */
    void clearQueue();

    /**
     * Get the weather source depended on the current weather request status.
     * 
     * @return If weather request status is invalid, it will return nullptr otherwise the source.
     */
    IOpenWeatherGeneric* getWeatherSourceByStatus();

    /**
     * Signals that a HTTP request was started and will maintain the internal
     * weather request status.
     */
    void weatherRequestStarted();
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* OPENWEATHERPLUGIN_H */

/** @} */