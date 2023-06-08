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
 * @brief  Brightness controller
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "BrightnessCtrl.h"
#include "AmbientLight.h"
#include "SensorDataProvider.h"

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

void BrightnessCtrl::init(IDisplay& display, uint8_t minBrightness, uint8_t maxBrightness)
{
    SensorDataProvider& sensorDataProv  = SensorDataProvider::getInstance();
    uint8_t             sensorIdx       = 0U;
    uint8_t             channelIdx      = 0U;
    
    m_display           = &display;
    m_minBrightness     = minBrightness;
    m_maxBrightness     = maxBrightness;
    m_brightnessGoal    = minBrightness;

    /* Find a sensor channel, which provides the current illuminance. */
    if (true == sensorDataProv.find(sensorIdx, channelIdx, ISensorChannel::TYPE_ILLUMINANCE_LUX, ISensorChannel::DATA_TYPE_FLOAT32))
    {
        ISensor*        sensor  = sensorDataProv.getSensor(sensorIdx);
        ISensorChannel* channel = (nullptr != sensor) ? sensor->getChannel(channelIdx) : nullptr;

        if (nullptr != channel)
        {
            m_illuminanceChannel = static_cast<SensorChannelFloat32*>(channel);
        }
    }

    if (nullptr != m_illuminanceChannel)
    {
        float lightNormalized = getNormalizedLight();

        m_recentShortTermAverage.setStartValue(lightNormalized);
        m_recentLongTermAverage.setStartValue(lightNormalized);

        setAmbientLight(m_recentShortTermAverage.getValue());
        updateBrightnessGoal();
    }
}

bool BrightnessCtrl::enable(bool state)
{
    bool status = true;

    /* Disable automatic brightness adjustment? */
    if (false == state)
    {
        m_autoBrightnessTimer.stop();
        m_lightSensorDebounceTimer.stop();
    }
    /* Enable automatic brightness adjustment */
    else
    {
        /* If no ambient light sensor is available, enable it makes no sense. */
        if (nullptr == m_illuminanceChannel)
        {
            status = false;
        }
        /* Ambient light sensor is available */
        else
        {
            float lightNormalized = getNormalizedLight();

            m_recentShortTermAverage.setStartValue(lightNormalized);
            m_recentLongTermAverage.setStartValue(lightNormalized);

            setAmbientLight(m_recentShortTermAverage.getValue());
            updateBrightnessGoal();

            /* Display brightness will be automatically adjusted in the process() method. */
            m_autoBrightnessTimer.start(AUTO_ADJUST_PERIOD);

            /* Start debouncing the ambient light sensor */
            if (AMBIENT_LIGHT_DIRECTION_BRIGHTER == m_direction)
            {
                m_lightSensorDebounceTimer.start(BRIGHTENING_LIGHT_DEBOUNCE);
            }
            else
            {
                m_lightSensorDebounceTimer.start(DARKENING_LIGHT_DEBOUNCE);
            }
        }
    }

    return status;
}

/**
 * Is automatic brightness adjustment enabled?
 *
 * @return If enabled, it will return true otherwise false.
 */
bool BrightnessCtrl::isEnabled() const
{
    bool isEnabled = false;

    if (m_autoBrightnessTimer.isTimerRunning())
    {
        isEnabled = true;
    }

    return isEnabled;
}

void BrightnessCtrl::process()
{
    /* Ambient light sensor available for automatic brightness adjustment? */
    if ((true == m_autoBrightnessTimer.isTimerRunning()) &&
        (true == m_autoBrightnessTimer.isTimeout()))
    {
        float lightNormalized = getNormalizedLight();

        applyLightSensorMeasurement(AUTO_ADJUST_PERIOD, lightNormalized);
        updateBrightness();

        /* The ambient environment appears to be brightening. */
        if ((m_brighteningThreshold < m_recentShortTermAverage.getValue()) &&
            (m_brighteningThreshold < m_recentLongTermAverage.getValue()))
        {
            if (AMBIENT_LIGHT_DIRECTION_BRIGHTER != m_direction)
            {
                m_direction = AMBIENT_LIGHT_DIRECTION_BRIGHTER;
                m_lightSensorDebounceTimer.start(BRIGHTENING_LIGHT_DEBOUNCE);

                LOG_INFO("Light: Its getting brighter.");
            }
            else if ((true == m_lightSensorDebounceTimer.isTimerRunning()) &&
                     (true == m_lightSensorDebounceTimer.isTimeout()))
            {
                setAmbientLight(m_recentShortTermAverage.getValue());
                updateBrightnessGoal();
            }
            else
            {
                ;
            }
        }
        /* The ambient environment appears to be darkening. */
        else if ((m_darkeningThreshold > m_recentShortTermAverage.getValue()) &&
                 (m_darkeningThreshold > m_recentLongTermAverage.getValue()))
        {
            if (AMBIENT_LIGHT_DIRECTION_DARKER != m_direction)
            {
                m_direction = AMBIENT_LIGHT_DIRECTION_DARKER;
                m_lightSensorDebounceTimer.start(DARKENING_LIGHT_DEBOUNCE);

                LOG_INFO("Light: Its getting darker.");
            }
            else if ((true == m_lightSensorDebounceTimer.isTimerRunning()) &&
                     (true == m_lightSensorDebounceTimer.isTimeout()))
            {
                setAmbientLight(m_recentShortTermAverage.getValue());
                updateBrightnessGoal();
            }
            else
            {
                ;
            }
        }

        m_autoBrightnessTimer.restart();
    }
}

