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
 * @brief  Timer setting
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup TIMER_SERVICE
 *
 * @{
 */

#ifndef TIMER_SETTING_H
#define TIMER_SETTING_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <IService.hpp>
#include <ArduinoJson.h>

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
 * Single timer setting.
 */
class TimerSetting
{
public:

    /** Generic display state which is request to be set. */
    enum DisplayState : uint8_t
    {
        DISPLAY_STATE_NONE = 0, /**< No action */
        DISPLAY_STATE_ON,       /**< Switch on */
        DISPLAY_STATE_OFF       /**< Switch off */
    };

    /**
     * Constructs a timer setting.
     */
    TimerSetting() :
        m_isEnabled(false),
        m_hour(0U),
        m_minute(0U),
        m_daysOfWeek(0U),
        m_displayState(DISPLAY_STATE_NONE),
        m_brightness(-1),
        m_isSignalling(false)
    {
    }

    /**
     * Destroys a timer setting.
     */
    ~TimerSetting()
    {
    }

    /**
     * Clear timer setting to default values.
     */
    void clear();

    /**
     * Convert setting to JSON.
     *
     * @param[out] jsonTimerSetting JSON object destination.
     */
    void toJson(JsonObject& jsonTimerSetting) const;

    /**
     * Convert from JSON to setting.
     *
     * @param[in] jsonTimerSetting  JSON object source.
     *
     * @return If successful, it will return true otherwise false.
     */
    bool fromJson(const JsonObjectConst& jsonTimerSetting);

    /**
     * Is timer enabled?
     *
     * @return If enabled, it will return true otherwise false.
     */
    bool isEnabled() const
    {
        return m_isEnabled;
    }

    /**
     * Is timer signalling?
     *
     * @param[in] currentTime   Current time.
     *
     * @return If signalling, it will return true otherwise false.
     */
    bool isSignalling(const struct tm& currentTime);

    /**
     * Get display state.
     *
     * @return Display state.
     */
    DisplayState getDisplayState() const
    {
        return m_displayState;
    }

    /**
     * Get brightness level.
     *
     * @return Brightness level ([0; 255], otherwise disabled).
     */
    int16_t getBrightness() const
    {
        return m_brightness;
    }

private:

    bool         m_isEnabled;    /**< Is timer enabled? */
    uint8_t      m_hour;         /**< Hour */
    uint8_t      m_minute;       /**< Minute */
    uint32_t     m_daysOfWeek;   /**< Days of week (bit 0: Su, bit 1: Mo and etc.) */
    DisplayState m_displayState; /**< Display state to set. */
    int16_t      m_brightness;   /**< Brightness level ([0; 255], otherwise disabled) to set. */
    bool         m_isSignalling; /**< Is timer signalling? Used to signal just once. */

    /**
     * Get the day of week.
     *
     * @param[in] dayOfWeek Day of week.
     *
     * @return If day is set, it will return true otherwise false.
     */
    bool isDayOfWeek(uint32_t dayOfWeek) const
    {
        return (0 != (m_daysOfWeek & (1U << dayOfWeek)));
    }
};

/******************************************************************************
 * Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* TIMER_SETTING_H */

/** @} */