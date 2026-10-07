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
 * @file   SensorSim.cpp
 * @brief  Simulated sensor
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "SensorSim.h"

#include <MemUtil.h>

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and classes
 *****************************************************************************/

/******************************************************************************
 * Prototypes
 *****************************************************************************/

static float    getSimulationRatio(uint32_t periodMs, uint32_t phaseMs);
static uint32_t getStateOfCharge();
static uint32_t getFreeHeap();
static uint32_t getMinFreeHeap();
static uint32_t getMaxAllocHeap();
static uint64_t getUptime();

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/** Period of the simulated temperature in ms. */
static const uint32_t TEMPERATURE_PERIOD_MS     = 120000U;

/** Period of the simulated humidity in ms. */
static const uint32_t HUMIDITY_PERIOD_MS        = 180000U;

/** Period of the simulated illuminance in ms. */
static const uint32_t ILLUMINANCE_PERIOD_MS     = 60000U;

/** Period of the simulated state of charge in ms. */
static const uint32_t STATE_OF_CHARGE_PERIOD_MS = 300000U;

/** Period of the simulated WiFi signal strength in ms. */
static const uint32_t SIGNAL_STRENGTH_PERIOD_MS = 90000U;

/** Lower limit of the simulated state of charge in [%]. */
static const uint32_t STATE_OF_CHARGE_MIN       = 20U;

/** Upper limit of the simulated state of charge in [%]. */
static const uint32_t STATE_OF_CHARGE_MAX       = 100U;

/******************************************************************************
 * Public Methods
 *****************************************************************************/

float SensorSimChannelFloat32::getValue()
{
    float ratio = getSimulationRatio(m_periodMs, m_phaseMs);

    return m_offset + m_minValue + ((m_maxValue - m_minValue) * ratio);
}

int32_t SensorSimChannelInt32::getValue()
{
    float   ratio = getSimulationRatio(m_periodMs, m_phaseMs);
    float   span  = static_cast<float>(m_maxValue - m_minValue);
    int32_t value = m_minValue + static_cast<int32_t>(span * ratio);

    return m_offset + value;
}

uint32_t SensorSimChannelUInt32::getValue()
{
    uint32_t value = 0U;

    if (nullptr != m_getValueFunc)
    {
        value = m_getValueFunc();
    }

    return m_offset + value;
}

uint64_t SensorSimChannelUInt64::getValue()
{
    uint64_t value = 0ULL;

    if (nullptr != m_getValueFunc)
    {
        value = m_getValueFunc();
    }

    return m_offset + value;
}

SensorSim::SensorSim() :
    ISensor(),
    m_temperatureChannel(ISensorChannel::TYPE_TEMPERATURE_DEGREE_CELSIUS, 17.0F, 25.0F, TEMPERATURE_PERIOD_MS, 0U),
    m_humidityChannel(ISensorChannel::TYPE_HUMIDITY_PERCENT, 35.0F, 65.0F, HUMIDITY_PERIOD_MS, 0U),
    m_illuminanceChannel(ISensorChannel::TYPE_ILLUMINANCE_LUX, 5.0F, 800.0F, ILLUMINANCE_PERIOD_MS, 0U),
    m_stateOfChargeChannel(ISensorChannel::TYPE_STATE_OF_CHARGE_PERCENT, getStateOfCharge),
    m_freeHeapChannel(ISensorChannel::TYPE_FREE_HEAP_BYTES, getFreeHeap),
    m_minFreeHeapChannel(ISensorChannel::TYPE_MIN_FREE_HEAP_BYTES, getMinFreeHeap),
    m_maxAllocHeapChannel(ISensorChannel::TYPE_MAX_ALLOC_HEAP_BYTES, getMaxAllocHeap),
    m_signalStrengthChannel(ISensorChannel::TYPE_SIGNAL_STRENGTH_DBM, -80, -40, SIGNAL_STRENGTH_PERIOD_MS, 0U),
    m_uptimeChannel(ISensorChannel::TYPE_UPTIME_S, getUptime)
{
}

ISensorChannel* SensorSim::getChannel(uint8_t index)
{
    ISensorChannel* channel = nullptr;

    switch (index)
    {
    case 0U:
        channel = &m_temperatureChannel;
        break;

    case 1U:
        channel = &m_humidityChannel;
        break;

    case 2U:
        channel = &m_illuminanceChannel;
        break;

    case 3U:
        channel = &m_stateOfChargeChannel;
        break;

    case 4U:
        channel = &m_freeHeapChannel;
        break;

    case 5U:
        channel = &m_minFreeHeapChannel;
        break;

    case 6U:
        channel = &m_maxAllocHeapChannel;
        break;

    case 7U:
        channel = &m_signalStrengthChannel;
        break;

    case 8U:
        channel = &m_uptimeChannel;
        break;

    default:
        break;
    }

    return channel;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/**
 * Get the current position inside a triangular wave.
 *
 * The result rises from 0.0 to 1.0 during the first half of the period and
 * falls back to 0.0 during the second half. Note, the system tick overflows
 * after about 49 days, which causes a single jump of the simulated value.
 *
 * @param[in] periodMs  Duration of one complete cycle in ms.
 * @param[in] phaseMs   Shift of the cycle start in ms.
 *
 * @return Position in the range [0.0; 1.0].
 */
static float getSimulationRatio(uint32_t periodMs, uint32_t phaseMs)
{
    uint32_t halfPeriodMs = periodMs / 2U;
    float    ratio        = 0.0F;

    if (0U == halfPeriodMs)
    {
        /* Guard path: a period shorter than 2 ms has no slope at all. */
    }
    else
    {
        uint32_t position = (millis() + phaseMs) % periodMs;

        if (position < halfPeriodMs)
        {
            ratio = static_cast<float>(position) / static_cast<float>(halfPeriodMs);
        }
        else
        {
            ratio = static_cast<float>(periodMs - position) / static_cast<float>(halfPeriodMs);
        }
    }

    return ratio;
}

/**
 * Get the simulated battery state of charge.
 *
 * @return State of charge in [%].
 */
static uint32_t getStateOfCharge()
{
    float    ratio = getSimulationRatio(STATE_OF_CHARGE_PERIOD_MS, 0U);
    uint32_t span  = STATE_OF_CHARGE_MAX - STATE_OF_CHARGE_MIN;

    return STATE_OF_CHARGE_MIN + static_cast<uint32_t>(static_cast<float>(span) * ratio);
}

/**
 * Get the free heap size of the host.
 *
 * @return Free heap size in [byte].
 */
static uint32_t getFreeHeap()
{
    return static_cast<uint32_t>(MemUtil::getFreeHeapSize());
}

/**
 * Get the min. free heap size of the host.
 *
 * @return Min. free heap size in [byte].
 */
static uint32_t getMinFreeHeap()
{
    return static_cast<uint32_t>(MemUtil::getMinFreeHeapSize());
}

/**
 * Get the max. allocatable heap block size of the host.
 *
 * @return Max. allocatable heap block size in [byte].
 */
static uint32_t getMaxAllocHeap()
{
    return static_cast<uint32_t>(MemUtil::getLargestFreeBlockSize());
}

/**
 * Get the uptime of the host.
 *
 * @return Uptime in [s].
 */
static uint64_t getUptime()
{
    return static_cast<uint64_t>(millis()) / 1000ULL;
}
