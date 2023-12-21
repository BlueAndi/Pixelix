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

/** Command: ping */
static const char*     PING                         = "ping";

/** Command length: ping */
static const size_t    PING_LEN                     = strlen(PING);

/** Command: reset */
static const char*     RESET                        = "reset";

/** Command length: reset */
static const size_t    RESET_LEN                    = strlen(RESET);

/** Command: write wifi passphrase */
static const char*     WRITE_WIFI_PASSPHRASE        = "write wifi passphrase ";

/** Command length: write wifi passphrase */
static const size_t    WRITE_WIFI_PASSPHRASE_LEN    = strlen(WRITE_WIFI_PASSPHRASE);

/** Command: write wifi ssid */
static const char*     WRITE_WIFI_SSID              = "write wifi ssid ";

/** Command length: write wifi ssid */
static const size_t    WRITE_WIFI_SSID_LEN 	        = strlen(WRITE_WIFI_SSID);

/** Command: get ip */
static const char*     GET_IP                       = "get ip";

/** Command length: get ipaddress */
static const size_t    GET_IP_LEN 	                = strlen(GET_IP);

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void MiniTerminal::process()
{
    char    buffer[LOCAL_BUFFER_SIZE];
    size_t  read    = m_stream.readBytes(buffer, LOCAL_BUFFER_SIZE);
    size_t  idx     = 0U;

    /* Process the read input data. */
    while(read > idx)
    {
        bool echoOn = false;

        /* Command finished? */
        if (ASCII_LF == buffer[idx])
        {
            /* Don't echo mechanism, because its too late in case the
                * command may write a result too.
                */
            (void)m_stream.write(buffer[idx]);

            m_input[m_writeIndex] = '\0';

            /* Execute command if there is one. */
            if (0 < m_writeIndex)
            {
                executeCommand(m_input);
            }
            
            /* Reset command line buffer */
            m_writeIndex = 0U;
            m_input[m_writeIndex] = '\0';
        }
        /* Remove the last character from command line? */
        else if ((ASCII_DEL == buffer[idx]) ||
                 (ASCII_BS == buffer[idx]))
        {
            if (0 < m_writeIndex)
            {
                char removeSeq[] =
                {
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
            if ((' ' <= buffer[idx]) &&
                ('~' >= buffer[idx]))
            {
                m_input[m_writeIndex] = buffer[idx];
                ++m_writeIndex;

                echoOn = true;
            }
        }

        if (true == echoOn)
        {
            (void)m_stream.write(buffer[idx]);
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
    if (0 == strcmp(cmdLine, PING))
    {
        cmdPing(&cmdLine[PING_LEN]);
    }
    else if (0 == strcmp(cmdLine, RESET))
    {
        cmdReset(&cmdLine[RESET_LEN]);
    }
    else if (0 == strncmp(cmdLine, WRITE_WIFI_PASSPHRASE, WRITE_WIFI_PASSPHRASE_LEN))
    {
        cmdWriteWifiPassphrase(&cmdLine[WRITE_WIFI_PASSPHRASE_LEN]);
    }
    else if (0 == strncmp(cmdLine, WRITE_WIFI_SSID, WRITE_WIFI_SSID_LEN))
    {
        cmdWriteWifiSSID(&cmdLine[WRITE_WIFI_SSID_LEN]);
    }
    else if (0 == strncmp(cmdLine, GET_IP, GET_IP_LEN))
    {
        cmdGetIPAddress(&cmdLine[GET_IP_LEN]);
    }
    else
    {
        writeError("Unknown command.\n");
    }
}

void MiniTerminal::cmdPing(const char* par)
{
    UTIL_NOT_USED(par);
    writeSuccessful("pong\n");
}

void MiniTerminal::cmdReset(const char* par)
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
    if (nullptr != par)
    {
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
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/