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
 * @brief  SHT3x sensor
 * @author Andreas Merkle <web@blue-andi.de>
 * 
 * @addtogroup hal
 *
 * @{
 */

#ifndef SENSOR_SHT3X_H
#define SENSOR_SHT3X_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <stddef.h>
#include <ISensor.hpp>
#include <SensorChannelType.hpp>
#include <SHTSensor.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Temperature channel of the SHT3x sensor.
 */
class Sht3XTemperatureChannel : public SensorChannelFloat32
{
public:

    /**
     * Constructs the temperature channel of the SHT3x sensor.
     * 
     * @param[in] driver    The SHT3x driver.
     */
    Sht3XTemperatureChannel(SHTSensor& driver) :
        m_driver(driver),
        m_offset(0.0F)
    {
    }

    /**
     * Destroys the temperature channel of the SHT3x sensor.
     */
    ~Sht3XTemperatureChannel()
    {
    }

    /**
     * Get sensor channel type.
     * 
     * @return Sensor channel type
     */
    Type getType() const final
    {
        return ISensorChannel::TYPE_TEMPERATURE_DEGREE_CELSIUS;
    }

    /**
     * Get the temperature.
     * If there is any error, it will return NaN.
     * 
     * @return Temperature in °C.
     */
    float getValue() final
    {
        float temperature = m_driver.getTemperature();

        if (false == isnan(temperature))
        {
            temperature += m_offset;
        }

        return temperature;
    }

    /**
     * Get the correction offset, used for sensor tolerance compensation.
     * 
     * @return Offset value in °C.
     */
    float getOffset() const final
    {
        return m_offset;
    }

    /**
     * Set correction offset to compensate sensor tolerance.
     * 
     * @param[in] offset    The correction offset value in °C.
     */
    void setOffset(float offset) final
    {
        m_offset = offset;
    }

private:

    SHTSensor&  m_driver;   /**< SHT3x sensor driver. */
    float       m_offset;   /**< Temperature offset in °C for sensor tolerance compensation. */

    Sht3XTemperatureChannel();
    Sht3XTemperatureChannel(const Sht3XTemperatureChannel& channel);
    Sht3XTemperatureChannel& operator=(const Sht3XTemperatureChannel& channel);
};

/**
 * Humidity channel of the SHT3x sensor.
 */
class Sht3XHumidityChannel : public SensorChannelFloat32
{
public:

    /**
     * Constructs the humidity channel of the SHT3x sensor.
     * 
     * @param[in] driver    The SHT3x driver.
     */
    Sht3XHumidityChannel(SHTSensor& driver) :
        m_driver(driver),
        m_offset(0.0F)
    {
    }

    /**
     * Destroys the humidity channel of the SHT3x sensor.
     */
    ~Sht3XHumidityChannel()
    {
    }

    /**
     * Get sensor channel type.
     * 
     * @return Sensor channel type
     */
    Type getType() const final
    {
        return ISensorChannel::TYPE_HUMIDITY_PERCENT;
    }

    /**
     * Get the humidity.
     * If there is any error, it will return NaN.
     * 
     * @return Humidity in %.
     */
    float getValue() final
    {
        float humidity = m_driver.getHumidity();

        if (false == isnan(humidity))
        {
            humidity += m_offset;
        }

        return humidity;
    }

    /**
     * Get the correction offset, used for sensor tolerance compensation.
     * 
     * @return Offset value in %.
     */
    float getOffset() const final
    {
        return m_offset;
    }

    /**
     * Set correction offset to compensate sensor tolerance.
     * 
     * @param[in] offset    The correction offset value in %.
     */
    void setOffset(float offset) final
    {
        m_offset = offset;
    }

private:

    SHTSensor&  m_driver;   /**< SHT3x sensor driver. */
    float       m_offset;   /**< Humidity offset in % for sensor tolerance compensation. */

    Sht3XHumidityChannel();
    Sht3XHumidityChannel(const Sht3XHumidityChannel& channel);
    Sht3XHumidityChannel& operator=(const Sht3XHumidityChannel& channel);
};

/**
 * SHT3x sensor adapter
 */
class SensorSht3X : public ISensor
{
public:

    /**
     * Constructs the SHT3x sensor.
     * 
     * @param[in] model SHT3x sensor model
     */
    SensorSht3X(SHTSensor::SHTSensorType model) :
        m_driver(model),
        m_isAvailable(false),
        m_temperatureChannel(m_driver),
        m_humidityChannel(m_driver)
    {
    }

    /**
     * Destroys the SHT3x sensor.
     */
    ~SensorSht3X()
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
    void process() final;

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
    bool isAvailable() const final
    {
        return m_isAvailable;
    }

    /**
     * Get number of data channels.
     * 
     * @return Number of data channels.
     */
    uint8_t getNumChannels() const final
    {
        return CHANNEL_ID_COUNT;
    }

    /**
     * Get data channel by index.
     * If sensor is not available or channel index is out of bounds, it will 
     * return nullptr.
     * 
     * @return Data channel
     */
    ISensorChannel* getChannel(uint8_t index) final;

private:

    /**
     * Channel id
     */
    enum ChannelId
    {
        CHANNEL_ID_TEMPERATURE = 0, /**< Temperature channel id */
        CHANNEL_ID_HUMIDITY,        /**< Humidity channel id */
        CHANNEL_ID_COUNT            /**< Number of channels */
    };

    SHTSensor               m_driver;               /**< SHT3x sensor driver. */
    bool                    m_isAvailable;          /**< Is a SHT3x sensor available or not? */
    Sht3XTemperatureChannel m_temperatureChannel;   /**< Temperature channel */
    Sht3XHumidityChannel    m_humidityChannel;      /**< Humidity channel */
    
    SensorSht3X();
    SensorSht3X(const SensorSht3X& sensor);
    SensorSht3X& operator=(const SensorSht3X& sensor);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* SENSOR_SHT3X_H */

/** @} */