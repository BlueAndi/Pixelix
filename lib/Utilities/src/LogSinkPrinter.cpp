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

const char* LogSinkPrinter::DIVIDER         = ":";      /* If you change, don't forget to update the DIVIDER_LEN! */
const char* LogSinkPrinter::STR_CUT_OFF_SEQ = "...\n";  /* If you change, don't forget to update the STR_CUT_OFF_SEQ_LEN! */

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void LogSinkPrinter::send(const Logging::Msg& msg)
{
    if (nullptr != m_output)
    {
        char            buffer[LOG_MESSAGE_BUFFER_SIZE] = { 0 };
        const size_t    BUFFER_SIZE_CUT_OFF             = LOG_MESSAGE_BUFFER_SIZE - STR_CUT_OFF_SEQ_LEN + 1U;
        const size_t    MAX_LOG_MSG_LEN                 = LOG_MESSAGE_BUFFER_SIZE - STR_CUT_OFF_SEQ_LEN - 1U;
        const char*     STR_LOG_LEVEL                   = logLevelToString(msg.level);
        int             written                         = 0;
        int             writtenTotal                    = 0;

        /* Timestamp */
        if (BUFFER_SIZE_CUT_OFF > writtenTotal)
        {
            written = snprintf(&buffer[writtenTotal], BUFFER_SIZE_CUT_OFF - writtenTotal, "%u", msg.timestamp);
            writtenTotal += written;
        }

        while((BUFFER_SIZE_CUT_OFF > writtenTotal) && (TIMESTAMP_LEN >= written))
        {
            buffer[writtenTotal] = ' ';
            ++written;
            ++writtenTotal;
        }

        /* Log level */
        written = 0;
        while((BUFFER_SIZE_CUT_OFF > writtenTotal) && ('\0' != STR_LOG_LEVEL[written]))
        {
            buffer[writtenTotal] = STR_LOG_LEVEL[written];
            ++written;
            ++writtenTotal;
        }

        while((BUFFER_SIZE_CUT_OFF > writtenTotal) && (LOG_LEVEL_LEN >= written))
        {
            buffer[writtenTotal] = ' ';
            ++written;
            ++writtenTotal;
        }

        /* Filename */
        written = 0;
        while((BUFFER_SIZE_CUT_OFF > writtenTotal) && ('\0' != msg.filename[written]))
        {
            buffer[writtenTotal] = msg.filename[written];
            ++written;
            ++writtenTotal;
        }

        /* Divider */
        written = 0;
        while((BUFFER_SIZE_CUT_OFF > writtenTotal) && ('\0' != DIVIDER[written]))
        {
            buffer[writtenTotal] = DIVIDER[written];
            ++written;
            ++writtenTotal;
        }

        /* Line number */
        if (BUFFER_SIZE_CUT_OFF > writtenTotal)
        {
            written = snprintf(&buffer[writtenTotal], BUFFER_SIZE_CUT_OFF - writtenTotal, "%d", msg.line);
            writtenTotal += written;
        }

        written = 0;
        while((BUFFER_SIZE_CUT_OFF > writtenTotal) && (MSG_INDEX >= writtenTotal))
        {
            buffer[writtenTotal] = ' ';
            ++writtenTotal;
        }

        /* It may happen that the filename was longer than exepected, in this
         * case ensure that at least one space is between the line number and
         * the message.
         */
        if ((BUFFER_SIZE_CUT_OFF > writtenTotal) &&
            (0 == written))
        {
            buffer[writtenTotal] = ' ';
            ++writtenTotal;
        }

        /* Message */
        if (BUFFER_SIZE_CUT_OFF > writtenTotal)
        {
            written = snprintf(&buffer[writtenTotal], BUFFER_SIZE_CUT_OFF - writtenTotal, "%s\n", msg.str);
            writtenTotal += written;
        }

        /* If the message string is cut off, notify the user about in the
         * log output.
         */
        if (MAX_LOG_MSG_LEN < writtenTotal)
        {
            buffer[MAX_LOG_MSG_LEN] = '\0';
            (void)strcpy(&buffer[MAX_LOG_MSG_LEN], STR_CUT_OFF_SEQ);
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