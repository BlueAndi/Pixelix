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
#include "IOpenWeatherSource.h"

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
        OPENWEATHER_SOURCE_CURRENT = 0, /**< Current weather data */
        OPENWEATHER_SOURCE_ONE_CALL_25, /**< OpenWeather One-Call API v2.5 */
        OPENWEATHER_SOURCE_ONE_CALL_30, /**< OpenWeather One-Call API v3.0 */
    };

    /**
     * Enumeration to choose an additional weather information to be displayed.
     */
    enum OtherWeatherInformation
    {
        OTHER_WEATHER_INFO_UVI = 0,     /**< Display UV Index as additional information. */
        OTHER_WEATHER_INFO_HUMIDITY,    /**< Display humidity in % as additional information. */
        OTHER_WEATHER_INFO_WIND,        /**< Display windspeed in m/s as additional information. */
        OTHER_WEATHER_INFO_OFF          /**< Display only general weather information. */
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
        m_sourceId(OPENWEATHER_SOURCE_ONE_CALL_25),
        m_updatePeriod(UPDATE_PERIOD),
        m_source(nullptr),
        m_additionalInformation(OTHER_WEATHER_INFO_OFF),
        m_configurationFilename(),
        m_client(),
        m_requestTimer(),
        m_updateContentTimer(),
        m_mutex(),
        m_isConnectionError(false),
        m_currentTemp("\\calign?"),
        m_currentWeatherIconFullPath(),
        m_currentUvIndex("\\calign?"),
        m_currentHumidity("\\calign?"),
        m_currentWindspeed("\\calign?"),
        m_hasWeatherIconChanged(true),
        m_slotInterf(nullptr),
        m_durationCounter(0u),
        m_isUpdateAvailable(false),
        m_hasTopicChanged(false),
        m_taskProxy()
    {
        (void)m_mutex.create();
        createOpenWeatherSource(m_sourceId); /* Default */
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
        destroyOpenWeatherSource();
        
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
     * Process the plugin.
     * Overwrite it if your plugin has cyclic stuff to do without being in a
     * active slot.
     * 
     * @param[in] isConnected   The network connection status. If network
     *                          connection is established, it will be true otherwise false.
     */
    void process(bool isConnected) final;

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
     * Update the display.
     * The scheduler will call this method periodically.
     *
     * @param[in] gfx   Display graphics interface
     */
    void update(YAGfx& gfx) final;

    /**
     * Get OpenWeather API key.
     * 
     * @return OpenWeather API key
     */
    String getApiKey() const;

    /**
     * Set OpenWeather API key.
     * 
     * @param[in] apiKey    OpenWeather API key
     */
    void setApiKey(const String& apiKey);

    /**
     * Get the latitude.
     * 
     * @return latitude
     */
    String getLatitude() const;

    /**
     * Set the latitude.
     * 
     * @param[in] latitude    The latitude
     */
    void setLatitude(const String& latitude);

   /**
     * Get the longitude.
     * 
     * @return longitude
     */
    String getLongitude() const;

    /**
     * Set the longitude.
     * 
     * @param[in] longitude    The longitude
     */
    void setLongitude(const String& longitude);

     /**
     * Get the additional weather information.
     * 
     * @return The configured additional weather information.
     */
    OtherWeatherInformation getAdditionalInformation() const;

    /**
     * Set the additional weather information.
     * 
     * @param[in] additionalInformation     The additional weather information.
     */
    void setAdditionalInformation(const OtherWeatherInformation& additionalInformation);

    /**
     * Get the configured unist.
     * 
     * @return The units.
     */
    String getUnits() const;

    /**
     * Get the units.
     * 
     * @param[in] units The units
     */
    void setUnits(const String& units);

private:

    /**
     * Image path within the filesystem to weather condition icons.
     */
    static const char*      IMAGE_PATH;

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

    _OpenWeatherPlugin::View    m_view;                         /**< View with all widgets. */
    OpenWeatherSource           m_sourceId;                     /**< OpenWeather source id. */
    uint32_t                    m_updatePeriod;                 /**< Period in ms for requesting data from server. This is used in case the last request to the server was successful. */
    IOpenWeatherSource*         m_source;                       /**< OpenWeather source to use to retrieve weather information. */
    OtherWeatherInformation     m_additionalInformation;        /**< The configured additional weather information. */
    String                      m_configurationFilename;        /**< String used for specifying the configuration filename. */
    AsyncHttpClient             m_client;                       /**< Asynchronous HTTP client. */
    SimpleTimer                 m_requestTimer;                 /**< Timer used for cyclic request of new data. */
    SimpleTimer                 m_updateContentTimer;           /**< Timer used for duration ticks in [s]. */
    mutable MutexRecursive      m_mutex;                        /**< Mutex to protect against concurrent access. */
    bool                        m_isConnectionError;            /**< Is connection error happened? */
    String                      m_currentTemp;                  /**< The current temperature. */
    String                      m_currentWeatherIconFullPath;   /**< The current weather condition icon full path. */
    String                      m_currentUvIndex;               /**< The current UV index. */
    String                      m_currentHumidity;              /**< The current humidity. */
    String                      m_currentWindspeed;             /**< The current wind speed. */
    bool                        m_hasWeatherIconChanged;        /**< Has weather icon changed? If yes, it will be updated otherwise skipped to not disturb running animations. */
    const ISlotPlugin*          m_slotInterf;                   /**< Slot interface */
    uint8_t                     m_durationCounter;              /**< Variable to count the Plugin duration in DURATION_TICK_PERIOD ticks. */
    bool                        m_isUpdateAvailable;            /**< Flag to indicate an updated date value. */
    bool                        m_hasTopicChanged;              /**< Has the topic content changed? */

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
     * Create OpenWeather source according to id.
     * 
     * @param[in] id    OpenWeather source id
     */
    void createOpenWeatherSource(OpenWeatherSource id);

    /**
     * Destroy OpenWeatherSource.
     */
    void destroyOpenWeatherSource();

    /**
     * Get configuration in JSON.
     * 
     * @param[out] cfg  Configuration
     */
    void getConfiguration(JsonObject& cfg) const final;

    /**
     * Set configuration in JSON.
     * 
     * @param[in] cfg   Configuration
     * 
     * @return If successful set, it will return true otherwise false.
     */
    bool setConfiguration(JsonObjectConst& cfg) final;

    /**
     * Map the UV index value to a color corresponding the the icon.
    */
    const char* uvIndexToColor(uint8_t uvIndex);

    /**
     * Updates the text and icon, which to be displayed.
     *
     * @param[in] force Force update.
     */
    void updateDisplay(bool force);

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
     * Prepares the data to show from the OpenWeather source data.
     */
    void prepareDataToShow();

    /**
     * Clear the task proxy queue.
     */
    void clearQueue();
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* OPENWEATHERPLUGIN_H */

/** @} */