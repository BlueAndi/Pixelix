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
 * @brief  Button driver
 * @author Andreas Merkle <web@blue-andi.de>
 * 
 * @addtogroup hal
 *
 * @{
 */

#ifndef BUTTONDRV_H
#define BUTTONDRV_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Arduino.h"
#include <SimpleTimer.hpp>
#include <Io.hpp>

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
 * Button id
 */
typedef enum
{
    BUTTON_ID_OK = 0,   /**< Button "ok" */
    BUTTON_ID_LEFT,     /**< Button "left "*/
    BUTTON_ID_RIGHT,    /**< Button "right" */
    BUTTON_ID_CNT       /**< Number of buttons */

} ButtonId;

/**
 * Button states
 */
typedef enum
{
    BUTTON_STATE_NC = 0,    /**< Button is not connected. */
    BUTTON_STATE_UNKNOWN,   /**< Button state is unknown yet. */
    BUTTON_STATE_RELEASED,  /**< Button is released. */
    BUTTON_STATE_PRESSED    /**< Button is pressed. */

} ButtonState;

/**
 * Abstract interface for a button observer.
 */
class IButtonObserver
{
public:

    /**
     * Destroys the button observer interface.
     */
    virtual ~IButtonObserver()
    {
    }

    /**
     * Notify the observer about the new button state.
     * 
     * @param[in] buttonId  The id of the related button.
     * @param[in] state     New button state of the button.
     */
    virtual void notify(ButtonId buttonId, ButtonState state) = 0;

protected:

    /**
     * Creates the button observer interface.
     */
    IButtonObserver()
    {
    }

};

/**
 * Button driver.
 */
class ButtonDrv
{
public:

    /**
     * Get the button driver instance.
     * 
     * @return Button driver instance.
     */
    static ButtonDrv&   getInstance()
    {
        static ButtonDrv instance; /* singleton idiom to force initialization in the first usage. */

        return instance;
    }

    /**
     * Initialize the driver.
     * 
     * @return If successful initialized it will return true otherwise false.
     */
    bool init();

    /**
     * Get button state.
     * 
     * @param[in] buttonId  The id of the related button.
     * 
     * @return Button state
     */
    ButtonState getState(ButtonId buttonId);

    /**
     * Register an observer to get notifyed about button
     * state changes. Only one observer is supported!
     * 
     * @param[in] observer  The button observer
     */
    void registerObserver(IButtonObserver& observer);

    /**
     * Unregister the current observer.
     */
    void unregisterObserver();

    /**
     * Enable all buttons as wakeup sources.
     * A low level of the wakeup source will trigger the wakeup.
     * Ensure that all buttons are released at the time of calling it,
     * otherwise the wakeup will occurre immediately.
     * 
     * @return If not all buttons are released, it will return false and the
     *          wakeup sources are not enabled. Otherwise it will return true
     *          and the wakeup sources are enabled.
     */
    bool enableWakeUpSources();

private:

    /**
     * The digital input buttons.
     */
    static const IoPin*     BUTTON_PIN[BUTTON_ID_CNT];

    /**
     * Debouncing time in ms.
     */
    static const uint32_t   DEBOUNCING_TIME         = 100U;

    TaskHandle_t        m_buttonTaskHandle;     /**< Button task handle */
    ButtonState         m_state[BUTTON_ID_CNT]; /**< Current button states */
    SimpleTimer         m_timer[BUTTON_ID_CNT]; /**< Timer used for debouncing */
    SemaphoreHandle_t   m_xSemaphore;           /**< Semaphore lock */
    IButtonObserver*    m_observer;             /**< Observer for button state changes */

    /** Button task stack size in bytes */
    static const uint32_t   BUTTON_TASK_STACKE_SIZE = 2048U;

    /** MCU core where the button task shall run */
    static const BaseType_t BUTTON_TASK_RUN_CORE    = APP_CPU_NUM;

    /** Task period in ms */
    static const uint32_t   BUTTON_TASK_PERIOD      = 10U;

    /** Button debouncing time in ms */
    static const uint32_t   BUTTON_DEBOUNCE_TIME    = 100U;

    /**
     * Constructs the button driver instance.
     */
    ButtonDrv() :
        m_buttonTaskHandle(nullptr),
        m_state(),
        m_timer(),
        m_xSemaphore(nullptr),
        m_observer(nullptr)
    {
        uint8_t buttonIdx = 0U;

        while(BUTTON_ID_CNT > buttonIdx)
        {
            /* No pin connected? */
            if (IoPin::NC == BUTTON_PIN[buttonIdx]->getPinNo())
            {
                m_state[buttonIdx] = BUTTON_STATE_NC;
            }
            /* Interrupt can not be attached to pin? */
            else if (NOT_AN_INTERRUPT == digitalPinToInterrupt(BUTTON_PIN[buttonIdx]->getPinNo()))
            {
                m_state[buttonIdx] = BUTTON_STATE_NC;
            }
            /* Configured pin is ok. */
            else
            {
                m_state[buttonIdx] = BUTTON_STATE_UNKNOWN;
            }

            ++buttonIdx;
        }
    }

    /**
     * Destroys the button driver instance.
     */
    ~ButtonDrv()
    {
        /* Never called. */
    }

    /* Singleton */
    ButtonDrv(const ButtonDrv& drv);
    ButtonDrv& operator=(const ButtonDrv& drv);

    /**
     * Set button state.
     * 
     * @param[in] buttonId  The id of the button.
     * @param[in] state     The state of the button.
     */
    void setState(ButtonId buttonId, ButtonState state);

    /**
     * Button task is responsible for debouncing and updating the user button state
     * accordingly.
     * 
     * @param[in]   parameters  Task pParameters
     */
    static void buttonTask(void* parameters);

    /**
     * Button task main loop running in object context.
     */
    void buttonTaskMainLoop();
};

/******************************************************************************
 * Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* BUTTONDRV_H */

/** @} */