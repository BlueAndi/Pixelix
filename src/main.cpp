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
#include "TaskMon.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/******************************************************************************
 * Prototypes
 *****************************************************************************/

static int main_espLogVPrintf(const char* szFormat, va_list args);

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

/** Buffer for esp_log_write() method output. */
static char gLogPrintBuffer[512U];

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

    /* Pipe esp_log_write() output through own logging system. */
    (void)esp_log_set_vprintf(main_espLogVPrintf);
    esp_log_level_set("*", ESP_LOG_VERBOSE);

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

    /* Task monitor */
    TaskMon::getInstance().process();

    return;
}

/******************************************************************************
 * Local functions
 *****************************************************************************/

/**
 * This method is called by esp_log_write() to write log messages.
 *
 * @param[in] szFormat  Print format
 * @param[in] args      Variable argument list
 *
 * @return Number of written characters.
 */
static int main_espLogVPrintf(const char* szFormat, va_list args)
{
    int ret = vsnprintf(gLogPrintBuffer, sizeof(gLogPrintBuffer), szFormat, args);

    if (0 <= ret)
    {
        Logging::LogLevel   logLevel    = Logging::LOGLEVEL_INFO;
        int                 index       = 0U;
        String              timestamp;
        String              logger;
        String              message;

        /* Determine log level */
        if (0 < ret)
        {
            switch(gLogPrintBuffer[index])
            {
            case 'E':
                logLevel = Logging::LOGLEVEL_ERROR;
                break;

            case 'W':
                logLevel = Logging::LOGLEVEL_WARNING;
                break;

            case 'I':
                logLevel = Logging::LOGLEVEL_INFO;
                break;

            case 'D':
                logLevel = Logging::LOGLEVEL_INFO;
                break;

            case 'V':
                logLevel = Logging::LOGLEVEL_INFO;
                break;

            default:
                break;
            }

            index += 2; /* Overstep log level and SP */
        }

        /* Determine timestamp */
        ++index; /* Overstep '(' */
        while((ret > index) && (')' != gLogPrintBuffer[index]))
        {
            timestamp += gLogPrintBuffer[index];
            ++index;
        }
        index += 2; /* Overstep ')' and SP */

        /* Determine logger */
        while((ret > index) && (':' != gLogPrintBuffer[index]))
        {
            logger += gLogPrintBuffer[index];
            ++index;
        }
        index += 2; /* Overstep ':' and SP */

        message = &gLogPrintBuffer[index];

        /* Cut message on the next CR or LF. */
        index = 0;
        while('\0' != message[index])
        {
            if (('\r' == message[index]) ||
                ('\n' == message[index]))
            {
                message.remove(index);
                break;
            }

            ++index;
        }

        Logging::getInstance().processLogMessage(timestamp.toInt(), logger, logLevel, message);
    }

    return ret;
}