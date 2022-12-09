/* MIT License
 *
 * Copyright (c) 2019 - 2022 Andreas Merkle <web@blue-andi.de>
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

#ifndef __BUTTONDRV_H__
#define __BUTTONDRV_H__

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Arduino.h"

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
 * Button states
 */
typedef enum
{
    BUTTON_STATE_UNKNOWN = 0,   /**< Button state is unknown yet */
    BUTTON_STATE_RELEASED,      /**< Button is released. */
    BUTTON_STATE_PRESSED        /**< Button is pressed. */

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
     * @param[in] state New button state
     */
    virtual void notify(ButtonState state) = 0;

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

    /** Status return values */
    enum Ret
    {
        RET_OK = 0, /**< Execution successful */
        RET_ERROR   /**< Execution failed */
    };

    /**
     * Initialize the driver.
     * 
     * @return Status
     */
    Ret init();

    /**
     * Get button state.
     * 
     * @return Button state
     */
    ButtonState getState();

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

private:

    TaskHandle_t        m_buttonTaskHandle; /**< Button task handle */
    ButtonState         m_state;            /**< Current button state */
    SemaphoreHandle_t   m_semaphore;        /**< Semaphore lock */
    IButtonObserver*    m_observer;         /**< Observer for button state changes */

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
        m_state(BUTTON_STATE_UNKNOWN),
        m_semaphore(nullptr),
        m_observer(nullptr)
    {
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
     * Button task is responsible for debouncing and updating the user button state
     * accordingly.
     * 
     * @param[in]   parameters  Task pParameters
     */
    static void buttonTask(void* parameters);
};

/******************************************************************************
 * Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __BUTTONDRV_H__ */

/** @} */