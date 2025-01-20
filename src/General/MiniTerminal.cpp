/* MIT License
 *
 * Copyright (c) 2019 - 2025 Andreas Merkle <web@blue-andi.de>
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
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "MiniTerminal.h"
#include "RestartState.h"

#include <Util.h>
#include <SettingsService.h>
#include <StateMachine.hpp>
#include <WiFi.h>
#include <ErrorState.h>

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

/** Command: restart */
static const char RESTART[]                                  = "restart";

/** Command: write wifi passphrase */
static const char WRITE_WIFI_PASSPHRASE[]                    = "write wifi passphrase ";

/** Command: write wifi ssid */
static const char WRITE_WIFI_SSID[]                          = "write wifi ssid ";

/** Command: get ip */
static const char GET_IP[]                                   = "get ip";

/** Command: status */
static const char GET_STATUS[]                               = "get status";

/** Command: help */
static const char                 HELP[]                     = "help";

const MiniTerminal::CmdTableEntry MiniTerminal::m_cmdTable[] = {
    { RESTART, &MiniTerminal::cmdRestart },
    { WRITE_WIFI_PASSPHRASE, &MiniTerminal::cmdWriteWifiPassphrase },
    { WRITE_WIFI_SSID, &MiniTerminal::cmdWriteWifiSSID },
    { GET_IP, &MiniTerminal::cmdGetIPAddress },
    { GET_STATUS, &MiniTerminal::cmdGetStatus },
    { HELP, &MiniTerminal::cmdHelp },
};

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void MiniTerminal::process()
{
    char   buffer[LOCAL_BUFFER_SIZE];
    size_t read = m_stream.readBytes(buffer, LOCAL_BUFFER_SIZE);
    size_t idx  = 0U;

    /* Process the read input data. */
    while (read > idx)
    {
        char currentChar = buffer[idx];

        /* Command finished? */
        if (ASCII_LF == currentChar)
        {
            /* Don't echo mechanism, because its too late in case the
             * command may write a result too.
             */
            (void)m_stream.write(currentChar);

            m_input[m_writeIndex] = '\0';

            /* Execute command if there is one. */
            if (0 < m_writeIndex)
            {
                executeCommand(m_input);
            }

            /* Reset command line buffer */
            m_writeIndex          = 0U;
            m_input[m_writeIndex] = '\0';
        }
        /* Remove the last character from command line? */
        else if ((ASCII_DEL == currentChar) ||
                 (ASCII_BS == currentChar))
        {
            if (0 < m_writeIndex)
            {
                static const char removeSeq[] = {
                    ASCII_BS,
                    ASCII_SP,
                    ASCII_BS
                };

                (void)m_stream.write(removeSeq, sizeof(removeSeq));
                --m_writeIndex;
            }
        }
        /* Command line buffer not full yet (consider string termination)? */
        else if (INPUT_BUFFER_SIZE > (m_writeIndex + 1U))
        {
            /* Valid character? */
            if ((' ' <= currentChar) &&
                ('~' >= currentChar))
            {
                m_input[m_writeIndex] = currentChar;
                ++m_writeIndex;
                (void)m_stream.write(currentChar);
            }
        }

        ++idx;
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void MiniTerminal::writeSuccessful(const char* result)
{
    if (nullptr != result)
    {
        (void)m_stream.write(result);
    }

    (void)m_stream.write("OK\n");
}

void MiniTerminal::writeError(const char* result)
{
    if (nullptr != result)
    {
        (void)m_stream.write(result);
    }

    (void)m_stream.write("ERR\n");
}

void MiniTerminal::executeCommand(const char* cmdLine)
{
    uint32_t idx = 0U;

    for (idx = 0U; UTIL_ARRAY_NUM(m_cmdTable) > idx; ++idx)
    {
        const CmdTableEntry entry = m_cmdTable[idx];
        const size_t        len   = strlen(entry.cmdStr);

        if (0 == strncmp(cmdLine, entry.cmdStr, len))
        {
            (this->*entry.handler)(&cmdLine[len]);
            break;
        }
    }

    if (UTIL_ARRAY_NUM(m_cmdTable) == idx)
    {
        writeError("Unknown command.\n");
    }
}

void MiniTerminal::cmdRestart(const char* par)
{
    UTIL_NOT_USED(par);
    m_isRestartRequested = true;
    writeSuccessful();
}

void MiniTerminal::cmdWriteWifiPassphrase(const char* par)
{
    if (nullptr != par)
    {
        SettingsService& settings = SettingsService::getInstance();

        if (false == settings.open(false))
        {
            writeError();
        }
        else
        {
            KeyValueString& wifiPassword = settings.getWifiPassphrase();

            wifiPassword.setValue(par);
            settings.close();

            writeSuccessful();
        }
    }
}

void MiniTerminal::cmdWriteWifiSSID(const char* par)
{
    if (nullptr != par)
    {
        SettingsService& settings = SettingsService::getInstance();

        if (false == settings.open(false))
        {
            writeError();
        }
        else
        {
            KeyValueString& wifiSSID = settings.getWifiSSID();

            wifiSSID.setValue(par);
            settings.close();

            writeSuccessful();
        }
    }
}

void MiniTerminal::cmdGetIPAddress(const char* par)
{
    UTIL_NOT_USED(par);

    String result;

    if (WIFI_MODE_AP == WiFi.getMode())
    {
        result = WiFi.softAPIP().toString();
    }
    else
    {
        result = WiFi.localIP().toString();
    }

    result += "\n";

    writeSuccessful(result.c_str());
}

void MiniTerminal::cmdGetStatus(const char* par)
{
    UTIL_NOT_USED(par);

    ErrorState::ErrorId status = ErrorState::getInstance().getErrorId();
    String              result;

    result += static_cast<int32_t>(status);
    result += "\n";

    writeSuccessful(result.c_str());
}

void MiniTerminal::cmdHelp(const char* par)
{
    UTIL_NOT_USED(par);

    m_stream.write("Supported commands:\n");

    for (size_t idx = 0U; UTIL_ARRAY_NUM(m_cmdTable) > idx; ++idx)
    {
        (void)m_stream.write("    ");
        (void)m_stream.write(m_cmdTable[idx].cmdStr);
        (void)m_stream.write("\n");
    }

    writeSuccessful();
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/