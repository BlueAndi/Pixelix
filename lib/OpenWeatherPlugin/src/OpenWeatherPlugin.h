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
#include <stdint.h>
#include "Plugin.hpp"
#include "AsyncHttpClient.h"

#include <WidgetGroup.h>
#include <BitmapWidget.h>
#include <TextWidget.h>
#include <TaskProxy.hpp>
#include <Mutex.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Shows weather informations provided by OpenWeather: https://openweathermap.org/
 */
class OpenWeatherPlugin : public Plugin
{
public:

    /**
     * Constructs the plugin.
     *
     * @param[in] name  Plugin name
     * @param[in] uid   Unique id
     */
    OpenWeatherPlugin(const String& name, uint16_t uid) :
        Plugin(name, uid),
        m_fontType(Fonts::FONT_TYPE_DEFAULT),
        m_textCanvas(),
        m_iconCanvas(),
        m_bitmapWidget(),
        m_textWidget("\\calign?"),
        m_apiKey(""),
        m_latitude("48.858"),/* Example data */
        m_longitude("2.295"),/* Example data */
        m_additionalInformation(OFF),
        m_units("metric"),
        m_configurationFilename(),
        m_client(),
        m_requestTimer(),
        m_updateContentTimer(),
        m_mutex(),
        m_isConnectionError(false),
        m_currentTemp("\\calign?"),
        m_currentWeatherIcon(IMAGE_PATH_STD_ICON),
        m_currentUvIndex("\\calign?"),
        m_currentHumidity("\\calign?"),
        m_currentWindspeed("\\calign?"),
        m_slotInterf(nullptr),
        m_durationCounter(0u),
        m_isUpdateAvailable(false),
        m_taskProxy()
    {
        (void)m_mutex.create();
    }

    /**
     * Enumeration to choose an additional weather information to be displayed.
     */
    enum OtherWeatherInformation
    {
        UVI = 0,    /**< Display UV Index as additional information. */
        HUMIDITY,   /**< Display humidity in % as additional information. */
        WIND,       /**< Display windspeed in m/s as additional information. */
        OFF         /**< Display only general weather information. */
    };

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
    static IPluginMaintenance* create(const String& name, uint16_t uid)
    {
        return new OpenWeatherPlugin(name, uid);
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
    bool setTopic(const String& topic, const JsonObject& value) final;

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
     * Icon width in pixels.
     */
    static const uint16_t   ICON_WIDTH          = 8U;

    /**
     * Icon height in pixels.
     */
    static const uint16_t   ICON_HEIGHT         = 8U;

    /**
     * Image path within the filesystem to standard icon.
     */
    static const char*      IMAGE_PATH_STD_ICON;

    /**
     * Image path within the filesystem to UV index icon.
     */
    static const char*      IMAGE_PATH_UVI_ICON;

    /**
     * Image path within the filesystem to humidity icon.
     */
    static const char*      IMAGE_PATH_HUMIDITY_ICON;

    /**
     * Image path within the filesystem to windspeed icon.
     */
    static const char*      IMAGE_PATH_WIND_ICON;

    /**
     * Image path within the filesystem to weather condition icons.
     */
    static const char*      IMAGE_PATH;

    /**
     * OpenWeather API base URI
     */
    static const char*      OPEN_WEATHER_BASE_URI;

    /**
     * Plugin topic, used for parameter exchange.
     */
    static const char*      TOPIC;

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

    Fonts::FontType             m_fontType;                 /**< Font type which shall be used if there is no conflict with the layout. */
    WidgetGroup                 m_textCanvas;               /**< Canvas used for the text widget. */
    WidgetGroup                 m_iconCanvas;               /**< Canvas used for the bitmap widget. */
    BitmapWidget                m_bitmapWidget;             /**< Bitmap widget, used to show the icon. */
    TextWidget                  m_textWidget;               /**< Text widget, used for showing the text. */
    String                      m_apiKey;                   /**< OpenWeather API Key */
    String                      m_latitude;                 /**< The latitude. */
    String                      m_longitude;                /**< The longitude. */
    OtherWeatherInformation     m_additionalInformation;    /**< The configured additional weather information. */
    String                      m_units;                    /**< The units. */
    String                      m_configurationFilename;    /**< String used for specifying the configuration filename. */
    AsyncHttpClient             m_client;                   /**< Asynchronous HTTP client. */
    SimpleTimer                 m_requestTimer;             /**< Timer used for cyclic request of new data. */
    SimpleTimer                 m_updateContentTimer;       /**< Timer used for duration ticks in [s]. */
    mutable MutexRecursive      m_mutex;                    /**< Mutex to protect against concurrent access. */
    bool                        m_isConnectionError;        /**< Is connection error happened? */
    String                      m_currentTemp;              /**< The current temperature. */
    String                      m_currentWeatherIcon;       /**< The current weather condition icon. */
    String                      m_currentUvIndex;           /**< The current UV index. */
    String                      m_currentHumidity;          /**< The current humidity. */
    String                      m_currentWindspeed;         /**< The current wind speed. */
    const ISlotPlugin*          m_slotInterf;               /**< Slot interface */
    uint8_t                     m_durationCounter;          /**< Variable to count the Plugin duration in DURATION_TICK_PERIOD ticks. */
    bool                        m_isUpdateAvailable;        /**< Flag to indicate an updated date value. */
    
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
     * Updates the text and icon, which to be displayed.
     *
     * @param[in] force Force update.
     */
    void updateDisplay(bool force);

    /**
     * Map the UV index value to a color corresponding the the icon.
    */
    const char* uvIndexToColor(uint8_t uvIndex);

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
    void handleWebResponse(DynamicJsonDocument& jsonDoc);

    /**
     * Saves current configuration to JSON file.
     */
    bool saveConfiguration() const;

    /**
     * Load configuration from JSON file.
     */
    bool loadConfiguration();

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