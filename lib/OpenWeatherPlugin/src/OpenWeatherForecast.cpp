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
 * @brief  OpenWeather source for forecast weather data
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "OpenWeatherForecast.h"

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

void OpenWeatherForecast::getUrl(String& url) const
{
    url += "/data/2.5/forecast?lat=";
    url += m_latitude;
    url += "&lon=";
    url += m_longitude;
    url += "&units=";
    url += m_units;
    url += "&cnt=";
    url += FORECAST_DAYS;
    url += "&appid=";
    url += m_apiKey;
}

void OpenWeatherForecast::getFilter(JsonDocument& jsonFilterDoc) const
{
    uint8_t day;

    /*
    
        {
        "cod": "200",
        "message": 0,
        "cnt": 3,
        "list": [
            {
            "dt": 1647356400,
            "main": {
                "temp": 281.28,
                "feels_like": 277.76,
                "temp_min": 280.57,
                "temp_max": 281.28,
                "pressure": 1015,
                "sea_level": 1015,
                "grnd_level": 1014,
                "humidity": 79,
                "temp_kf": 0.71
            },
            "weather": [
                {
                "id": 500,
                "main": "Rain",
                "description": "light rain",
                "icon": "10d"
                }
            ],
            "clouds": {
                "all": 95
            },
            "wind": {
                "speed": 6.78,
                "deg": 184,
                "gust": 12.14
            },
            "visibility": 10000,
            "pop": 0.34,
            "rain": {
                "3h": 0.27
            },
            "sys": {
                "pod": "d"
            },
            "dt_txt": "2022-03-15 15:00:00"
            },
            {
            "dt": 1647367200,
            "main": {
                "temp": 281.03,
                "feels_like": 277.59,
                "temp_min": 280.73,
                "temp_max": 281.03,
                "pressure": 1015,
                "sea_level": 1015,
                "grnd_level": 1014,
                "humidity": 79,
                "temp_kf": 0.3
            },
            "weather": [
                {
                "id": 804,
                "main": "Clouds",
                "description": "overcast clouds",
                "icon": "04d"
                }
            ],
            "clouds": {
                "all": 98
            },
            "wind": {
                "speed": 6.31,
                "deg": 189,
                "gust": 11.74
            },
            "visibility": 10000,
            "pop": 0.2,
            "sys": {
                "pod": "d"
            },
            "dt_txt": "2022-03-15 18:00:00"
            },
            {
            "dt": 1647378000,
            "main": {
                "temp": 280.39,
                "feels_like": 276.74,
                "temp_min": 280.39,
                "temp_max": 280.39,
                "pressure": 1014,
                "sea_level": 1014,
                "grnd_level": 1014,
                "humidity": 82,
                "temp_kf": 0
            },
            "weather": [
                {
                "id": 804,
                "main": "Clouds",
                "description": "overcast clouds",
                "icon": "04n"
                }
            ],
            "clouds": {
                "all": 100
            },
            "wind": {
                "speed": 6.41,
                "deg": 199,
                "gust": 12.52
            },
            "visibility": 10000,
            "pop": 0,
            "sys": {
                "pod": "n"
            },
            "dt_txt": "2022-03-15 21:00:00"
            }
        ],
        "city": {
            "id": 2641549,
            "name": "Newtonhill",
            "coord": {
            "lat": 57,
            "lon": -2.15
            },
            "country": "GB",
            "population": 3284,
            "timezone": 0,
            "sunrise": 1647325488,
            "sunset": 1647367827
        }
        }
    
    */

    for(day = 0U; day < FORECAST_DAYS; ++day)
    {
        jsonFilterDoc["list"][day]["main"]["temp_min"]  = true;
        jsonFilterDoc["list"][day]["main"]["temp_max"]  = true;
        jsonFilterDoc["list"][day]["weather"]["icon"]   = true;
    }
}

void OpenWeatherForecast::parse(const JsonDocument& jsonDoc)
{
    uint8_t day;

    for(day = 0U; day < FORECAST_DAYS; ++day)
    {
        JsonVariantConst    jsonTemperatureMin  = jsonDoc["list"][day]["main"]["temp_min"];
        JsonVariantConst    jsonTemperatureMax  = jsonDoc["list"][day]["main"]["temp_max"];
        JsonVariantConst    jsonIcon            = jsonDoc["list"][day]["weather"]["icon"];

        if (false == jsonTemperatureMin.isNull())
        {
            m_weatherInfo[day].temperatureMin = jsonTemperatureMin.as<float>();
        }

        if (false == jsonTemperatureMax.isNull())
        {
            m_weatherInfo[day].temperatureMax = jsonTemperatureMax.as<float>();
        }

        if (false == jsonIcon.isNull())
        {
            m_weatherInfo[day].weatherIconId = jsonIcon.as<String>();
        }
    }
}

float OpenWeatherForecast::getTemperatureMin(uint8_t day) const
{
    float temperatureMin = std::numeric_limits<float>::quiet_NaN();

    if (FORECAST_DAYS > day)
    {
        temperatureMin = m_weatherInfo[day].temperatureMin;
    }

    return temperatureMin;
}

float OpenWeatherForecast::getTemperatureMax(uint8_t day) const
{
    float temperatureMax = std::numeric_limits<float>::quiet_NaN();

    if (FORECAST_DAYS > day)
    {
        temperatureMax = m_weatherInfo[day].temperatureMax;
    }

    return temperatureMax;
}

const String OpenWeatherForecast::getWeatherIconId(uint8_t day) const
{
    String iconId;

    if (FORECAST_DAYS > day)
    {
        iconId = m_weatherInfo[day].weatherIconId;
    }

    return iconId;
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
