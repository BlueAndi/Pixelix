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
 * @brief  OpenWeather source interface
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup plugin
 *
 * @{
 */

#ifndef IOPENWEATHERSOURCE_H
#define IOPENWEATHERSOURCE_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <WString.h>
#include <stdint.h>
#include <ArduinoJson.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/** Default latitude (Berlin) */
#define DEFAULT_LATITUDE    "52.519"

/** Default longitude (Berlin) */
#define DEFAULT_LONGITUDE   "13.376"

/** Default units */
#define DEFAULT_UNITS       "metric"

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** This type is the abstract interface for a OpenWeather source. */
class IOpenWeatherSource
{
public:

    /**
     * Get the API key.
     * 
     * @return API key
     */
    virtual const String& getApiKey() const = 0;

    /**
     * Set the API key.
     * 
     * @param[in] apiKey    The API key which to set.
     */
    virtual void setApiKey(const String& apiKey) = 0;

    /**
     * Get the latitude.
     * 
     * @return Latitude
     */
    virtual const String& getLatitude() const = 0;

    /**
     * Set thel latidue.
     * 
     * @param[in] latitude  The latitude which to set.
     */
    virtual void setLatitude(const String& latitude) = 0;

    /**
     * Get the longitude.
     * 
     * @return Longitude
     */
    virtual const String& getLongitude() const = 0;

    /**
     * Set the longitude.
     * 
     * @param[in] longitude The longitude which to set.
     */
    virtual void setLongitude(const String& longitude) = 0;

    /**
     * Get the units which are used for temperature and
     * wind speed.
     * 
     * @return Units
     */
    virtual const String& getUnits() const = 0;

    /**
     * Set the units to use temperature and wind speed.
     * 
     * @param[in] units The units which to set.
     */
    virtual void setUnits(const String& units) = 0;

    /**
     * Adds the URI to the base URL.
     * 
     * @param[out] url  The base URL to use.
     */
    virtual void getUrl(String& url) const = 0;

    /**
     * Get the filter which to apply on the response from the weather source.
     * Its a positive filter, which means everything marked with true, will
     * be used. Everything else will not be considered.
     * 
     * @param[out] jsonFilterDoc    The filter which to use.
     */
    virtual void getFilter(JsonDocument& jsonFilterDoc) const = 0;
    
    /**
     * Parse a response from the weather source and will update its internal
     * data.
     * 
     * @param[out] jsonDoc  The JSON response which to parse.
     */
    virtual void parse(const JsonDocument& jsonDoc) = 0;

    /**
     * Get the temperature.
     * Might be NaN in case no response was never parsed
     * or its not supported by the OpenWeather source.
     * 
     * @return Temperature, the unit is according to configuration.
     */
    virtual float getTemperature() const = 0;

    /**
     * Get the weather icon id.
     * 
     * @return Weather icon id
     */
    virtual const String& getWeatherIconId() const = 0;

    /**
     * Get the UV-index.
     * Might be NaN in case no response was never parsed
     * or its not supported by the OpenWeather source.
     * 
     * @return UV-index.
     */
    virtual float getUvIndex() const = 0;

    /**
     * Get the humidity.
     * 
     * @return Humidity in %.
     */
    virtual int getHumidity() const = 0;

    /**
     * Get the wind speed.
     * Might be NaN in case no response was never parsed
     * or its not supported by the OpenWeather source.
     * 
     * @return Wind speed, the unit is according to configuration.
     */
    virtual float getWindSpeed() const = 0;

protected:
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* IOPENWEATHERSOURCE_H */

/** @} */