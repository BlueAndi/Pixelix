/* MIT License
 *
 * Copyright (c) 2019 - 2021 Andreas Merkle <web@blue-andi.de>
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

#ifndef __SENSOR_DATA_PROVIDER_IMPL_H__
#define __SENSOR_DATA_PROVIDER_IMPL_H__

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
     * @param[in] sensors   Array with all registered sensors.
     * @param[in] cnt       Number of sensors in the array sensors.
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
     * Get number of available sensors.
     * 
     * @return Number of available sensors.
     */
    uint8_t getAvailSensors() const
    {
        return m_cnt;
    }

    /**
     * Get specific sensor by sensor index.
     * 
     * @return If sensor index is valid, it will return the sensor interface otherwise nullptr.
     */
    ISensor* getSensor(uint8_t index);

private:

    ISensor**       m_sensors;  /**< A list with all registered sensors. */
    const uint8_t   m_cnt;      /**< Number of registered sensors. */

    SensorDataProviderImpl();
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __SENSOR_DATA_PROVIDER_IMPL_H__ */

/** @} */