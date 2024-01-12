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
 * @brief  Light depended resistor driver
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup hal
 *
 * @{
 */

#ifndef SENSOR_LDR_H
#define SENSOR_LDR_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <Arduino.h>
#include <ISensor.hpp>
#include <SensorChannelType.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/* Forward declaration */
class SensorLdr;

/**
 * Illuminance channel of the LDR sensor.
 */
class LdrChannelIlluminance : public SensorChannelFloat32
{
public:

    /**
     * Constructs the illuminance channel of the LDR sensor.
     */
    LdrChannelIlluminance() :
        m_driver(nullptr),
        m_offset(0.0F)
    {
    }

    /**
     * Destroys the illuminance channel of the LDR sensor.
     */
    ~LdrChannelIlluminance()
    {
    }

    /**
     * Get sensor channel type.
     * 
     * @return Sensor channel type
     */
    Type getType() const final
    {
        return ISensorChannel::TYPE_ILLUMINANCE_LUX;
    }

    /**
     * Get data value.
     * 
     * @return Sensor data value in lux.
     */
    float getValue() final;

    /**
     * Set LDR sensor driver.
     * 
     * @param[in] driver    LDR driver
     */
    void setDriver(SensorLdr* driver)
    {
        m_driver = driver;
    }

    /**
     * Get the correction offset, used for sensor tolerance compensation.
     * 
     * @return Offset value in lux.
     */
    float getOffset() const final
    {
        return m_offset;
    }

    /**
     * Set correction offset to compensate sensor tolerance.
     * 
     * @param[in] offset    The correction offset value in lux.
     */
    void setOffset(float offset) final
    {
        m_offset = offset;
    }

private:

    SensorLdr*  m_driver;   /**< LDR sensor driver. */
    float       m_offset;   /**< Illuminance offset in lux for sensor tolerance compensation. */

    LdrChannelIlluminance(const LdrChannelIlluminance& channel);
    LdrChannelIlluminance& operator=(const LdrChannelIlluminance& channel);
};

/**
 * Light depended resistor, connected with a series pull-down resistor as
 * voltage divider.
 */
class SensorLdr : public ISensor
{
public:

    /** Supported LDR types. */
    enum LdrType
    {
        LDR_TYPE_GL5516 = 0,    /**< GL5516 */
        LDR_TYPE_GL5528,        /**< GL5528 */
        LDR_TYPE_GL5537_1,      /**< GL5537-1 */
        LDR_TYPE_GL5537_2,      /**< GL5537-2 */
        LDR_TYPE_GL5539,        /**< GL5539 */
        LDR_TYPE_GL5549,        /**< GL5549 */
        LDR_TYPE_MAX            /**< Number of supported LDR types. */
    };

    /**
     * Constructs the driver for the LDR.
     * 
     * @param[in] ldrType       The type of the LDR.
     * @param[in] resistance    The series resistance in Ohm.
     */
    SensorLdr(LdrType ldrType, float resistance) :
        m_isAvailable(false),
        m_illuminanceChannel(),
        m_ldrType(ldrType),
        m_resistance(resistance)
    {
        m_illuminanceChannel.setDriver(this);

        if (LDR_TYPE_MAX < m_ldrType)
        {
            m_ldrType = LDR_TYPE_MAX;
        }
    }

    /**
     * Destroys the driver for the LDR.
     */
    ~SensorLdr()
    {
    }

    /**
     * Configures the sensor, so it is able to provide sensor data.
     */
    void begin() final;

    /**
     * Process the sensor driver. Mainly used to read the sensor value and
     * provide its data cached to the sensor channels.
     */
    void process() final
    {
        /* Nothing to do.*/
    }

    /**
     * Get sensor name.
     * 
     * @return Sensor name
     */
    const char* getName() const final;

    /**
     * Is sensor available?
     * If a sensor is physically not available or the initialization failed (see begin()),
     * this can be checked with this method.
     *
     * @return If sensor is available, it will return true otherwise false.
     */
    bool isAvailable() const final;

    /**
     * Get number of data channels.
     * 
     * @return Number of data channels.
     */
    uint8_t getNumChannels() const final
    {
        return 1U;
    }

    /**
     * Get data channel by index.
     * If sensor is not available or channel index is out of bounds, it will 
     * return nullptr.
     * 
     * @return Data channel
     */
    ISensorChannel* getChannel(uint8_t index) final;

    /**
     * Get illuminance in Lux.
     *
     * @return Illuminance level in Lux.
     */
    float getIlluminance(void);

private:

    bool                    m_isAvailable;          /**< Is a sensor available or not? */
    LdrChannelIlluminance   m_illuminanceChannel;   /**< Illuminance channel. */
    LdrType                 m_ldrType;              /**< Type of the LDR. */
    float                   m_resistance;           /**< The series restistance in Ohm. */

    SensorLdr();
    SensorLdr(const SensorLdr& sensor);
    SensorLdr& operator=(const SensorLdr& sensor);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* SENSOR_LDR_H */

/** @} */