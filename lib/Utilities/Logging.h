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
 *
 * @addtogroup utilities
 *
 * @{
 */

#ifndef LOGGING_H
#define LOGGING_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

#ifndef LOG_FATAL_ENABLE
#define LOG_FATAL_ENABLE    (1)
#endif  /* LOG_FATAL_ENABLE */

#ifndef LOG_ERROR_ENABLE
#define LOG_ERROR_ENABLE    (1)
#endif  /* LOG_ERROR_ENABLE */

#ifndef LOG_WARNING_ENABLE
#define LOG_WARNING_ENABLE  (1)
#endif  /* LOG_WARNING_ENABLE */

#ifndef LOG_INFO_ENABLE
#define LOG_INFO_ENABLE     (1)
#endif  /* LOG_INFO_ENABLE */

#ifndef LOG_DEBUG_ENABLE
#define LOG_DEBUG_ENABLE    (0)
#endif  /* LOG_DEBUG_ENABLE */

#ifndef LOG_TRACE_ENABLE
#define LOG_TRACE_ENABLE    (0)
#endif  /* LOG_TRACE_ENABLE */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <Arduino.h>
#include <stdarg.h>
#include <stdint.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

#if (0 == LOG_FATAL_ENABLE)

    #define LOG_FATAL(...)

#else/* (0 == LOG_FATAL_ENABLE) */

    /** Log fatal error message. */
    #define LOG_FATAL(...)      do{ Logging::getInstance().processLogMessage(__FILE__, __LINE__, Logging::LOG_LEVEL_FATAL, __VA_ARGS__); }while(0)

#endif  /* (0 == LOG_FATAL_ENABLE) */

#if (0 == LOG_ERROR_ENABLE)

    #define LOG_ERROR(...)

#else/* (0 == LOG_ERROR_ENABLE) */

    /** Log error message. */
    #define LOG_ERROR(...)      do{ Logging::getInstance().processLogMessage(__FILE__, __LINE__, Logging::LOG_LEVEL_ERROR, __VA_ARGS__); }while(0)

#endif  /* (0 == LOG_ERROR_ENABLE) */

#if (0 == LOG_WARNING_ENABLE)

    #define LOG_WARNING(...)

#else/* (0 == LOG_WARNING_ENABLE) */

    /** Log warning message. */
    #define LOG_WARNING(...)    do{ Logging::getInstance().processLogMessage(__FILE__, __LINE__, Logging::LOG_LEVEL_WARNING, __VA_ARGS__); }while(0)

#endif  /* (0 == LOG_WARNING_ENABLE) */

#if (0 == LOG_INFO_ENABLE)

    #define LOG_INFO(...)

#else/* (0 == LOG_INFO_ENABLE) */

    /** Log info error message. */
    #define LOG_INFO(...)       do{ Logging::getInstance().processLogMessage(__FILE__, __LINE__, Logging::LOG_LEVEL_INFO, __VA_ARGS__); }while(0)

#endif  /* (0 == LOG_INFO_ENABLE) */

#if (0 == LOG_DEBUG_ENABLE)

    #define LOG_DEBUG(...)

#else  /* (0 == LOG_DEBUG_ENABLE) */

    /** Log debug message. */
    #define LOG_DEBUG(...)      do{ Logging::getInstance().processLogMessage(__FILE__, __LINE__, Logging::LOG_LEVEL_DEBUG, __VA_ARGS__); }while(0)

#endif  /* (0 == LOG_DEBUG_ENABLE) */

#if (0 == LOG_TRACE_ENABLE)

    #define LOG_TRACE(...)

#else/* (0 == LOG_TRACE_ENABLE) */

    /** Log trace message. */
    #define LOG_TRACE(...)      do{ Logging::getInstance().processLogMessage(__FILE__, __LINE__, Logging::LOG_LEVEL_TRACE, __VA_ARGS__); }while(0)

#endif  /* (0 == LOG_TRACE_ENABLE) */

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/* Forward declaration */
class LogSink;

/**
 * Logging class for log messages depending on the previously set log level.
 */
class Logging
{
public:

    /**
     * Enumeration to distinguish between different levels of severity.
     */
    enum LogLevel
    {
        LOG_LEVEL_FATAL = 0,    /**< Any error that is forcing a shutdown of service or application, because there is no way out. */
        LOG_LEVEL_ERROR,        /**< Any error that is fatal for the operating, but not for the service or application. */
        LOG_LEVEL_WARNING,      /**< Anything that shows the user to pay attention, but can be automatically be recovered. */
        LOG_LEVEL_INFO,         /**< General useful information for the user. */
        LOG_LEVEL_DEBUG,        /**< A diagnostic message helpful for the developer. */
        LOG_LEVEL_TRACE         /**< Only used for tracing code. */
    };

    /**
     * A single log message.
     */
    struct Msg
    {
        uint32_t            timestamp;  /**< Timestamp in ms */
        Logging::LogLevel   level;      /**< Log level */
        const char*         filename;   /**< Name of the file where this message is thrown. */
        int                 line;       /**< Line number in the file, where this message is thrown. */
        const char*         str;        /**< Message text */

        /**
         * Initializes a empty message.
         */
        Msg() :
            timestamp(0U),
            level(LOG_LEVEL_INFO),
            filename(nullptr),
            line(0),
            str(nullptr)
        {
        }
    };

    /** The maximum size of the logMessage buffer to get the variable arguments. */
    static const uint16_t MESSAGE_BUFFER_SIZE   = 80U;

    /**
     * Get the Logging instance.
     *
     * @return Logging instance.
     */
    static Logging& getInstance()
    {
        static Logging instance; /* singleton idiom to force initialization in the first usage. */

        return instance;
    }

    /**
     * Register a log sink.
     *
     * @param[in] sink  Log sink
     *
     * @return If successful registered it will return true otherwise false.
     */
    bool registerSink(LogSink* sink);

    /**
     * Unregister a log sink.
     *
     * @param[in] sink  Log sink
     */
    void unregisterSink(LogSink* sink);

    /**
     * Select log sink.
     *
     * @param[in] name Log sink name
     *
     * @return If sink is selected, it will return true otherwise false.
     */
    bool selectSink(const String& name);

    /**
     * Get selected sink.
     *
     * @return Selected sink
     */
    LogSink* getSelectedSink();

    /**
     * Set the logLevel.
     *
     * @param[in] logLevel The new logLevel.
    */
    void setLogLevel(const LogLevel logLevel);

    /**
     * Get the current logLevel.
     *
     * @return The current logLevel.
    */
    LogLevel getLogLevel() const;

    /**
     * Write a formatable logMessage to the current output,
     * if the severity is >= the current logLevel, otherwise the logMessage is discarded.
     *
     * @param[in] file              Name of the file
     * @param[in] line              Line number in the file
     * @param[in] messageLogLevel   The logLevel.
     * @param[in] format            The format of the variable arguments.
     * @param[in] ...               The variable arguments.
     *
     * @note The max. size of a logMessage is restricted by MESSAGE_BUFFER_SIZE.
     */
    void processLogMessage(const char* file, int line, const LogLevel messageLogLevel, const char* format, ...);

    /**
     * Write a formatable logMessage to the current output,
     * if the severity is >= the current logLevel, otherwise the logMessage is discarded.
     *
     * @param[in] file              Name of the file
     * @param[in] line              Line number in the file
     * @param[in] messageLogLevel   The logLevel.
     * @param[in] message           The message as string.
     *
     * @note The max. size of a logMessage is restricted by MESSAGE_BUFFER_SIZE.
     */
    void processLogMessage(const char* file, int line, const LogLevel messageLogLevel, const String& message);

    /**
     * Write a formatable logMessage to the current output,
     * if the severity is >= the current logLevel, otherwise the logMessage is discarded.
     *
     * @param[in] timestamp         Timestamp in ms.
     * @param[in] logger            Logger name.
     * @param[in] messageLogLevel   The logLevel.
     * @param[in] message           The message as string.
     *
     * @note The max. size of a logMessage is restricted by MESSAGE_BUFFER_SIZE.
     */
    void processLogMessage(uint32_t timestamp, const String& logger, const LogLevel messageLogLevel, const String& message);

    /** Number of supported log sinks. */
    static const uint8_t MAX_SINKS = 2U;

private:

    /** The current log level. */
    LogLevel    m_currentLogLevel;

    /** List of log sinks */
    LogSink*    m_sinks[MAX_SINKS];

    /** Active sink */
    LogSink*    m_selectedSink;

    /**
     * Checks wether the given severity of a logMessage is enabled to be printed.
     *
     * @param[in] logLevel The logLevel that has to be checked.
     *
     * @return If the severity is enabled, it will return true otherwise false.
    */
    bool isSeverityEnabled(LogLevel logLevel) const;

    /**
     * Extracts the basename of a file from a given path.
     *
     * @param[in] path The path as retrieved from __FILE__.
     *
     * @return The basename.
    */
    const char* getBaseNameFromPath(const char* path) const;

    /**
     * Construct Logging.
     */
    Logging() :
        m_currentLogLevel(LOG_LEVEL_INFO),
        m_sinks(),
        m_selectedSink(nullptr)
    {
        uint8_t index = 0U;

        for(index = 0U; index < MAX_SINKS; ++index)
        {
            m_sinks[index] = nullptr;
        }
    }

    /**
     * Destroys Logging.
     */
    ~Logging()
    {

    }

    /* Prevent copying */
    Logging(const Logging&);
    Logging&operator=(const Logging&);
};

/**
 * Logging sink interface.
 */
class LogSink
{
public:

    /**
     * Constructs the log sink.
     */
    LogSink()
    {
    }

    /**
     * Destroys the log sink.
     */
    virtual ~LogSink()
    {
    }

    /**
     * Get sink name.
     *
     * @return Name of the sink.
     */
    virtual const String& getName() const = 0;

    /**
     * Send a log message to this sink.
     *
     * @param[in] msg   Log message
     */
    virtual void send(const Logging::Msg& msg) = 0;

private:
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* LOGGING_H */

/** @} */