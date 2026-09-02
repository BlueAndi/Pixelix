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
 * @file   Board.h
 * @brief  Board abstraction
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup HAL
 *
 * @{
 */

#ifndef BOARD_H
#define BOARD_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <IBoard.h>
#include <ButtonDrv.h>
#include <BuzzerDrv.h>
#include <LedDrv.h>
#include <SystemDrv.h>
#include <DisplayDrv.h>
#include "Pin.h"

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
 * The board abstraction provides access to the hardware of the electronic board.
 */
class Board : public IBoard
{
public:

    /**
     * Get the singleton instance of the board abstraction.
     *
     * @return Board instance
     */
    static Board& getInstance()
    {
        static Board instance; /* idiom */

        return instance;
    }

    /**
     * Initialize the board.
     *
     * @return If successful initialized it will return true otherwise false.
     */
    bool init() override;

    /**
     * Get the button driver.
     *
     * @return Button driver
     */
    IButtonDrv& getButtonDrv() override
    {
        return m_buttonDrv;
    }

    /**
     * Get the buzzer driver.
     *
     * @return Buzzer driver
     */
    IBuzzerDrv& getBuzzerDrv() override
    {
        return m_buzzerDrv;
    }

    /**
     * Get the onboard LED driver.
     *
     * @return LED driver
     */
    ILedDrv& getLedDrv() override
    {
        return m_ledDrv;
    }

    /**
     * Get the system driver.
     *
     * @return System driver
     */
    ISystemDrv& getSystemDrv() override
    {
        return m_systemDrv;
    }

    /**
     * Get the display driver.
     *
     * @return Display driver
     */
    IDisplayDrv& getDisplayDrv() override
    {
        return m_displayDrv;
    }

private:

    ButtonDrv  m_buttonDrv;  /**< Button driver */
    BuzzerDrv  m_buzzerDrv;  /**< Buzzer driver */
    LedDrv     m_ledDrv;     /**< Onboard LED driver */
    SystemDrv  m_systemDrv;  /**< System driver */
    DisplayDrv m_displayDrv; /**< Display driver */

    /**
     * Constructs the board interface.
     */
    Board() :
        m_buttonDrv(),
        m_buzzerDrv(),
        m_ledDrv(),
        m_systemDrv(),
        m_displayDrv()
    {
    }

    /**
     * Destroys the board interface.
     */
    virtual ~Board()
    {
    }
};

/******************************************************************************
 * Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* BOARD_H */

/** @} */