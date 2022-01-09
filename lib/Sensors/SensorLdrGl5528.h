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
 * @brief  Light depended resistor GL5528 driver
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup hal
 *
 * @{
 */

#ifndef __AMBIENT_LIGHT_SENSOR_H__
#define __AMBIENT_LIGHT_SENSOR_H__

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
class SensorLdrGl5528;

/**
 * Illuminance channel of the LDR GL5528 sensor.
 */
class LdrChannelIluminance : public SensorChannelFloat32
{
public:

    /**
     * Constructs the illuminance channel of the LDR GL5528 sensor.
     */
    LdrChannelIluminance() :
        m_driver(nullptr)
    {
    }

    /**
     * Destroys the illuminance channel of the LDR GL5528 sensor.
     */
    ~LdrChannelIluminance()
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
     * @return Sensor data value
     */
    float getValue() final;

    /**
     * Set LDR GL5528 sensor driver.
     * 
     * @param[in] driver    LDR GL5528 driver
     */
    void setDriver(SensorLdrGl5528* driver)
    {
        m_driver = driver;
    }

private:

    SensorLdrGl5528*    m_driver;   /**< LDR GL5528 sensor driver. */

    LdrChannelIluminance(const LdrChannelIluminance& channel);
    LdrChannelIluminance& operator=(const LdrChannelIluminance& channel);
};

/**
 * Light depended resistor GL5528, connected with a 1k pull-down resistor as
 * voltage divider.
 */
class SensorLdrGl5528 : public ISensor
{
public:

    /**
     * Constructs the driver or the GL5528.
     */
    SensorLdrGl5528() :
        m_isAvailable(false),
        m_illuminanceChannel()
    {
        m_illuminanceChannel.setDriver(this);
    }

    /**
     * Destroys the driver for the GL5528.
     */
    ~SensorLdrGl5528()
    {
    }

    /**
     * Configures the sensor, so it is able to provide sensor data.
     */
    void begin() final;

    /**
     * Get sensor name.
     * 
     * @return Sensor name
     */
    const char* getName() const final
    {
        return "GL5528";
    }

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

    /**
     * Threshold to detect a not connected LDR.
     */
    static const uint16_t   NO_LDR_THRESHOLD;

    bool                    m_isAvailable;          /**< Is a sensor available or not? */
    LdrChannelIluminance    m_illuminanceChannel;   /**< Illuminance channel */

    SensorLdrGl5528(const SensorLdrGl5528& sensor);
    SensorLdrGl5528& operator=(const SensorLdrGl5528& sensor);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __AMBIENT_LIGHT_SENSOR_H__ */

/** @} */