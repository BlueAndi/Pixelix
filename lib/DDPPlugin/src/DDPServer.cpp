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
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "DDPServer.h"

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/** Bit index for the version in the DDP header flags byte. */
#define DDP_HEADER_FLAGS_VERSION_BIT            (6U)

/** Bit mask for the version in the DDP header flags byte. */
#define DDP_HEADER_FLAGS_VERSION_MASK           (0x03U)

/** Bit index for the timecode flag in the DDP header flags byte. */
#define DDP_HEADER_FLAGS_TIMECODE_BIT           (4U)

/** Bit mask for the timecode flag in the DDP header flags byte. */
#define DDP_HEADER_FLAGS_TIMECODE_MASK          (0x01U)

/** Bit index for the storage flag in the DDP header flags byte. */
#define DDP_HEADER_FLAGS_STORAGE_BIT            (3U)

/** Bit mask for the storage flag in the DDP header flags byte. */
#define DDP_HEADER_FLAGS_STORAGE_MASK           (0x01U)

/** Bit index for the reply flag in the DDP header flags byte. */
#define DDP_HEADER_FLAGS_REPLY_BIT              (2U)

/** Bit mask for the reply flagn in the DDP header flags byte. */
#define DDP_HEADER_FLAGS_REPLY_MASK             (0x01U)

/** Bit index for the query flag in the DDP header flags byte. */
#define DDP_HEADER_FLAGS_QUERY_BIT              (1U)

/** Bit mask for the query flag in the DDP header flags byte. */
#define DDP_HEADER_FLAGS_QUERY_MASK             (0x01U)

/** Bit index for the push flag in the DDP header flags byte. */
#define DDP_HEADER_FLAGS_PUSH_BIT               (0U)

/** Bit mask for the push flag in the DDP header flags byte. */
#define DDP_HEADER_FLAGS_PUSH_MASK              (0x01U)

/** Bit index for the sequence number in the DDP header control byte. */
#define DDP_HEADER_CONTROL_SEQ_NO_BIT           (0U)

/** Bit mask for the sequence number in the DDP header control byte. */
#define DDP_HEADER_CONTROL_SEQ_NO_MASK          (0x0fU)

/** Bit index for the customer bit in the DDP header data type byte. */
#define DDP_HEADER_DT_CUSTOM_BIT                (7U)

/** Bit mask for the customer bit in the DDP header data type byte. */
#define DDP_HEADER_DT_CUSTOM_MASK               (0x01U)

/** Bit index for the data type in the DDP header data type byte. */
#define DDP_HEADER_DT_DATA_TYPE_BIT             (3U)

/** Bit mask for the data type in the DDP header data type byte. */
#define DDP_HEADER_DT_DATA_TYPE_MASK            (0x07U)

/** Bit index for the pixel size in the DDP header data type byte. */
#define DDP_HEADER_DT_PIXEL_ELEMENT_SIZE_BIT    (0U)

/** Bit mask for the pixel size in the DDP header data type byte. */
#define DDP_HEADER_DT_PIXEL_ELEMENT_SIZE_MASK   (0x07U)

/** DDP data type - undefined */
#define DDP_DATA_TYPE_UNDEFINED                 (0U)

/** DDP data type - RGB order */
#define DDP_DATA_TYPE_RGB                       (1U)

/** DDP data type - HSL order */
#define DDP_DATA_TYPE_HSL                       (2U)

/** DDP data type - RGBW order */
#define DDP_DATA_TYPE_RGBW                      (3U)

/** DDP data type - Grayscale (shades of gray) */
#define DDP_DATA_TYPE_GRAYSCALE                 (4U)

/** DDP pixel size - undefined */
#define DDP_PIXEL_ELEMENT_SIZE_UNDEFINED        (0U)

/** DDP pixel size - 1 bit per pixel element */
#define DDP_PIXEL_ELEMENT_SIZE_1                (1U)

/** DDP pixel size - 4 bit per pixel element */
#define DDP_PIXEL_ELEMENT_SIZE_4                (2U)

/** DDP pixel size - 8 bit per pixel element */
#define DDP_PIXEL_ELEMENT_SIZE_8                (3U)

/** DDP pixel size - 16 bit per pixel element */
#define DDP_PIXEL_ELEMENT_SIZE_16               (4U)

/** DDP pixel size - 24 bit per pixel element */
#define DDP_PIXEL_ELEMENT_SIZE_24               (5U)

