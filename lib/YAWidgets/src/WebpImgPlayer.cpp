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
 * @file   WebpImgPlayer.cpp
 * @brief  WebP image player
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "WebpImgPlayer.h"

#if (0 != CONFIG_IMAGE_FORMAT_WEBP_ENABLE)

#include <string.h>
#include <Logging.h>
#include <webp/decode.h>

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

static bool     isRiffContainerComplete(const uint8_t* data, size_t size);
static uint32_t getUInt32Le(const uint8_t* data);

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/** Size of the RIFF/WEBP container header in bytes. */
static const size_t RIFF_HEADER_SIZE   = 12U;

/** Largest canvas dimension in pixels which the player's int16_t width and
 * height, and the uint16_t of YAGfxDynamicBitmap::create(), can represent.
 * This is a representability limit of the types, not a size policy.
 */
static const int DIM_MAX_REPRESENTABLE = static_cast<int>(INT16_MAX);

/******************************************************************************
 * Public Methods
 *****************************************************************************/

WebpImgPlayer::WebpImgPlayer() :
    m_dataAllocator(),
    m_webpData(nullptr),
    m_webpDataSize(0U),
    m_animDecoder(nullptr),
    m_bitmap(),
    m_width(0),
    m_height(0),
    m_isAnimation(false),
    m_isFinished(false),
    m_isStopped(false),
    m_loopCount(0U),
    m_currentLoop(0U),
    m_isInfiniteLoop(false),
    m_timer(),
    m_lastTimestamp(0),
    m_hasMoreFrames(false),
    m_frameRgba(nullptr)
{
}

WebpImgPlayer::WebpImgPlayer(const WebpImgPlayer& player) :
    m_dataAllocator(),
    m_webpData(nullptr),
    m_webpDataSize(0U),
    m_animDecoder(nullptr),
    m_bitmap(),
    m_width(0),
    m_height(0),
    m_isAnimation(false),
    m_isFinished(false),
    m_isStopped(false),
    m_loopCount(0U),
    m_currentLoop(0U),
    m_isInfiniteLoop(false),
    m_timer(),
    m_lastTimestamp(0),
    m_hasMoreFrames(false),
    m_frameRgba(nullptr)
{
    copyFrom(player);
}

WebpImgPlayer::~WebpImgPlayer()
{
    cleanup();
}

WebpImgPlayer& WebpImgPlayer::operator=(const WebpImgPlayer& player)
{
    if (this != &player)
    {
        cleanup();
        copyFrom(player);
    }

    return *this;
}

WebpImgPlayer::Ret WebpImgPlayer::open(FS& fs, const String& fileName)
{
    Ret ret = RET_OK;

    if (nullptr != m_webpData)
    {
        ret = RET_FILE_ALREADY_OPENED;
    }
    else
    {
        ret = readFileIntoMemory(fs, fileName);

        if (RET_OK == ret)
        {
            ret = processWebPData();
        }

        /* Never leave the player half initialized. */
        if (RET_OK != ret)
        {
            cleanup();
        }
    }

    return ret;
}

void WebpImgPlayer::close()
{
    cleanup();
}

