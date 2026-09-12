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
 * @file   SocketCompat.hpp
 * @brief  Socket abstraction for Windows and Linux
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * The native environment shall build on Windows (MinGW, which provides winsock)
 * as well as on Linux (which provides the BSD sockets), e.g. in the CI. The
 * differences are hidden here, so the AsyncTCP implementation itself is free of
 * any platform switch.
 *
 * @addtogroup TEST
 *
 * @{
 */

#ifndef SOCKET_COMPAT_HPP
#define SOCKET_COMPAT_HPP

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32

#include <winsock2.h>
#include <ws2tcpip.h>

#else /* _WIN32 */

#include <sys/socket.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#endif /* _WIN32 */

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Platform differences of the socket API.
 */
namespace SocketCompat
{

#ifdef _WIN32

/** Socket handle type. */
typedef SOCKET Socket;

/** Value of an invalid socket. */
static const Socket INVALID_SOCKET_HANDLE = INVALID_SOCKET;

#else /* _WIN32 */

/** Socket handle type. */
typedef int Socket;

/** Value of an invalid socket. */
static const Socket INVALID_SOCKET_HANDLE = -1;

#endif /* _WIN32 */

/******************************************************************************
 * Functions
 *****************************************************************************/

/**
 * Initialize the socket API.
 * Only Windows requires it, on Linux it does nothing.
 *
 * @return If successful initialized, it will return true otherwise false.
 */
static inline bool init()
{
    bool isSuccessful = true;

#ifdef _WIN32

    static bool isInitialized = false;

    if (false == isInitialized)
    {
        WSADATA wsaData;

        if (0 != WSAStartup(MAKEWORD(2, 2), &wsaData))
        {
            isSuccessful = false;
        }
        else
        {
            isInitialized = true;
        }
    }

#endif /* _WIN32 */

    return isSuccessful;
}

/**
 * Close the given socket.
 *
 * @param[in] sock  The socket.
 */
static inline void closeSocket(Socket sock)
{
    if (INVALID_SOCKET_HANDLE != sock)
    {
#ifdef _WIN32
        (void)closesocket(sock);
#else  /* _WIN32 */
        (void)::close(sock);
#endif /* _WIN32 */
    }
}

/**
 * Set the given socket to non-blocking mode.
 *
 * @param[in] sock  The socket.
 *
 * @return If successful set, it will return true otherwise false.
 */
static inline bool setNonBlocking(Socket sock)
{
    bool isSuccessful = false;

#ifdef _WIN32

    u_long mode  = 1UL;

    isSuccessful = (0 == ioctlsocket(sock, FIONBIO, &mode));

#else /* _WIN32 */

    int flags = fcntl(sock, F_GETFL, 0);

    if (0 <= flags)
    {
        isSuccessful = (0 == fcntl(sock, F_SETFL, flags | O_NONBLOCK));
    }

#endif /* _WIN32 */

    return isSuccessful;
}

/**
 * Would the last operation block?
 * It is used to distinguish a real error from a non-blocking socket which has
 * nothing to do right now.
 *
 * @return If the operation would block, it will return true otherwise false.
 */
static inline bool wouldBlock()
{
#ifdef _WIN32
    return (WSAEWOULDBLOCK == WSAGetLastError());
#else  /* _WIN32 */
    return ((EWOULDBLOCK == errno) || (EAGAIN == errno) || (EINTR == errno));
#endif /* _WIN32 */
}

/**
 * Is a connection establishment of a non-blocking socket in progress?
 *
 * A non-blocking connect() reports this instead of blocking until the peer
 * answered.
 *
 * @return If the connection establishment is in progress, it will return true
 *         otherwise false.
 */
static inline bool connectInProgress()
{
#ifdef _WIN32
    return (WSAEINPROGRESS == WSAGetLastError());
#else  /* _WIN32 */
    return (EINPROGRESS == errno);
#endif /* _WIN32 */
}

/**
 * Enable or disable the nagle algorithm.
 *
 * @param[in] sock      The socket.
 * @param[in] noDelay   Disable the nagle algorithm or not.
 */
static inline void setNoDelay(Socket sock, bool noDelay)
{
    int value = (false == noDelay) ? 0 : 1;

    (void)setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<const char*>(&value), sizeof(value));
}

/**
 * Allow to reuse the address of the socket.
 *
 * @param[in] sock  The socket.
 */
static inline void setReuseAddr(Socket sock)
{
    int value = 1;

    (void)setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&value), sizeof(value));
}

/**
 * Send data.
 *
 * @param[in] sock  The socket.
 * @param[in] data  Data buffer
 * @param[in] size  Data buffer size in byte
 *
 * @return Number of sent bytes or a negative value in case of an error.
 */