/** DDP pixel size - 32 bit per pixel element */
#define DDP_PIXEL_ELEMENT_SIZE_32               (6U)

/** DDP id - reserved */
#define DDP_ID_RESERVED                         (0U)

/** DDP id - default device */
#define DDP_ID_DEFAULT                          (1U)

/** DDP id - custom id defined via JSON config */
#define DDP_ID_CUSTOM                           (2U)

/** DDP id - JSON control */
#define DDP_ID_JSON_CONTROL                     (246U)

/** DDP id - JSON config */
#define DDP_ID_JSON_CONFIG                      (250U)

/** DDP id - JSON status */
#define DDP_ID_JSON_STATUS                      (251U)

/** DDP id - DMX legay mode */
#define DDP_ID_DMX_TRANSIT                      (254U)

/** DDP id - all devices */
#define DDP_ID_ALL_DEVICES                      (255U)

/** DDP timecode field size in byte */
#define DDP_TIMECODE_SIZE                       (4U)

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

bool DDPServer::begin(const String& deviceManufacturer, const String& deviceModel, const String& deviceVersion, const String& deviceMac)
{
    bool isSuccessful = false;

    m_deviceManufacturer    = deviceManufacturer;
    m_deviceModel           = deviceModel;
    m_deviceVersion         = deviceVersion;
    
    if (true != deviceMac.isEmpty())
    {
        m_deviceMac = deviceMac;
    }

    if (true == m_udpServer.listen(PORT))
    {
        m_udpServer.onPacket([](void* arg, AsyncUDPPacket& packet)
        {
            DDPServer*  tthis = static_cast<DDPServer*>(arg);

            if (nullptr != tthis)
            {
                tthis->onPacket(packet);
            }

        }, this);

        m_isPause = false;

        isSuccessful = true;
    }

    return isSuccessful;
}

void DDPServer::end()
{
    m_udpServer.close();
}

void DDPServer::notifyUpState()
{
    DDPPacket       ddpReply;
    const char*     DISCOVERY_RSP       = "{\"status\":{\"update\":\"change\",\"state\":\"up\"}}";
    const size_t    DISCOVERY_RSP_LEN   = strlen(DISCOVERY_RSP);

    setVersion(ddpReply.header, PROTOCOL_VERSION);
    setReplyFlag(ddpReply.header);
    setPushFlag(ddpReply.header);
    ddpReply.header.detail.id = DDP_ID_JSON_STATUS;
    setPayloadSize(ddpReply.header, DISCOVERY_RSP_LEN);
    ddpReply.data = reinterpret_cast<const uint8_t*>(DISCOVERY_RSP);

    (void)broadcast(ddpReply);
}

