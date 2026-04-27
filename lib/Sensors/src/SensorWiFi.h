/* MIT License
 *
 * Copyright (c) 2019 - 2026 Andreas Merkle <web@blue-andi.de>
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
 * @file   SensorWiFi.h
 * @brief  WiFi sensor driver
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup SENSORS
 *
 * @{
 */

#ifndef SENSOR_WIFI_H
#define SENSOR_WIFI_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <Arduino.h>
#include <ISensor.hpp>
#include <SensorChannelType.hpp>
#include <WiFi.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Signal strength channel of the wifi sensor sensor.
 */
class WiFiChannelSignalStrength : public SensorChannelInt32
{
public:

    /**
     * Constructs the channel of the wifi sensor sensor.
     */
    WiFiChannelSignalStrength()
    {
    }

    /**
     * Destroys the channel of the wifi sensor sensor.
     */
    ~WiFiChannelSignalStrength()
    {
    }

    /**
     * Get sensor channel type.
     *
     * @return Sensor channel type
     */
    Type getType() const final
    {
        return ISensorChannel::TYPE_SIGNAL_STRENGTH_DBM;
    }

    /**
     * Get signal strength.
     *
     * @return Signal strength in dBm.
     */
    int32_t getValue() final
    {
        return WiFi.RSSI();
    }

    /**
     * Get the correction offset, used for sensor tolerance compensation.
     * Not used, will always return 0.
     *
     * @return Offset value in bytes.
     */
    int32_t getOffset() const final
    {
        return 0;
    }

    /**
     * Set correction offset to compensate sensor tolerance.
     * Not used, will do nothing.
     *
     * @param[in] offset    The correction offset value in bytes.
     */
    void setOffset(int32_t offset) final
    {
        (void)offset;
    }

private:

    WiFiChannelSignalStrength(const WiFiChannelSignalStrength& channel);
    WiFiChannelSignalStrength& operator=(const WiFiChannelSignalStrength& channel);
};

/**
 * WiFi sensor driver.
 */
class SensorWiFi : public ISensor
{
public:

    /**
     * Constructs the driver for the wifi sensor.
     */
    SensorWiFi() :
        m_signalStrengthChannel()
    {
    }

    /**
     * Destroys the driver for the wifi sensor.
     */
    ~SensorWiFi()
    {
    }

    /**
     * Configures the sensor, so it is able to provide sensor data.
     */
    void begin() final
    {
        /* Nothing to do. */
    }

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
        return "WiFi";
    }

    /**
     * Is sensor available?
     * If a sensor is physically not available or the initialization failed (see begin()),
     * this can be checked with this method.
     *
     * @return If sensor is available, it will return true otherwise false.
     */
    bool isAvailable() const final
    {
        return true;
    }

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
     * @param[in] index Index of the data channel.
     *
     * @return Data channel
     */
    ISensorChannel* getChannel(uint8_t index) final;

private:

    WiFiChannelSignalStrength m_signalStrengthChannel; /**< Signal strength channel. */

    SensorWiFi(const SensorWiFi& sensor);
    SensorWiFi& operator=(const SensorWiFi& sensor);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* SENSOR_WIFI_H */

/** @} */