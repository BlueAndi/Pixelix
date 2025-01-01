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
 * @brief  Interface for the forecast weather
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup PLUGIN
 *
 * @{
 */

#ifndef IOPEN_WEATHER_FORECAST_H
#define IOPEN_WEATHER_FORECAST_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <WString.h>
#include <stdint.h>
#include <ArduinoJson.h>
#include "IOpenWeatherGeneric.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** This type is the abstract interface for the forecast weather. */
class IOpenWeatherForecast : public IOpenWeatherGeneric
{
public:

    /**
     * Get the min. temperature.
     * Might be NaN in case no response was never parsed
     * or its not supported by the OpenWeather source.
     * 
     * @param[in] day   Forecast day [0; 4]
     * 
     * @return Temperature, the unit is according to configuration.
     */
    virtual float getTemperatureMin(uint8_t day) const = 0;

    /**
     * Get the max. temperature.
     * Might be NaN in case no response was never parsed
     * or its not supported by the OpenWeather source.
     * 
     * @param[in] day   Forecast day [0; 4]
     * 
     * @return Temperature, the unit is according to configuration.
     */
    virtual float getTemperatureMax(uint8_t day) const = 0;

    /**
     * Get the weather icon id.
     * 
     * @param[in] day   Forecast day [0; 4]
     * 
     * @return Weather icon id
     */
    virtual const String getWeatherIconId(uint8_t day) const = 0;

protected:
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* IOPEN_WEATHER_FORECAST_H */

/** @} */