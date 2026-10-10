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
 * @file   tcpbase.h
 * @brief  Stub for the lwIP TCP base definitions
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * The webserver uses the TCP state to show the connection status of a client.
 * The native AsyncTCP implementation reports the very same states.
 *
 * @addtogroup TEST
 *
 * @{
 */

#ifndef LWIP_TCPBASE_H
#define LWIP_TCPBASE_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** TCP state, compatible to the lwIP tcp_state. */
enum tcp_state
{
    CLOSED      = 0, /**< Connection is closed. */
    LISTEN      = 1, /**< Listening for a connection. */
    SYN_SENT    = 2, /**< Connection request sent. */
    SYN_RCVD    = 3, /**< Connection request received. */
    ESTABLISHED = 4, /**< Connection is established. */
    FIN_WAIT_1  = 5, /**< Waiting for the connection termination request. */
    FIN_WAIT_2  = 6, /**< Waiting for the connection termination request. */
    CLOSE_WAIT  = 7, /**< Waiting for the local connection termination request. */
    CLOSING     = 8, /**< Waiting for the connection termination request acknowledge. */
    LAST_ACK    = 9, /**< Waiting for the connection termination request acknowledge. */
    TIME_WAIT   = 10 /**< Waiting to ensure the remote received the acknowledge. */
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* LWIP_TCPBASE_H */

/** @} */
