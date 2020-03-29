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
 * @brief  Printer log sink
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "LogSinkPrinter.h"

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

void LogSinkPrinter::send(const Logging::Msg& msg)
{
    if (nullptr != m_output)
    {
        char            buffer[LOG_MESSAGE_BUFFER_SIZE] = { 0 };
        int             written                         = 0;
        const char*     STR_CUT_OFF_SEQ                 = "...";
        const uint16_t  STR_CUT_OFF_SEQ_LEN             = strlen(STR_CUT_OFF_SEQ);

        written = snprintf( buffer,
                            LOG_MESSAGE_BUFFER_SIZE - STR_CUT_OFF_SEQ_LEN,
                            "|%lu| %s %s:%d %s\r\n",
                            msg.timestamp,
                            logLevelToString(msg.level),
                            msg.filename,
                            msg.line,
                            msg.str);

        /* If buffer was too small or any other error happended, it shall be shown in the
         * output string message with the STR_CUT_OFF_SEQ.
         */
        if ((0 > written) ||
            ((LOG_MESSAGE_BUFFER_SIZE - STR_CUT_OFF_SEQ_LEN) <= written))
        {
            strncat(buffer, STR_CUT_OFF_SEQ, LOG_MESSAGE_BUFFER_SIZE - strlen(buffer) - 1U);
        }

        (void)m_output->print(buffer);
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

const char* LogSinkPrinter::logLevelToString(const Logging::LogLevel LogLevel) const
{
    const char* logLevelString = nullptr;

    switch (LogLevel)
    {
        case Logging::LOGLEVEL_INFO:
            logLevelString = "INFO:";
            break;

        case Logging::LOGLEVEL_WARNING :
            logLevelString = "WARNING:";
            break;

        case Logging::LOGLEVEL_ERROR :
            logLevelString = "ERROR:";
        break;

        case Logging::LOGLEVEL_FATAL :
            logLevelString = "FATAL:";
            break;

        default:
            logLevelString = "UNKNOWN:";
            break;
    }

    return logLevelString;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/


/******************************************************************************
 * Local Functions
 *****************************************************************************/