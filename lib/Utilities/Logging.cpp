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
    char buffer[MESSAGE_BUFFER_SIZE];

    va_list args;

    va_start(args, format);
    vsnprintf(buffer, MESSAGE_BUFFER_SIZE, format, args);
    va_end(args);

    if (isSeverityValid(messageLogLevel))
    {
        printLogMessage(file, line, messageLogLevel, buffer);
    }
    else
    {
        /* LogMessage is discarded! */
    }
}

void Logging::processLogMessage(const char* file, int line, const Logging::LogLevel messageLogLevel, const String& message)
{
    if (isSeverityValid(messageLogLevel))
    {
        printLogMessage(file, line, messageLogLevel, message);
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

    if (NULL != path)
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
    if (NULL != m_logOutput)
    {
        m_logOutput->print("|");
        m_logOutput->print(millis());
        m_logOutput->print("|");
        m_logOutput->print(logLevelToString(messageLogLevel));
        m_logOutput->print(getBaseNameFromPath(file));
        m_logOutput->print(":");
        m_logOutput->print(line);
        m_logOutput->print(" ");
        m_logOutput->println(message);
    }
}

void Logging::printLogMessage(const char* file, int line, const Logging::LogLevel messageLogLevel, const String& message) const
{
    if (NULL != m_logOutput)
    {
        m_logOutput->print("|");
        m_logOutput->print(millis());
        m_logOutput->print("|");
        m_logOutput->print(logLevelToString(messageLogLevel));
        m_logOutput->print(getBaseNameFromPath(file));
        m_logOutput->print(":");
        m_logOutput->print(line);
        m_logOutput->print(" ");
        m_logOutput->println(message);
    }
}

String Logging::logLevelToString(const Logging::LogLevel LogLevel) const
{
    String logLevelString;

    switch (LogLevel)
    {
        case Logging::LOGLEVEL_INFO:
            logLevelString = " INFO: ";
            break;

        case Logging::LOGLEVEL_WARNING :
            logLevelString = " WARNING: ";
            break;

        case Logging::LOGLEVEL_ERROR :
            logLevelString = " ERROR: ";
        break;

        case Logging::LOGLEVEL_FATAL :
            logLevelString = " FATAL: ";
            break;

        default:
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