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
 * @brief  Sensors
 * @author Andreas Merkle <web@blue-andi.de>
 * 
 * @addtogroup hal
 *
 * @{
 */

#ifndef SENSORS_H
#define SENSORS_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <SensorDataProviderImpl.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Sensor channel default value configuration.
 */
typedef struct
{
    uint8_t     sensorId;       /**< Sensor id */
    uint8_t     channelId;      /**< Channel id */
    const char* jsonStrValue;   /**< Default value as JSON string. */
    
} SensorChannelDefaultValue;

/******************************************************************************
 * Functions
 *****************************************************************************/

/**
 * Available sensors.
 */
namespace Sensors
{

/**
 * Get the concrete sensor data provider, which contains all available sensors.
 * 
 * @return Sensor data provider implementation instance
 */
extern SensorDataProviderImpl* getSensorDataProviderImpl();

/**
 * Get the sensor channel default values.
 * 
 * @param[in] values    Number of values in the list.
 * 
 * @return List of default values
 */
extern const SensorChannelDefaultValue* getSensorChannelDefaultValues(uint8_t& values);

}

#endif  /* SENSORS_H */

/** @} */