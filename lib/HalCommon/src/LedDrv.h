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
 * @file   LedDrv.h
 * @brief  LED driver
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup HAL
 *
 * @{
 */

#ifndef LEDDRV_H
#define LEDDRV_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <ILedDrv.h>
#include <Io.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * LED driver.
 */
class LedDrv : public ILedDrv
{
public:

    /**
     * Construct LedDrv.
     */
    LedDrv() :
        m_ledOut(nullptr)
    {
    }

    /**
     * Destroys LedDrv.
     */
    virtual ~LedDrv()
    {
    }

    /**
     * Initialize the driver.
     *
     * @param[in] ledOut  Digital output for LED.
     */
    void init(const DOutPin& ledOut) override
    {
        m_ledOut = &ledOut;
    }

    /**
     * Switch LED off.
     */
    void off() override;

    /**
     * Switch LED on.
     */
    void on() override;

    /**
     * Is the LED on?
     *
     * @return If LED is on, it will return true otherwise false.
     */
    bool isOn() override;

private:

    const DOutPin* m_ledOut; /**< Digital output for LED */

    /* Prevent copying */
    LedDrv(const LedDrv&);
    LedDrv& operator=(const LedDrv&);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* LEDDRV_H */

/** @} */