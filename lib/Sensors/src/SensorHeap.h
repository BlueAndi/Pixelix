/* MIT License
 *
 * Copyright (c) 2019 - 2025 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Heap memory observer driver
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup SENSORS
 *
 * @{
 */

#ifndef SENSOR_HEAP_H
#define SENSOR_HEAP_H

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

/**
 * Available heap memory channel of the heap memory observer sensor.
 */
class HeapChannelFreeHeap : public SensorChannelUInt32
{
public:

    /**
     * Constructs the channel of the heap memory observer sensor.
     */
    HeapChannelFreeHeap()
    {
    }

    /**
     * Destroys the channel of the heap memory observer sensor.
     */
    ~HeapChannelFreeHeap()
    {
    }

    /**
     * Get sensor channel type.
     *
     * @return Sensor channel type
     */
    Type getType() const final
    {
        return ISensorChannel::TYPE_FREE_HEAP_BYTES;
    }

    /**
     * Get heap memory size.
     *
     * @return Heap memory size in bytes
     */
    uint32_t getValue() final
    {
        return ESP.getFreeHeap();
    }

    /**
     * Get the correction offset, used for sensor tolerance compensation.
     * Not used, will always return 0.
     *
     * @return Offset value in bytes.
     */
    uint32_t getOffset() const final
    {
        return 0U;
    }

    /**
     * Set correction offset to compensate sensor tolerance.
     * Not used, will do nothing.
     *
     * @param[in] offset    The correction offset value in bytes.
     */
    void setOffset(uint32_t offset) final
    {
        (void)offset;
    }

private:

    HeapChannelFreeHeap(const HeapChannelFreeHeap& channel);
    HeapChannelFreeHeap& operator=(const HeapChannelFreeHeap& channel);
};

/**
 * Lowest heap memory level since boot channel of the heap memory observer sensor.
 */
class HeapChannelMinFreeHeap : public SensorChannelUInt32
{
public:

    /**
     * Constructs the channel of the heap memory observer sensor.
     */
    HeapChannelMinFreeHeap()
    {
    }

    /**
     * Destroys the channel of the heap memory observer sensor.
     */
    ~HeapChannelMinFreeHeap()
    {
    }

    /**
     * Get sensor channel type.
     *
     * @return Sensor channel type
     */
    Type getType() const final
    {
        return ISensorChannel::TYPE_MIN_FREE_HEAP_BYTES;
    }

    /**
     * Get heap memory size.
     *
     * @return Heap memory size in bytes
     */
    uint32_t getValue() final
    {
        return ESP.getMinFreeHeap();
    }

    /**
     * Get the correction offset, used for sensor tolerance compensation.
     * Not used, will always return 0.
     *
     * @return Offset value in bytes.
     */
    uint32_t getOffset() const final
    {
        return 0U;
    }

    /**
     * Set correction offset to compensate sensor tolerance.
     * Not used, will do nothing.
     *
     * @param[in] offset    The correction offset value in bytes.
     */
    void setOffset(uint32_t offset) final
    {
        (void)offset;
    }

private:

    HeapChannelMinFreeHeap(const HeapChannelMinFreeHeap& channel);
    HeapChannelMinFreeHeap& operator=(const HeapChannelMinFreeHeap& channel);
};

/**
 * Largest block of heap that can be allocated channel of the heap memory observer sensor.
 */
class HeapChannelMaxAllocHeap : public SensorChannelUInt32
{
public:

    /**
     * Constructs the channel of the heap memory observer sensor.
     */
    HeapChannelMaxAllocHeap()
    {
    }

    /**
     * Destroys the channel of the heap memory observer sensor.
     */
    ~HeapChannelMaxAllocHeap()
    {
    }

    /**
     * Get sensor channel type.
     *
     * @return Sensor channel type
     */
    Type getType() const final
    {
        return ISensorChannel::TYPE_MAX_ALLOC_HEAP_BYTES;
    }

    /**
     * Get heap memory size.
     *
     * @return Heap memory size in bytes
     */
    uint32_t getValue() final
    {
        return ESP.getMaxAllocHeap();
    }

    /**
     * Get the correction offset, used for sensor tolerance compensation.
     * Not used, will always return 0.
     *
     * @return Offset value in bytes.
     */
    uint32_t getOffset() const final
    {
        return 0U;
    }

    /**
     * Set correction offset to compensate sensor tolerance.
     * Not used, will do nothing.
     *
     * @param[in] offset    The correction offset value in bytes.
     */
    void setOffset(uint32_t offset) final
    {
        (void)offset;
    }

private:

    HeapChannelMaxAllocHeap(const HeapChannelMaxAllocHeap& channel);
    HeapChannelMaxAllocHeap& operator=(const HeapChannelMaxAllocHeap& channel);
};

/**
 * Heap memory observer driver.
 */
class SensorHeap : public ISensor
{
public:

    /**
     * Constructs the driver for the heap memory observer.
     */
    SensorHeap() :
        m_freeHeapChannel(),
        m_minFreeHeapChannel(),
        m_maxAllocHeapChannel()
    {
    }

    /**
     * Destroys the driver for the heap memory observer.
     */
    ~SensorHeap()
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
        return "Heap";
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
        return 3U;
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

    HeapChannelFreeHeap     m_freeHeapChannel;     /**< Channel to retrieve the available heap. */
    HeapChannelMinFreeHeap  m_minFreeHeapChannel;  /**< Channel to retrieve the lowest available heap since boot. */
    HeapChannelMaxAllocHeap m_maxAllocHeapChannel; /**< Channel to retrieve the largest block of heap that can be allocated at once. */

    SensorHeap(const SensorHeap& sensor);
    SensorHeap& operator=(const SensorHeap& sensor);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* SENSOR_HEAP_H */

/** @} */