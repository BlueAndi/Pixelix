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
 * @brief  Distributed Display Protocol server
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup plugin
 *
 * @{
 */

#ifndef DDPSERVER_H
#define DDPSERVER_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <AsyncUDP.h>
#include <Mutex.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Server for the Distributed Display Protocol (DDP).
 * A controller will connect to the server to send some data on the
 * display.
 * 
 * Specification: http://www.3waylabs.com/ddp/
 */
class DDPServer
{
public:

    /** Format data type of the DDP header data type field. */
    enum Format
    {
        FORMAT_UNDEFINED = 0,   /**< Undefined order */
        FORMAT_RGB,             /**< RGB base color order */
        FORMAT_HSL,             /**< HSL base color order */
        FORMAT_RGBW,            /**< RGBW base color order, including separate white */
        FORMAT_GRAYSCALE        /**< From black to white in different shades of gray. */
    };

    /**
     * DDP application callback prototype.
     * 
     * It provides received data to the application. If the final flag is set, the
     * data is complete and ready for showing it.
     */
    typedef std::function<void(Format format, uint32_t offset, uint8_t bitsPerPixelElement, uint8_t* payload, uint16_t payloadSize, bool isFinal)> DDPCallback;

    /**
     * DDP application callback prototype for DMX legacy mode.
     * 
     * It provides received data to the application with additional DMX related
     * information.
     */
    typedef std::function<void(uint32_t universe, uint8_t startCode, uint8_t* payload, uint16_t payloadSize)> DMXCallback;

    /**
     * Constructs a DDP server.
     */
    DDPServer() :
        m_udpServer(),
        m_ddpCallback(nullptr),
        m_dmxCallback(nullptr),
        m_mutex(),
        m_seqNo(0U),
        m_isPause(false),
        m_deviceManufacturer("device-manufacturer"),
        m_deviceModel("device-model"),
        m_deviceVersion("device-version"),
        m_deviceMac("xx:xx:xx:xx:xx:xx:xx")
    {
        (void)m_mutex.create();
    }

    /**
     * Destroys the DDP server.
     */
    ~DDPServer()
    {
        m_mutex.destroy();
    }

    /**
     * Starts the server to listen for controllers (clients).
     * The given information will be distributed to the controller. Its used there
     * to maintain one or more displays.
     * 
     * @param[in] deviceManufacturer    Device manufacturer
     * @param[in] deviceModel           Device model
     * @param[in] deviceVersion         Device version
     * @param[in] deviceMac             Device MAC address
     * 
     * @return If successful, it will return true otherwise false.
     */
    bool begin(const String& deviceManufacturer, const String& deviceModel, const String& deviceVersion, const String& deviceMac);

    /**
     * Stops the server to listen. A existing connection will be disconnected.
     */
    void end();

    /**
     * After the server is started, the display may broadcast its availability.
     */
    void notifyUpState();

    /**
     * Before the server is stopped, the display may broadcast that it will disappear.
     */
    void notifyDownState();

    /**
     * Pause the reception of further data.
     * A existing connection will be kept.
     */
    void pause()
    {
        MutexGuard<Mutex> guard(m_mutex);

        m_isPause = true;
    }

    /**
     * Resume the reception of further data.
     */
    void resume()
    {
        MutexGuard<Mutex> guard(m_mutex);

        m_isPause = false;
    }

    /**
     * Register a callback to receive DDP data which to display.
     * 
     * @param[in] cb    The callback.
     */
    void registerDDPCallback(DDPCallback cb)
    {
        MutexGuard<Mutex> guard(m_mutex);

        m_ddpCallback = cb;
    }

    /**
     * Register a callback to receive DMX data which to display.
     * Only used in case the DMX legacy mode is supported by the
     * application.
     * 
     * @param[in] cb    The callback.
     */
    void registerDMXCallback(DMXCallback cb)
    {
        MutexGuard<Mutex> guard(m_mutex);

        m_dmxCallback = cb;
    }

private:

    /** DDP packet header without timecode. */
    typedef union _DDPHeader
    {
        /** Header data detail */
        struct _Detail
        {
            uint8_t     flags;      /**< Flags */
            uint8_t     control;    /**< Control information */
            uint8_t     dataType;   /**< Data type information */
            uint8_t     id;         /**< ID (source / destination) */
            uint32_t    offset;     /**< Data offset in byte (MSB first) */
            uint16_t    dataLen;    /**< Data length in byte (MSB first) */

        } __attribute__ ((packed)) detail;

        /** Raw header data */
        uint8_t raw[10U];

    } __attribute__ ((packed)) DDPHeader;

    /** The DDP packet. */
    typedef struct _DDPPacket
    {
        DDPHeader       header; /**< Packet header */
        const uint8_t*  data;   /**< Payload data */

    } DDPPacket;

    /** Displays always receive packets on UDP/TCP Port 4048. */
    static const uint16_t   PORT                    = 4048U;

    /** DDP version */
    static const uint8_t    PROTOCOL_VERSION        = 1U;

    /** DDP standard header size in byte (without timecode) */
    static const size_t     STD_PACKET_HEADER_SIZE  = 10U;

    /** DDP extended header size in byte (with timecode) */
    static const size_t     EXT_PACKET_HEADER_SIZE  = 14U;

    /** If the sequence number is 0, it will be ignored. */
    static const uint8_t    SEQ_NO_IGNORE           = 0U;

    /** Lowest value of a applied sequence number. */
    static const uint8_t    SEQ_NO_BEGIN            = 1U;

    /** Highest value of a applied sequence number. */
    static const uint8_t    SEQ_NO_MAX              = 15U;

    AsyncUDP    m_udpServer;            /**< UDP server */
    DDPCallback m_ddpCallback;          /**< Callback for receiving DDP data */
    DMXCallback m_dmxCallback;          /**< Callback for received DMX data (DMX legacy mode) */
    Mutex       m_mutex;                /**< For concurrent access protection. */
    uint8_t     m_seqNo;                /**< Last sequence number used for packet tracking. */
    bool        m_isPause;              /**< Is reception paused? */
    String      m_deviceManufacturer;   /**< Device manufacturer */
    String      m_deviceModel;          /**< Device model */
    String      m_deviceVersion;        /**< Device version */
    String      m_deviceMac;            /**< Device MAC address */

    /* Copy DDP server is not allowed. */
    DDPServer(const DDPServer& server);
    DDPServer& operator=(const DDPServer& server);

    /**
     * Get the 16-bit big endian value in little endian.
     * 
     * @param[in] valueBE   Value in big endian
     * 
     * @return 16-bit value in little endian
     */
    uint16_t getValueInLE(uint16_t valueBE);

    /**
     * Get the 32-bit big endian value in little endian.
     * 
     * @param[in] valueBE   Value in big endian
     * 
     * @return 32-bit value in little endian
     */
    uint32_t getValueInLE(uint32_t valueBE);

    /**
     * Get the 16-bit little endian value in big endian.
     * 
     * @param[in] valueLE   Value in little endian
     * 
     * @return 16-bit value in big endian
     */
    uint16_t getValueInBE(uint16_t valueLE);

    /**
     * Get the 32-bit little endian value in big endian.
     * 
     * @param[in] valueLE   Value in little endian
     * 
     * @return 32-bit value in big endian
     */
    uint32_t getValueInBE(uint32_t valueLE);

    /**
     * Get protocol version from DDP packet header.
     * 
     * @param[in] header    DDP header
     * 
     * @return Protocol version
     */
    uint8_t getVersion(const DDPHeader& header);

    /**
     * Set protocol version to DDP packet header.
     * 
     * @param[in,out]   header  DDP header
     * @param[in]       version DDP version
     * 
     * @return Protocol version
     */
    void setVersion(DDPHeader& header, uint8_t version);

    /**
     * Is the timecode flag set in DDP packet header?
     * 
     * @param[in] header    DDP header
     * 
     * @return If timecode flag is set, it will return true otherwise false.
     */
    bool isTimeCodeFlagSet(const DDPHeader& header);

    /**
     * Is the storage flag set in DDP packet header?
     * 
     * @param[in] header    DDP header
     * 
     * @return If storage flag is set, it will return true otherwise false.
     */
    bool isStorageFlagSet(const DDPHeader& header);

    /**
     * Is the reply flag set in DDP packet header?
     * 
     * @param[in] header    DDP header
     * 
     * @return If reply flag is set, it will return true otherwise false.
     */
    bool isReplyFlagSet(const DDPHeader& header);

    /**
     * Set the reply flag in DDP packet header?
     * 
     * @param[in,out] header    DDP header
     */
    void setReplyFlag(DDPHeader& header);

    /**
     * Is the query flag set in DDP packet header?
     * 
     * @param[in] header    DDP header
     * 
     * @return If query flag is set, it will return true otherwise false.
     */
    bool isQueryFlagSet(const DDPHeader& header);

    /**
     * Is the push flag set in DDP packet header?
     * 
     * @param[in] header    DDP header
     * 
     * @return If push flag is set, it will return true otherwise false.
     */
    bool isPushFlagSet(const DDPHeader& header);

    /**
     * Set the push flag in DDP packet header?
     * 
     * @param[in,out] header    DDP header
     */
    void setPushFlag(DDPHeader& header);

    /**
     * Get the sequence number from the DDP packet header.
     * 
     * @param[in] header    DDP header
     * 
     * @return Sequence number
     */
    uint8_t getSeqNo(const DDPHeader& header);

    /**
     * Checks whether a sequence number is valid or not.
     * It depends on the last received sequence number.
     * 
     * If the received sequence number is 0, it will be ignored and returns
     * successful.
     * 
     * @param[in] seqNo Sequence number
     * 
     * @return If sequence number is valid or ignored, it will return true otherwise false.
     */
    bool isSeqNoValid(uint8_t seqNo);

    /**
     * Get the data type from the DDP header.
     * 
     * @param[in] header    DDP header
     * 
     * @return Data type
     */
    uint8_t getDataType(const DDPHeader& header);

    /**
     * Get the bits per pixel element from the DDP header.
     * Pixel element means for just one base color, not for the whole pixel.
     * 
     * @param[in] header    DDP header
     * 
     * @return Bits per pixel element
     */
    uint8_t getBitsPerPixelElement(const DDPHeader& header);

    /**
     * Get the offset from the DDP header.
     * 
     * @param[in] header    DDP header
     * 
     * @return Offset in byte
     */
    uint16_t getOffset(const DDPHeader& header);

    /**
     * Get the payload size from the DDP header.
     * 
     * @param[in] header    DDP header
     * 
     * @return Payload size in byte
     */
    uint16_t getPayloadSize(const DDPHeader& header);

    /**
     * Set the payload size in the DDP header.
     * 
     * @param[in,out]   header  DDP header
     * @param[in]       size    Payload size in byte
     */
    void setPayloadSize(DDPHeader& header, uint16_t size);

    /**
     * On UDP packet reception, this method will be called.
     * It will parse the payload for valid DDP content and distribute it
     * to the registered application.
     * 
     * @param[in] udpPacket UDP packet
     */
    void onPacket(AsyncUDPPacket& udpPacket);

    /**
     * Handles received query.
     * 
     * @param[in] header        DDP header
     * @param[in] payload       DDP payload
     * @param[in] payloadSize   DDP payload size in byte
     */
    void handleQuery(const DDPHeader& header, uint8_t* payload, uint16_t payloadSize);

    /**
     * Handles received data.
     * 
     * @param[in] header        DDP header
     * @param[in] payload       DDP payload
     * @param[in] payloadSize   DDP payload size in byte
     */
    void handleData(const DDPHeader& header, uint8_t* payload, uint16_t payloadSize);

    /**
     * Notifys a registered application and provides the DDP received data.
     * The application needs to copy the data into its own context!
     * 
     * @param[in] format                Format of the payload data
     * @param[in] offset                Byte offset in display framebuffer where to continue
     * @param[in] bitsPerPixelElement   Bits per pixel element in payload data
     * @param[in] payload               Payload data
     * @param[in] payloadSize           Payload data size in byte
     * @param[in] isFinal               If final, its the last data and display shall show it. Otherwise more data will come.
     */
    void ddpNotify(Format format, uint32_t offset, uint8_t bitsPerPixelElement, uint8_t* payload, uint16_t payloadSize, bool isFinal);

    /**
     * Notifys a registered application and provides the DMX received data.
     * The application needs to copy the data into its own context!
     * 
     * @param[in] universe      The universe number.
     * @param[in] startCode     The DMX start code.
     * @param[in] payload       Payload data
     * @param[in] payloadSize   Payload data size in byte
     */
    void dmxNotify(uint32_t universe, uint8_t startCode, uint8_t* payload, uint16_t payloadSize);

    /**
     * Send a DDP packet to the connected controller.
     * 
     * @param[in] packet    DDP packet which to send
     * 
     * @return If successful sent, it will return true otherwise false.
     */
    bool send(const DDPPacket& packet);

    /**
     * Broadcast a DDP packet.
     * 
     * @param[in] packet    DDP packet which to broadcast
     * 
     * @return If successful sent, it will return true otherwise false.
     */
    bool broadcast(const DDPPacket& packet);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* DDPSERVER_H */

/** @} */
