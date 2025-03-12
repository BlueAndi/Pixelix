/* MIT License
 *
 * Copyright (c) 2019 - 2025 Andreas Merkle <web@blue-andi.de>
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
 * @brief  OpenWeather view with icon and text for 64x64 LED matrix
 * @author Andreas Merkle <web@blue-andi.de>
 * @addtogroup PLUGIN
 *
 * @{
 */

#ifndef OPEN_WEATHER_VIEW_64X64_H
#define OPEN_WEATHER_VIEW_64X64_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <YAGfx.h>
#include <Fonts.h>
#include <IOpenWeatherView.h>
#include <BitmapWidget.h>
#include <TextWidget.h>
#include <Util.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * OpenWeather view for 64x64 LED matrix with icon and text.
 */
class OpenWeatherView64x64 : public IOpenWeatherView
{
public:

    /**
     * Construct the view.
     */
    OpenWeatherView64x64();

    /**
     * Destroy the view.
     */
    virtual ~OpenWeatherView64x64()
    {
    }

    /**
     * Initialize view, which will prepare the widgets and the default values.
     * 
     * @param[in] width     Display width in pixel.
     * @param[in] height    Display height in pixel.
     */
    void init(uint16_t width, uint16_t height) override
    {
        UTIL_NOT_USED(width);
        UTIL_NOT_USED(height);
    }

    /**
     * Get font type.
     * 
     * @return The font type the view uses.
     */
    Fonts::FontType getFontType() const override
    {
        return m_fontType;
    }

    /**
     * Set font type.
     * 
     * @param[in] fontType  The font type which the view shall use.
     */
    void setFontType(Fonts::FontType fontType) override
    {
        /* Not supported. */
    }

    /**
     * Update the underlying canvas.
     * 
     * @param[in] gfx   Graphic functionality to draw on the underlying canvas.
     */
    void update(YAGfx& gfx) override;

    /**
     * Set the duration in ms, how long the view will be shown on the display.
     * It will be used to derive how long every enabled weather info is shown.
     * All enabled weather infos together will be equal to the configured duration.
     * 
     * Note, the view itself decides whether to use it or not.
     * 
     * @param[in] duration  View duration in ms
     */
    void setViewDuration(uint32_t duration) override
    {
        if (m_viewDuration != duration)
        {
            m_viewDuration = duration;

            /* Force recalculation of the duration of every weather info. */
            m_viewDurationTimer.stop();
        }
    }

    /**
     * Set the unit to use temperature.
     * 
     * @param[in] unit The temperature unit.
     */
    void setTemperatureUnit(const String& unit) override
    {
        m_temperatureUnit = unit;
    }

    /**
     * Set the unit to use wind speed.
     * 
     * @param[in] unit The wind speed unit.
     */
    void setWindSpeedUnit(const String& unit) override
    {
        m_windSpeedUnit = unit;
    }

    /**
     * Get the enabled weather information.
     * See WeatherInfo for every bit.
     * 
     * @return Weather information which is enabled.
     */
    uint8_t getWeatherInfo() const override
    {
        return m_weatherInfo;
    }

    /**
     * Set weather information, which shall be shown.
     * Use the bitfield WeatherInfo to combine the information by OR.
     * 
     * @param[in] weatherInfo   The weather info which to show.
     */
    void setWeatherInfo(uint8_t weatherInfo) override
    {
        if (m_weatherInfo != weatherInfo)
        {
            m_weatherInfo = weatherInfo;
            
            restartWeatherInfo();

            /* Force recalculation of the duration of every weather info. */
            m_viewDurationTimer.stop();
        }
    }

    /**
     * Restart showing the first weather info again.
     * The order is according to the order of WeatherInfo.
     */
    void restartWeatherInfo() override
    {
        m_weatherInfoId = 0U;

        /* Force recalculation of the duration of every weather info. */
        m_viewDurationTimer.stop();
    }

    /**
     * Set current weather information.
     * 
     * @param[in] info  Weather information
     */
    void setWeatherInfoCurrent(const WeatherInfoCurrent& info) override;

    /**
     * Set forecast weather information.
     * 
     * Meaning of day:
     * - 0: Forecast for the next day
     * - 1: Forecast in two days
     * - N: Forecast in N days
     * 
     * @param[in] day   Weather information for the upcoming day [0; 4].
     * @param[in] info  Weather information
     */
    void setWeatherInfoForecast(uint8_t day, const WeatherInfoForecast& info) override;

    /**
     * Is the weather forecast feature supported by the view?
     * 
     * @return If feature is supported, it will return true otherwise false.
     */
    static constexpr bool isWeatherForecastSupported()
    {
        return true;
    }

protected:

    /**
     * Image path within the filesystem to weather condition icons.
     */
    static const char*      IMAGE_PATH;

    /**
     * Image path within the filesystem to standard icon.
     */
    static const char*      IMAGE_PATH_STD_ICON;

    /**
     * Image path within the filesystem to standard icon in 16x16 size.
     */
    static const char*      IMAGE_PATH_STD_ICON_16X16;

    /**
     * Image path within the filesystem to UV index icon in 16x16 size.
     */
    static const char*      IMAGE_PATH_UVI_ICON_16X16;

