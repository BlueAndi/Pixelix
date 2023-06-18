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
 * @brief  Abstract sensor data channel interface
 * @author Andreas Merkle <web@blue-andi.de>
 * 
 * @addtogroup hal
 *
 * @{
 */

#ifndef ISENSOR_CHANNEL_HPP
#define ISENSOR_CHANNEL_HPP

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <WString.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Abstract sensor channel, which provides the sensor data.
 */
class ISensorChannel
{
public:

    /**
     * Supported sensor channel data types.
     */
    enum DataType
    {
        DATA_TYPE_INVALID = 0,  /**< Invalid data type */
        DATA_TYPE_UINT32,       /**< 32 bit unsigned integer */
        DATA_TYPE_INT32,        /**< 32 bit signed integer */
        DATA_TYPE_FLOAT32,      /**< 32 bit float */
        DATA_TYPE_BOOL          /**< Boolean value */
    };

    /**
     * Supported sensor channel type. Note, currently the SI unit is directly bound.
     */
    enum Type
    {
        TYPE_RAW_NONE = 0,                  /**< Raw digits */
        TYPE_TEMPERATURE_DEGREE_CELSIUS,    /**< Temperature in [°C] */
        TYPE_HUMIDITY_PERCENT,              /**< Humidity in [%] */
        TYPE_ILLUMINANCE_LUX,               /**< Illuminance in [lux] */
        TYPE_STATE_OF_CHARGE_PERCENT        /**< State of Charge in [%] */
    };

    /**
     * Destroys the sensor channel interface.
     */
    ~ISensorChannel()
    {
    }

    /**
     * Get the data type.
     * 
     * @return Sensor data type
     */
    virtual DataType getDataType() const = 0;

    /**
     * Get sensor channel type.
     * 
     * @return Sensor channel type
     */
    virtual Type getType() const = 0;

    /**
     * Get value as string.
     * 
     * @param[in] precision The precision (ignored for integer values) of the value.
     * 
     * @return Value as string
     */
    virtual String getValueAsString(uint32_t precision) = 0;

    /**
     * Get the channel type as string from the corresponding sensor channel type.
     * 
     * @param[in] channelType   Channel type
     * 
     * @return Unit as string
     */
    static String channelTypeToName(ISensorChannel::Type channelType)
    {
        String name;

        switch(channelType)
        {
        case ISensorChannel::TYPE_RAW_NONE:
            name = "raw";
            break;

        case ISensorChannel::TYPE_TEMPERATURE_DEGREE_CELSIUS:
            name = "temperature";
            break;

        case ISensorChannel::TYPE_HUMIDITY_PERCENT:
            name = "humidity";
            break;

        case ISensorChannel::TYPE_ILLUMINANCE_LUX:
            name = "illuminance";
            break;

        case ISensorChannel::TYPE_STATE_OF_CHARGE_PERCENT:
            name = "soc";
            break;

        default:
            break;
        }

        return name;
    }

    /**
     * Get the unit as string from the corresponding sensor channel type.
     * 
     * @param[in] channelType   Channel type
     * 
     * @return Unit as string
     */
    static String channelTypeToUnit(ISensorChannel::Type channelType)
    {
        String unit;

        switch(channelType)
        {
        case ISensorChannel::TYPE_RAW_NONE:
            unit = "digits";
            break;

        case ISensorChannel::TYPE_TEMPERATURE_DEGREE_CELSIUS:
            unit  = "\x8E";
            unit += "C";
            break;

        case ISensorChannel::TYPE_HUMIDITY_PERCENT:
            unit = "%";
            break;

        case ISensorChannel::TYPE_ILLUMINANCE_LUX:
            unit = "lux";
            break;

        case ISensorChannel::TYPE_STATE_OF_CHARGE_PERCENT:
            unit = "%";
            break;

        default:
            break;
        }

        return unit;
    }

protected:

    /**
     * Constructs the sensor channel interface.
     */
    ISensorChannel()
    {
    }

private:

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* ISENSOR_CHANNEL_HPP */

/** @} */