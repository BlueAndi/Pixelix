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
 * @brief  Interface for the current weather
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup PLUGIN
 *
 * @{
 */

#ifndef IOPEN_WEATHER_CURRENT_H
#define IOPEN_WEATHER_CURRENT_H

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

/** This type is the abstract interface for the current weather. */
class IOpenWeatherCurrent : public IOpenWeatherGeneric
{
public:

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

#endif  /* IOPEN_WEATHER_CURRENT_H */

/** @} */