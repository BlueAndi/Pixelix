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
 * @file   RestartMgr.cpp
 * @brief  Restart manager
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "RestartMgr.h"

#include <esp_ota_ops.h>
#include <esp_partition.h>
#include <esp_err.h>
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

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void RestartMgr::process()
{
    /* Delayed restart request? */
    if ((true == m_timer.isTimerRunning()) &&
        (true == m_timer.isTimeout()))
    {
        m_isRestartReq = true;
        m_timer.stop();
    }
}

RestartMgr::RestartReqStatus RestartMgr::reqRestart(uint32_t delay, bool isPartitionChange)
{
    RestartReqStatus status = RESTART_REQ_STATUS_OK;

    /* Cannot be overwritten by a later restart request before restart is carried out. */
    if (true == isPartitionChange)
    {
        if (false == m_isPartitionChange)
        {
            status = setFactoryAsBootPartition();

            if (RESTART_REQ_STATUS_OK == status)
            {
                m_isPartitionChange = true;
            }
        }
    }

    if (RESTART_REQ_STATUS_OK == status)
    {
        if (0U == delay)
        {
            m_isRestartReq = true;
        }
        else
        {
            m_timer.start(delay);
        }
    }

    return status;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

RestartMgr::RestartMgr() :
    m_isRestartReq(false),
    m_timer(),
    m_isPartitionChange(false)
{
}

RestartMgr::~RestartMgr()
{
}

RestartMgr::RestartReqStatus RestartMgr::setFactoryAsBootPartition()
{
    RestartReqStatus       result    = RESTART_REQ_STATUS_ERR;
    const esp_partition_t* partition = esp_partition_find_first(
        esp_partition_type_t::ESP_PARTITION_TYPE_APP,
        esp_partition_subtype_t::ESP_PARTITION_SUBTYPE_APP_FACTORY,
        nullptr);

    if (nullptr != partition)
    {
        esp_err_t err = esp_ota_set_boot_partition(partition);

        LOG_INFO("Setting factory partition '%s' as boot partition", partition->label);

        if (ESP_OK != err)
        {
            LOG_ERROR("Failed to set factory partition '%s' as boot partition: %d", partition->label, err);
            result = RESTART_REQ_STATUS_FACTORY_SET_FAILED;
        }
        else
        {
            result = RESTART_REQ_STATUS_OK;
        }
    }
    else
    {
        LOG_ERROR("Factory partition not found!");
        result = RESTART_REQ_STATUS_FACTORY_PARTITION_NOT_FOUND;
    }

    return result;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
