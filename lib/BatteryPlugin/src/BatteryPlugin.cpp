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
 * @brief  Shows current battery state of charge.
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "BatteryPlugin.h"
#include "FileSystem.h"
#include "SensorDataProvider.h"

#include <Board.h>
#include <SensorChannelType.hpp>
#include <YAColor.h>
#include <Logging.h>

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

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void BatteryPlugin::start(uint16_t width, uint16_t height)
{
    uint8_t                     sensorIdx           = 0U;
    uint8_t                     channelIdx          = 0U;
    uint16_t                    spaceLeftRight      = divideAndRound(width, 8U); /* 12.5 % */
    uint16_t                    spaceTopBottom      = divideAndRound(height, 8U); /* 12.5 % */
    uint16_t                    batteryBorder       = 1U;
    uint16_t                    batteryPoleWidth    = divideAndRound(width, 20U); /*  5 % */
    uint16_t                    batteryPoleHeight   = height - 2U * (spaceTopBottom + 2U * batteryBorder);
    uint16_t                    batteryWdith        = width - 2U * spaceLeftRight - batteryPoleWidth;
    uint16_t                    batteryHeight       = height - 2U * spaceTopBottom;
    MutexGuard<MutexRecursive>  guard(m_mutex);
    SensorDataProvider&         sensorDataProv      = SensorDataProvider::getInstance();

    m_batterySymbol.create(width, height);
    m_batterySymbol.drawRectangle(spaceLeftRight + batteryPoleWidth, spaceTopBottom, batteryWdith, batteryHeight, ColorDef::WHITE);
    m_batterySymbol.fillRect(spaceLeftRight, spaceTopBottom + 2U * batteryBorder, batteryPoleWidth, batteryPoleHeight, ColorDef::WHITE);

    m_socBarX       = spaceLeftRight + batteryPoleWidth + batteryBorder;
    m_socBarY       = spaceTopBottom + batteryBorder;
    m_socBarWidth   = batteryWdith - 2U * batteryBorder;
    m_socBarHeight  = batteryHeight - 2U * batteryBorder;

    /* Use just the first found sensor for battery state of charge. */
    if (true == sensorDataProv.find(sensorIdx, channelIdx, ISensorChannel::TYPE_STATE_OF_CHARGE_PERCENT, ISensorChannel::DATA_TYPE_UINT32))
    {
        m_socSensorCh = sensorDataProv.getSensor(sensorIdx)->getChannel(channelIdx);
    }
}

void BatteryPlugin::stop()
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_batterySymbol.release();
    m_sensorUpdateTimer.stop();
}

void BatteryPlugin::process(bool isConnected) 
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    PLUGIN_NOT_USED(isConnected);

    /* Read only if update period not reached or sensor has never been read. */
    if ((false == m_sensorUpdateTimer.isTimerRunning()) ||
        (true == m_sensorUpdateTimer.isTimeout()))
    {
        if (nullptr != m_socSensorCh)
        {
            if (ISensorChannel::DATA_TYPE_UINT32 == m_socSensorCh->getDataType())
            {
                SensorChannelUInt32* channel = static_cast<SensorChannelUInt32*>(m_socSensorCh);

                m_stateOfCharge = channel->getValue();
            }
        }

        m_sensorUpdateTimer.start(SENSOR_UPDATE_PERIOD);
    }
}

void BatteryPlugin::update(YAGfx& gfx)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    gfx.drawBitmap(0, 0, m_batterySymbol);
    drawStateOfCharge(gfx);
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

uint16_t BatteryPlugin::divideAndRound(uint16_t dividend, uint16_t divisor)
{
    uint32_t rest   = (static_cast<uint32_t>(dividend) * 10U / static_cast<uint32_t>(divisor)) % 10U;
    uint16_t result = dividend / divisor;

    if (5U <= rest)
    {
        result += 1U;
    }

    return result;
}

void BatteryPlugin::drawStateOfCharge(YAGfx& gfx)
{
    uint16_t    widthDependedOnSOC  = (m_socBarWidth * m_stateOfCharge) / 100U;
    int16_t     barXDependedOnSOC   = m_socBarX + m_socBarWidth - widthDependedOnSOC;
    Color       color;

    /* SOC > 70 % */
    if (70U < m_stateOfCharge)
    {
        color = ColorDef::LIGHTGREEN;
    }
    /* SOC > 40 % */
    else if (40U < m_stateOfCharge)
    {
        color = ColorDef::GREEN;
    }
    /* SOC > 10 % */
    else if (10U > m_stateOfCharge)
    {
        color = ColorDef::ORANGE;
    }
    /* SOC <= 10 % */
    else
    {
        color = ColorDef::RED;
    }

    gfx.fillRect(barXDependedOnSOC, m_socBarY, widthDependedOnSOC, m_socBarHeight, color);
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
