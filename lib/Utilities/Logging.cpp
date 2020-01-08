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
 * @brief  Logging
 * @author Yann Le Glaz <yann_le@web.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Logging.h"

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

/* Initialize Logging instance */
Logging Logging::m_instance;

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void Logging::init(Print* output)
{
    m_logOutput = output;
}

void Logging::setLogLevel(const LogLevel logLevel)
{
    m_currentLogLevel = logLevel;
}

Logging::LogLevel Logging::getLogLevel() const
{
    return m_currentLogLevel;
}

void Logging::processLogMessage(const char* file, int line, const Logging::LogLevel messageLogLevel, const char* format, ...)
{
    if (true == isSeverityValid(messageLogLevel))
    {
        char            buffer[MESSAGE_BUFFER_SIZE];
        int             written             = 0;
        const char*     STR_CUT_OFF_SEQ     = "...";
        const uint16_t  STR_CUT_OFF_SEQ_LEN = strlen(STR_CUT_OFF_SEQ);
        va_list         args;

        va_start(args, format);
        written = vsnprintf(buffer, MESSAGE_BUFFER_SIZE - STR_CUT_OFF_SEQ_LEN, format, args);
        va_end(args);

        /* If buffer was too small or any other error happended, it shall be shown in the
        * output string message with the STR_CUT_OFF_SEQ.
        */
        if ((0 > written) ||
            ((MESSAGE_BUFFER_SIZE - STR_CUT_OFF_SEQ_LEN) <= written))
        {
            strcat(buffer, STR_CUT_OFF_SEQ);
        }

        printLogMessage(file, line, messageLogLevel, buffer);
    }
    else
    {
        /* LogMessage is discarded! */
    }
}

void Logging::processLogMessage(const char* file, int line, const Logging::LogLevel messageLogLevel, const String& message)
{
    if (true == isSeverityValid(messageLogLevel))
    {
        printLogMessage(file, line, messageLogLevel, message.c_str());
    }
    else
    {
        /* LogMessage is discarded! */
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

bool Logging::isSeverityValid(const Logging::LogLevel logLevel)
{
    return (logLevel >= m_currentLogLevel);
}

const char* Logging::getBaseNameFromPath(const char* path) const
{
    const char* basename = path;
    const char* p        = path;

    if (nullptr != path)
    {
        for (p = path; *p != '\0'; p++)
        {
            if ((*p == '\\') || (*p == '/'))
            {
                basename = p + 1;
            }
        }
    }

    return basename;
}

void Logging::printLogMessage(const char* file, int line, const Logging::LogLevel messageLogLevel, const char* message) const
{
    if (nullptr != m_logOutput)
    {
        char            buffer[LOG_MESSAGE_BUFFER_SIZE] = { 0 };
        int             written                         = 0;
        const char*     STR_CUT_OFF_SEQ                 = "...";
        const uint16_t  STR_CUT_OFF_SEQ_LEN             = strlen(STR_CUT_OFF_SEQ);

        written = snprintf(buffer, LOG_MESSAGE_BUFFER_SIZE - STR_CUT_OFF_SEQ_LEN, "|%ld| %s %s:%d %s\r\n", millis(), logLevelToString(messageLogLevel), getBaseNameFromPath(file), line, message);

        /* If buffer was too small or any other error happended, it shall be shown in the
        * output string message with the STR_CUT_OFF_SEQ.
        */
        if ((0 > written) ||
            ((LOG_MESSAGE_BUFFER_SIZE - STR_CUT_OFF_SEQ_LEN) <= written))
        {
            strcat(buffer, STR_CUT_OFF_SEQ);
        }

        (void)m_logOutput->print(buffer);
    }
}

const char* Logging::logLevelToString(const Logging::LogLevel LogLevel) const
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