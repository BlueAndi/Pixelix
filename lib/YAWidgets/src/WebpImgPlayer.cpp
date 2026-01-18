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

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/******************************************************************************
 * Public Methods
 *****************************************************************************/

WebpImgPlayer::WebpImgPlayer() :
    m_dataAllocator(),
    m_webpData(nullptr),
    m_webpDataSize(0U),
    m_demuxer(nullptr),
    m_animDecoder(nullptr),
    m_bitmap(),
    m_width(0),
    m_height(0),
    m_isAnimation(false),
    m_isFinished(false),
    m_loopCount(0U),
    m_currentLoop(0U),
    m_timer(),
    m_lastTimestamp(0),
    m_hasMoreFrames(false)
{
}

WebpImgPlayer::WebpImgPlayer(const WebpImgPlayer& player) :
    m_dataAllocator(),
    m_webpData(nullptr),
    m_webpDataSize(0U),
    m_demuxer(nullptr),
    m_animDecoder(nullptr),
    m_bitmap(player.m_bitmap),
    m_width(player.m_width),
    m_height(player.m_height),
    m_isAnimation(player.m_isAnimation),
    m_isFinished(player.m_isFinished),
    m_loopCount(player.m_loopCount),
    m_currentLoop(player.m_currentLoop),
    m_timer(player.m_timer),
    m_lastTimestamp(player.m_lastTimestamp),
    m_hasMoreFrames(player.m_hasMoreFrames)
{
    /* Copy WebP data if available */
    if ((nullptr != player.m_webpData) && (0U < player.m_webpDataSize))
    {
        m_webpData = m_dataAllocator.allocateArray(player.m_webpDataSize);

        if (nullptr != m_webpData)
        {
            memcpy(m_webpData, player.m_webpData, player.m_webpDataSize);
            m_webpDataSize = player.m_webpDataSize;

            /* Note: Demuxer and animation decoder are not copied.
             * They need to be re-initialized when needed. */
        }
    }
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

        m_bitmap        = player.m_bitmap;
        m_width         = player.m_width;
        m_height        = player.m_height;
        m_isAnimation   = player.m_isAnimation;
        m_isFinished    = player.m_isFinished;
        m_loopCount     = player.m_loopCount;
        m_currentLoop   = player.m_currentLoop;
        m_timer         = player.m_timer;
        m_lastTimestamp = player.m_lastTimestamp;
        m_hasMoreFrames = player.m_hasMoreFrames;

        /* Copy WebP data if available */
        if ((nullptr != player.m_webpData) && (0U < player.m_webpDataSize))
        {
            m_webpData = m_dataAllocator.allocateArray(player.m_webpDataSize);

            if (nullptr != m_webpData)
            {
                memcpy(m_webpData, player.m_webpData, player.m_webpDataSize);
                m_webpDataSize = player.m_webpDataSize;
            }
        }
    }

    return *this;
}

