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
 * @brief  SHT3x sensor
 * @author Andreas Merkle <web@blue-andi.de>
 * 
 * @addtogroup hal
 *
 * @{
 */

#ifndef __SENSOR_SHT3X_H__
#define __SENSOR_SHT3X_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
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
        m_driver(driver)
    {
    }

    /**
     * Destroys the temperature channel of the SHT3x sensor.
     */
    ~Sht3XTemperatureChannel()
    {
    }

    /**
     * Get the kind of data and its unit.
     * 
     * @return Sensor data unit
     */
    DataWithUnit getDataWithUnit() const final
    {
        return ISensorChannel::DATA_TEMPERATURE_DEGREE_CELSIUS;
    }

    /**
     * Get data value.
     * 
     * @return Sensor data value
     */
    float getValue()
    {
        return m_driver.getTemperature();
    }

private:

    SHTSensor&  m_driver;   /**< SHT3x sensor driver. */

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
        m_driver(driver)
    {
    }

    /**
     * Destroys the humidity channel of the SHT3x sensor.
     */
    ~Sht3XHumidityChannel()
    {
    }

    /**
     * Get the kind of data and its unit.
     * 
     * @return Sensor data unit
     */
    DataWithUnit getDataWithUnit() const final
    {
        return ISensorChannel::DATA_HUMIDITY_PERCENT;
    }

    /**
     * Get data value.
     * 
     * @return Sensor data value
     */
    float getValue()
    {
        return m_driver.getHumidity();
    }

private:

    SHTSensor&  m_driver;   /**< SHT3x sensor driver. */

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

#endif  /* __SENSOR_SHT3X_H__ */

/** @} */