void BrightnessCtrl::setBrightness(uint8_t level)
{
    if (false == isEnabled())
    {
        if (m_minBrightness > level)
        {
            m_brightness = m_minBrightness;
        }
        else
        {
            m_brightness = level;
        }

        if (nullptr != m_display)
        {
            m_display->setBrightness(m_brightness);
        }
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

BrightnessCtrl::BrightnessCtrl() :
    m_display(nullptr),
    m_illuminanceChannel(nullptr),
    m_autoBrightnessTimer(),
    m_brightness(0U),
    m_minBrightness(0U),
    m_maxBrightness(0U),
    m_recentShortTermAverage(SHORT_TERM_AVG_LIGHT_TIME_CONST, 0.0F),
    m_recentLongTermAverage(LONG_TERM_AVG_LIGHT_TIME_CONST, 0.0F),
    m_brighteningThreshold(0.0F),
    m_darkeningThreshold(0.0F),
    m_ambientLight(0.0F),
    m_lightSensorDebounceTimer(),
    m_direction(AMBIENT_LIGHT_DIRECTION_BRIGHTER),
    m_brightnessGoal(0U)
{
}

BrightnessCtrl::~BrightnessCtrl()
{
}

float BrightnessCtrl::getNormalizedLight()
{
    float lightNormalized   = 0.0F;

    if (nullptr != m_illuminanceChannel)
    {
        float illuminance = m_illuminanceChannel->getValue();
        
        lightNormalized = AmbientLight::normalizeIlluminance(illuminance);
    }

    return lightNormalized;
}

void BrightnessCtrl::setAmbientLight(float light)
{
    m_ambientLight          = light;
    m_brighteningThreshold  = m_ambientLight * (1.0F + BRIGHTENING_LIGHT_HYSTERESIS);
    m_darkeningThreshold    = m_ambientLight * (1.0F - DARKENING_LIGHT_HYSTERESIS);

    LOG_DEBUG("Light: %0.3f (b-thr %0.3f < x < d-thr %0.3f)", m_ambientLight, m_brighteningThreshold, m_darkeningThreshold);
}

void BrightnessCtrl::applyLightSensorMeasurement(uint32_t dTime, float light)
{
    (void)m_recentShortTermAverage.calc(light, dTime);
    (void)m_recentLongTermAverage.calc(light, dTime);

    /* LOG_DEBUG("Light: m %0.3f s-avg %0.3f l-avg %0.3f", light, m_recentShortTermAverage, m_recentLongTermAverage); */
}

void BrightnessCtrl::updateBrightnessGoal()
{
    float   fBrightnessDynamicRange = static_cast<float>(m_maxBrightness - m_minBrightness);
    float   fMinBrightness          = static_cast<float>(m_minBrightness);
    float   fBrightness             = fMinBrightness + (fBrightnessDynamicRange * m_ambientLight );

    m_brightnessGoal = static_cast<uint8_t>(fBrightness);

    LOG_DEBUG("Change brightness goal to %u.", m_brightnessGoal);
}

void BrightnessCtrl::updateBrightness()
{
    const uint8_t   STEP    = 2U;

    if (m_brightnessGoal > m_brightness)
    {
        if ((m_brightnessGoal - STEP) < m_brightness)
        {
            m_brightness = m_brightnessGoal;
        }
        else
        {
            m_brightness += STEP;
        }

        if (nullptr != m_display)
        {
            m_display->setBrightness(m_brightness);
        }
    }
    else if (m_brightnessGoal < m_brightness)
    {
        if ((m_brightnessGoal + STEP) > m_brightness)
        {
            m_brightness = m_brightnessGoal;
        }
        else
        {
            m_brightness -= STEP;
        }

        if (nullptr != m_display)
        {
            m_display->setBrightness(m_brightness);
        }
    }
    else
    {
        ;
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
