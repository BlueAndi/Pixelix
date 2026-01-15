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
 * @file   RestartMgr.h
 * @brief  Restart manager
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup RESTART
 *
 * @{
 */

#ifndef RESTARTMGR_H
#define RESTARTMGR_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <SimpleTimer.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * The restart manager handles everything around restart requests.
 * If requested, it will set the factory partition as boot partition.
 */
class RestartMgr
{
public:

    /**
     * This type defines the possible results when requesting a restart.
     */
    typedef enum
    {
        RESTART_REQ_STATUS_OK = 0,                      /**< Restart request was accepted. */
        RESTART_REQ_STATUS_ERR,                         /**< Restart request failed. */
        RESTART_REQ_STATUS_FACTORY_PARTITION_NOT_FOUND, /**< Partition could not be found. */
        RESTART_REQ_STATUS_FACTORY_SET_FAILED           /**< Partition could not be set as boot partition. */

    } RestartReqStatus;

    /**
     * Get restart manager instance.
     *
     * @return Restart manager
     */
    static RestartMgr& getInstance()
    {
        static RestartMgr instance; /* singleton idiom to force initialization in the first usage. */

        return instance;
    }

    /**
     * Is a restart requested?
     * A restart will be requested after a user initiates a change to the PixelixUpdater partition (factory) via the webinterface.
     *
     * @return If restart is requested, it will return true otherwise false.
     */
    bool isRestartRequested()
    {
        bool isRestartReq = m_isRestartReq;

        m_isRestartReq    = false;

        return isRestartReq;
    }

    /**
     * Will active partition change after restart?
     * Active partition will change after a user initiates a change to the PixelixUpdater partition (factory) via the webinterface.
     *
     * @return If active partition changes after restart, it will return true otherwise false.
     */
    bool isPartitionChange() const
    {
        return m_isPartitionChange;
    }

    /**
     * Handle delayed restart requests.
     */
    void process(void);

    /**
     * Request a restart.
     *
     * @param[in] delay How long the restart shall be delayed in ms.
     * @param[in] isPartitionChange Whether or not active partition will change after restart.
     *
     * @return Status
     */
    RestartReqStatus reqRestart(uint32_t delay, bool isPartitionChange);

private:

    /** Restart requested? */
    bool m_isRestartReq;

    /** Timer used to delay a restart request. */
    SimpleTimer m_timer;

    /** Partition change following after restart? */
    bool m_isPartitionChange;

    /**
     * Constructs the restart manager.
     */
    RestartMgr();

    /**
     * Destroys the restart manager.
     */
    ~RestartMgr();

    /**
     * Set the factory partition active to be the next boot partition.
     *
     * @return RestartReqStatus Indicating whether factory was set as boot partition successfully or not.
     */
    RestartReqStatus setFactoryAsBootPartition();
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* RESTARTMGR_H */

/** @} */