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
 * @brief  Main entry point
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <Arduino.h>
#include <Logging.h>
#include <LogSinkPrinter.h>
#include "LogSinkWebsocket.h"
#include <StateMachine.hpp>

#include "Board.h"
#include "InitState.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/******************************************************************************
 * Prototypes
 *****************************************************************************/

/******************************************************************************
 * Variables
 *****************************************************************************/

/** System state machine */
static StateMachine     gSysStateMachine(InitState::getInstance());

/** Serial log sink */
static LogSinkPrinter   gLogSinkSerial("Serial", &Serial);

/** Websocket log sink */
static LogSinkWebsocket gLogSinkWebsocket("Websocket", &WebSocketSrv::getInstance());

/** Serial interface baudrate. */
static const uint32_t   SERIAL_BAUDRATE = 115200U;

/******************************************************************************
 * External functions
 *****************************************************************************/

/**
 * Setup the system.
 */
void setup()
{
    /* Setup serial interface */
    Serial.begin(SERIAL_BAUDRATE);

    /* Register serial log sink and select it per default. */
    if (true == Logging::getInstance().registerSink(&gLogSinkSerial))
    {
        (void)Logging::getInstance().selectSink("Serial");
    }

    /* Register websocket log sink. */
    (void)Logging::getInstance().registerSink(&gLogSinkWebsocket);

    /* Set severity */
    Logging::getInstance().setLogLevel(Logging::LOGLEVEL_INFO);

    /* The setup routine shall handle only the initialization state.
     * All other states are handled in the loop routine.
     */
    do
    {
        gSysStateMachine.process();
    }
    while(static_cast<AbstractState*>(&InitState::getInstance()) == gSysStateMachine.getState());

    return;
}

/**
 * Main loop, which is called periodically.
 */
void loop()
{
    /* Process system state machine */
    gSysStateMachine.process();

    return;
}

/******************************************************************************
 * Local functions
 *****************************************************************************/
