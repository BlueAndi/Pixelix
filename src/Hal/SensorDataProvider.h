/* MIT License
 *
 * Copyright (c) 2019 - 2022 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Sensor data provider
 * @author Andreas Merkle <web@blue-andi.de>
 * 
 * @addtogroup hal
 *
 * @{
 */

#ifndef __SENSOR_DATA_PROVIDER_H__
#define __SENSOR_DATA_PROVIDER_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <ISensor.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/* Forward declaration */
class SensorDataProviderImpl;

/**
 * It provides access to all installed sensor drivers and the
 * data of physical available sensors in the system.
 */
class SensorDataProvider
{
public:

    /**
     * Get instance of the sensor data provider.
     * 
     * @return Sensor data provider
     */
    static SensorDataProvider& getInstance()
    {
        static SensorDataProvider   instance;

        return instance;
    }

    /**
     * Destroys the sensor data provider.
     */
    ~SensorDataProvider()
    {
    }

    /**
     * Initialize the sensor data provider.
     */
    void begin();

    /**
     * Get number of installed sensor drivers, independed of the physical
     * sensor availability.
     * 
     * @return Number of installed sensor drivers.
     */
    uint8_t getNumSensors() const;

    /**
     * Get specific sensor by sensor index.
     * 
     * @param[in] index Index of the sensor
     * 
     * @return If sensor index is valid, it will return the sensor interface otherwise nullptr.
     */
    ISensor* getSensor(uint8_t index);

    /**
     * Find sensor channel by its data, unit and value data type.
     * It considers the physical sensor availablity.
     * 
     * @param[out]  sensorIndex     The index of the sensor.
     * @param[out]  channelIndex    The index of the channel from the sensor.
     * @param[in]   type            The sensor channel type to search for.
     * @param[in]   dataType        The sensor channel data type to search for. Default: disabled
     * @param[in]   sensorStartIdx  The sensor index, where to start to search. Default: 0
     * @param[in]   channelStartIdx The channel index, where to start to search. Default: 0
     * 
     * @return If sensor found, it will return its index otherwise INVALID_SENSOR_IDX.
     */
    bool find(  uint8_t& sensorIndex,
                uint8_t& channelIndex,
                ISensorChannel::Type type,
                ISensorChannel::DataType dataType = ISensorChannel::DATA_TYPE_INVALID,
                uint8_t sensorStartIdx = 0U,
                uint8_t channelStartIdx = 0U);

    /**
     * Invalid sensor index.
     */
    static const uint8_t    INVALID_SENSOR_IDX  = UINT8_MAX;

private:

    /**
     * Hidden implementation to avoid to include here all available sensors directly.
     */
    SensorDataProviderImpl* m_impl;

    /**
     * Constructs the sensor data provder.
     */
    SensorDataProvider();

    SensorDataProvider(const SensorDataProvider& instance);
    SensorDataProvider& operator=(const SensorDataProvider& instance);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __SENSOR_DATA_PROVIDER_H__ */

/** @} */