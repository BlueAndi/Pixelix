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
 * @file   SystemDrv.cpp
 * @brief  System driver
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "SystemDrv.h"

#include <mbedtls/version.h>
#include <lwip/init.h>
#include <esp_littlefs.h>

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and classes
 *****************************************************************************/

/******************************************************************************
 * Prototypes
 *****************************************************************************/

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/******************************************************************************
 * Public Methods
 *****************************************************************************/

const char* SystemDrv::getFlashChipModeStr() const
{
    const char* flashChipMode = "UNKNOWN";

    switch (ESP.getFlashChipMode())
    {
    case FM_QIO:
        flashChipMode = "QUIO";
        break;

    case FM_QOUT:
        flashChipMode = "QOUT";
        break;

    case FM_DIO:
        flashChipMode = "DIO";
        break;

    case FM_DOUT:
        flashChipMode = "DOUT";
        break;

    case FM_FAST_READ:
        flashChipMode = "FAST_READ";
        break;

    case FM_SLOW_READ:
        flashChipMode = "SLOW_READ";
        break;

    case FM_UNKNOWN:
        /* fallthrough */

    default:
        break;
    }

    return flashChipMode;
}

const char* SystemDrv::getLwIPVersion() const
{
    return LWIP_VERSION_STRING;
}

const char* SystemDrv::getLittleFSVersion() const
{
    return ESP_LITTLEFS_VERSION_NUMBER;
}

const char* SystemDrv::getMbedTlsVersion() const
{
    return MBEDTLS_VERSION_STRING;
}

void SystemDrv::getEFuseMAC(String& macAddr) const
{
    const uint64_t efuseMac = ESP.getEfuseMac();
    const uint8_t  byte1    = (efuseMac >> 40U) & 0xffU;
    const uint8_t  byte2    = (efuseMac >> 32U) & 0xffU;
    const uint8_t  byte3    = (efuseMac >> 24U) & 0xffU;
    const uint8_t  byte4    = (efuseMac >> 16U) & 0xffU;
    const uint8_t  byte5    = (efuseMac >> 8U) & 0xffU;
    const uint8_t  byte6    = (efuseMac >> 0U) & 0xffU;
    const size_t   bufferSize = 18U;
    char           buffer[bufferSize];

    (void)snprintf(buffer, sizeof(buffer), "%02X:%02X:%02X:%02X:%02X:%02X", byte6, byte5, byte4, byte3, byte2, byte1);

    macAddr = buffer;
}

void SystemDrv::getChipId(String& chipId) const
{
    const uint64_t efuseMac = ESP.getEfuseMac();
    const int32_t  highPart = (efuseMac >> 8U) & 0x0000ffffU;
    const int32_t  lowPart  = (efuseMac >> 0U) & 0xffffffffU;
    const size_t   bufferSize = 13U;
    char           buffer[bufferSize];

    (void)snprintf(buffer, sizeof(buffer), "%04X%08X", highPart, lowPart);

    chipId = buffer;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