    /**
     * Image path within the filesystem to humidity icon in 16x16 size.
     */
    static const char*      IMAGE_PATH_HUMIDITY_ICON_16X16;

    /**
     * Image path within the filesystem to windspeed icon in 16x16 size.
     */
    static const char*      IMAGE_PATH_WIND_ICON_16X16;

    /**
     * Default duration in ms used for the view.
     * If the view duration is INFINITE (value is 0), the default one is used.
     */
    static const uint32_t   VIEW_DURATION_DEFAULT   = SIMPLE_TIMER_SECONDS(30U);

    /**
     * Minimum duration of one single weather information in ms used by the view.
     */
    static const uint32_t   VIEW_DURATION_MIN       = SIMPLE_TIMER_SECONDS(4U);

    Fonts::FontType     m_fontType;                                     /**< Font type which shall be used if there is no conflict with the layout. */
    BitmapWidget        m_weatherIconCurrent;                           /**< Current weather icon. */
    TextWidget          m_weatherInfoCurrentText;                       /**< Current weather info text. */
    TextWidget          m_forecastDayNames[FORECAST_DAYS];              /**< Forecast day names */
    BitmapWidget        m_forecastIcons[FORECAST_DAYS];                 /**< Forecast weather icons. */
    TextWidget          m_forecastTemperatures[FORECAST_DAYS];          /**< Forecast temperature (min. and max.) */
    uint32_t            m_viewDuration;                                 /**< The duration in ms, this view will be shown on the display. */
    SimpleTimer         m_viewDurationTimer;                            /**< The timer used to determine which weather info to show on the display. */
    String              m_temperatureUnit;                              /**< Temperature unit */
    String              m_windSpeedUnit;                                /**< Wind speed unit */
    uint8_t             m_weatherInfo;                                  /**< Use the bits to determine which weather info to show. */
    uint8_t             m_weatherInfoId;                                /**< The weather info id is used to mask the weather info flag. Its the number of bit shifts. */
    WeatherInfoCurrent  m_weatherInfoCurrent;                           /**< Current weather information. */
    WeatherInfoForecast m_weatherInfoForecast[FORECAST_DAYS];           /**< Forecast wheather information. */
    bool                m_isWeatherInfoCurrentUpdated;                  /**< Is current weather info updated? */
    bool                m_isWeatherIconCurrentUpdated;                  /**< Is the current weather icon updated in the weather info? */
    bool                m_isWeatherInfoForecastUpdated;                 /**< Is forecast weather info updated? */
    bool                m_isWeatherIconForecastUpdated[FORECAST_DAYS];  /**< Is the forecast weather icon updated in the weather info? */

private:
    OpenWeatherView64x64(const OpenWeatherView64x64& other);
    OpenWeatherView64x64& operator=(const OpenWeatherView64x64& other);

    /**
     * Get number of enabled weather infos.
     * 
     * @return Count of weather infos
     */
    uint8_t getWeatherInfoCount() const;

    /**
     * Select next weather info.
     */
    void nextWeatherInfo();

    /**
     * Get current active weather info.
     * 
     * @return Weather info 
     */
    WeatherInfo getActiveWeatherInfo() const;

    /**
     * Update the current  weather info on the view by considering the current active
     * weather info.
     */
    void updateWeatherInfoCurrentOnView();

    /**
     * Update the forecast  weather info on the view.
     */
    void updateWeatherInfoForecastOnView();

    /**
     * Handle main weather info, which to show.
     */
    void handleWeatherInfo();

    /**
     * Get the full path to the icon in the filesystem by the weather icon id.
     * 
     * @param[out]  fullPath        Full path to icon in the filesystem.
     * @param[in]   weatherIconId   The weather icon id.
     * @param[in]   addition        The addition will be added at the tail of the filename.
     */
    void getIconPathByWeatherIconId(String& fullPath, const String& weatherIconId, const String&addition) const;

    /**
     * Map the UV index value to a color corresponding the the icon.
    */
    const char* uvIndexToColor(uint8_t uvIndex);

    /**
     * Appends temperature to destination string.
     * If value is invalid, it will write "-".
     * 
     * @param[out]  dst             Destination string
     * @param[in]   temperature     Temperature
     * @param[in]   noFraction      No fraction required
     * @param[in]   noUnit          No unit required
     */
    void appendTemperature(String& dst, float temperature, bool noFraction = false, bool noUnit = false);

    /**
     * Appends humidity with unit to destination string.
     * 
     * @param[out]  dst         Destination string
     * @param[in]   humidity    Humidity
     */
    void appendHumidity(String& dst, uint8_t humidity);

    /**
     * Appends wind speed with unit to destination string.
     * If value is invalid, it will write "-".
     * 
     * @param[out]  dst         Destination string
     * @param[in]   windSpeed   Wind speed
     */
    void appendWindSpeed(String& dst, float windSpeed);

    /**
     * Appends uv-index with unit to destination string.
     * If value is invalid, it will write "-".
     * 
     * @param[out]  dst     Destination string
     * @param[in]   uvIndex UV-index
     */
    void appendUvIndex(String& dst, float uvIndex);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* OPEN_WEATHER_VIEW_64X64_H */

/** @} */
