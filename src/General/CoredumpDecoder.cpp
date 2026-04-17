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
 * @file   CoredumpDecoder.cpp
 * @brief  Coredump partition decoder
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "CoredumpDecoder.h"

#include <esp_partition.h>
#include <esp_core_dump.h>
#include <Logging.h>

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

static const esp_partition_t* findCoredumpPartition();
static const char*            getExceptionCauseName(uint32_t causeCode);

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/******************************************************************************
 * Public Methods
 *****************************************************************************/

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

namespace CoredumpDecoder
{

CoredumpStatus hasCoredump(bool& hasCoredump)
{
    CoredumpStatus         status    = COREDUMP_STATUS_OK;
    const esp_partition_t* partition = findCoredumpPartition();

    hasCoredump                      = false;

    if (nullptr == partition)
    {
        status = COREDUMP_STATUS_NO_PARTITION;
    }
    else
    {
        /* Check for coredump using ESP-IDF API */
        esp_err_t err = esp_core_dump_image_check();

        if (ESP_OK == err)
        {
            hasCoredump = true;
        }
        else if (ESP_ERR_NOT_FOUND == err)
        {
            hasCoredump = false;
        }
        else
        {
            LOG_WARNING("Coredump check failed with error: %d", err);
            status = COREDUMP_STATUS_READ_ERROR;
        }
    }

    return status;
}

CoredumpStatus getCoredumpInfo(JsonObject& jsonData)
{
    CoredumpStatus         status    = COREDUMP_STATUS_OK;
    const esp_partition_t* partition = findCoredumpPartition();
    bool                   hasDump   = false;

    if (nullptr == partition)
    {
        status = COREDUMP_STATUS_NO_PARTITION;
    }
    else
    {
        status = hasCoredump(hasDump);

        if (COREDUMP_STATUS_OK == status)
        {
            jsonData["hasCoredump"]     = hasDump;
            jsonData["partitionSize"]   = partition->size;
            jsonData["partitionOffset"] = partition->address;

            if (true == hasDump)
            {
                /* Get additional info using esp_core_dump_image_get() */
                size_t    outAddr = 0U;
                size_t    outSize = 0U;
                esp_err_t err     = esp_core_dump_image_get(&outAddr, &outSize);

                if (ESP_OK == err)
                {
                    jsonData["coredumpSize"]   = outSize;
                    jsonData["coredumpOffset"] = outAddr;
                }
                else
                {
                    LOG_WARNING("Failed to get coredump image info: %d", err);
                }
            }
        }
    }

    return status;
}

CoredumpStatus getCoredumpSummary(JsonObject& jsonData)
{
    CoredumpStatus         status    = COREDUMP_STATUS_OK;
    const esp_partition_t* partition = findCoredumpPartition();
    bool                   hasDump   = false;

    if (nullptr == partition)
    {
        status = COREDUMP_STATUS_NO_PARTITION;
    }
    else
    {
        status = hasCoredump(hasDump);

        if (COREDUMP_STATUS_OK == status)
        {
            if (false == hasDump)
            {
                status = COREDUMP_STATUS_NO_COREDUMP;
            }
            else
            {
                /* Get coredump summary using ESP-IDF API */
                esp_core_dump_summary_t summary;
                esp_err_t               err = esp_core_dump_get_summary(&summary);

                if (ESP_OK != err)
                {
                    LOG_WARNING("Failed to get coredump summary: %d", err);
                    status = COREDUMP_STATUS_INVALID_FORMAT;
                }
                else
                {
                    /* Populate JSON with human-readable crash information */
                    jsonData["crashedTask"]    = summary.exc_task;
                    jsonData["exceptionCause"] = getExceptionCauseName(summary.ex_info.exc_cause);
                    jsonData["exceptionCode"]  = summary.ex_info.exc_cause;
                    jsonData["exceptionAddr"]  = String("0x") + String(summary.ex_info.exc_vaddr, HEX);

                    /* Add register content - Address registers (A0-A15) from exception frame */
                    JsonObject registers       = jsonData.createNestedObject("registers");

                    for (uint32_t idx = 0U; idx < 16U; ++idx)
                    {
                        String regName     = "A" + String(idx);
                        registers[regName] = String("0x") + String(summary.ex_info.exc_a[idx], HEX);
                    }

                    /* Exception cause and address (also shown at top level for convenience) */
                    registers["EXCCAUSE"]      = summary.ex_info.exc_cause;
                    registers["EXCVADDR"]      = String("0x") + String(summary.ex_info.exc_vaddr, HEX);

                    /* Add backtrace array - use actual depth from structure */
                    JsonArray backtrace        = jsonData.createNestedArray("backtrace");
                    uint32_t  maxBacktraceSize = sizeof(summary.exc_bt_info.bt) / sizeof(summary.exc_bt_info.bt[0]);

                    for (uint32_t idx = 0U; (idx < summary.exc_bt_info.depth) && (idx < maxBacktraceSize); ++idx)
                    {
                        String addr = String("0x") + String(summary.exc_bt_info.bt[idx], HEX);
                        backtrace.add(addr);
                    }

                    jsonData["backtraceDepth"] = summary.exc_bt_info.depth;
                }
            }
        }
    }

    return status;
}

CoredumpStatus clearCoredump()
{
    CoredumpStatus         status    = COREDUMP_STATUS_OK;
    const esp_partition_t* partition = findCoredumpPartition();

    if (nullptr == partition)
    {
        return COREDUMP_STATUS_NO_PARTITION;
    }

    /* Erase the entire coredump partition */
    esp_err_t err = esp_partition_erase_range(partition, 0U, partition->size);

    if (ESP_OK != err)
    {
        LOG_ERROR("Failed to erase coredump partition: %d", err);
        status = COREDUMP_STATUS_ERASE_ERROR;
    }
    else
    {
        LOG_INFO("Coredump partition cleared successfully");
    }

    return status;
}

} /* namespace CoredumpDecoder */

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/**
 * Find the coredump partition.
 *
 * @return Pointer to partition structure or nullptr if not found
 */
