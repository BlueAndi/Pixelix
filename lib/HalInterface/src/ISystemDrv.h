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
 * @file   ISystemDrv.h
 * @brief  Abstract system driver interface
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup HAL
 *
 * @{
 */

#ifndef ISYSTEMDRV_H
#define ISYSTEMDRV_H

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <WString.h>

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
 * Abstract system driver interface.
 */
class ISystemDrv
{
public:

    /**
     * Destroys the system driver interface.
     */
    virtual ~ISystemDrv()
    {
    }

    /**
     * Initialize the driver.
     */
    virtual void init() = 0;

    /**
     * Reset the system.
     */
    virtual void reset() = 0;

    /**
     * Get the chip revision.
     *
     * @return Chip revision
     */
    virtual uint8_t getChipRevision() const = 0;

    /**
     * Get the CPU frequency.
     *
     * @return CPU frequency in MHz
     */
    virtual uint32_t getCpuFreqMHz() const = 0;

    /**
     * Get the flash chip read mode as a string.
     *
     * @return Flash chip read mode string
     */
    virtual const char* getFlashChipModeStr() const = 0;

    /**
     * Get the flash chip speed.
     *
     * @return Flash chip speed in Hz
     */
    virtual uint32_t getFlashChipSpeed() const = 0;

    /**
     * Get the flash chip size.
     *
     * @return Flash chip size in byte
     */
    virtual uint32_t getFlashChipSize() const = 0;

    /**
     * Get the SDK version.
     *
     * @return SDK version string
     */
    virtual const char* getSdkVersion() const = 0;

    /**
     * Get the LwIP version.
     *
     * @return LwIP version string
     */
    virtual const char* getLwIPVersion() const = 0;

    /**
     * Get the LittleFS version.
     *
     * @return LittleFS version string
     */
    virtual const char* getLittleFSVersion() const = 0;

    /**
     * Get the mbedTLS version.
     *
     * @return mbedTLS version string
     */
    virtual const char* getMbedTlsVersion() const = 0;

    /**
     * Get the factory programmed wifi MAC address.
     *
     * @param[out] macAddr  MAC address in "00:00:00:00:00:00" syntax.
     */
    virtual void getEFuseMAC(String& macAddr) const = 0;

    /**
     * Get the unique chip id.
     *
     * @param[out] chipId   Chip id
     */
    virtual void getChipId(String& chipId) const = 0;

    /**
     * Get the PSRAM size.
     *
     * @return PSRAM size in byte
     */
    virtual uint32_t getPsramSize() const = 0;

    /**
     * Get the currently free PSRAM.
     *
     * @return Free PSRAM in byte
     */
    virtual uint32_t getFreePsram() const = 0;

protected:

    /**
     * Constructs the system driver interface.
     */
    ISystemDrv()
    {
    }
};

/******************************************************************************
 * Variables
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* ISYSTEMDRV_H */

/** @} */