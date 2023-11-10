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
 *
 * @addtogroup utilities
 *
 * @{
 */

#ifndef LOG_SINK_PRINTER_H
#define LOG_SINK_PRINTER_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Logging.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Printer log sink.
 */
class LogSinkPrinter : public LogSink
{
public:

    /**
     * Constructs a empty printer log sink.
     */
    LogSinkPrinter() :
        m_name(),
        m_output(nullptr)
    {

    }

    /**
     * Constructs a printer log sink.
     *
     * @param[in] name      Name of the sink
     * @param[in] output    Printer
     */
    LogSinkPrinter(const String& name, Print* output) :
        m_name(name),
        m_output(output)
    {
    }

    /**
     * Destroys the printer log sink.
     */
    ~LogSinkPrinter()
    {
    }

    /**
     * Get printer.
     *
     * @return Printer
     */
    Print* getPrinter()
    {
        return m_output;
    }

    /**
     * Set printer.
     *
     * @param[in] printer   Printer
     */
    void setPrinter(Print* printer)
    {
        m_output = printer;
    }

    /**
     * Set sink name.
     *
     * @param[in] name Name of the sink
     */
    void setName(const String& name)
    {
        m_name = name;
    }

    /**
     * Get sink name.
     *
     * @return Name of the sink.
     */
    const String& getName() const final
    {
        return m_name;
    }

    /**
     * Send a log message to this sink.
     *
     * @param[in] msg   Log message
     */
    void send(const Logging::Msg& msg) final;

    /** Maximum timestamp string length. */
    static const uint32_t   TIMESTAMP_LEN   = 10U;

    /** Maximum log level string length. */
    static const uint32_t   LOG_LEVEL_LEN   = 7U;

    /** Maximum filename string length. */
    static const uint32_t   FILENAME_LEN    = 22U;

    /** Maximum line number string length. */
    static const uint32_t   LINE_LEN        = 5U;

    /** The maximum size of the whole log message. */
    static const uint16_t LOG_MESSAGE_BUFFER_SIZE = Logging::MESSAGE_BUFFER_SIZE + TIMESTAMP_LEN + LOG_LEVEL_LEN + FILENAME_LEN + LINE_LEN + 6u /* others */;

private:

    String  m_name;     /**< Name of the sink */
    Print*  m_output;   /**< Log sink output */

    LogSinkPrinter(const LogSinkPrinter& sink);
    LogSinkPrinter& operator=(const LogSinkPrinter& sink);

    /**
     * Get a string representation of the given logLevel.
     *
     * @param[in] logLevel The logLevel.
     *
     * @return The severity of the given logLevel as string.
     */
    const char* logLevelToString(const Logging::LogLevel logLevel) const;
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* LOG_SINK_PRINTER_H */

/** @} */