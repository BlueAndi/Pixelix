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
 *
 * @addtogroup gfx
 *
 * @{
 */

#ifndef BRIGHTNESS_CTRL_H
#define BRIGHTNESS_CTRL_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <SimpleTimer.hpp>
#include <IDisplay.hpp>
#include <SensorChannelType.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Recursive average IIR filter.
 * 
 * y[n] = ax[n] + (1-a)y[n-1]
 * 
 * n: Sample number
 * x[n]: Input value
 * y[n]: Output value
 * y[n-1]: Last output value
 * a: Defines how deep or sharp the filter is [0;1].
 * 
 * => y[n] = y[n-1] + a * (x[n] - y[n-1])
 * 
 * The weight for a depends on the filter time constant and the delta time
 * for each input value.
 * 
 * a = dT / (b + dT)
 * 
 * dT: Delta time (duration between current input value and last one)
 * b: Filter time constant
 * 
 * @tparam T    The type of the filter values.
 */
template <typename T>
class RecursiveAverageIIR
{
public:

    /**
     * Create the filter, based on given information.
     * 
     * @param[in] filterTimeConstant    The filter time constant.
     * @param[in] startValue            Defines the start value for the internal filter memory.
     */
    RecursiveAverageIIR(uint32_t filterTimeConstant, T startValue) :
        m_filterTimeConstant(filterTimeConstant),
        m_value(startValue)
    {
    }

    /**
     * Destroy the filter.
     */
    ~RecursiveAverageIIR()
    {
    }

    /**
     * Calculate new filter output value, based on given input value.
     * 
     * @param[in] input     Input value
     * @param[in] deltaTime Time difference from current input value to last given input value.
     * 
     * @return Filter output value
     */
    T calc(T input, uint32_t deltaTime)
    {
        m_value += (input - m_value) * deltaTime / (m_filterTimeConstant + deltaTime);

        return m_value;
    }

    /**
     * Set the start value of the filter or set the current filter output value.
     * 
     * @param[in] startValue    The new filter start value.
     */
    void setStartValue(T startValue)
    {
        m_value = startValue;
    }

    /**
     * Get current filter output value.
     * 
     * @return Filter output value
     */
    T getValue() const
    {
        return m_value;
    }

private:

    uint32_t    m_filterTimeConstant;   /** Filter time constant. */
    T           m_value;                /** Current filter output value. */

    RecursiveAverageIIR();
};

/**
 * The brightness controller sets the display brightness depended on the
 * ambient light.
 */
class BrightnessCtrl
{
public:

    /**
     * Get brightness controller instance.
     *
     * @return Brightness controller instance
     */
    static BrightnessCtrl& getInstance()
    {
        static BrightnessCtrl instance; /* singleton idiom to force initialization in the first usage. */

        return instance;
    }

    /**
     * Initialize brightness controller and set initial display brightness.
     * 
     * @param[in] display       The display, which to control.
     * @param[in] minBrightness The minimal brightness in digits [0; 255].
     * @param[in] maxBrightness The maximal brightness in digits [0; 255].
     */
    void init(IDisplay& display, uint8_t minBrightness, uint8_t maxBrightness);

    /**
     * Enable/Disable automatic brightness adjustment.
     * Note, it will fail if no ambient light sensor is available.
     *
     * @param[in] state  Enable (true) or disable (false) it.
     *
     * @return If successful, it will return true otherwise false.
     */
    bool enable(bool state);

    /**
     * Is automatic brightness adjustment enabled?
     *
     * @return If enabled, it will return true otherwise false.
     */
    bool isEnabled() const;

    /**
     * Process brightness controller.
     */
    void process();

    /**
     * Set display brightness level in digits.
     *
     * @param[in] level Brightness level in digits [0; 255].
     */
    void setBrightness(uint8_t level);

    /**
     * Get brightness level in digits.
     *
     * @return Brightness level in digits [0; 255].
     */
    uint8_t getBrightness() const
    {
        return m_brightness;
    }

    /**
     * IIR filter time constant in ms for calculating the short-term moving average
     * of the light samples. Used for low latency measurement.
     */
    static const uint32_t   SHORT_TERM_AVG_LIGHT_TIME_CONST = 1000U;

    /**
     * IIR filter time constant in ms for calculating the long-term moving average
     * of the light samples. Used for the trend analysis.
     */
    static const uint32_t   LONG_TERM_AVG_LIGHT_TIME_CONST  = 5000U;

    /** Default period for automatic brightness adjustment in ms. */
    static const uint32_t   AUTO_ADJUST_PERIOD              = 250U;

    /**
     * Stability requirement in ms for accepting a new brightness level.
     * It controls how quickly a brightness change occurs in response
     * to an observed change in light level that exceeds the hysteresis
     * threshold.
     */
    static const uint32_t   BRIGHTENING_LIGHT_DEBOUNCE      = 4000U;

    /**
     * Stability requirement in ms for accepting a new brightness level.
     * It controls how quickly a brightness change occurs in response
     * to an observed change in light level that exceeds the hysteresis
     * threshold.
     */
    static const uint32_t   DARKENING_LIGHT_DEBOUNCE        = 8000U;

    /**
     * Hysteresis constraint for brightening in percent [0.0; 1.0].
     * The recent measured light must have changed at least this fraction
     * relative to the current ambient light before a change will be
     * considered.
     */
    static constexpr float  BRIGHTENING_LIGHT_HYSTERESIS    = 0.1F;

    /**
     * Hysteresis constraint for darkening in percent [0.0; 1.0].
     * The recent measured light must have changed at least this fraction
     * relative to the current ambient light before a change will be
     * considered.
     */
    static constexpr float  DARKENING_LIGHT_HYSTERESIS      = 0.2F;

private:

    /** Direction of ambient light changes. */
    enum AmbientLightDirection
    {
        AMBIENT_LIGHT_DIRECTION_BRIGHTER = 0,   /**< Its getting brighter. */
        AMBIENT_LIGHT_DIRECTION_DARKER          /**< Its getting darker. */
    };

    /** The interface for the display, where to control the brightness. */
    IDisplay*                   m_display;

    /** Channel where to get current illuminance values. */
    SensorChannelFloat32*       m_illuminanceChannel;

    /** Timer, used for automatic brightness adjustment. */
    SimpleTimer                 m_autoBrightnessTimer;

    /** Display brightness in digits [0; 255]. */
    uint8_t                     m_brightness;

    /** Min. brightness level in digits [0; 255]. */
    uint8_t                     m_minBrightness;

    /** Max. brightness level in digits [0; 255]. */
    uint8_t                     m_maxBrightness;

    /** Short-term moving average of light (normalized) [0.0; 1.0]. */
    RecursiveAverageIIR<float>  m_recentShortTermAverage;

    /** Long-term moving average of light (normalized) [0.0; 1.0]. */
    RecursiveAverageIIR<float>  m_recentLongTermAverage;

    /**
     * Brightening threshold (normalized) [0.0; 1.0]. The ambient light value
     * must be greater than this threshold to consider it.
     */
    float                       m_brighteningThreshold;

    /**
     * Darkening threshold (normalized) [0.0; 1.0]. The ambient light value
     * must be lower than this threshold to consider it.
     */
    float                       m_darkeningThreshold;

    /**
     * Ambient light (normalized) [0.0; 1.0].
     */
    float                       m_ambientLight;

    /**
     * Light sensor debounce timestamp value.
     */
    SimpleTimer                 m_lightSensorDebounceTimer;

    /**
     * Direction of changing ambient light.
     */
    AmbientLightDirection       m_direction;

    /**
     * Brightness goal in digits [0; 255].
     */
    uint8_t                     m_brightnessGoal;

    /**
     * Constructs a brightness controller instance.
     */
    BrightnessCtrl();

    /**
     * Destroys the brightness controller instance.
     */
    ~BrightnessCtrl();

    BrightnessCtrl(const BrightnessCtrl& ctrl);
    BrightnessCtrl& operator=(const BrightnessCtrl& ctrl);

    /**
     * Get current normalized light value.
     * 
     * @return Normalized light
     */
    float getNormalizedLight();

    /**
     * Set ambient light, which will be used to determine the display brightness.
     * It will update the brightening and darkening thresholds.
     *
     * @param[in] light Ambient light (normalized) [0.0; 1.0]
     */
    void setAmbientLight(float light);

    /**
     * Apply the measured light sensor value and calculate the short-term and
     * long-term moving average.
     *
     * @param[in] dTime Delta time in ms, between current and last measurement.
     * @param[in] light Measured light (normalized) [0.0; 1.0]
     */
    void applyLightSensorMeasurement(uint32_t dTime, float light);

    /**
     * Update the display brightness goal. This doesn't changes the display
     * brightness directly, but it sets the destination which the display
     * brightness shall reach.
     */
    void updateBrightnessGoal();

    /**
     * Update the display brightness.
     */
    void updateBrightness();
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* BRIGHTNESS_CTRL_H */

/** @} */