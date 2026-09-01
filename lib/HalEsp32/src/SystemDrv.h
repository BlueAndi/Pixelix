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
 * @file   SystemDrv.h
 * @brief  System driver
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup HAL
 *
 * @{
 */

#ifndef SYSTEMDRV_H
#define SYSTEMDRV_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <ISystemDrv.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * System driver.
 */
class SystemDrv : public ISystemDrv
{
public:

    /**
     * Constructs the system driver instance.
     */
    SystemDrv()
    {
    }

    /**
     * Destroys the system driver instance.
     */
    virtual ~SystemDrv()
    {
    }

    /**
     * Initialize the driver.
     */
    void init() override
    {
        /* Nothing to do */
    };

    /**
     * Reset the system.
     */
    void reset() override
    {
        /* Reset the system */
        ESP.restart();
    };

private:

    /* An instance shall not be copied. */
    SystemDrv(const SystemDrv& systemDrv);

    /* An instance shall not be copied. */
    SystemDrv& operator=(const SystemDrv& systemDrv);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* SYSTEMDRV_H */

/** @} */