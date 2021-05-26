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
 * @brief  Sensor channel type classes
 * @author Andreas Merkle <web@blue-andi.de>
 * 
 * @addtogroup hal
 *
 * @{
 */

#ifndef __SENSOR_CHANNEL_TYPE_HPP__
#define __SENSOR_CHANNEL_TYPE_HPP__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include "ISensorChannel.hpp"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Concrete sensor channel, considering the data type of the channel values.
 */
template <typename T, ISensorChannel::DataType dataType>
class SensorChannelType : public ISensorChannel
{
public:

    /** The data type of the channel value. */
    static const DataType   SENSOR_TYPE = dataType;

    /**
     * Destroys the sensor channel.
     */
    ~SensorChannelType()
    {
    }

    /**
     * Get the data type.
     * 
     * @return Sensor data type
     */
    DataType getType() const
    {
        return SENSOR_TYPE;
    }

    /**
     * Get the kind of data and its unit.
     * 
     * @return Sensor data unit
     */
    virtual DataWithUnit getDataWithUnit() const = 0;

    /**
     * Get data value.
     * 
     * @return Sensor data value
     */
    virtual T getValue() = 0;

protected:

    /**
     * Constructs the sensor channel.
     */
    SensorChannelType()
    {
    }

private:

};

/** Sensor, which provides data as 32 bit unsigned integer. */
typedef SensorChannelType<uint32_t, ISensorChannel::DATA_TYPE_UINT32> SensorChannelUInt32;

/** Sensor, which provides data as 32 bit signed integer. */
typedef SensorChannelType<int32_t, ISensorChannel::DATA_TYPE_INT32> SensorChannelInt32;

/** Sensor, which provides data as 32 bit floating point. */
typedef SensorChannelType<float, ISensorChannel::DATA_TYPE_FLOAT32> SensorChannelFloat32;

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __SENSOR_CHANNEL_TYPE_HPP__ */

/** @} */