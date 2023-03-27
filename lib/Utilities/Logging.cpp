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

/******************************************************************************
 * Public Methods
 *****************************************************************************/

bool Logging::registerSink(LogSink* sink)
{
    bool status = false;

    if (nullptr != sink)
    {
        uint8_t index = 0U;

        while((MAX_SINKS > index) && (false == status))
        {
            if (nullptr == m_sinks[index])
            {
                m_sinks[index] = sink;
                status = true;
            }
            else
            {
                ++index;
            }
        }
    }

    return status;
}

void Logging::unregisterSink(LogSink* sink)
{
    uint8_t index = 0U;

    while((MAX_SINKS > index) && (nullptr != sink))
    {
        if (sink == m_sinks[index])
        {
            m_sinks[index] = nullptr;

            if (sink == m_selectedSink)
            {
                m_selectedSink = nullptr;
            }

            sink = nullptr;
        }
        else
        {
            ++index;
        }
    }
}

bool Logging::selectSink(const String& name)
{
    bool    status  = false;
    uint8_t index   = 0U;

    while((MAX_SINKS > index) && (false == status))
    {
        if (m_sinks[index]->getName() == name)
        {
            m_selectedSink = m_sinks[index];
            status = true;
        }
        else
        {
            ++index;
        }
    }

    return status;
}

LogSink* Logging::getSelectedSink()
{
    return m_selectedSink;
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
    if ((true == isSeverityEnabled(messageLogLevel)) &&
        (nullptr != m_selectedSink))
    {
        char            buffer[MESSAGE_BUFFER_SIZE];
        int             written             = 0;
        const char*     STR_CUT_OFF_SEQ     = "...";
        const uint16_t  STR_CUT_OFF_SEQ_LEN = strlen(STR_CUT_OFF_SEQ);
        va_list         args;
        Msg             msg;

        va_start(args, format);
        written = vsnprintf(buffer, MESSAGE_BUFFER_SIZE - STR_CUT_OFF_SEQ_LEN, format, args); /* NOLINT(clang-analyzer-valist.Uninitialized) */
        va_end(args);

        /* If buffer was too small or any other error happended, it shall be shown in the
         * output string message with the STR_CUT_OFF_SEQ.
         */
        if ((0 > written) ||
            ((MESSAGE_BUFFER_SIZE - STR_CUT_OFF_SEQ_LEN) <= written))
        {
            strncat(buffer, STR_CUT_OFF_SEQ, MESSAGE_BUFFER_SIZE - strlen(buffer) - 1U);
        }

        msg.timestamp   = esp_log_timestamp();
        msg.level       = messageLogLevel;
        msg.filename    = getBaseNameFromPath(file);
        msg.line        = line;
        msg.str         = buffer;

        m_selectedSink->send(msg);
    }
    else
    {
        /* LogMessage is discarded! */
    }
}

void Logging::processLogMessage(const char* file, int line, const Logging::LogLevel messageLogLevel, const String& message)
{
    if ((true == isSeverityEnabled(messageLogLevel)) &&
        (nullptr != m_selectedSink))
    {
        Msg msg;

        msg.timestamp   = esp_log_timestamp();
        msg.level       = messageLogLevel;
        msg.filename    = getBaseNameFromPath(file);
        msg.line        = line;
        msg.str         = message.c_str();

        m_selectedSink->send(msg);
    }
    else
    {
        /* LogMessage is discarded! */
    }
}

void Logging::processLogMessage(uint32_t timestamp, const String& logger, const LogLevel messageLogLevel, const String& message)
{
    if ((true == isSeverityEnabled(messageLogLevel)) &&
        (nullptr != m_selectedSink))
    {
        Msg msg;

        msg.timestamp   = timestamp;
        msg.level       = messageLogLevel;
        msg.filename    = logger.c_str();
        msg.line        = 0;
        msg.str         = message.c_str();

        m_selectedSink->send(msg);
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

bool Logging::isSeverityEnabled(Logging::LogLevel logLevel) const
{
    return (logLevel <= m_currentLogLevel);
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

/******************************************************************************
 * External Functions
 *****************************************************************************/


/******************************************************************************
 * Local Functions
 *****************************************************************************/