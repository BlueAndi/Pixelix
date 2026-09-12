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
 * @brief  Button driver for the native environment
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * There are no buttons on the host. The buttons can be triggered by the
 * webinterface instead, see the websocket command BUTTON.
 *
 * @addtogroup TEST
 *
 * @{
 */

#ifndef BUTTON_DRV_H
#define BUTTON_DRV_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <IButtonDrv.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Button driver for the native environment.
 */
class ButtonDrv : public IButtonDrv
{
public:

    /**
     * Constructs the button driver.
     */
    ButtonDrv() :
        IButtonDrv(),
        m_observer(nullptr)
    {
    }

    /**
     * Destroys the button driver.
     */
    virtual ~ButtonDrv()
    {
    }

    /**
     * Initialize the button driver.
     *
     * @param[in] buttonOkIn    Pin of the "ok" button.
     * @param[in] buttonLeftIn  Pin of the "left" button.
     * @param[in] buttonRightIn Pin of the "right" button.
     *
     * @return Always true, there are no buttons on the host.
     */
    bool init(const DInPin& buttonOkIn, const DInPin& buttonLeftIn, const DInPin& buttonRightIn) final
    {
        (void)buttonOkIn;
        (void)buttonLeftIn;
        (void)buttonRightIn;

        return true;
    }

    /**
     * Get the state of the given button.
     *
     * @param[in] buttonId  Id of the button.
     *
     * @return Always released, there are no buttons on the host.
     */
    ButtonState getState(ButtonId buttonId) final
    {
        (void)buttonId;

        return BUTTON_STATE_RELEASED;
    }

    /**
     * Register an observer, which is notified about button state changes.
     *
     * @param[in] observer  The observer.
     */
    void registerObserver(IButtonObserver& observer) final
    {
        m_observer = &observer;
    }

    /**
     * Unregister the observer.
     */
    void unregisterObserver() final
    {
        m_observer = nullptr;
    }

    /**
     * Enable the wake up sources, used to wake up from deep sleep.
     *
     * @return Always false, there is no deep sleep on the host.
     */
    bool enableWakeUpSources() final
    {
        return false;
    }

private:

    IButtonObserver* m_observer; /**< Observer, which is notified about button state changes. */

    ButtonDrv(const ButtonDrv& drv);
    ButtonDrv& operator=(const ButtonDrv& drv);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* BUTTON_DRV_H */

/** @} */
