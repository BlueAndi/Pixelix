/* MIT License
 *
 * Copyright (c) 2019 - 2026 Andreas Merkle <web@blue-andi.de>
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
 * @file   ImprovSerialService.cpp
 * @brief  Improv Wi-Fi Serial provisioning service
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "ImprovSerialService.h"

#include "RestartMgr.h"

#include <Logging.h>
#include <SettingsService.h>
#include <esp_log.h>
#include <string.h>

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/** Stringify a macro value after expanding it. */
#define IMPROV_STRINGIFY_INNER(value) #value

/** Expand and stringify a build macro. */
#define IMPROV_STRINGIFY(value) IMPROV_STRINGIFY_INNER(value)

/******************************************************************************
 * Types and classes
 *****************************************************************************/

/******************************************************************************
 * Prototypes
 *****************************************************************************/

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/** Improv Serial frame header. */
static const uint8_t IMPROV_HEADER[6U] = { 'I', 'M', 'P', 'R', 'O', 'V' };

/******************************************************************************
 * Public Methods
 *****************************************************************************/

ImprovSerialService::ImprovSerialService(Stream& stream, MiniTerminal& miniTerminal) :
    m_stream(stream),
    m_miniTerminal(miniTerminal),
    m_frame(),
    m_frameLength(0U),
    m_expectedFrameLength(0U),
    m_headerMatchLength(0U),
    m_previousButtonStates{ BUTTON_STATE_UNKNOWN, BUTTON_STATE_UNKNOWN, BUTTON_STATE_UNKNOWN },
    m_authorizedAt(0U),
    m_lastActivityAt(0U),
    m_state(STATE_AWAITING_AUTH),
    m_isAuthorized(false),
    m_isSessionActive(false),
    m_isLoggingSuppressed(false),
    m_ignoreLineFeed(false),
    m_previousLogSink()
{
}

