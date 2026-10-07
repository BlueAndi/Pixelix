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
#include <Esp.h>

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
        randomSeed(ESP.getCycleCount());
    };

    /**
     * Reset the system.
     */
    void reset() override
    {
        /* Reset the system */
        ESP.restart();
    };

    /**
     * Get the chip revision.
     *
     * @return Chip revision
     */
    uint8_t getChipRevision() const override
    {
        return ESP.getChipRevision();
    }

    /**
     * Get the CPU frequency.
     *
     * @return CPU frequency in MHz
     */
    uint32_t getCpuFreqMHz() const override
    {
        return ESP.getCpuFreqMHz();
    }

    /**
     * Get the flash chip read mode as a string.
     *
     * @return Flash chip read mode string
     */
    const char* getFlashChipModeStr() const override;

    /**
     * Get the flash chip speed.
     *
     * @return Flash chip speed in Hz
     */
    uint32_t getFlashChipSpeed() const override
    {
        return ESP.getFlashChipSpeed();
    }

    /**
     * Get the flash chip size.
     *
     * @return Flash chip size in byte
     */
    uint32_t getFlashChipSize() const override
    {
        return ESP.getFlashChipSize();
    }

    /**
     * Get the SDK version.
     *
     * @return SDK version string
     */
    const char* getSdkVersion() const override
    {
        return ESP.getSdkVersion();
    }

    /**
     * Get the LwIP version.
     *
     * @return LwIP version string
     */
    const char* getLwIPVersion() const override;

    /**
     * Get the LittleFS version.
     *
     * @return LittleFS version string
     */
    const char* getLittleFSVersion() const override;

    /**
     * Get the mbedTLS version.
     *
     * @return mbedTLS version string
     */
    const char* getMbedTlsVersion() const override;

    /**
     * Get the factory programmed wifi MAC address.
     *
     * @param[out] macAddr  MAC address in "00:00:00:00:00:00" syntax.
     */
    void getEFuseMAC(String& macAddr) const override;

    /**
     * Get the unique chip id.
     *
     * @param[out] chipId   Chip id
     */
    void getChipId(String& chipId) const override;

    /**
     * Get the PSRAM size.
     *
     * @return PSRAM size in byte
     */
    uint32_t getPsramSize() const override
    {
        return ESP.getPsramSize();
    }

    /**
     * Get the currently free PSRAM.
     *
     * @return Free PSRAM in byte
     */
    uint32_t getFreePsram() const override
    {
        return ESP.getFreePsram();
    }

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