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
 * @brief  OpenWeather source for One-Call API to retrieve forecast weather
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "OpenWeatherOneCallForecast.h"

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

void OpenWeatherOneCallForecast::getUrl(String& url) const
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
    url += "&exclude=current,minutely,hourly,alerts";
}

void OpenWeatherOneCallForecast::getFilter(JsonDocument& jsonFilterDoc) const
{
    uint8_t     day;
    JsonArray   jsonDaily   = jsonFilterDoc.createNestedArray("daily");

    /*
    
        {
        "lat":33.44,
        "lon":-94.04,
        "timezone":"America/Chicago",
        "timezone_offset":-18000,
        "daily":[
            {
                "dt":1684951200,
                "sunrise":1684926645,
                "sunset":1684977332,
                "moonrise":1684941060,
                "moonset":1684905480,
                "moon_phase":0.16,
                "summary":"Expect a day of partly cloudy with rain",
                "temp":{
                    "day":299.03,
                    "min":290.69,
                    "max":300.35,
                    "night":291.45,
                    "eve":297.51,
                    "morn":292.55
                },
                "feels_like":{
                    "day":299.21,
                    "night":291.37,
                    "eve":297.86,
                    "morn":292.87
                },
                "pressure":1016,
                "humidity":59,
                "dew_point":290.48,
                "wind_speed":3.98,
                "wind_deg":76,
                "wind_gust":8.92,
                "weather":[
                    {
                    "id":500,
                    "main":"Rain",
                    "description":"light rain",
                    "icon":"10d"
                    }
                ],
                "clouds":92,
                "pop":0.47,
                "rain":0.15,
                "uvi":9.23
            },
            ...
        ]
    
    */

   for(day = 0U; day < FORECAST_DAYS; ++day)
   {
        jsonDaily[day]["temp"]["min"]           = true;
        jsonDaily[day]["temp"]["max"]           = true;
        jsonDaily[day]["weather"][0]["icon"]    = true;
   }
}

void OpenWeatherOneCallForecast::parse(const JsonDocument& jsonDoc)
{
    uint8_t             day;
    JsonVariantConst    jsonDaily   = jsonDoc["daily"];

    for(day = 0U; day < FORECAST_DAYS; ++day)
    {
        JsonVariantConst    jsonTemperatureMin  = jsonDaily[day]["temp"]["min"];
        JsonVariantConst    jsonTemperatureMax  = jsonDaily[day]["temp"]["max"];
        JsonVariantConst    jsonIcon            = jsonDaily[day]["weather"][0]["icon"];

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

float OpenWeatherOneCallForecast::getTemperatureMin(uint8_t day) const
{
    float temperatureMin = std::numeric_limits<float>::quiet_NaN();

    if (FORECAST_DAYS > day)
    {
        temperatureMin = m_weatherInfo[day].temperatureMin;
    }

    return temperatureMin;
}

float OpenWeatherOneCallForecast::getTemperatureMax(uint8_t day) const
{
    float temperatureMax = std::numeric_limits<float>::quiet_NaN();

    if (FORECAST_DAYS > day)
    {
        temperatureMax = m_weatherInfo[day].temperatureMax;
    }

    return temperatureMax;
}

const String OpenWeatherOneCallForecast::getWeatherIconId(uint8_t day) const
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
