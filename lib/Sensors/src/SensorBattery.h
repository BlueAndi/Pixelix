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
 * @brief  Battery state of charge driver
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup hal
 *
 * @{
 */

#ifndef SENSOR_BATTERY_H
#define SENSOR_BATTERY_H

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
class SensorBattery;

/**
 * State of charge channel of the battery sensor.
 */
class BatteryChannelSoC : public SensorChannelUInt32
{
public:

    /**
     * Constructs the SoC channel of the battery sensor.
     */
    BatteryChannelSoC() :
        m_driver(nullptr),
        m_offset(0U)
    {
    }

    /**
     * Destroys the SoC channel of the battery sensor.
     */
    ~BatteryChannelSoC()
    {
    }

    /**
     * Get sensor channel type.
     * 
     * @return Sensor channel type
     */
    Type getType() const final
    {
        return ISensorChannel::TYPE_STATE_OF_CHARGE_PERCENT;
    }

    /**
     * Get data value.
     * 
     * @return Sensor data value in %.
     */
    uint32_t getValue() final;

    /**
     * Set battery sensor driver.
     * 
     * @param[in] driver    Battery driver
     */
    void setDriver(SensorBattery* driver)
    {
        m_driver = driver;
    }

    /**
     * Get the correction offset, used for sensor tolerance compensation.
     * 
     * @return Offset value in lux.
     */
    uint32_t getOffset() const final
    {
        return m_offset;
    }

    /**
     * Set correction offset to compensate sensor tolerance.
     * 
     * @param[in] offset    The correction offset value in %.
     */
    void setOffset(uint32_t offset) final
    {
        m_offset = offset;
    }

private:

    SensorBattery*  m_driver;   /**< Battery sensor driver. */
    uint32_t        m_offset;   /**< State of Charge offset in percent. */

    BatteryChannelSoC(const BatteryChannelSoC& channel);
    BatteryChannelSoC& operator=(const BatteryChannelSoC& channel);
};

/**
 * The sensor derives the state of charge from a battery by reading
 * its voltage.
 */
class SensorBattery : public ISensor
{
public:

    /**
     * Constructs the driver for the battery sensor.
     */
    SensorBattery() :
        m_isAvailable(false),
        m_socChannel(),
        m_adcRawAvg(0U),
        m_isInit(true)
    {
        m_socChannel.setDriver(this);
    }

    /**
     * Destroys the driver for the battery sensor.
     */
    ~SensorBattery()
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
    const char* getName() const final
    {
        return "Battery";
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
        return static_cast<uint8_t>(CHANNEL_ID_MAX);
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
     * Get state of charge in percent.
     *
     * @return State of Charge in %.
     */
    float getStateOfCharge(void);

private:

    /**
     * Supported channels.
     */
    enum ChannelId
    {
        CHANNEL_ID_SOC = 0, /**< Id of SOC channel. */
        CHANNEL_ID_MAX      /**< Max. number of supported channels. */
    };

    /** Raw ADC value for a empty battery (0%). Note, this is for the Ulanzi TC001. */
    static const uint16_t   ADC_RAW_EMPTY   = 2160U;

    /** Raw ADC value for a full battery (100%). Note, this is for the Ulanzi TC001. */
    static const uint16_t   ADC_RAW_FULL    = 2500U;

    bool                    m_isAvailable;  /**< Is a sensor available or not? */
    BatteryChannelSoC       m_socChannel;   /**< State of charge channel. */
    uint16_t                m_adcRawAvg;    /**< Moving average of raw battery ADC value. */
    bool                    m_isInit;       /**< First time the battery ADC value is read? */

    SensorBattery(const SensorBattery& sensor);
    SensorBattery& operator=(const SensorBattery& sensor);

    /**
     * Get the moving average of the battery ADC raw value.
     * 
     * @return Moving average of battery AC raw value in digits.
     */
    uint16_t getAdcRawAvg();
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* SENSOR_BATTERY_H */

/** @} */