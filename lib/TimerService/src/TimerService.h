/* MIT License
 *
 * Copyright (c) 2019 - 2024 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Timer service
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup TIMER_SERVICE
 *
 * @{
 */

#ifndef TIMER_SERVICE_H
#define TIMER_SERVICE_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <IService.hpp>
#include <Mutex.hpp>
#include <SimpleTimer.hpp>

#include "TimerSetting.h"

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
 * The timer service provides a timer functionality to switch on/off the
 * display and set the brightness level on specific times.
 */
class TimerService : public IService
{
public:

    /**
     * Get the timer service instance.
     *
     * @return Timer service instance
     */
    static TimerService& getInstance()
    {
        static TimerService instance; /* idiom */

        return instance;
    }

    /**
     * Start the timer service.
     */
    bool start() final;

    /**
     * Stop the timer service.
     */
    void stop() final;

    /**
     * Process the service.
     */
    void process() final;

private:

    static const uint32_t PROCESS_PERIOD  = 100U; /**< Process period in ms. */
    static const uint8_t  MAX_TIMER_COUNT = 8U;   /**< Maximum number of timer. */
    static const char*    FILE_NAME;              /**< File name of the timer settings. */
    static const char*    TOPIC;                  /**< Topic for timer settings. */
    static const char*    ENTITY;                 /**< Entity for timer settings. */

    String                m_deviceId;                  /**< Device id. */
    TimerSetting          m_settings[MAX_TIMER_COUNT]; /**< Timer settings. */
    bool                  m_hasSettingsChanged;        /**< Has any timer setting changed since last request? */
    Mutex                 m_mutex;                     /**< Mutex to protect the settings. */
    SimpleTimer           m_processTimer;              /**< Process timer */

    TimerService(const TimerService& drv);
    TimerService& operator=(const TimerService& drv);

    /**
     * Constructs the timer service instance.
     */
    TimerService() :
        IService(),
        m_deviceId(),
        m_settings(),
        m_hasSettingsChanged(true),
        m_mutex(),
        m_processTimer()
    {
    }

    /**
     * Destroys the timer service instance.
     */
    ~TimerService()
    {
        /* Never called. */
    }

    /**
     * Clear all timer settings.
     */
    void clear();

    /**
     * Load timer settings from file.
     *
     * @return If successful, it will return true otherwise false.
     */
    bool loadSettings();

    /**
     * Save timer settings to file.
     *
     * @return If successful, it will return true otherwise false.
     */
    bool saveSettings();

    /**
     * Get timer settings.
     *
     * @param[in]       topic       The topic name.
     * @param[in,out]   jsonValue   The JSON value.
     *
     * @return If successful, it will return true otherwise false.
     */
    bool getTopic(const String& topic, JsonObject& jsonValue);

    /**
     * Has any timer setting changed since last request?
     *
     * @param[in] topic The topic name.
     *
     * @return If changed, it will return true otherwise false.
     */
    bool hasTopicChanged(const String& topic);

    /**
     * Set timer settings.
     *
     * @param[in] topic The topic name.
     * @param[in] value The JSON value.
     *
     * @return If successful, it will return true otherwise false.
     */
    bool setTopic(const String& topic, const JsonObjectConst& value);
};

/******************************************************************************
 * Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* TIMER_SERVICE_H */

/** @} */