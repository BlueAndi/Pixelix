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
 * @file   IBoard.h
 * @brief  Abstract board interface
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup HAL
 *
 * @{
 */

#ifndef IBOARD_H
#define IBOARD_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "IButtonDrv.h"
#include "IBuzzerDrv.h"
#include "ILedDrv.h"
#include "ISystemDrv.h"
#include "IDisplayDrv.h"

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
 * Abstract board interface.
 */
class IBoard
{
public:

    /**
     * Destroys the board interface.
     */
    virtual ~IBoard()
    {
    }

    /**
     * Initialize the board.
     *
     * @return If successful initialized it will return true otherwise false.
     */
    virtual bool init()                  = 0;

    /**
     * Get the button driver.
     *
     * @return Button driver
     */
    virtual IButtonDrv& getButtonDrv()   = 0;

    /**
     * Get the buzzer driver.
     *
     * @return Buzzer driver
     */
    virtual IBuzzerDrv& getBuzzerDrv()   = 0;

    /**
     * Get the onboard LED driver.
     *
     * @return LED driver
     */
    virtual ILedDrv& getLedDrv()         = 0;

    /**
     * Get the system driver.
     *
     * @return System driver
     */
    virtual ISystemDrv& getSystemDrv()   = 0;

    /**
     * Get the display driver.
     *
     * @return Display driver
     */
    virtual IDisplayDrv& getDisplayDrv() = 0;

protected:

    /**
     * Constructs the board interface.
     */
    IBoard()
    {
    }
};

/******************************************************************************
 * Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* IBOARD_H */

/** @} */