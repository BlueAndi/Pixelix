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
 * @brief  System driver for the native environment
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * The system information is shown in the webinterface. On the host there is no
 * ESP32, therefore virtual values are provided.
 *
 * @addtogroup TEST
 *
 * @{
 */

#ifndef SYSTEM_DRV_H
#define SYSTEM_DRV_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <ISystemDrv.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * System driver for the native environment.
 */
class SystemDrv : public ISystemDrv
{
public:

    /**
     * Constructs the system driver.
     */
    SystemDrv() :
        ISystemDrv()
    {
    }

    /**
     * Destroys the system driver.
     */
    virtual ~SystemDrv()
    {
    }

    /**
     * Initialize the system driver.
     */
    void init() final
    {
    }

    /**
     * Reset the system.
     * On the host the program is terminated.
     */
    void reset() final;

    /**
     * Get the chip revision.
     *
     * @return Chip revision
     */
    uint8_t getChipRevision() const final
    {
        return 0U;
    }

    /**
     * Get the CPU frequency.
     *
     * @return CPU frequency in MHz
     */
    uint32_t getCpuFreqMHz() const final
    {
        return 0U;
    }

    /**
     * Get the flash chip mode.
     *
     * @return Flash chip mode
     */
    const char* getFlashChipModeStr() const final
    {
        return "N/A";
    }

    /**
     * Get the flash chip speed.
     *
     * @return Flash chip speed in Hz
     */
    uint32_t getFlashChipSpeed() const final
    {
        return 0U;
    }

    /**
     * Get the flash chip size.
     *
     * @return Flash chip size in byte
     */
    uint32_t getFlashChipSize() const final
    {
        return 0U;
    }

    /**
     * Get the SDK version.
     *
     * @return SDK version
     */
    const char* getSdkVersion() const final
    {
        return "native";
    }

    /**
     * Get the lwIP version.
     *
     * @return lwIP version
     */
    const char* getLwIPVersion() const final
    {
        return "N/A";
    }

    /**
     * Get the LittleFS version.
     *
     * @return LittleFS version
     */
    const char* getLittleFSVersion() const final
    {
        return "N/A";
    }

    /**
     * Get the mbedTLS version.
     *
     * @return mbedTLS version
     */
    const char* getMbedTlsVersion() const final
    {
        return "N/A";
    }

    /**
     * Get the MAC address, which is programmed in the eFuse.
     *
     * @param[out] macAddr  MAC address
     */
    void getEFuseMAC(String& macAddr) const final
    {
        macAddr = VIRTUAL_MAC_ADDRESS;
    }

    /**
     * Get the chip id.
     *
     * @param[out] chipId   Chip id
     */
    void getChipId(String& chipId) const final
    {
        chipId = VIRTUAL_CHIP_ID;
    }

    /**
     * Get the PSRAM size.
     *
     * @return PSRAM size in byte
     */
    uint32_t getPsramSize() const final
    {
        return 0U;
    }

    /**
     * Get the free PSRAM.
     *
     * @return Free PSRAM in byte
     */
    uint32_t getFreePsram() const final
    {
        return 0U;
    }

private:

    /** Virtual MAC address, which is shown in the webinterface. */
    static constexpr const char* VIRTUAL_MAC_ADDRESS = "02:00:00:00:00:01";

    /** Virtual chip id, which is shown in the webinterface. */
    static constexpr const char* VIRTUAL_CHIP_ID     = "000000000001";

    SystemDrv(const SystemDrv& drv);
    SystemDrv& operator=(const SystemDrv& drv);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* SYSTEM_DRV_H */

/** @} */
