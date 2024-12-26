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
 * @brief  OpenWeather view interface
 * @author Andreas Merkle <web@blue-andi.de>
 * @addtogroup plugin
 *
 * @{
 */

#ifndef IOPEN_WEATHER_VIEW_H
#define IOPEN_WEATHER_VIEW_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <YAGfx.h>
#include <Fonts.h>
#include <WString.h>
#include <limits>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Interface for a view with weather informations.
 */
class IOpenWeatherView
{
public:

    /**
     * Destroy the interface.
     */
    virtual ~IOpenWeatherView()
    {
    }

    /**
     * Initialize view, which will prepare the widgets and the default values.
     */
    virtual void init(uint16_t width, uint16_t height) = 0;

    /**
     * Get font type.
     * 
     * @return The font type the view uses.
     */
    virtual Fonts::FontType getFontType() const = 0;

    /**
     * Set font type.
     * 
     * @param[in] fontType  The font type which the view shall use.
     */
    virtual void setFontType(Fonts::FontType fontType) = 0;

    /**
     * Update the underlying canvas.
     * 
     * @param[in] gfx   Graphic functionality to draw on the underlying canvas.
     */
    virtual void update(YAGfx& gfx) = 0;

    /**
     * Set the duration in ms, how long the view will be shown on the display.
     * It will be used to derive how long every enabled weather info is shown.
     * All enabled weather infos together will be equal to the configured duration.
     * 
     * Note, the view itself decides whether to use it or not.
     * 
     * @param[in] duration  View duration in ms
     */
    virtual void setViewDuration(uint32_t duration) = 0;

    /**
     * Set the units to use temperature and wind speed.
     * 
     * @param[in] units The units which to set.
     */
    virtual void setUnits(const String& units) = 0;

    /** Weather info */
    enum WeatherInfo : uint8_t
    {
        WEATHER_INFO_EMPTY          = 0x00U,    /**< Nothing enabled. */
        WEATHER_INFO_TEMPERATURE    = 0x01U,    /**< Temperature */
        WEATHER_INFO_HUMIDITY       = 0x02U,    /**< Humidity */
        WEATHER_INFO_WIND_SPEED     = 0x04U,    /**< Wind speed */
        WEATHER_INFO_UV_INDEX       = 0x08U,    /**< UV-index */
        WEATHER_INFO_ALL            = 0x0FU     /**< All weather information */
    };

    /**
     * Get the enabled weather information.
     * See WeatherInfo for every bit.
     * 
     * @return Weather information which is enabled.
     */
    virtual uint8_t getWeatherInfo() const = 0;

    /**
     * Set weather information, which shall be shown.
     * Use the bitfield WeatherInfo to combine the information by OR.
     * 
     * @param[in] weatherInfo   The weather info which to show.
     */
    virtual void setWeatherInfo(uint8_t weatherInfo) = 0;

    /**
     * Restart showing the first weather info again.
     * The order is according to the order of WeatherInfo.
     */
    virtual void restartWeatherInfo() = 0;

    /** Current weather information */
    struct WeatherInfoCurrent
    {
        String  iconId;         /**< Weather icon id */
        float   temperature;    /**< Temperature (unit depends on tempUnit) */
        uint8_t humidity;       /**< Humidity in percent [0; 100] */
        float   windSpeed;      /**< Wind speed (unit depends on windUnit) */
        float   uvIndex;        /**< UV-index */

        /**
         * Construct current weather information object.
         */
        WeatherInfoCurrent() :
            iconId(),
            temperature(std::numeric_limits<float>::quiet_NaN()),
            humidity(0U),
            windSpeed(std::numeric_limits<float>::quiet_NaN()),
            uvIndex(std::numeric_limits<float>::quiet_NaN())
        {
        }

        /**
         * Destroy current weather information object.
         * 
         */
        ~WeatherInfoCurrent()
        {
        }
    };

    /**
     * Set current weather information.
     * 
     * @param[in] info  Weather information
     */
    virtual void setWeatherInfoCurrent(const WeatherInfoCurrent& info) = 0;

    /** Forecast weather information */
    struct WeatherInfoForecast
    {
        String  iconId;         /**< Weather icon id */
        float   temperatureMin; /**< Min. temperature (unit depends on tempUnit) */
        float   temperatureMax; /**< Max. temperature (unit depends on tempUnit) */

        /**
         * Construct forecast weather information object.
         */
        WeatherInfoForecast() :
            iconId(),
            temperatureMin(std::numeric_limits<float>::quiet_NaN()),
            temperatureMax(std::numeric_limits<float>::quiet_NaN())
        {
        }

        /**
         * Destroy forecast weather information object.
         * 
         */
        ~WeatherInfoForecast()
        {
        }
    };

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
    virtual void setWeatherInfoForecast(uint8_t day, const WeatherInfoForecast& info) = 0;

    /**
     * Number of forecast days.
     */
    static const uint8_t    FORECAST_DAYS   = 5U;

protected:

    /**
     * Construct the interface.
     */
    IOpenWeatherView()
    {
    }

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* IOPEN_WEATHER_VIEW_H */

/** @} */