static inline int sendData(Socket sock, const char* data, size_t size)
{
    return static_cast<int>(::send(sock, data, static_cast<int>(size), 0));
}

/**
 * Receive data.
 *
 * @param[in]  sock The socket.
 * @param[out] data Data buffer
 * @param[in]  size Data buffer size in byte
 *
 * @return Number of received bytes, 0 if the peer closed the connection or a
 *         negative value in case of an error.
 */
static inline int receiveData(Socket sock, char* data, size_t size)
{
    return static_cast<int>(::recv(sock, data, static_cast<int>(size), 0));
}

/**
 * Get the number of bytes which can be read without blocking.
 *
 * @param[in] sock  The socket.
 *
 * @return Number of available bytes.
 */
static inline size_t bytesAvailable(Socket sock)
{
    size_t available = 0U;

#ifdef _WIN32

    u_long count = 0UL;

    if (0 == ioctlsocket(sock, FIONREAD, &count))
    {
        available = static_cast<size_t>(count);
    }

#else /* _WIN32 */

    int count = 0;

    if (0 == ioctl(sock, FIONREAD, &count))
    {
        if (0 < count)
        {
            available = static_cast<size_t>(count);
        }
    }

#endif /* _WIN32 */

    return available;
}

/**
 * Wait until the socket is readable or writable.
 *
 * @param[in] sock          The socket.
 * @param[in] timeout       Max. time to wait in ms.
 * @param[in] forWriting    Wait for writability instead of readability.
 *
 * @return If the socket is ready, it will return true otherwise false.
 */
static inline bool waitFor(Socket sock, uint32_t timeout, bool forWriting)
{
    fd_set         set;
    struct timeval tv;
    int            result = 0;

    FD_ZERO(&set);
    FD_SET(sock, &set);

    tv.tv_sec  = static_cast<long>(timeout / 1000U);
    tv.tv_usec = static_cast<long>((timeout % 1000U) * 1000U);

    if (false == forWriting)
    {
        result = ::select(static_cast<int>(sock) + 1, &set, nullptr, nullptr, &tv);
    }
    else
    {
        result = ::select(static_cast<int>(sock) + 1, nullptr, &set, nullptr, &tv);
    }

    return (0 < result);
}

/**
 * Get the error of a socket, which was connected in non-blocking mode.
 *
 * @param[in] sock  The socket.
 *
 * @return 0 if the connection was established, otherwise the error code.
 */
static inline int connectError(Socket sock)
{
    int       error  = 0;
    socklen_t length = static_cast<socklen_t>(sizeof(error));

    if (0 != getsockopt(sock, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&error), &length))
    {
        error = -1;
    }

    return error;
}

/**
 * Connect to the given host, with a timeout.
 *
 * The name resolution is part of it, because the host may be given by name or
 * by address.
 *
 * @param[in] host      Host name or address.
 * @param[in] port      Port of the host.
 * @param[in] timeout   Max. time to wait for the connection in ms.
 *
 * @return The connected socket or INVALID_SOCKET_HANDLE in case of an error.
 */
static inline Socket connectTo(const char* host, uint16_t port, uint32_t timeout)
{
    Socket           sock = INVALID_SOCKET_HANDLE;
    struct addrinfo  hints;
    struct addrinfo* result = nullptr;
    char             service[6];

    if ((nullptr == host) ||
        (false == init()))
    {
        return INVALID_SOCKET_HANDLE;
    }

    (void)snprintf(service, sizeof(service), "%u", static_cast<unsigned int>(port));

    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if (0 != getaddrinfo(host, service, &hints, &result))
    {
        return INVALID_SOCKET_HANDLE;
    }

    for (struct addrinfo* it = result; (nullptr != it) && (INVALID_SOCKET_HANDLE == sock); it = it->ai_next)
    {
        Socket candidate = ::socket(it->ai_family, it->ai_socktype, it->ai_protocol);

        if (INVALID_SOCKET_HANDLE == candidate)
        {
            continue;
        }

        /* Non-blocking, otherwise the timeout of the operating system applies,
         * which is much longer than the caller expects.
         */
        if (false == setNonBlocking(candidate))
        {
            closeSocket(candidate);
            continue;
        }

        if (0 == ::connect(candidate, it->ai_addr, static_cast<int>(it->ai_addrlen)))
        {
            sock = candidate;
        }
        else if ((true == wouldBlock()) ||
                 (true == connectInProgress()))
        {
            if ((true == waitFor(candidate, timeout, true)) &&
                (0 == connectError(candidate)))
            {
                sock = candidate;
            }
            else
            {
                closeSocket(candidate);
            }
        }
        else
        {
            closeSocket(candidate);
        }
    }

    freeaddrinfo(result);

    return sock;
}

} /* namespace SocketCompat */

#endif /* SOCKET_COMPAT_HPP */

/** @} */
