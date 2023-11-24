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
 * @brief  OpenWeather source for current weather data
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "OpenWeatherCurrent.h"

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

void OpenWeatherCurrent::getUrl(String& url) const
{
    url += "/data/2.5/weather?lat=";
    url += m_latitude;
    url += "&lon=";
    url += m_longitude;
    url += "&units=";
    url += m_units;
    url += "&appid=";
    url += m_apiKey;
}

void OpenWeatherCurrent::getFilter(JsonDocument& jsonFilterDoc) const
{
    /*
    
        {
        "coord": {
            "lon": 10.99,
            "lat": 44.34
        },
        "weather": [
            {
            "id": 501,
            "main": "Rain",
            "description": "moderate rain",
            "icon": "10d"
            }
        ],
        "base": "stations",
        "main": {
            "temp": 298.48,
            "feels_like": 298.74,
            "temp_min": 297.56,
            "temp_max": 300.05,
            "pressure": 1015,
            "humidity": 64,
            "sea_level": 1015,
            "grnd_level": 933
        },
        "visibility": 10000,
        "wind": {
            "speed": 0.62,
            "deg": 349,
            "gust": 1.18
        },
        "rain": {
            "1h": 3.16
        },
        "clouds": {
            "all": 100
        },
        "dt": 1661870592,
        "sys": {
            "type": 2,
            "id": 2075663,
            "country": "IT",
            "sunrise": 1661834187,
            "sunset": 1661882248
        },
        "timezone": 7200,
        "id": 3163858,
        "name": "Zocca",
        "cod": 200
        }
    
    */

    jsonFilterDoc["main"]["temp"]       = true;
    jsonFilterDoc["main"]["humidity"]   = true;
    jsonFilterDoc["wind"]["speed"]      = true;
    jsonFilterDoc["weather"][0]["icon"] = true;
}

void OpenWeatherCurrent::parse(const JsonDocument& jsonDoc)
{
    JsonVariantConst    jsonTemperature = jsonDoc["main"]["temp"];
    JsonVariantConst    jsonHumidity    = jsonDoc["main"]["humidity"];
    JsonVariantConst    jsonWindSpeed   = jsonDoc["wind"]["speed"];
    JsonVariantConst    jsonIcon        = jsonDoc["weather"][0]["icon"];

    if (false == jsonTemperature.isNull())
    {
        m_temperature = jsonTemperature.as<float>();
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
