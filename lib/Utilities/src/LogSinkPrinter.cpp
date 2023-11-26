/* MIT License
 *
 * Copyright (c) 2019 - 2023 Andreas Merkle <web@blue-andi.de>
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
        const char*     STR_CUT_OFF_SEQ                 = "...\n";
        const uint16_t  STR_CUT_OFF_SEQ_LEN             = strlen(STR_CUT_OFF_SEQ);

        written = snprintf( buffer,
                            LOG_MESSAGE_BUFFER_SIZE - STR_CUT_OFF_SEQ_LEN + 1U, /* + 1U for cut off detection. */
                            "%*u %*s %*s:%.*d %s\n",
                            TIMESTAMP_LEN,
                            msg.timestamp,
                            LOG_LEVEL_LEN,
                            logLevelToString(msg.level),
                            FILENAME_LEN,
                            msg.filename,
                            LINE_LEN,
                            msg.line,
                            msg.str);

        /* Encoding error is skipped. */
        if (0 <= written)
        {
            const uint16_t MAX_LOG_MSG_LEN  = LOG_MESSAGE_BUFFER_SIZE - STR_CUT_OFF_SEQ_LEN - 1U;

            /* If the message string is cut off, notify the user about in the
             * log output.
             */
            if (MAX_LOG_MSG_LEN < written)
            {
                buffer[MAX_LOG_MSG_LEN] = '\0';
                strcat(buffer, STR_CUT_OFF_SEQ);
            }

            (void)m_output->print(buffer);
        }
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

const char* LogSinkPrinter::logLevelToString(const Logging::LogLevel logLevel) const
{
    const char* logLevelString = nullptr;

    switch (logLevel)
    {
        case Logging::LOG_LEVEL_FATAL:
            logLevelString = "FATAL  ";
            break;

        case Logging::LOG_LEVEL_ERROR:
            logLevelString = "ERROR  ";
        break;

        case Logging::LOG_LEVEL_WARNING:
            logLevelString = "WARNING";
            break;

        case Logging::LOG_LEVEL_INFO:
            logLevelString = "INFO   ";
            break;

        case Logging::LOG_LEVEL_DEBUG:
            logLevelString = "DEBUG  ";
            break;

        case Logging::LOG_LEVEL_TRACE:
            logLevelString = "TRACE  ";
            break;

        default:
            logLevelString = "UNKNOWN";
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