static const esp_partition_t* findCoredumpPartition()
{
    const esp_partition_t* partition = esp_partition_find_first(
        ESP_PARTITION_TYPE_DATA,
        ESP_PARTITION_SUBTYPE_DATA_COREDUMP,
        nullptr);

    if (nullptr == partition)
    {
        LOG_WARNING("Coredump partition not found");
    }

    return partition;
}

/**
 * Get human-readable name for exception cause code.
 * Based on ESP32 Xtensa exception causes.
 *
 * @param[in] causeCode  Exception cause code
 *
 * @return Human-readable exception cause name
 */
static const char* getExceptionCauseName(uint32_t causeCode)
{
    const char* causeName = "Unknown";

    switch (causeCode)
    {
    case 0U:
        causeName = "IllegalInstruction";
        break;

    case 1U:
        causeName = "Syscall";
        break;

    case 2U:
        causeName = "InstructionFetchError";
        break;

    case 3U:
        causeName = "LoadStoreError";
        break;

    case 4U:
        causeName = "Level1Interrupt";
        break;

    case 5U:
        causeName = "Alloca";
        break;

    case 6U:
        causeName = "IntegerDivideByZero";
        break;

    case 8U:
        causeName = "Privileged";
        break;

    case 9U:
        causeName = "LoadStoreAlignment";
        break;

    case 12U:
        causeName = "InstrPIFDataError";
        break;

    case 13U:
        causeName = "LoadStorePIFDataError";
        break;

    case 14U:
        causeName = "InstrPIFAddrError";
        break;

    case 15U:
        causeName = "LoadStorePIFAddrError";
        break;

    case 16U:
        causeName = "InstTLBMiss";
        break;

    case 17U:
        causeName = "InstTLBMultiHit";
        break;

    case 18U:
        causeName = "InstFetchPrivilege";
        break;

    case 20U:
        causeName = "InstFetchProhibited";
        break;

    case 24U:
        causeName = "LoadStoreTLBMiss";
        break;

    case 25U:
        causeName = "LoadStoreTLBMultiHit";
        break;

    case 26U:
        causeName = "LoadStorePrivilege";
        break;

    case 28U:
        causeName = "LoadProhibited";
        break;

    case 29U:
        causeName = "StoreProhibited";
        break;

    default:
        causeName = "Unknown";
        break;
    }

    return causeName;
}
