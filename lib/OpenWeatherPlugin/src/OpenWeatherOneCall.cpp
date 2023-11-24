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
 * @brief  OpenWeather source for One-Call API
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "OpenWeatherOneCall.h"

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and classes
 *****************************************************************************/

/******************************************************************************
 * Prototypes
 *****************************************************************************/

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void OpenWeatherOneCall::getUrl(String& url) const
{
    url += "/data/";
    url += m_oneCallApiVersion;
    url += "/onecall?lat=";
    url += m_latitude;
    url += "&lon=";
    url += m_longitude;
    url += "&units=";
    url += m_units;
    url += "&appid=";
    url += m_apiKey;
    url += "&exclude=minutely,hourly,daily,alerts";
}

void OpenWeatherOneCall::getFilter(JsonDocument& jsonFilterDoc) const
{
    JsonObject jsonCurrent = jsonFilterDoc.createNestedObject("current");

    /*
    
        {
        "lat": 33.44,
        "lon": -94.04,
        "timezone": "America/Chicago",
        "timezone_offset": -21600,
        "current": {
            "dt": 1618317040,
            "sunrise": 1618282134,
            "sunset": 1618333901,
            "temp": 284.07,
            "feels_like": 282.84,
            "pressure": 1019,
            "humidity": 62,
            "dew_point": 277.08,
            "uvi": 0.89,
            "clouds": 0,
            "visibility": 10000,
            "wind_speed": 6,
            "wind_deg": 300,
            "weather": [
            {
                "id": 500,
                "main": "Rain",
                "description": "light rain",
                "icon": "10d"
            }
            ],
            "rain": {
            "1h": 0.21
            }
        }
    
    */

    jsonCurrent["temp"]                 = true;
    jsonCurrent["uvi"]                  = true;
    jsonCurrent["humidity"]             = true;
    jsonCurrent["wind_speed"]           = true;
    jsonCurrent["weather"][0]["icon"]   = true;
}

void OpenWeatherOneCall::parse(const JsonDocument& jsonDoc)
{
    JsonVariantConst    jsonCurrent     = jsonDoc["current"];
    JsonVariantConst    jsonTemperature = jsonCurrent["temp"];
    JsonVariantConst    jsonUvi         = jsonCurrent["uvi"];
    JsonVariantConst    jsonHumidity    = jsonCurrent["humidity"];
    JsonVariantConst    jsonWindSpeed   = jsonCurrent["wind_speed"];
    JsonVariantConst    jsonIcon        = jsonCurrent["weather"][0]["icon"];

    if (false == jsonTemperature.isNull())
    {
        m_temperature = jsonTemperature.as<float>();
    }

    if (false == jsonUvi.isNull())
    {
        m_uvIndex = jsonUvi.as<float>();
    }

    if (false == jsonHumidity.isNull())
    {
        m_humidity = jsonHumidity.as<int>();
    }

    if (false == jsonWindSpeed.isNull())
    {
        m_windSpeed = jsonWindSpeed.as<float>();
    }

    if (false == jsonIcon.isNull())
    {
        m_weatherIconId = jsonIcon.as<String>();
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