WebpImgPlayer::Ret WebpImgPlayer::open(FS& fs, const String& fileName)
{
    Ret ret = RET_OK;

    /* Close any previously opened file */
    if ((nullptr != m_webpData) || (nullptr != m_demuxer) || (nullptr != m_animDecoder))
    {
        ret = RET_FILE_ALREADY_OPENED;
    }
    else if (false == fs.exists(fileName))
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
            size_t fileSize = file.size();

            if (0U == fileSize)
            {
                LOG_ERROR("WebP file is empty.");
                file.close();
                ret = RET_FILE_FORMAT_INVALID;
            }
            else
            {
                /* Allocate memory for file data */
                m_webpData = m_dataAllocator.allocateArray(fileSize);

                if (nullptr == m_webpData)
                {
                    LOG_ERROR("Out of memory for WebP file.");
                    file.close();
                    ret = RET_OUT_OF_MEMORY;
                }
                else
                {
                    size_t bytesRead = file.read(m_webpData, fileSize);

                    file.close();

                    if (bytesRead != fileSize)
                    {
                        LOG_ERROR("Failed to read WebP file.");
                        m_dataAllocator.deallocateArray(m_webpData);
                        m_webpData = nullptr;
                        ret        = RET_FILE_FORMAT_INVALID;
                    }
                    else
                    {
                        m_webpDataSize = fileSize;

                        /* Validate and process WebP data */
                        ret            = processWebPData();
                    }
                }
            }
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

    if (false == m_isAnimation)
    {
        /* Static image - just draw it */
        gfx.drawBitmap(x, y, m_bitmap);
        isSuccessful = true;
    }
    else
    {
        /* Animated image */
        if (nullptr != m_animDecoder)
        {
            /* Check if it's time to decode next frame */
            if (true == m_timer.isTimeout())
            {
                if (false == decodeNextFrame())
                {
                    /* Animation finished */
                    m_isFinished = true;

                    /* Check if we should loop */
                    if ((0U == m_loopCount) || (m_currentLoop < m_loopCount))
                    {
                        /* Reset for next loop */
                        WebPAnimDecoderReset(m_animDecoder);
                        m_lastTimestamp = 0;
                        m_hasMoreFrames = WebPAnimDecoderHasMoreFrames(m_animDecoder);
                        m_isFinished    = false;
                        ++m_currentLoop;

                        /* Decode first frame of next loop */
                        (void)decodeNextFrame();
                    }
                }
            }

            /* Draw current frame */
            gfx.drawBitmap(x, y, m_bitmap);
            isSuccessful = true;
        }
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
    /* Delete animation decoder */
    if (nullptr != m_animDecoder)
    {
        WebPAnimDecoderDelete(m_animDecoder);
        m_animDecoder = nullptr;
    }

    /* Delete demuxer */
    if (nullptr != m_demuxer)
    {
        WebPDemuxDelete(m_demuxer);
        m_demuxer = nullptr;
    }

    /* Free WebP data */
    if (nullptr != m_webpData)
    {
        m_dataAllocator.deallocateArray(m_webpData);
        m_webpData     = nullptr;
        m_webpDataSize = 0U;
    }

    /* Release bitmap */
    m_bitmap.release();

    /* Reset state */
    m_width         = 0;
    m_height        = 0;
    m_isAnimation   = false;
    m_isFinished    = false;
    m_loopCount     = 0U;
    m_currentLoop   = 0U;
    m_lastTimestamp = 0;
    m_hasMoreFrames = false;
}

WebpImgPlayer::Ret WebpImgPlayer::processWebPData()
{
    Ret ret    = RET_OK;
    int width  = 0;
    int height = 0;

    /* Validate WebP format */
    if (0 == WebPGetInfo(m_webpData, m_webpDataSize, &width, &height))
    {
        ret = RET_FILE_FORMAT_INVALID;
    }
    else if ((0 > width) || (0 > height))
    {
        ret = RET_FILE_FORMAT_INVALID;
    }
    else if ((INT16_MAX < width) || (height > INT16_MAX))
    {
        ret = RET_IMG_TOO_BIG;
    }
    else
    {
        WebPData webpData = { m_webpData, m_webpDataSize };

        m_width           = static_cast<int16_t>(width);
        m_height          = static_cast<int16_t>(height);

        /* Check if it's an animated WebP */
        m_demuxer         = WebPDemux(&webpData);

        if (nullptr == m_demuxer)
        {
            ret = RET_FILE_FORMAT_INVALID;
        }
        else
        {
            uint32_t flags = WebPDemuxGetI(m_demuxer, WEBP_FF_FORMAT_FLAGS);

            /* Check if animation flag is set */
            if (flags & 0x02U) /* ANIMATION_FLAG */
            {
                m_isAnimation = true;
                WebPDemuxDelete(m_demuxer);
                ret = initAnimatedImage();
            }
            else
            {
                m_isAnimation = false;
                WebPDemuxDelete(m_demuxer);
                ret = initStaticImage();
            }
        }
    }

    if (RET_OK != ret)
    {
        cleanup();
    }

    return ret;
}

WebpImgPlayer::Ret WebpImgPlayer::initStaticImage()
{
    Ret ret       = RET_OK;
    int width     = 0;
    int height    = 0;

    /* Decode the image */
    uint8_t* rgba = WebPDecodeRGBA(m_webpData, m_webpDataSize, &width, &height);

    if (nullptr == rgba)
    {
        ret = RET_FILE_FORMAT_INVALID;
    }
    else
    {
        /* Create bitmap and copy image data */
        if (false == m_bitmap.create(static_cast<uint16_t>(width), static_cast<uint16_t>(height)))
        {
            ret = RET_OUT_OF_MEMORY;
        }
        else
        {
            /* Convert RGBA to RGB565 format used by the bitmap */
            for (int y = 0; y < height; y++)
            {
                for (int x = 0; x < width; x++)
                {
                    size_t  idx   = (y * width + x) * 4;
                    uint8_t red   = rgba[idx + 0U];
                    uint8_t green = rgba[idx + 1U];
                    uint8_t blue  = rgba[idx + 2U];
                    uint8_t alpha = rgba[idx + 3U];

                    /* Handle transparency by blending with black background */
                    if (255U > alpha)
                    {
                        red   = (red * alpha) / 255U;
                        green = (green * alpha) / 255U;
                        blue  = (blue * alpha) / 255U;
                    }

                    Color color(red, green, blue);
                    m_bitmap.drawPixel(x, y, color);
                }
            }
        }

        /* Free the RGBA buffer allocated by WebP */
        WebPFree(rgba);
    }

    return ret;
}

WebpImgPlayer::Ret WebpImgPlayer::initAnimatedImage()
{
    Ret ret                         = RET_OK;

    /* Initialize animation decoder */
    WebPData               webpData = { m_webpData, m_webpDataSize };
    WebPAnimDecoderOptions dec_options;

    if (0 == WebPAnimDecoderOptionsInit(&dec_options))
    {
        ret = RET_FILE_FORMAT_UNSUPPORTED;
    }
    else
    {
        dec_options.color_mode = MODE_RGBA;

        m_animDecoder          = WebPAnimDecoderNew(&webpData, &dec_options);

        if (nullptr == m_animDecoder)
        {
            ret = RET_FILE_FORMAT_INVALID;
        }
        else
        {
            WebPAnimInfo anim_info;

            if (0 == WebPAnimDecoderGetInfo(m_animDecoder, &anim_info))
            {
                ret = RET_FILE_FORMAT_INVALID;
            }
            else
            {
                m_width       = static_cast<int16_t>(anim_info.canvas_width);
                m_height      = static_cast<int16_t>(anim_info.canvas_height);
                m_loopCount   = anim_info.loop_count;
                m_currentLoop = 0U;

                /* Create bitmap for frames */
                if (false == m_bitmap.create(static_cast<uint16_t>(m_width), static_cast<uint16_t>(m_height)))
                {
                    ret = RET_OUT_OF_MEMORY;
                }
                else
                {
                    m_hasMoreFrames = WebPAnimDecoderHasMoreFrames(m_animDecoder);

                    /* Decode first frame */
                    if (false == decodeNextFrame())
                    {
                        ret = RET_FILE_FORMAT_INVALID;
                    }
                }
            }
        }
    }

    return ret;
}

bool WebpImgPlayer::decodeNextFrame()
{
    bool isSuccessful = false;

    if ((nullptr != m_animDecoder) && (true == m_hasMoreFrames))
    {
        uint8_t* buf       = nullptr;
        int      timestamp = 0;

        if (0 != WebPAnimDecoderGetNext(m_animDecoder, &buf, &timestamp))
        {
            /* Calculate delay until next frame */
            int32_t delay = timestamp - m_lastTimestamp;

            /* Convert RGBA to RGB565 format used by the bitmap */
            for (int y = 0; y < m_height; y++)
            {
                for (int x = 0; x < m_width; x++)
                {
                    size_t  idx   = (y * m_width + x) * 4;
                    uint8_t red   = buf[idx + 0U];
                    uint8_t green = buf[idx + 1U];
                    uint8_t blue  = buf[idx + 2U];
                    uint8_t alpha = buf[idx + 3U];

                    /* Handle transparency by blending with black background */
                    if (255U > alpha)
                    {
                        red   = (red * alpha) / 255U;
                        green = (green * alpha) / 255U;
                        blue  = (blue * alpha) / 255U;
                    }

                    Color color(red, green, blue);
                    m_bitmap.drawPixel(x, y, color);
                }
            }

            /* Start timer for next frame */
            if (0 < delay)
            {
                m_timer.start(static_cast<uint32_t>(delay));
            }

            m_lastTimestamp = timestamp;
            m_hasMoreFrames = WebPAnimDecoderHasMoreFrames(m_animDecoder);
            isSuccessful    = true;
        }
    }

    return isSuccessful;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
