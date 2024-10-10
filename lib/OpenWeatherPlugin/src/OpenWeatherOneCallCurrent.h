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
 * @brief  OpenWeather source for One-Call API to retrieve current weather
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup plugin
 *
 * @{
 */

#ifndef OPENWEATHER_ONECALL_CURRENT_H
#define OPENWEATHER_ONECALL_CURRENT_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "IOpenWeatherCurrent.h"
#include <limits>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * The OpenWeather source for One-Call API
 * v2.5 see https://openweathermap.org/api/one-call-api
 * 
 * Note: The One Call API 2.5 has been deprecated on June 2024.
 *       https://openweathermap.org/api/one-call-api
 */
class OpenWeatherOneCallCurrent : public IOpenWeatherCurrent
{
public:

    /**
     * Constructs the OpenWeather source.
     * 
     * @param[in] oneCallApiVersion Version of the One-Call API to use. Supported: "3.0"
     */
    OpenWeatherOneCallCurrent(const String& oneCallApiVersion) :
        IOpenWeatherCurrent(),
        m_oneCallApiVersion(oneCallApiVersion),
        m_apiKey(),
        m_latitude(DEFAULT_LATITUDE),
        m_longitude(DEFAULT_LONGITUDE),
        m_units(DEFAULT_UNITS),
        m_temperature(std::numeric_limits<float>::quiet_NaN()),
        m_weatherIconId(),
        m_uvIndex(std::numeric_limits<float>::quiet_NaN()),
        m_humidity(0),
        m_windSpeed(std::numeric_limits<float>::quiet_NaN())
    {
    }

    /**
     * Destroys the OpenWeather source.
     */
    virtual ~OpenWeatherOneCallCurrent()
    {
    }

    /**
     * Get the API key.
     * 
     * @return API key
     */
    const String& getApiKey() const final
    {
        return m_apiKey;
    }

    /**
     * Set the API key.
     * 
     * @param[in] apiKey    The API key which to set.
     */
    void setApiKey(const String& apiKey) final
    {
        m_apiKey = apiKey;
    }

    /**
     * Get the latitude.
     * 
     * @return Latitude
     */
    const String& getLatitude() const final
    {
        return m_latitude;
    }

    /**
     * Set thel latidue.
     * 
     * @param[in] latitude  The latitude which to set.
     */
    void setLatitude(const String& latitude) final
    {
        m_latitude = latitude;
    }

    /**
     * Get the longitude.
     * 
     * @return Longitude
     */
    const String& getLongitude() const final
    {
        return m_longitude;
    }

    /**
     * Set the longitude.
     * 
     * @param[in] longitude The longitude which to set.
     */
    void setLongitude(const String& longitude) final
    {
        m_longitude = longitude;
    }

    /**
     * Get the units which are used for temperature and
     * wind speed.
     * 
     * @return Units
     */
    const String& getUnits() const final
    {
        return m_units;
    }

    /**
     * Set the units to use temperature and wind speed.
     * 
     * @param[in] units The units which to set.
     */
    void setUnits(const String& units) final
    {
        m_units = units;
    }

    /**
     * Adds the URI to the base URL.
     * 
     * @param[out] url  The base URL to use.
     */
    void getUrl(String& url) const final;

    /**
     * Get the filter which to apply on the response from the weather source.
     * Its a positive filter, which means everything marked with true, will
     * be used. Everything else will not be considered.
     * 
     * @param[out] jsonFilterDoc    The filter which to use.
     */
    void getFilter(JsonDocument& jsonFilterDoc) const final;

    /**
     * Parse a response from the weather source and will update its internal
     * data.
     * 
     * @param[out] jsonDoc  The JSON response which to parse.
     */
    void parse(const JsonDocument& jsonDoc) final;

    /**
     * Get the temperature.
     * Might be NaN in case no response was never parsed
     * or its not supported by the OpenWeather source.
     * 
     * @return Temperature, the unit is according to configuration.
     */
    float getTemperature() const final
    {
        return m_temperature;
    }

    /**
     * Get the weather icon id.
     * 
     * @return Weather icon id
     */
    const String& getWeatherIconId() const final
    {
        return m_weatherIconId;
    }

    /**
     * Get the UV-index.
     * Might be NaN in case no response was never parsed
     * or its not supported by the OpenWeather source.
     * 
     * @return UV-index.
     */
    float getUvIndex() const final
    {
        return m_uvIndex;
    }

    /**
     * Get the humidity.
     * 
     * @return Humidity in %.
     */
    int getHumidity() const final
    {
        return m_humidity;
    }

    /**
     * Get the wind speed.
     * Might be NaN in case no response was never parsed
     * or its not supported by the OpenWeather source.
     * 
     * @return Wind speed, the unit is according to configuration.
     */
    float getWindSpeed() const final
    {
        return m_windSpeed;
    }

private:

    String  m_oneCallApiVersion;    /**< OpenWeather One-Call API version */
    String  m_apiKey;               /**< OpenWeather API Key */
    String  m_latitude;             /**< The latitude. */
    String  m_longitude;            /**< The longitude. */
    String  m_units;                /**< The units to use for temperature and wind speed. */
    float   m_temperature;          /**< Temperature, unit according to configuration. */
    String  m_weatherIconId;        /**< Weather icon id. */
    float   m_uvIndex;              /**< UV-index */
    int     m_humidity;             /**< Humidity in %. */
    float   m_windSpeed;            /**< Wind speed, unit according to configuration. */

    /* Not allowed. */
    OpenWeatherOneCallCurrent();
    OpenWeatherOneCallCurrent(const OpenWeatherOneCallCurrent& other);
    OpenWeatherOneCallCurrent& operator=(const OpenWeatherOneCallCurrent& other);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* OPENWEATHER_ONECALL_CURRENT_H */

/** @} */