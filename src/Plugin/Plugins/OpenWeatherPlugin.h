/* MIT License
 *
 * Copyright (c) 2019 - 2021 Andreas Merkle <web@blue-andi.de>
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

#ifndef __OPENWEATHERPLUGIN_H__
#define __OPENWEATHERPLUGIN_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include "Plugin.hpp"
#include "AsyncHttpClient.h"

#include <Canvas.h>
#include <BitmapWidget.h>
#include <TextWidget.h>

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
        m_textCanvas(nullptr),
        m_iconCanvas(nullptr),
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
        m_xMutex(nullptr),
        m_isConnectionError(false),
        m_currentTemp("\\calign?"),
        m_currentWeatherIcon(IMAGE_PATH_STD_ICON),
        m_currentUvIndex("\\calign?"),
        m_currentHumidity("\\calign?"),
        m_currentWindspeed("\\calign?"),
        m_slotInterf(nullptr),
        m_configurationHasChanged(false),
        m_durationCounter(0u),
        m_isUpdateAvailable(false)
    {
        /* Move the text widget one line lower for better look. */
        m_textWidget.move(0, 1);

        m_xMutex = xSemaphoreCreateMutex();
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
        /* Abort any pending TCP request to avoid getting a callback after the
         * object is destroyed.
         */
        m_client.abort();
        
        if (nullptr != m_iconCanvas)
        {
            delete m_iconCanvas;
            m_iconCanvas = nullptr;
        }

        if (nullptr != m_textCanvas)
        {
            delete m_textCanvas;
            m_textCanvas = nullptr;
        }

        if (nullptr != m_xMutex)
        {
            vSemaphoreDelete(m_xMutex);
            m_xMutex = nullptr;
        }
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
     * Start the plugin.
     * Overwrite it if your plugin needs to know that it was installed.
     */
    void start() final;

    /**
     * Stop the plugin.
     * Overwrite it if your plugin needs to know that it will be uninstalled.
     */
    void stop() final;

    /**
     * Process the plugin.
     * Overwrite it if your plugin has cyclic stuff to do without being in a
     * active slot.
     */
    void process(void) final;
    
    /**
     * Set the slot interface, which the plugin can used to request information
     * from the slot, it is plugged in.
     *
     * @param[in] slotInterf    Slot interface
     */
    void setSlot(const ISlotPlugin* slotInterf) final;

    /**
     * This method will be called in case the plugin is set active, which means
     * it will be shown on the display in the next step.
     *
     * @param[in] gfx   Display graphics interface
     */
    void active(IGfx& gfx) final;

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
    void update(IGfx& gfx) final;

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
    static const uint32_t   UPDATE_PERIOD           = (10U * 60U * 1000U);

    /**
     * Short period in ms for requesting data from server.
     * This is used in case the request to the server failed.
     */
    static const uint32_t   UPDATE_PERIOD_SHORT     = (10U * 1000U);

    /** Time for duration tick period in ms */
    static const uint32_t   DURATION_TICK_PERIOD    = 1000U;

    Canvas*                     m_textCanvas;               /**< Canvas used for the text widget. */
    Canvas*                     m_iconCanvas;               /**< Canvas used for the bitmap widget. */
    BitmapWidget                m_bitmapWidget;             /**< Bitmap widget, used to show the icon. */
    TextWidget                  m_textWidget;               /**< Text widget, used for showing the text. */
    String                      m_apiKey;                   /**< OpenWeather API Key */
    String                      m_latitude;                 /**< The latitude. */
    String                      m_longitude;                /**< The langitude. */
    OtherWeatherInformation     m_additionalInformation;    /**< The configured additional weather information. */
    String                      m_units;                    /**< The units. */
    String                      m_configurationFilename;    /**< String used for specifying the configuration filename. */
    AsyncHttpClient             m_client;                   /**< Asynchronous HTTP client. */
    SimpleTimer                 m_requestTimer;             /**< Timer used for cyclic request of new data. */
    SimpleTimer                 m_updateContentTimer;       /**< Timer used for duration ticks in [s]. */
    SemaphoreHandle_t           m_xMutex;                   /**< Mutex to protect against concurrent access. */
    bool                        m_isConnectionError;        /**< Is connection error happened? */
    String                      m_currentTemp;              /**< The current temperature. */
    String                      m_currentWeatherIcon;       /**< The current weather condition icon. */
    String                      m_currentUvIndex;           /**< The current UV index. */
    String                      m_currentHumidity;          /**< The current humidity. */
    String                      m_currentWindspeed;         /**< The current windspeed. */
    const ISlotPlugin*          m_slotInterf;               /**< Slot interface */
    bool                        m_configurationHasChanged;  /**< Flag to indicate whether the configuration has changed. */
    uint8_t                     m_durationCounter;          /**< Variable to count the Plugin duration in DURATION_TICK_PERIOD ticks. */
    bool                        m_isUpdateAvailable;        /**< Flag to indicate an updated date value. */

    /**
     * Updates the text and icon, which to be displayed.
     *
     * @param[in] force Force update.
     */
    void updateDisplay(bool force);

    /**
     * Map the UV index value to a color corresponding the the icon.
    */
    String uvIndexToColor(float uvIndex);

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
     * Saves current configuration to JSON file.
     */
    bool saveConfiguration() const;

    /**
     * Load configuration from JSON file.
     */
    bool loadConfiguration();

    /**
     * Protect against concurrent access.
     */
    void lock(void) const;

    /**
     * Unprotect against concurrent access.
     */
    void unlock(void) const;
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __OPENWEATHERPLUGIN_H__ */

/** @} */