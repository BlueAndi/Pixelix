/* MIT License
 *
 * Copyright (c) 2019 - 2020 Andreas Merkle <web@blue-andi.de>
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
 * @brief  System state: Restart
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "RestartState.h"
#include "DisplayMgr.h"
#include "LedMatrix.h"
#include "Board.h"
#include "MyWebServer.h"
#include "UpdateMgr.h"

#include <Logging.h>
#include <Util.h>
#include <SPIFFS.h>
#include <ESPmDNS.h>

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

void RestartState::entry(StateMachine& sm)
{
    UTIL_NOT_USED(sm);

    LOG_INFO("Going in restart state.");

    m_timer.start(WAIT_TILL_STOP_SVC);

    return;
}

void RestartState::process(StateMachine& sm)
{
    UTIL_NOT_USED(sm);

    UpdateMgr::getInstance().process();

    if ((true == m_timer.isTimerRunning()) &&
        (true == m_timer.isTimeout()))
    {
        /* Stop all servers */
        MyWebServer::end();
        UpdateMgr::getInstance().end();
        MDNS.end();

        /* Unmount filesystem */
        SPIFFS.end();

        /* Stop display manager */
        DisplayMgr::getInstance().end();

        /* Clear display */
        LedMatrix::getInstance().clear();
        LedMatrix::getInstance().show();

        /* Wait till all physical pixels are cleared. */
        while(false == LedMatrix::getInstance().isReady())
        {
            /* Just wait ... */
            ;
        }

        /* Reset */
        Board::reset();
    }

    return;
}

void RestartState::exit(StateMachine& sm)
{
    UTIL_NOT_USED(sm);

    /* Nothing to do. */
    return;
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
