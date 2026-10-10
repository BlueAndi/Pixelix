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
 * @file   ButtonDrv.h
 * @brief  Button driver
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup HAL
 *
 * @{
 */

#ifndef BUTTONDRV_H
#define BUTTONDRV_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <IButtonDrv.h>
#include <SimpleTimer.hpp>
#include <Io.hpp>
#include <Task.hpp>

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
 * Button driver.
 */
class ButtonDrv : public IButtonDrv
{
public:

    /**
     * Constructs the button driver instance.
     */
    ButtonDrv();

    /**
     * Destroys the button driver instance.
     */
    virtual ~ButtonDrv();

    /**
     * Initialize the driver.
     *
     * @param[in] buttonOkIn    Digital input for button "ok".
     * @param[in] buttonLeftIn  Digital input for button "left".
     * @param[in] buttonRightIn Digital input for button "right".
     *
     * @return If successful initialized it will return true otherwise false.
     */
    bool init(const DInPin& buttonOkIn, const DInPin& buttonLeftIn, const DInPin& buttonRightIn) override;

    /**
     * Get button state.
     *
     * @param[in] buttonId  The id of the related button.
     *
     * @return Button state
     */
    ButtonState getState(ButtonId buttonId) override;

    /**
     * Register an observer to get notifyed about button
     * state changes. Only one observer is supported!
     *
     * @param[in] observer  The button observer
     */
    void registerObserver(IButtonObserver& observer) override;

    /**
     * Unregister the current observer.
     */
    void unregisterObserver() override;

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
    bool enableWakeUpSources() override;

private:

    /**
     * Debouncing time in ms.
     */
    static const uint32_t DEBOUNCING_TIME = 100U;

    Task<ButtonDrv>       m_buttonTask;               /**< Button task. */
    SemaphoreHandle_t     m_xSemaphore;               /**< Semaphore to protect button state member. */
    ButtonState           m_state[BUTTON_ID_CNT];     /**< Current button states */
    SimpleTimer           m_timer[BUTTON_ID_CNT];     /**< Timer used for debouncing */
    IButtonObserver*      m_observer;                 /**< Observer for button state changes */
    const DInPin*         m_buttonPin[BUTTON_ID_CNT]; /**< Digital input buttons */

    /** Button task stack size in bytes. */
    static const uint32_t BUTTON_TASK_STACKE_SIZE = 1536U;

    /** Button task priority. */
    static const UBaseType_t BUTTON_TASK_PRIORITY = 1U;

    /** MCU core where the button task shall run. */
    static const BaseType_t BUTTON_TASK_RUN_CORE  = APP_CPU_NUM;

    /** Task period in ms. */
    static const uint32_t BUTTON_TASK_PERIOD      = 10U;

    /** Button debouncing time in ms. */
    static const uint32_t BUTTON_DEBOUNCE_TIME    = 100U;

    /* Not allowed to be copied. */
    ButtonDrv(const ButtonDrv& drv);
    ButtonDrv& operator=(const ButtonDrv& drv);

    /**
     * Attach buttons to the interrupt service routine (ISR).
     */
    void attachButtonsToInterrupt();

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
     * @param[in] self Button driver instance.
     */
    static void buttonTask(ButtonDrv* self);

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

#endif /* BUTTONDRV_H */

/** @} */