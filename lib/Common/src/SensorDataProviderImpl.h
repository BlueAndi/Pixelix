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
 * @brief  Sensor data provider implementation
 * @author Andreas Merkle <web@blue-andi.de>
 * 
 * @addtogroup hal
 *
 * @{
 */

#ifndef SENSOR_DATA_PROVIDER_IMPL_H
#define SENSOR_DATA_PROVIDER_IMPL_H

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

/**
 * Sensor data provider implementation.
 */
class SensorDataProviderImpl
{
public:

    /**
     * Constructs the sensor data provider.
     * 
     * @param[in] sensors   Array with all installed sensor drivers.
     * @param[in] cnt       Number of sensor drivers in the array sensors.
     */
    SensorDataProviderImpl(ISensor* sensors[], uint8_t cnt) :
        m_sensors(sensors),
        m_cnt(cnt)
    {
    }

    /**
     * Destroys the object.
     */
    ~SensorDataProviderImpl()
    {
    }

    /**
     * Initialize the sensor data provider.
     */
    void begin();

    /**
     * Process the sensor drivers.
     */
    void process();

    /**
     * Get number of installed sensor drivers, independed of the physical
     * sensor availability.
     * 
     * @return Number of installed sensor drivers.
     */
    uint8_t getNumSensors() const
    {
        return m_cnt;
    }

    /**
     * Get specific sensor by sensor index.
     * 
     * @param[in] index Index of the sensor
     * 
     * @return If sensor index is valid, it will return the sensor interface otherwise nullptr.
     */
    ISensor* getSensor(uint8_t index);

private:

    ISensor**       m_sensors;  /**< A list with all installed sensor drivers. */
    const uint8_t   m_cnt;      /**< Number of installed sensor drivers. */

    SensorDataProviderImpl();
    SensorDataProviderImpl(const SensorDataProviderImpl& impl);
    SensorDataProviderImpl& operator=(const SensorDataProviderImpl& impl);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* SENSOR_DATA_PROVIDER_IMPL_H */

/** @} */