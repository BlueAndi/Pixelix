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
 * @brief  Mini-Terminal
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup common
 *
 * @{
 */

#ifndef MINI_TERMINAL_H
#define MINI_TERMINAL_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <Arduino.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * The mini terminal provides the possiblity to interact with the system
 * by any stream.
 * 
 * Supported:
 * - Write wifi SSID setting
 * - Write wifi passphrase setting
 */
class MiniTerminal
{
public:

    /**
     * Construct the mini terminal instance.
     * 
     * @param[in] stream    In-/Out-stream
     */
    MiniTerminal(Stream& stream) :
        m_stream(stream),
        m_input(),
        m_writeIndex(0U),
        m_isRestartRequested(false)
    {
        /* Don't wait for any input. */
        m_stream.setTimeout(0U);
    }

    /**
     * Destroys the instance.
     */
    ~MiniTerminal()
    {
    }

    /**
     * Process the mini terminal.
     * It will handle the stream input.
     */
    void process();

    /**
     * Is restart requested?
     * 
     * @return If restart requested, it will return true otherwise false.
     */
    bool isRestartRequested()
    {
        bool isRestartRequested = m_isRestartRequested;

        m_isRestartRequested = false;

        return isRestartRequested;
    }

private:

    static const char   ASCII_BS            = 8;    /**< ASCII backspace value */
    static const char   ASCII_LF            = 10;   /**< ASCII line feed value */
    static const char   ASCII_SP            = 32;   /**< ASCII space value */
    static const char   ASCII_DEL           = 127;  /**< ASCII delete value */
    static const size_t LOCAL_BUFFER_SIZE   = 10U;  /**< Buffer size in byte to read during processing. */
    static const size_t INPUT_BUFFER_SIZE   = 80U;  /**< Buffer size of one input command line in byte. */

    Stream& m_stream;                   /**< In/Out-stream. */
    char    m_input[INPUT_BUFFER_SIZE]; /**< Input command line buffer. */
    size_t  m_writeIndex;               /**< Write index to the command line buffer. */
    bool    m_isRestartRequested;       /**< Restart requested? */

    /**
     * Write successful response.
     * 
     * @param[in] result    A result may be printed additional.
     */
    void writeSuccessful(const char* result = nullptr);

    /**
     * Write error response.
     * 
     * @param[in] result    A result may be printed additional.
     */
    void writeError(const char* result = nullptr);

    /**
     * Execute command.
     * 
     * @param[in] cmdLine   Command line
     */
    void executeCommand(const char* cmdLine);

    /**
     * Ping command.
     * 
     * @param[in] par   Parameter
     */
    void cmdPing(const char* par);

    /**
     * Reset the device.
     * 
     * @param[in] par   Parameter
     */
    void cmdReset(const char* par);

    /**
     * Write wifi passphrase.
     * 
     * @param[in] par   Parameter
     */
    void cmdWriteWifiPassphrase(const char* par);

    /**
     * Write wifi SSID.
     * 
     * @param[in] par   Parameter
     */
    void cmdWriteWifiSSID(const char* par);

    /**
     * Get the IP-address.
     * 
     * @param[in] par   Parameter
     */
    void cmdGetIPAddress(const char* par);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* MINI_TERMINAL_H */

/** @} */