/* MIT License
 *
 * Copyright (c) 2019 Andreas Merkle <web@blue-andi.de>
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
@brief  System state: Connected
@author Andreas Merkle <web@blue-andi.de>

@section desc Description
@see ConnectedState.h

*******************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "ConnectedState.h"
#include "DisplayMgr.h"
#include "UpdateMgr.h"
#include "MyWebServer.h"
#include "ConnectingState.h"

#include <Arduino.h>
#include <WiFi.h>

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

/* Connected state instance */
ConnectedState  ConnectedState::m_instance;

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void ConnectedState::entry(StateMachine& sm)
{
    String infoStr = "Hostname: ";
    infoStr += WiFi.getHostname();

    Serial.println(infoStr);
    DisplayMgr::getInstance().showSysMsg(infoStr);
    DisplayMgr::getInstance().delay(SYS_MSG_WAIT_TIME_STD);

    /* Enable slots */
    DisplayMgr::getInstance().enableSlots(true);
    DisplayMgr::getInstance().startRotating(true);
    
    return;
}

void ConnectedState::process(StateMachine& sm)
{
    /* Connection lost? */
    if (false == WiFi.isConnected())
    {
        sm.setState(ConnectingState::getInstance());
    }
    /* Connection is still established */
    else
    {
        /* Handle update, there may be one in the background. */
        UpdateMgr::getInstance().process();

        /* As long as no update is running, do handle all other connections. */
        if (false == UpdateMgr::getInstance().isUpdateRunning())
        {
            /* Handle all clients */
            MyWebServer::getInstance().handleClient();
        }
    }

    return;
}

void ConnectedState::exit(StateMachine& sm)
{
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
