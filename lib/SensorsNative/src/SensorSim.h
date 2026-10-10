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
 * @file   SensorSim.h
 * @brief  Simulated sensor
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * The host has no physical sensor at all. To be able to run and test the
 * complete application on the host, this sensor simulates every sensor channel
 * which a real device may provide.
 *
 * The simulated values are derived from the system tick, therefore they change
 * over time without using a random number generator. That keeps a program run
 * reproducible.
 *
 * @addtogroup SENSORS
 *
 * @{
 */

#ifndef SENSOR_SIM_H
#define SENSOR_SIM_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <Arduino.h>
#include <ISensor.hpp>
#include <SensorChannelType.hpp>

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Simulated sensor channel, which provides a 32 bit floating point value.
 *
 * The value moves continuously between a minimum and a maximum, like a
 * triangular wave.
 */
class SensorSimChannelFloat32 : public SensorChannelFloat32
{
public:

    /**
     * Constructs a simulated sensor channel.
     *
     * @param[in] channelType   The type of the sensor channel.
     * @param[in] minValue      Lower limit of the simulated value.
     * @param[in] maxValue      Upper limit of the simulated value.
     * @param[in] periodMs      Duration of one complete up and down cycle in ms.
     * @param[in] phaseMs       Shift of the cycle start in ms.
     */
    SensorSimChannelFloat32(Type channelType, float minValue, float maxValue, uint32_t periodMs, uint32_t phaseMs) :
        SensorChannelFloat32(),
        m_channelType(channelType),
        m_minValue(minValue),
        m_maxValue(maxValue),
        m_periodMs(periodMs),
        m_phaseMs(phaseMs),
        m_offset(0.0F)
    {
    }

    /**
     * Destroys the simulated sensor channel.
     */
    ~SensorSimChannelFloat32()
    {
    }

    /**
     * Get sensor channel type.
     *
     * @return Sensor channel type
     */
    Type getType() const final
    {
        return m_channelType;
    }

    /**
     * Get the simulated value, including the correction offset.
     *
     * @return Sensor value
     */
    float getValue() final;

    /**
     * Get the correction offset, used for sensor tolerance compensation.
     *
     * @return Offset value
     */
    float getOffset() const final
    {
        return m_offset;
    }

    /**
     * Set correction offset to compensate sensor tolerance.
     *
     * @param[in] offset    The correction offset value.
     */
    void setOffset(float offset) final
    {
        m_offset = offset;
    }

private:

    const Type     m_channelType; /**< Type of the sensor channel. */
    const float    m_minValue;    /**< Lower limit of the simulated value. */
    const float    m_maxValue;    /**< Upper limit of the simulated value. */
    const uint32_t m_periodMs;    /**< Duration of one complete cycle in ms. */
    const uint32_t m_phaseMs;     /**< Shift of the cycle start in ms. */
    float          m_offset;      /**< Correction offset. */

    SensorSimChannelFloat32();
    SensorSimChannelFloat32(const SensorSimChannelFloat32& channel);
    SensorSimChannelFloat32& operator=(const SensorSimChannelFloat32& channel);
};

/**
 * Simulated sensor channel, which provides a 32 bit signed integer value.
 *
 * The value moves continuously between a minimum and a maximum, like a
 * triangular wave.
 */
class SensorSimChannelInt32 : public SensorChannelInt32
{
public:

    /**
     * Constructs a simulated sensor channel.
     *
     * @param[in] channelType   The type of the sensor channel.
     * @param[in] minValue      Lower limit of the simulated value.
     * @param[in] maxValue      Upper limit of the simulated value.
     * @param[in] periodMs      Duration of one complete up and down cycle in ms.
     * @param[in] phaseMs       Shift of the cycle start in ms.
     */
    SensorSimChannelInt32(Type channelType, int32_t minValue, int32_t maxValue, uint32_t periodMs, uint32_t phaseMs) :
        SensorChannelInt32(),
        m_channelType(channelType),
        m_minValue(minValue),
        m_maxValue(maxValue),
        m_periodMs(periodMs),
        m_phaseMs(phaseMs),
        m_offset(0)
    {
    }

    /**
     * Destroys the simulated sensor channel.
     */
    ~SensorSimChannelInt32()
    {
    }

    /**
     * Get sensor channel type.
     *
     * @return Sensor channel type
     */
    Type getType() const final
    {
        return m_channelType;
    }

    /**
     * Get the simulated value, including the correction offset.
     *
     * @return Sensor value
     */
    int32_t getValue() final;

    /**
     * Get the correction offset, used for sensor tolerance compensation.
     *
     * @return Offset value
     */
    int32_t getOffset() const final
    {
        return m_offset;
    }

    /**
     * Set correction offset to compensate sensor tolerance.
     *
     * @param[in] offset    The correction offset value.
     */
    void setOffset(int32_t offset) final
    {
        m_offset = offset;
    }

private:

    const Type     m_channelType; /**< Type of the sensor channel. */
    const int32_t  m_minValue;    /**< Lower limit of the simulated value. */
    const int32_t  m_maxValue;    /**< Upper limit of the simulated value. */
    const uint32_t m_periodMs;    /**< Duration of one complete cycle in ms. */
    const uint32_t m_phaseMs;     /**< Shift of the cycle start in ms. */
    int32_t        m_offset;      /**< Correction offset. */

    SensorSimChannelInt32();
    SensorSimChannelInt32(const SensorSimChannelInt32& channel);
    SensorSimChannelInt32& operator=(const SensorSimChannelInt32& channel);
};

/**
 * Sensor channel, which provides a 32 bit unsigned integer value by a callback.
 * Used for values the host is able to provide for real, like the heap
 * statistics.
 */
class SensorSimChannelUInt32 : public SensorChannelUInt32
{
public:

    /**
     * Function which provides the sensor channel value.
     */
    typedef uint32_t (*GetValueFunc)(void);

    /**
     * Constructs a sensor channel.
     *
     * @param[in] channelType   The type of the sensor channel.
     * @param[in] getValueFunc  Function which provides the value. May be nullptr.
     */
    SensorSimChannelUInt32(Type channelType, GetValueFunc getValueFunc) :
        SensorChannelUInt32(),
        m_channelType(channelType),
        m_getValueFunc(getValueFunc),
        m_offset(0U)
    {
    }

    /**
     * Destroys the sensor channel.
     */
    ~SensorSimChannelUInt32()
    {
    }

    /**
     * Get sensor channel type.
     *
     * @return Sensor channel type
     */
    Type getType() const final
    {
        return m_channelType;
    }

    /**
     * Get the value, including the correction offset.
     *
     * @return Sensor value
     */
    uint32_t getValue() final;

    /**
     * Get the correction offset, used for sensor tolerance compensation.
     *
     * @return Offset value
     */
    uint32_t getOffset() const final
    {
        return m_offset;
    }

    /**
     * Set correction offset to compensate sensor tolerance.
     *
     * @param[in] offset    The correction offset value.
     */
    void setOffset(uint32_t offset) final
    {
        m_offset = offset;
    }

private:

    const Type         m_channelType;  /**< Type of the sensor channel. */
    const GetValueFunc m_getValueFunc; /**< Function which provides the value. */
    uint32_t           m_offset;       /**< Correction offset. */

    SensorSimChannelUInt32();
    SensorSimChannelUInt32(const SensorSimChannelUInt32& channel);
    SensorSimChannelUInt32& operator=(const SensorSimChannelUInt32& channel);
};

/**
 * Sensor channel, which provides a 64 bit unsigned integer value by a callback.
 */
class SensorSimChannelUInt64 : public SensorChannelUInt64
{
public:

    /**
     * Function which provides the sensor channel value.
     */
    typedef uint64_t (*GetValueFunc)(void);

    /**
     * Constructs a sensor channel.
     *
     * @param[in] channelType   The type of the sensor channel.
     * @param[in] getValueFunc  Function which provides the value. May be nullptr.
     */
    SensorSimChannelUInt64(Type channelType, GetValueFunc getValueFunc) :
        SensorChannelUInt64(),
        m_channelType(channelType),
        m_getValueFunc(getValueFunc),
        m_offset(0ULL)
    {
    }

    /**
     * Destroys the sensor channel.
     */
    ~SensorSimChannelUInt64()
    {
    }

    /**
     * Get sensor channel type.
     *
     * @return Sensor channel type
     */
    Type getType() const final
    {
        return m_channelType;
    }

    /**
     * Get the value, including the correction offset.
     *
     * @return Sensor value
     */
    uint64_t getValue() final;

    /**
     * Get the correction offset, used for sensor tolerance compensation.
     *
     * @return Offset value
     */
    uint64_t getOffset() const final
    {
        return m_offset;
    }

    /**
     * Set correction offset to compensate sensor tolerance.
     *
     * @param[in] offset    The correction offset value.
     */
    void setOffset(uint64_t offset) final
    {
        m_offset = offset;
    }

private:

    const Type         m_channelType;  /**< Type of the sensor channel. */
    const GetValueFunc m_getValueFunc; /**< Function which provides the value. */
    uint64_t           m_offset;       /**< Correction offset. */

    SensorSimChannelUInt64();
    SensorSimChannelUInt64(const SensorSimChannelUInt64& channel);
    SensorSimChannelUInt64& operator=(const SensorSimChannelUInt64& channel);
};

/**
 * Simulated sensor, which provides every sensor channel the REST API and the
 * MQTT API know about. It replaces the physical sensors on the host.
 */
class SensorSim : public ISensor
{
public:

    /** Number of provided sensor channels. */
    static const uint8_t CHANNEL_COUNT = 9U;

    /**
     * Constructs the simulated sensor.
     */
    SensorSim();

    /**
     * Destroys the simulated sensor.
     */
    ~SensorSim()
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
        /* Nothing to do. */
    }

    /**
     * Get sensor name.
     *
     * @return Sensor name
     */
    const char* getName() const final
    {
        return "Simulated";
    }

    /**
     * Is sensor available?
     * The simulation is always available.
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
        return CHANNEL_COUNT;
    }

    /**
     * Get data channel by index.
     * If the channel index is out of bounds, it will return nullptr.
     *
     * @param[in] index Index of the data channel.
     *
     * @return Data channel
     */
    ISensorChannel* getChannel(uint8_t index) final;

private:

    SensorSimChannelFloat32 m_temperatureChannel;    /**< Temperature in [deg. C]. */
    SensorSimChannelFloat32 m_humidityChannel;       /**< Humidity in [%]. */
    SensorSimChannelFloat32 m_illuminanceChannel;    /**< Illuminance in [lux]. */
    SensorSimChannelUInt32  m_stateOfChargeChannel;  /**< State of charge in [%]. */
    SensorSimChannelUInt32  m_freeHeapChannel;       /**< Free heap in [byte]. */
    SensorSimChannelUInt32  m_minFreeHeapChannel;    /**< Min. free heap in [byte]. */
    SensorSimChannelUInt32  m_maxAllocHeapChannel;   /**< Max. allocatable heap in [byte]. */
    SensorSimChannelInt32   m_signalStrengthChannel; /**< WiFi signal strength in [dBm]. */
    SensorSimChannelUInt64  m_uptimeChannel;         /**< Uptime in [s]. */

    SensorSim(const SensorSim& sensor);
    SensorSim& operator=(const SensorSim& sensor);
};

/******************************************************************************
 * Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* SENSOR_SIM_H */

/** @} */
