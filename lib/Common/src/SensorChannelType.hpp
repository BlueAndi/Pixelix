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
 * @brief  Sensor channel type classes
 * @author Andreas Merkle <web@blue-andi.de>
 * 
 * @addtogroup hal
 *
 * @{
 */

#ifndef SENSOR_CHANNEL_TYPE_HPP
#define SENSOR_CHANNEL_TYPE_HPP

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <stdio.h>
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
    DataType getDataType() const final
    {
        return dataType;
    }

    /**
     * Get sensor channel type.
     * 
     * @return Sensor channel type
     */
    virtual Type getType() const = 0;

    /**
     * Get data value.
     * 
     * @return Sensor data value
     */
    virtual T getValue() = 0;

    /**
     * Get the correction offset, used for sensor tolerance compensation.
     * 
     * @return Offset value
     */
    virtual T getOffset() const = 0;

    /**
     * Set correction offset to compensate sensor tolerance.
     * 
     * @param[in] offset    The correction offset value.
     */
    virtual void setOffset(T offset) = 0;

    /**
     * Get value as string.
     * 
     * @param[in] precision The precision (ignored for integer values) of the value.
     * 
     * @return Value as string
     */
    String getValueAsString(uint32_t precision) override
    {
        (void)precision;
        return String(getValue());
    }

protected:

    /**
     * Constructs the sensor channel.
     */
    SensorChannelType()
    {
    }

private:

};

/**
 * Specialization for 32 bit floating point value channel.
 */
template <>
class SensorChannelType<float, ISensorChannel::DATA_TYPE_FLOAT32> : public ISensorChannel
{
public:

    /**
     * Get the data type.
     * 
     * @return Sensor data type
     */
    DataType getDataType() const final
    {
        return DATA_TYPE_FLOAT32;
    }

    /**
     * Get sensor channel type.
     * 
     * @return Sensor channel type
     */
    virtual Type getType() const = 0;

    /**
     * Get data value.
     * If there is any error, it will return NaN.
     * 
     * @return Sensor data value
     */
    virtual float getValue() = 0;

    /**
     * Get the correction offset, used for sensor tolerance compensation.
     * 
     * @return Offset value
     */
    virtual float getOffset() const = 0;

    /**
     * Set correction offset to compensate sensor tolerance.
     * 
     * @param[in] offset    The correction offset value.
     */
    virtual void setOffset(float offset) = 0;

    /**
     * Get value as string.
     * If there is any error, it will return "NAN".
     * 
     * @param[in] precision The precision (ignored for integer values) of the value.
     * 
     * @return Value as string
     */
    String getValueAsString(uint32_t precision) override
    {
        float   value       = getValue();
        String  valueStr;
        char    buffer[20U];

        (void)snprintf(buffer, sizeof(buffer), "%.*F", precision, value);
        valueStr = buffer;

        return valueStr;
    }
};

/**
 * Specialization for boolean value channel.
 */
template <>
class SensorChannelType<bool, ISensorChannel::DATA_TYPE_BOOL> : public ISensorChannel
{
public:

    /**
     * Get the data type.
     * 
     * @return Sensor data type
     */
    DataType getDataType() const final
    {
        return DATA_TYPE_BOOL;
    }

    /**
     * Get sensor channel type.
     * 
     * @return Sensor channel type
     */
    virtual Type getType() const = 0;

    /**
     * Get data value.
     * 
     * @return Sensor data value
     */
    virtual bool getValue() = 0;

    /**
     * Get the correction offset, used for sensor tolerance compensation.
     * 
     * @return Offset value
     */
    virtual bool getOffset() const = 0;

    /**
     * Set correction offset to compensate sensor tolerance.
     * 
     * @param[in] offset    The correction offset value.
     */
    virtual void setOffset(bool offset) = 0;

    /**
     * Get value as string.
     * 
     * @param[in] precision The precision (ignored for integer values) of the value.
     * 
     * @return Value as string
     */
    String getValueAsString(uint32_t precision) override
    {
        bool value = getValue();

        (void)precision;

        return (false == value) ? "false" : "true";
    }
};

/** Sensor, which provides data as 32 bit unsigned integer. */
typedef SensorChannelType<uint32_t, ISensorChannel::DATA_TYPE_UINT32> SensorChannelUInt32;

/** Sensor, which provides data as 32 bit signed integer. */
typedef SensorChannelType<int32_t, ISensorChannel::DATA_TYPE_INT32> SensorChannelInt32;

/** Sensor, which provides data as 32 bit floating point. */
typedef SensorChannelType<float, ISensorChannel::DATA_TYPE_FLOAT32> SensorChannelFloat32;

/** Sensor, which provides data as boolean. */
typedef SensorChannelType<bool, ISensorChannel::DATA_TYPE_BOOL> SensorChannelBool;

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* SENSOR_CHANNEL_TYPE_HPP */

/** @} */