bool WebpImgPlayer::play(YAGfx& gfx, int16_t x, int16_t y)
{
    bool isSuccessful = false;

    /* Nothing opened? */
    if (0 >= m_width)
    {
        /* Guard path: there is nothing to show. */
    }
    /* Single image? */
    else if (false == m_isAnimation)
    {
        gfx.drawBitmap(x, y, m_bitmap);
        isSuccessful = true;
    }
    else if (nullptr == m_animDecoder)
    {
        /* Guard path: an animation without decoder can not be shown. */
    }
    else
    {
        /* The information about a finished frame loop cycle is valid only
         * until the next play() call, same as GifImgPlayer handles it.
         */
        m_isFinished = false;

        if (true == m_isStopped)
        {
            /* Guard path: the animation ended, the last frame stays visible. */
        }
        /* Time for the next frame? */
        else if ((false == m_timer.isTimerRunning()) ||
                 (true == m_timer.isTimeout()))
        {
            advanceAnimation();
        }
        else
        {
            /* The current frame is still on time. */
        }

        if (nullptr != m_frameRgba)
        {
            drawRgba(gfx, x, y, m_frameRgba, static_cast<size_t>(m_width) * BYTES_PER_PIXEL);
        }

        isSuccessful = true;
    }

    return isSuccessful;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void WebpImgPlayer::cleanup()
{
    /* The frame belongs to the decoder, it is invalid after its deletion. */
    m_frameRgba = nullptr;

    if (nullptr != m_animDecoder)
    {
        WebPAnimDecoderDelete(m_animDecoder);
        m_animDecoder = nullptr;
    }

    if (nullptr != m_webpData)
    {
        m_dataAllocator.deallocateArray(m_webpData);
        m_webpData = nullptr;
    }

    m_bitmap.release();
    m_timer.stop();

    m_webpDataSize  = 0U;
    m_width         = 0;
    m_height        = 0;
    m_isAnimation   = false;
    m_isFinished    = false;
    m_isStopped     = false;
    m_loopCount     = 0U;
    m_currentLoop   = 0U;
    m_lastTimestamp = 0;
    m_hasMoreFrames = false;
}

void WebpImgPlayer::copyFrom(const WebpImgPlayer& player)
{
    /* The infinite loop mode is a general setting, not part of the image data,
     * therefore it is carried over independent of the file data.
     */
    m_isInfiniteLoop = player.m_isInfiniteLoop;

    if ((nullptr != player.m_webpData) &&
        (0U < player.m_webpDataSize))
    {
        m_webpData = m_dataAllocator.allocateArray(player.m_webpDataSize);

        if (nullptr == m_webpData)
        {
            LOG_ERROR("Out of memory to copy WebP image.");
        }
        else
        {
            m_webpDataSize = player.m_webpDataSize;
            (void)memcpy(m_webpData, player.m_webpData, m_webpDataSize);

            /* The decoder is not copyable, therefore it is set up from the
             * copied file data. An animation restarts with its first frame.
             */
            if (RET_OK != processWebPData())
            {
                cleanup();
            }
        }
    }
}

WebpImgPlayer::Ret WebpImgPlayer::readFileIntoMemory(FS& fs, const String& fileName)
{
    Ret ret = RET_OK;

    if (false == fs.exists(fileName))
    {
        LOG_ERROR("File %s not found.", fileName.c_str());
        ret = RET_FILE_NOT_FOUND;
    }
    else
    {
        File file = fs.open(fileName, "r");

        if (false == file)
        {
            LOG_ERROR("Failed to open file %s.", fileName.c_str());
            ret = RET_FILE_NOT_FOUND;
        }
        else
        {
            const size_t fileSize = file.size();

            if (RIFF_HEADER_SIZE > fileSize)
            {
                LOG_ERROR("File %s is too small for a WebP file.", fileName.c_str());
                ret = RET_FILE_FORMAT_INVALID;
            }
            else
            {
                m_webpData = m_dataAllocator.allocateArray(fileSize);

                if (nullptr == m_webpData)
                {
                    LOG_ERROR("Out of memory for WebP file %s.", fileName.c_str());
                    ret = RET_OUT_OF_MEMORY;
                }
                else if (fileSize != file.read(m_webpData, fileSize))
                {
                    LOG_ERROR("Failed to read WebP file %s.", fileName.c_str());
                    ret = RET_FILE_FORMAT_INVALID;
                }
                else
                {
                    m_webpDataSize = fileSize;
                }
            }

            file.close();
        }
    }

    return ret;
}

WebpImgPlayer::Ret WebpImgPlayer::processWebPData()
{
    Ret                   ret = RET_OK;
    WebPBitstreamFeatures features;

    if (false == isRiffContainerComplete(m_webpData, m_webpDataSize))
    {
        LOG_ERROR("No complete RIFF/WEBP container.");
        ret = RET_FILE_FORMAT_INVALID;
    }
    else if (VP8_STATUS_OK != WebPGetFeatures(m_webpData, m_webpDataSize, &features))
    {
        LOG_ERROR("Invalid WebP bitstream.");
        ret = RET_FILE_FORMAT_INVALID;
    }
    else if ((0 >= features.width) ||
             (0 >= features.height))
    {
        LOG_ERROR("Invalid WebP image size.");
        ret = RET_FILE_FORMAT_INVALID;
    }
    /* WebP's VP8X canvas fields are 24 bit wide and reach 16.7 million, while
     * the bitmap dimensions are 16 bit. A canvas which does not fit must be
     * rejected here, because narrowing it would silently wrap and create a
     * small bitmap which then reports success for a huge image.
     */
    else if ((DIM_MAX_REPRESENTABLE < features.width) ||
             (DIM_MAX_REPRESENTABLE < features.height))
    {
        LOG_ERROR("WebP canvas of %dx%d can not be represented.", features.width, features.height);
        ret = RET_IMG_TOO_BIG;
    }
    else
    {
        m_width       = static_cast<int16_t>(features.width);
        m_height      = static_cast<int16_t>(features.height);
        m_isAnimation = (0 != features.has_animation);

        if (false == m_isAnimation)
        {
            ret = initStaticImage();
        }
        else
        {
            ret = initAnimatedImage();
        }
    }

    return ret;
}

WebpImgPlayer::Ret WebpImgPlayer::initStaticImage()
{
    Ret               ret = RET_OK;
    WebPDecoderConfig config;

    if (0 == WebPInitDecoderConfig(&config))
    {
        LOG_ERROR("libwebp version mismatch.");
        ret = RET_FILE_FORMAT_UNSUPPORTED;
    }
    else if (false == m_bitmap.create(static_cast<uint16_t>(m_width), static_cast<uint16_t>(m_height)))
    {
        LOG_ERROR("WebP image is too big for the available memory.");
        ret = RET_IMG_TOO_BIG;
    }
    else
    {
        /* MODE_rgbA delivers premultiplied alpha, which composites the image
         * against a black background. The LED matrix has no alpha channel.
         */
        config.output.colorspace = MODE_rgbA;

        if (VP8_STATUS_OK != WebPDecode(m_webpData, m_webpDataSize, &config))
        {
            LOG_ERROR("Failed to decode WebP image.");
            ret = RET_FILE_FORMAT_INVALID;
        }
        else if ((static_cast<int>(m_width) != config.output.width) ||
                 (static_cast<int>(m_height) != config.output.height))
        {
            LOG_ERROR("WebP image size mismatch.");
            ret = RET_FILE_FORMAT_INVALID;
        }
        else
        {
            copyPixelsToBitmap(config.output.u.RGBA.rgba, static_cast<size_t>(config.output.u.RGBA.stride));
        }

        WebPFreeDecBuffer(&config.output);
    }

    return ret;
}

WebpImgPlayer::Ret WebpImgPlayer::initAnimatedImage()
{
    Ret                    ret = RET_OK;
    WebPData               webpData;
    WebPAnimDecoderOptions decOptions;

    webpData.bytes = m_webpData;
    webpData.size  = m_webpDataSize;

    if (0 == WebPAnimDecoderOptionsInit(&decOptions))
    {
        LOG_ERROR("libwebp version mismatch.");
        ret = RET_FILE_FORMAT_UNSUPPORTED;
    }
    else
    {
        WebPAnimInfo animInfo;

        /* MODE_rgbA delivers premultiplied alpha, which composites every frame
         * against a black background. The LED matrix has no alpha channel.
         */
        decOptions.color_mode = MODE_rgbA;
        m_animDecoder         = WebPAnimDecoderNew(&webpData, &decOptions);

        if (nullptr == m_animDecoder)
        {
            LOG_ERROR("Failed to create WebP animation decoder.");
            ret = RET_FILE_FORMAT_INVALID;
        }
        else if (0 == WebPAnimDecoderGetInfo(m_animDecoder, &animInfo))
        {
            LOG_ERROR("Failed to get WebP animation info.");
            ret = RET_FILE_FORMAT_INVALID;
        }
        else if ((static_cast<uint32_t>(m_width) != animInfo.canvas_width) ||
                 (static_cast<uint32_t>(m_height) != animInfo.canvas_height))
        {
            LOG_ERROR("WebP canvas size mismatch.");
            ret = RET_FILE_FORMAT_INVALID;
        }
        /* No own bitmap for an animation: the decoder keeps the current frame
         * in its canvas and the player draws from it.
         */
        else
        {
            m_loopCount     = animInfo.loop_count;
            m_currentLoop   = 0U;
            m_lastTimestamp = 0;
            m_hasMoreFrames = (0 != WebPAnimDecoderHasMoreFrames(m_animDecoder));

            if (false == decodeNextFrame())
            {
                LOG_ERROR("Failed to decode the first WebP frame.");
                ret = RET_FILE_FORMAT_INVALID;
            }
        }
    }

    return ret;
}

void WebpImgPlayer::advanceAnimation()
{
    if (true == m_hasMoreFrames)
    {
        if (false == decodeNextFrame())
        {
            /* A decoding error ends the animation. The last frame stays. */
            m_isFinished = true;
            m_isStopped  = true;
            m_timer.stop();
        }
    }
    else
    {
        /* All frames of one loop cycle are shown. */
        m_isFinished = true;
        ++m_currentLoop;

        /* Is another loop cycle allowed? The infinite loop mode overrules the
         * loop count of the stream. A loop count of 0 means infinite as well.
         */
        if ((true == m_isInfiniteLoop) ||
            (0U == m_loopCount) ||
            (m_currentLoop < m_loopCount))
        {
            restartAnimation();
        }
        else
        {
            m_isStopped = true;
            m_timer.stop();
        }
    }
}

bool WebpImgPlayer::decodeNextFrame()
{
    bool isSuccessful = false;

    if ((nullptr != m_animDecoder) &&
        (true == m_hasMoreFrames))
    {
        uint8_t* buffer    = nullptr;
        int      timestamp = 0;

        if (0 == WebPAnimDecoderGetNext(m_animDecoder, &buffer, &timestamp))
        {
            LOG_ERROR("Failed to decode WebP frame.");
        }
        else
        {
            /* The timestamp is the end of the frame, therefore the difference
             * to the previous one is the duration of the current frame.
             */
            const int32_t duration = timestamp - m_lastTimestamp;

            /* The buffer is owned by the decoder and valid until the next
             * frame is decoded or the decoder is deleted.
             */
            m_frameRgba            = buffer;

            if (0 < duration)
            {
                m_timer.start(static_cast<uint32_t>(duration));
            }
            else
            {
                m_timer.stop();
            }

            m_lastTimestamp = timestamp;
            m_hasMoreFrames = (0 != WebPAnimDecoderHasMoreFrames(m_animDecoder));
            isSuccessful    = true;
        }
    }

    return isSuccessful;
}

void WebpImgPlayer::restartAnimation()
{
    WebPAnimDecoderReset(m_animDecoder);

    m_lastTimestamp = 0;
    m_hasMoreFrames = (0 != WebPAnimDecoderHasMoreFrames(m_animDecoder));

    if (false == decodeNextFrame())
    {
        /* A decoding error ends the animation. The last frame stays. */
        m_isStopped = true;
        m_timer.stop();
    }
}

void WebpImgPlayer::copyPixelsToBitmap(const uint8_t* rgba, size_t stride)
{
    if (nullptr != rgba)
    {
        int16_t y;

        for (y = 0; y < m_height; ++y)
        {
            const uint8_t* line = &rgba[static_cast<size_t>(y) * stride];
            int16_t        x;

            for (x = 0; x < m_width; ++x)
            {
                const size_t idx = static_cast<size_t>(x) * BYTES_PER_PIXEL;

                /* The alpha channel is dropped, because the decoder delivers
                 * premultiplied pixels, which are composited against black.
                 */
                const Color color(line[idx + 0U], line[idx + 1U], line[idx + 2U]);

                m_bitmap.drawPixel(x, y, color);
            }
        }
    }
}

void WebpImgPlayer::drawRgba(YAGfx& gfx, int16_t x, int16_t y, const uint8_t* rgba, size_t stride) const
{
    if (nullptr != rgba)
    {
        int16_t dy;

        for (dy = 0; dy < m_height; ++dy)
        {
            const uint8_t* line = &rgba[static_cast<size_t>(dy) * stride];
            int16_t        dx;

            for (dx = 0; dx < m_width; ++dx)
            {
                const size_t idx = static_cast<size_t>(dx) * BYTES_PER_PIXEL;

                /* The alpha channel is dropped, because the decoder delivers
                 * premultiplied pixels, which are composited against black.
                 */
                const Color color(line[idx + 0U], line[idx + 1U], line[idx + 2U]);

                gfx.drawPixel(x + dx, y + dy, color);
            }
        }
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/**
 * Get a 32-bit value in little endian byte order.
 *
 * @param[in] data  Buffer with at least 4 bytes. Must not be nullptr.
 *
 * @return Value in host byte order.
 */
static uint32_t getUInt32Le(const uint8_t* data)
{
    return (static_cast<uint32_t>(data[0U]) << 0U) |
           (static_cast<uint32_t>(data[1U]) << 8U) |
           (static_cast<uint32_t>(data[2U]) << 16U) |
           (static_cast<uint32_t>(data[3U]) << 24U);
}

/**
 * Is the given buffer a complete RIFF/WEBP container?
 *
 * It verifies the RIFF and WEBP signatures and that the buffer is not shorter
 * than the size which the container header announces. This detects a truncated
 * file before any decoder allocates memory for it.
 *
 * @param[in] data  Buffer with the file content. May be nullptr.
 * @param[in] size  Size of the buffer in bytes.
 *
 * @return If the container is complete, it will return true otherwise false.
 */
static bool isRiffContainerComplete(const uint8_t* data, size_t size)
{
    bool isComplete = false;

    if ((nullptr != data) &&
        (RIFF_HEADER_SIZE <= size))
    {
        const bool isSignatureValid = (0 == memcmp(&data[0U], "RIFF", 4U)) &&
                                      (0 == memcmp(&data[8U], "WEBP", 4U));

        if (true == isSignatureValid)
        {
            /* The RIFF size counts everything after the size field itself. */
            const uint64_t announcedSize = static_cast<uint64_t>(getUInt32Le(&data[4U])) + 8U;

            if (announcedSize <= static_cast<uint64_t>(size))
            {
                isComplete = true;
            }
        }
    }

    return isComplete;
}

#endif /* (0 != CONFIG_IMAGE_FORMAT_WEBP_ENABLE) */
