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
 * @file   WiFiClient.cpp
 * @brief  TCP client for the native environment
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
/* The socket headers must come first, see AsyncTCP.cpp. */
#include "SocketCompat.hpp"

#include "WiFiClient.h"

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

WiFiClient::WiFiClient() :
    Stream(),
    m_socket(-1),
    m_timeout(DEFAULT_TIMEOUT),
    m_peeked(-1)
{
}

WiFiClient::~WiFiClient()
{
    WiFiClient::stop();
}

int WiFiClient::connect(const char* host, uint16_t port)
{
    int result = 0;

    stop();

    if (nullptr != host)
    {
        SocketCompat::Socket sock = SocketCompat::connectTo(host, port, m_timeout);

        if (SocketCompat::INVALID_SOCKET_HANDLE != sock)
        {
            /* The nagle algorithm would delay a short request, which is the
             * usual case for a REST API.
             */
            SocketCompat::setNoDelay(sock, true);

            m_socket = static_cast<int>(sock);
            m_peeked = -1;
            result   = 1;
        }
    }

    return result;
}

uint8_t WiFiClient::connected()
{
    uint8_t isConnected = 0U;

    if (-1 != m_socket)
    {
        /* A peeked byte belongs to the connection, even if the peer closed it
         * in the meantime.
         */
        if (-1 != m_peeked)
        {
            isConnected = 1U;
        }
        else
        {
            SocketCompat::Socket sock = static_cast<SocketCompat::Socket>(m_socket);

            /* A readable socket without any data means, that the peer closed
             * the connection.
             */
            if (false == SocketCompat::waitFor(sock, 0U, false))
            {
                isConnected = 1U;
            }
            else if (0U < SocketCompat::bytesAvailable(sock))
            {
                isConnected = 1U;
            }
            else
            {
                stop();
            }
        }
    }

    return isConnected;
}

void WiFiClient::stop()
{
    if (-1 != m_socket)
    {
        SocketCompat::closeSocket(static_cast<SocketCompat::Socket>(m_socket));
        m_socket = -1;
    }

    m_peeked = -1;
}

int WiFiClient::available()
{
    int available = 0;

    if (-1 != m_socket)
    {
        available = static_cast<int>(SocketCompat::bytesAvailable(static_cast<SocketCompat::Socket>(m_socket)));

        if (-1 != m_peeked)
        {
            ++available;
        }
    }

    return available;
}

int WiFiClient::read()
{
    int     data = -1;
    uint8_t byte = 0U;

    if (0 < read(&byte, sizeof(byte)))
    {
        data = static_cast<int>(byte);
    }

    return data;
}

int WiFiClient::read(uint8_t* buffer, size_t size)
{
    int received = -1;

    if ((nullptr == buffer) ||
        (0U == size))
    {
        /* Guard: nothing to read. */
    }
    else if (-1 == m_socket)
    {
        /* Guard: not connected. */
    }
    else
    {
        size_t offset = 0U;

        /* A peeked byte is returned first, it was already taken from the
         * socket.
         */
        if (-1 != m_peeked)
        {
            buffer[0U] = static_cast<uint8_t>(m_peeked);
            m_peeked   = -1;
            offset     = 1U;
        }

        if (size == offset)
        {
            received = static_cast<int>(offset);
        }
        else
        {
            SocketCompat::Socket sock = static_cast<SocketCompat::Socket>(m_socket);

            if (false == SocketCompat::waitFor(sock, m_timeout, false))
            {
                /* Timeout, the peeked byte may be there anyway. */
                received = (0U < offset) ? static_cast<int>(offset) : 0;
            }
            else
            {
                int result = SocketCompat::receiveData(sock, reinterpret_cast<char*>(&buffer[offset]), size - offset);

                if (0 < result)
                {
                    received = static_cast<int>(offset) + result;
                }
                else if (0 == result)
                {
                    /* The peer closed the connection. */
                    stop();
                    received = (0U < offset) ? static_cast<int>(offset) : 0;
                }
                else if (true == SocketCompat::wouldBlock())
                {
                    received = (0U < offset) ? static_cast<int>(offset) : 0;
                }
                else
                {
                    stop();
                    received = (0U < offset) ? static_cast<int>(offset) : -1;
                }
            }
        }
    }

    return received;
}

int WiFiClient::peek()
{
    int data = m_peeked;

    if ((-1 == data) &&
        (-1 != m_socket))
    {
        uint8_t byte = 0U;

        if (0 < read(&byte, sizeof(byte)))
        {
            m_peeked = static_cast<int>(byte);
            data     = m_peeked;
        }
    }

    return data;
}

size_t WiFiClient::write(uint8_t data)
{
    return write(&data, sizeof(data));
}

size_t WiFiClient::write(const uint8_t* buffer, size_t size)
{
    size_t written = 0U;

    if ((nullptr != buffer) &&
        (-1 != m_socket))
    {
        SocketCompat::Socket sock      = static_cast<SocketCompat::Socket>(m_socket);
        bool                 isAborted = false;

        while ((size > written) &&
               (false == isAborted))
        {
            int result = SocketCompat::sendData(sock, reinterpret_cast<const char*>(&buffer[written]), size - written);

            if (0 < result)
            {
                written += static_cast<size_t>(result);
            }
            else if (true == SocketCompat::wouldBlock())
            {
                /* The transmit buffer of the socket is full, wait until there
                 * is space again.
                 */
                if (false == SocketCompat::waitFor(sock, m_timeout, true))
                {
                    isAborted = true;
                }
            }
            else
            {
                stop();
                isAborted = true;
            }
        }
    }

    return written;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