void ImprovSerialService::process(IButtonDrv& buttonDrv)
{
    while (0 < m_stream.available())
    {
        const int input = m_stream.read();

        if (0 <= input)
        {
            consumeByte(static_cast<uint8_t>(input));
        }
    }

    processButtons(buttonDrv);
    processTimeouts();
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void ImprovSerialService::consumeByte(uint8_t value)
{
    bool isForwarded = false;

    if (true == m_ignoreLineFeed)
    {
        m_ignoreLineFeed = false;
        if ('\n' == value)
        {
            isForwarded = true;
        }
    }

    if (false == isForwarded)
    {
        if (0U < m_expectedFrameLength)
        {
            if (FRAME_MAX_SIZE > m_frameLength)
            {
                m_frame[m_frameLength] = value;
                ++m_frameLength;

                if (FRAME_HEADER_SIZE == m_frameLength)
                {
                    m_expectedFrameLength = FRAME_HEADER_SIZE + static_cast<size_t>(m_frame[8]) + 1U;
                }

                if ((0U < m_expectedFrameLength) && (m_expectedFrameLength == m_frameLength))
                {
                    processFrame();
                    resetFrame();
                    m_ignoreLineFeed = true;
                }
            }
            else
            {
                resetFrame();
            }
        }
        else if (0U < m_headerMatchLength)
        {
            if (value == IMPROV_HEADER[m_headerMatchLength])
            {
                m_frame[m_frameLength] = value;
                ++m_frameLength;
                ++m_headerMatchLength;

                if (HEADER_SIZE == m_headerMatchLength)
                {
                    m_expectedFrameLength = FRAME_HEADER_SIZE;
                    m_lastActivityAt      = millis();
                    enterProtocolMode();
                }
            }
            else
            {
                m_miniTerminal.process(m_frame, m_frameLength);
                resetFrame();

                if ('I' == value)
                {
                    m_frame[0]          = value;
                    m_frameLength       = 1U;
                    m_headerMatchLength = 1U;
                }
                else
                {
                    m_miniTerminal.process(&value, 1U);
                }
            }
        }
        else if ('I' == value)
        {
            m_frame[0]          = value;
            m_frameLength       = 1U;
            m_headerMatchLength = 1U;
        }
        else
        {
            m_miniTerminal.process(&value, 1U);
        }
    }
}

void ImprovSerialService::processFrame()
{
    uint8_t checksum = 0U;
    size_t  index    = 0U;

    while ((m_frameLength - 1U) > index)
    {
        checksum = static_cast<uint8_t>(checksum + m_frame[index]);
        ++index;
    }

    if ((checksum == m_frame[m_frameLength - 1U]) &&
        (SERIAL_VERSION == m_frame[6]) &&
        (TYPE_RPC == m_frame[7]))
    {
        m_lastActivityAt = millis();
        sendError(ERROR_NONE);
        processRpc(&m_frame[FRAME_HEADER_SIZE], static_cast<size_t>(m_frame[8]));
    }
    else if (checksum != m_frame[m_frameLength - 1U])
    {
        sendError(ERROR_INVALID_RPC);
    }
    else
    {
        sendError(ERROR_INVALID_RPC);
    }
}

void ImprovSerialService::processRpc(const uint8_t* data, size_t dataLength)
{
    if ((nullptr == data) || (2U > dataLength) ||
        (dataLength != (static_cast<size_t>(data[1]) + 2U)))
    {
        sendError(ERROR_INVALID_RPC);
    }
    else if (0x01U == data[0])
    {
        const size_t ssidLength = (3U <= dataLength) ? static_cast<size_t>(data[2]) : 0U;

        if ((false == m_isAuthorized) || (STATE_AUTHORIZED != m_state))
        {
            sendError(ERROR_NOT_AUTHORIZED);
        }
        else if ((3U > dataLength) || (0U == ssidLength) || (32U < ssidLength) ||
                 ((ssidLength + 4U) > dataLength))
        {
            sendError(ERROR_INVALID_RPC);
        }
        else
        {
            const size_t passphraseLengthIndex = 3U + ssidLength;
            const size_t passphraseLength      = static_cast<size_t>(data[passphraseLengthIndex]);
            const size_t expectedLength        = passphraseLengthIndex + passphraseLength + 1U;

            if ((64U < passphraseLength) || (expectedLength != dataLength))
            {
                sendError(ERROR_INVALID_RPC);
            }
            else
            {
                char   ssid[33U]       = { 0 };
                char   passphrase[65U] = { 0 };
                size_t index           = 0U;

                while (ssidLength > index)
                {
                    ssid[index] = static_cast<char>(data[3U + index]);
                    ++index;
                }

                index = 0U;
                while (passphraseLength > index)
                {
                    passphrase[index] = static_cast<char>(data[passphraseLengthIndex + 1U + index]);
                    ++index;
                }

                SettingsService& settings = SettingsService::getInstance();

                if (false == settings.open(false))
                {
                    sendError(ERROR_UNKNOWN);
                }
                else
                {
                    settings.getWifiSSID().setValue(String(ssid));
                    settings.getWifiPassphrase().setValue(String(passphrase));
                    settings.close();

                    if (RestartMgr::RESTART_REQ_STATUS_OK == RestartMgr::getInstance().reqRestart(RESTART_DELAY_MS, false))
                    {
                        m_state        = STATE_PROVISIONING;
                        m_isAuthorized = false;

                        sendEmptyRpcResponse(data[0]);
                        sendState(m_state);
                    }
                    else
                    {
                        sendError(ERROR_UNKNOWN);
                    }
                }
            }
        }
    }
    else if (0x02U == data[0])
    {
        if (0U != data[1])
        {
            sendError(ERROR_INVALID_RPC);
        }
        else
        {
            sendState(m_state);
        }
    }
    else if (0x03U == data[0])
    {
        if (0U != data[1])
        {
            sendError(ERROR_INVALID_RPC);
        }
        else
        {
            sendDeviceInfo();
        }
    }
    else
    {
        sendError(ERROR_UNKNOWN_RPC);
    }
}

void ImprovSerialService::processButtons(IButtonDrv& buttonDrv)
{
    uint8_t buttonIndex = 0U;

    while (BUTTON_ID_CNT > buttonIndex)
    {
        const ButtonId    buttonId    = static_cast<ButtonId>(buttonIndex);
        const ButtonState buttonState = buttonDrv.getState(buttonId);

        if ((BUTTON_STATE_RELEASED == m_previousButtonStates[buttonIndex]) &&
            (BUTTON_STATE_PRESSED == buttonState))
        {
            m_isAuthorized = true;
            m_state        = STATE_AUTHORIZED;
            m_authorizedAt = millis();

            if (true == m_isSessionActive)
            {
                sendState(m_state);
            }
        }

        m_previousButtonStates[buttonIndex] = buttonState;
        ++buttonIndex;
    }
}

void ImprovSerialService::processTimeouts()
{
    const uint32_t currentTime = millis();

    if ((true == m_isAuthorized) && ((currentTime - m_authorizedAt) >= AUTH_TIMEOUT_MS))
    {
        m_isAuthorized = false;
        m_state        = STATE_AWAITING_AUTH;

        if (true == m_isSessionActive)
        {
            sendState(m_state);
        }
    }

    if ((true == m_isLoggingSuppressed) && ((currentTime - m_lastActivityAt) >= SESSION_TIMEOUT_MS))
    {
        leaveProtocolMode();
    }
}

void ImprovSerialService::sendState(uint8_t state)
{
    sendPacket(TYPE_CURRENT_STATE, &state, 1U);
}

void ImprovSerialService::sendError(uint8_t error)
{
    sendPacket(TYPE_ERROR_STATE, &error, 1U);
}

void ImprovSerialService::sendEmptyRpcResponse(uint8_t command)
{
    const uint8_t response[] = { command, 0U };

    sendPacket(TYPE_RPC_RESPONSE, response, sizeof(response));
}

void ImprovSerialService::sendDeviceInfo()
{
#if defined(SW_VERSION)
    static const char FIRMWARE_VERSION[] = IMPROV_STRINGIFY(SW_VERSION);
#else
    static const char FIRMWARE_VERSION[] = "unknown";
#endif

#if defined(CONFIG_IDF_TARGET_ESP32S3)
    static const char CHIP_FAMILY[] = "esp32-s3";
#elif defined(CONFIG_IDF_TARGET_ESP32S2)
    static const char CHIP_FAMILY[] = "esp32-s2";
#else
    static const char CHIP_FAMILY[] = "esp32";
#endif

    static const char FIRMWARE_NAME[] = "Pixelix";
    static const char DEVICE_NAME[]   = "Pixelix";
    uint8_t           response[128U]  = { 0U };
    size_t            position        = 2U;
    const char* const info[]          = { FIRMWARE_NAME, FIRMWARE_VERSION, CHIP_FAMILY, DEVICE_NAME };
    uint8_t           index           = 0U;

    response[0]                       = 0x03U;

    while (4U > index)
    {
        const size_t length = strlen(info[index]);

        if ((127U > position) && (length <= (127U - position - 1U)))
        {
            response[position] = static_cast<uint8_t>(length);
            ++position;

            (void)memcpy(&response[position], info[index], length);
            position += length;
        }

        ++index;
    }

    response[1] = static_cast<uint8_t>(position - 2U);
    sendPacket(TYPE_RPC_RESPONSE, response, position);
}

void ImprovSerialService::sendPacket(uint8_t type, const uint8_t* data, size_t dataLength)
{
    if ((nullptr != data) && (255U >= dataLength))
    {
        uint8_t packet[FRAME_MAX_SIZE] = { 0U };
        size_t  position               = 0U;
        uint8_t checksum               = 0U;
        size_t  index                  = 0U;

        while (HEADER_SIZE > index)
        {
            packet[position] = IMPROV_HEADER[index];
            checksum         = static_cast<uint8_t>(checksum + packet[position]);
            ++position;
            ++index;
        }

        packet[position] = SERIAL_VERSION;
        checksum         = static_cast<uint8_t>(checksum + packet[position]);
        ++position;
        packet[position] = type;
        checksum         = static_cast<uint8_t>(checksum + packet[position]);
        ++position;
        packet[position] = static_cast<uint8_t>(dataLength);
        checksum         = static_cast<uint8_t>(checksum + packet[position]);
        ++position;

        index = 0U;
        while (dataLength > index)
        {
            packet[position] = data[index];
            checksum         = static_cast<uint8_t>(checksum + packet[position]);
            ++position;
            ++index;
        }

        packet[position] = checksum;
        ++position;
        (void)m_stream.write(packet, position);
    }
}

void ImprovSerialService::enterProtocolMode()
{
    if (false == m_isLoggingSuppressed)
    {
        LogSink* const selectedSink = Logging::getInstance().getSelectedSink();

        if (nullptr != selectedSink)
        {
            m_previousLogSink = selectedSink->getName();
        }

        (void)Logging::getInstance().selectSink("Websocket");
        esp_log_level_set("*", ESP_LOG_NONE);
        m_isLoggingSuppressed = true;
    }

    m_isSessionActive = true;
}

void ImprovSerialService::leaveProtocolMode()
{
    if (true == m_isLoggingSuppressed)
    {
        if (0U < m_previousLogSink.length())
        {
            (void)Logging::getInstance().selectSink(m_previousLogSink);
        }

        esp_log_level_set("*", CONFIG_ESP_LOG_SEVERITY);
        m_isLoggingSuppressed = false;
        m_isSessionActive     = false;
        m_previousLogSink     = "";
    }
}

void ImprovSerialService::resetFrame()
{
    m_frameLength         = 0U;
    m_expectedFrameLength = 0U;
    m_headerMatchLength   = 0U;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/