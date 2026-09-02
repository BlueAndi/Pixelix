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
 * @file   IButtonDrv.h
 * @brief  Abstract button driver interface
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup HAL
 *
 * @{
 */

#ifndef IBUTTONDRV_H
#define IBUTTONDRV_H

/******************************************************************************
 * Includes
 *****************************************************************************/
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
    BUTTON_ID_OK = 0, /**< Button "ok" */
    BUTTON_ID_LEFT,   /**< Button "left "*/
    BUTTON_ID_RIGHT,  /**< Button "right" */
    BUTTON_ID_CNT     /**< Number of buttons */

} ButtonId;

/**
 * Button states
 */
typedef enum
{
    BUTTON_STATE_NC = 0,   /**< Button is not connected. */
    BUTTON_STATE_UNKNOWN,  /**< Button state is unknown yet. */
    BUTTON_STATE_RELEASED, /**< Button is released. */
    BUTTON_STATE_PRESSED   /**< Button is pressed. */

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
 * Abstract button driver interface.
 */
class IButtonDrv
{
public:

    /**
     * Destroys the button driver instance.
     */
    virtual ~IButtonDrv()
    {
    }

    /**
     * Initialize the driver.
     *
     * @param[in] buttonOkIn    Digital input for button "ok".
     * @param[in] buttonLeftIn  Digital input for button "left".
     * @param[in] buttonRightIn Digital input for button "right".
     *
     * @return If successful initialized it will return true otherwise false.
     */
    virtual bool init(const DInPin& buttonOkIn, const DInPin& buttonLeftIn, const DInPin& buttonRightIn) = 0;

    /**
     * Get button state.
     *
     * @param[in] buttonId  The id of the related button.
     *
     * @return Button state
     */
    virtual ButtonState getState(ButtonId buttonId)                                                      = 0;

    /**
     * Register an observer to get notifyed about button
     * state changes. Only one observer is supported!
     *
     * @param[in] observer  The button observer
     */
    virtual void registerObserver(IButtonObserver& observer)                                             = 0;

    /**
     * Unregister the current observer.
     */
    virtual void unregisterObserver()                                                                    = 0;

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
    virtual bool enableWakeUpSources()                                                                   = 0;

protected:

    /**
     * Constructs the button driver instance.
     */
    IButtonDrv()
    {
    }
};

/******************************************************************************
 * Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* IBUTTONDRV_H */

/** @} */