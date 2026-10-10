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
 * @file   ImprovSerialService.h
 * @brief  Improv Wi-Fi Serial provisioning service
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup APP_LAYER
 *
 * @{
 */

#ifndef IMPROV_SERIAL_SERVICE_H
#define IMPROV_SERIAL_SERVICE_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "MiniTerminal.h"

#include <Arduino.h>
#include <IButtonDrv.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Routes Improv Serial packets and legacy terminal input on one stream.
 */
class ImprovSerialService
{
public:

    /**
     * Construct the service.
     *
     * @param[in] stream      Serial stream shared with the terminal
     * @param[in] miniTerminal Text terminal receiving non-Improv input
     */
    ImprovSerialService(Stream& stream, MiniTerminal& miniTerminal);

    /**
     * Process serial input and physical button state changes.
     *
     * @param[in] buttonDrv Button driver initialized by the board
     */
    void process(IButtonDrv& buttonDrv);

private:

    static const size_t   FRAME_MAX_SIZE       = 265U;   /**< Largest supported serial frame. */
    static const size_t   HEADER_SIZE          = 6U;     /**< Improv serial signature length. */
    static const size_t   FRAME_HEADER_SIZE    = 9U;     /**< Signature and fixed frame fields. */
    static const uint8_t  SERIAL_VERSION       = 1U;     /**< Supported protocol version. */
    static const uint8_t  TYPE_CURRENT_STATE   = 0x01U;  /**< Current-state packet type. */
    static const uint8_t  TYPE_ERROR_STATE     = 0x02U;  /**< Error-state packet type. */
    static const uint8_t  TYPE_RPC             = 0x03U;  /**< RPC request packet type. */
    static const uint8_t  TYPE_RPC_RESPONSE    = 0x04U;  /**< RPC response packet type. */
    static const uint8_t  STATE_AWAITING_AUTH  = 0x01U;  /**< Waiting for a physical button press. */
    static const uint8_t  STATE_AUTHORIZED     = 0x02U;  /**< Authorized to provision Wi-Fi. */
    static const uint8_t  STATE_PROVISIONING   = 0x03U;  /**< Credentials stored; restart pending. */
    static const uint8_t  ERROR_NONE           = 0x00U;  /**< No protocol error. */
    static const uint8_t  ERROR_INVALID_RPC    = 0x01U;  /**< Invalid packet or command data. */
    static const uint8_t  ERROR_UNKNOWN_RPC    = 0x02U;  /**< Unsupported RPC command. */
    static const uint8_t  ERROR_UNABLE_TO_CONN = 0x03U;  /**< Wi-Fi connection failed. */
    static const uint8_t  ERROR_NOT_AUTHORIZED = 0x04U;  /**< Physical authorization required. */
    static const uint8_t  ERROR_UNKNOWN        = 0xFFU;  /**< Unclassified service error. */
    static const uint32_t AUTH_TIMEOUT_MS      = 60000U; /**< Authorization lifetime in ms. */
    static const uint32_t RESTART_DELAY_MS     = 2500U;  /**< Delay before applying Wi-Fi settings. */
    static const uint32_t SESSION_TIMEOUT_MS   = 30000U; /**< Idle time before restoring serial logs. */

    Stream&               m_stream;                              /**< Serial transport. */
    MiniTerminal&         m_miniTerminal;                        /**< Text terminal. */
    uint8_t               m_frame[FRAME_MAX_SIZE];               /**< Bounded incoming frame buffer. */
    size_t                m_frameLength;                         /**< Bytes currently buffered. */
    size_t                m_expectedFrameLength;                 /**< Expected complete frame length. */
    size_t                m_headerMatchLength;                   /**< Matched header prefix length. */
    ButtonState           m_previousButtonStates[BUTTON_ID_CNT]; /**< Last sampled button states. */
    uint32_t              m_authorizedAt;                        /**< Time authorization began. */
    uint32_t              m_lastActivityAt;                      /**< Last valid Improv activity. */
    uint8_t               m_state;                               /**< Current Improv state. */
    bool                  m_isAuthorized;                        /**< Credentials may be accepted. */
    bool                  m_isSessionActive;                     /**< Improv traffic was detected. */
    bool                  m_isLoggingSuppressed;                 /**< Serial logging was redirected. */
    bool                  m_ignoreLineFeed;                      /**< Consume SDK frame terminator. */
    String                m_previousLogSink;                     /**< Sink active before Improv mode. */

    /**
     * Consume one serial byte.
     *
     * @param[in] value Input byte
     */
    void consumeByte(uint8_t value);

    /**
     * Process a complete Improv Serial frame.
     */
    void processFrame();

    /**
     * Process an Improv RPC payload.
     *
     * @param[in] data      RPC payload
     * @param[in] dataLength RPC payload length
     */
    void processRpc(const uint8_t* data, size_t dataLength);

    /**
     * Detect a newly pressed physical button and update authorization state.
     *
     * @param[in] buttonDrv Button driver
     */
    void processButtons(IButtonDrv& buttonDrv);

    /**
     * Expire authorization and idle serial sessions.
     */
    void processTimeouts();

    /**
     * Send a state packet.
     *
     * @param[in] state Improv state
     */
    void sendState(uint8_t state);

    /**
     * Send an error packet.
     *
     * @param[in] error Improv error
     */
    void sendError(uint8_t error);

    /**
     * Send an RPC response without result strings.
     *
     * @param[in] command RPC command identifier
     */
    void sendEmptyRpcResponse(uint8_t command);

    /**
     * Send device information required by Improv clients.
     */
    void sendDeviceInfo();

    /**
     * Send a complete Improv Serial frame.
     *
     * @param[in] type      Improv packet type
     * @param[in] data      Packet data
     * @param[in] dataLength Packet data length
     */
    void sendPacket(uint8_t type, const uint8_t* data, size_t dataLength);

    /**
     * Enter protocol mode and keep logs off the binary serial stream.
     */
    void enterProtocolMode();

    /**
     * Leave protocol mode and restore the previous log sink.
     */
    void leaveProtocolMode();

    /**
     * Reset the in-progress frame parser.
     */
    void resetFrame();
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* IMPROV_SERIAL_SERVICE_H */

/** @} */