void DDPServer::notifyDownState()
{
    DDPPacket       ddpReply;
    const char*     DISCOVERY_RSP       = "{\"status\":{\"update\":\"change\",\"state\":\"down\"}}";
    const size_t    DISCOVERY_RSP_LEN   = strlen(DISCOVERY_RSP);

    setVersion(ddpReply.header, PROTOCOL_VERSION);
    setReplyFlag(ddpReply.header);
    setPushFlag(ddpReply.header);
    ddpReply.header.detail.id = DDP_ID_JSON_STATUS;
    setPayloadSize(ddpReply.header, DISCOVERY_RSP_LEN);
    ddpReply.data = reinterpret_cast<const uint8_t*>(DISCOVERY_RSP);

    (void)broadcast(ddpReply);
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

uint16_t DDPServer::getValueInLE(uint16_t valueBE)
{
    uint8_t*    byteStream  = reinterpret_cast<uint8_t*>(&valueBE);
    uint16_t    b0          = byteStream[0U];
    uint16_t    b1          = byteStream[1U];

    return (b0 << 8U) | (b1 << 0U);
}

uint32_t DDPServer::getValueInLE(uint32_t valueBE)
{
    uint8_t*    byteStream  = reinterpret_cast<uint8_t*>(&valueBE);
    uint32_t    b0          = byteStream[0U];
    uint32_t    b1          = byteStream[1U];
    uint32_t    b2          = byteStream[2U];
    uint32_t    b3          = byteStream[3U];

    return (b0 << 24U) | (b1 << 16U) | (b2 << 8U) | (b3 << 0U);
}

uint16_t DDPServer::getValueInBE(uint16_t valueLE)
{
    uint8_t*    byteStream  = reinterpret_cast<uint8_t*>(&valueLE);
    uint16_t    b0          = byteStream[0U];
    uint16_t    b1          = byteStream[1U];

    return (b0 << 0U) | (b1 << 8U);
}

uint32_t DDPServer::getValueInBE(uint32_t valueLE)
{
    uint8_t*    byteStream  = reinterpret_cast<uint8_t*>(&valueLE);
    uint32_t    b0          = byteStream[0U];
    uint32_t    b1          = byteStream[1U];
    uint32_t    b2          = byteStream[2U];
    uint32_t    b3          = byteStream[3U];

    return (b0 << 0U) | (b1 << 8U) | (b2 << 16U) | (b3 << 24U);
}

uint8_t DDPServer::getVersion(const DDPHeader& header)
{
    uint8_t version = (header.detail.flags >> DDP_HEADER_FLAGS_VERSION_BIT) & DDP_HEADER_FLAGS_VERSION_MASK;

    return version;
}

void DDPServer::setVersion(DDPHeader& header, uint8_t version)
{
    header.detail.flags &= ~(DDP_HEADER_FLAGS_VERSION_MASK << DDP_HEADER_FLAGS_VERSION_BIT);
    header.detail.flags |= (version & DDP_HEADER_FLAGS_VERSION_MASK) << DDP_HEADER_FLAGS_VERSION_BIT;
}

bool DDPServer::isTimeCodeFlagSet(const DDPHeader& header)
{
    uint8_t flag = (header.detail.flags >> DDP_HEADER_FLAGS_TIMECODE_BIT) & DDP_HEADER_FLAGS_TIMECODE_MASK;

    return (0U != flag);
}

bool DDPServer::isStorageFlagSet(const DDPHeader& header)
{
    uint8_t flag = (header.detail.flags >> DDP_HEADER_FLAGS_STORAGE_BIT) & DDP_HEADER_FLAGS_STORAGE_MASK;

    return (0U != flag);
}

bool DDPServer::isReplyFlagSet(const DDPHeader& header)
{
    uint8_t flag = (header.detail.flags >> DDP_HEADER_FLAGS_REPLY_BIT) & DDP_HEADER_FLAGS_REPLY_MASK;

    return (0U != flag);
}

void DDPServer::setReplyFlag(DDPHeader& header)
{
    header.detail.flags |= DDP_HEADER_FLAGS_REPLY_MASK << DDP_HEADER_FLAGS_REPLY_BIT;
}

bool DDPServer::isQueryFlagSet(const DDPHeader& header)
{
    uint8_t flag = (header.detail.flags >> DDP_HEADER_FLAGS_QUERY_BIT) & DDP_HEADER_FLAGS_QUERY_MASK;

    return (0U != flag);
}

bool DDPServer::isPushFlagSet(const DDPHeader& header)
{
    uint8_t flag = (header.detail.flags >> DDP_HEADER_FLAGS_PUSH_BIT) & DDP_HEADER_FLAGS_PUSH_MASK;

    return (0U != flag);
}

void DDPServer::setPushFlag(DDPHeader& header)
{
    header.detail.flags |= DDP_HEADER_FLAGS_PUSH_MASK << DDP_HEADER_FLAGS_PUSH_BIT;
}

uint8_t DDPServer::getSeqNo(const DDPHeader& header)
{
    uint8_t seqNo = (header.detail.control >> DDP_HEADER_CONTROL_SEQ_NO_BIT) & DDP_HEADER_CONTROL_SEQ_NO_MASK;

    return seqNo;
}

bool DDPServer::isSeqNoValid(uint8_t seqNo)
{
    bool isValid = true;

    if (SEQ_NO_IGNORE < seqNo)
    {
        uint8_t expectedSeqNo = m_seqNo + 1U;
        
        if (SEQ_NO_MAX < expectedSeqNo)
        {
            expectedSeqNo = SEQ_NO_BEGIN;
        }

        if (expectedSeqNo != seqNo)
        {
            isValid = false;
        }
    }

    return isValid;
}

uint8_t DDPServer::getDataType(const DDPHeader& header)
{
    uint8_t dataType = (header.detail.dataType >> DDP_HEADER_DT_DATA_TYPE_BIT) & DDP_HEADER_DT_DATA_TYPE_MASK;

    return dataType;
}

uint8_t DDPServer::getBitsPerPixelElement(const DDPHeader& header)
{
    uint8_t bitsPerPixelElement = (header.detail.dataType >> DDP_HEADER_DT_PIXEL_ELEMENT_SIZE_BIT) & DDP_HEADER_DT_PIXEL_ELEMENT_SIZE_MASK;

    switch(bitsPerPixelElement)
    {
    case DDP_PIXEL_ELEMENT_SIZE_UNDEFINED:
        bitsPerPixelElement = 0U;
        break;

    case DDP_PIXEL_ELEMENT_SIZE_1:
        bitsPerPixelElement = 1U;
        break;

    case DDP_PIXEL_ELEMENT_SIZE_4:
        bitsPerPixelElement = 4U;
        break;

    case DDP_PIXEL_ELEMENT_SIZE_8:
        bitsPerPixelElement = 8U;
        break;

    case DDP_PIXEL_ELEMENT_SIZE_16:
        bitsPerPixelElement = 16U;
        break;

    case DDP_PIXEL_ELEMENT_SIZE_24:
        bitsPerPixelElement = 24U;
        break;

    case DDP_PIXEL_ELEMENT_SIZE_32:
        bitsPerPixelElement = 32U;
        break;

    default:
        bitsPerPixelElement = 0U;
        break;
    }

    return bitsPerPixelElement;
}

uint16_t DDPServer::getOffset(const DDPHeader& header)
{
    return getValueInLE(header.detail.offset);
}

uint16_t DDPServer::getPayloadSize(const DDPHeader& header)
{
    return getValueInLE(header.detail.dataLen);
}

void DDPServer::setPayloadSize(DDPHeader& header, uint16_t size)
{
    header.detail.dataLen = getValueInBE(size);
}

void DDPServer::onPacket(AsyncUDPPacket& udpPacket)
{
    bool isPause = false;

    {
        MutexGuard<Mutex> guard(m_mutex);
        isPause = m_isPause;
    }

    /* At least the packet header must be received to determine which kind of
     * packet (standard or extended) it is.
     */
    if (sizeof(DDPHeader) <= udpPacket.length())
    {
        DDPHeader*  ddpHeader       = reinterpret_cast<DDPHeader*>(udpPacket.data());
        uint16_t    payloadSize     = getPayloadSize(*ddpHeader);
        size_t      packetSize      = 0U;
        uint8_t*    payload         = nullptr;
        bool        takeOverSeqNo   = false;

        /* Without timecode? */
        if (false == isTimeCodeFlagSet(*ddpHeader))
        {
            packetSize  = sizeof(DDPHeader) + payloadSize;
            payload     = &udpPacket.data()[sizeof(DDPHeader)];
        }
        else
        {
            packetSize = sizeof(DDPHeader) + DDP_TIMECODE_SIZE + payloadSize;
            payload     = &udpPacket.data()[sizeof(DDPHeader) + DDP_TIMECODE_SIZE];
        }

        /* The UDP packet must contain a complete DDP packet. */
        if (packetSize > udpPacket.length())
        {
            /* Skip */
        }
        /* The DDP packet sequence number must be valid. */
        else if (false == isSeqNoValid(getSeqNo(*ddpHeader)))
        {
            /* Skip packet, but take over sequence number in case a packet
             * was lost otherwise all following packets will be dropped.
             */
            takeOverSeqNo = true;
        }
        else
        {
            /* Is it a query? */
            if (true == isQueryFlagSet(*ddpHeader))
            {
                /* Handle query */
                handleQuery(*ddpHeader, payload, payloadSize);
            }
            else
            {
                /* If pause, data will be skipped. */
                if (false == isPause)
                {
                    handleData(*ddpHeader, payload, payloadSize);
                }
            }

            takeOverSeqNo = true;
        }

        if (true == takeOverSeqNo)
        {
            MutexGuard<Mutex> guard(m_mutex);
            m_seqNo = getSeqNo(*ddpHeader);
        }
    }
}

void DDPServer::handleQuery(const DDPHeader& header, uint8_t* payload, uint16_t payloadSize)
{
    DDPPacket   ddpReply;
    String      ddpReplyPayload;

    (void)payload;
    (void)payloadSize;

    /* Prepare reply in general */
    setVersion(ddpReply.header, PROTOCOL_VERSION);
    setReplyFlag(ddpReply.header);
    setPushFlag(ddpReply.header);
    ddpReply.header.detail.id = header.detail.id;

    /* Discovery? */
    if (DDP_ID_JSON_STATUS == header.detail.id)
    {
        ddpReplyPayload = "{\"status\":{";

        ddpReplyPayload += "\"man\":\"" + m_deviceManufacturer + "\",";
        ddpReplyPayload += "\"mod\":\"" + m_deviceModel + "\",";
        ddpReplyPayload += "\"ver\":\"" + m_deviceVersion + "\",";
        ddpReplyPayload += "\"mac\":\"" + m_deviceMac + "\",";
        ddpReplyPayload += "\"push\":false,";
        ddpReplyPayload += "\"ntp\":false";

        ddpReplyPayload += "}}";
        
        setPayloadSize(ddpReply.header, ddpReplyPayload.length());
        ddpReply.data = reinterpret_cast<const uint8_t*>(ddpReplyPayload.c_str());
    }
    /* Query not supported. */
    else
    {
        /* Nothing to do, because offset and length are already 0. */
        ;
    }

    (void)send(ddpReply);
}

void DDPServer::handleData(const DDPHeader& header, uint8_t* payload, uint16_t payloadSize)
{
    /* Data from storage is not supported. */
    if (true == isStorageFlagSet(header))
    {
        /* Skip */
    }
    /* DMX legay mode data? */
    else if (DDP_ID_DMX_TRANSIT == header.detail.id)
    {
        uint32_t    universe        = getOffset(header);
        uint8_t     startCode       = payload[0U];
        uint8_t*    dmxPayload      = &payload[1U];
        uint16_t    dmxPayloadSize  = payloadSize - 1U;

        dmxNotify(universe, startCode, dmxPayload, dmxPayloadSize);
    }
    /* Handle data */
    else if ((DDP_ID_ALL_DEVICES == header.detail.id) ||
             (DDP_ID_DEFAULT == header.detail.id))
    {
        ddpNotify(static_cast<DDPServer::Format>(getDataType(header)), getOffset(header), getBitsPerPixelElement(header), payload, payloadSize, isPushFlagSet(header));
    }
    else
    {
        /* Skip */
    }
}

void DDPServer::ddpNotify(Format format, uint32_t offset, uint8_t bitsPerPixelElement, uint8_t* payload, uint16_t payloadSize, bool isFinal)
{
    DDPCallback callback = nullptr;

    {
        MutexGuard<Mutex> guard(m_mutex);

        callback = m_ddpCallback;
    }

    if (nullptr != callback)
    {
        callback(format, offset, bitsPerPixelElement, payload, payloadSize, isFinal);
    }
}

void DDPServer::dmxNotify(uint32_t universe, uint8_t startCode, uint8_t* payload, uint16_t payloadSize)
{
    DMXCallback callback = nullptr;

    {
        MutexGuard<Mutex> guard(m_mutex);

        callback = m_dmxCallback;
    }

    if (nullptr != callback)
    {
        callback(universe, startCode, payload, payloadSize);
    }
}

bool DDPServer::send(const DDPPacket& packet)
{
    AsyncUDPMessage udpMessage;
    bool            isSuccessful    = true;
    uint16_t        payloadSize     = getPayloadSize(packet.header);

    if (sizeof(packet.header.raw) != udpMessage.write(packet.header.raw, sizeof(packet.header.raw)))
    {
        isSuccessful = false;
    }
    else if (payloadSize != udpMessage.write(packet.data, payloadSize))
    {
        isSuccessful = false;
    }
    else if (udpMessage.length() != m_udpServer.send(udpMessage))
    {
        isSuccessful = false;
    }
    else
    {
        ;
    }

    return isSuccessful;
}

bool DDPServer::broadcast(const DDPPacket& packet)
{
    AsyncUDPMessage udpMessage;
    bool            isSuccessful    = true;
    uint16_t        payloadSize     = getPayloadSize(packet.header);

    if (sizeof(packet.header.raw) != udpMessage.write(packet.header.raw, sizeof(packet.header.raw)))
    {
        isSuccessful = false;
    }
    else if (payloadSize != udpMessage.write(packet.data, payloadSize))
    {
        isSuccessful = false;
    }
    else if (udpMessage.length() != m_udpServer.broadcast(udpMessage))
    {
        isSuccessful = false;
    }
    else
    {
        ;
    }

    return isSuccessful;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
