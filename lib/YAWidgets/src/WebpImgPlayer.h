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
 * @file   WebpImgPlayer.h
 * @brief  WebP image player
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup GFX
 *
 * @{
 */

#ifndef WEBP_IMG_PLAYER_H
#define WEBP_IMG_PLAYER_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/* The WebP support is only available if the build switch is enabled. Note, an
 * undefined switch evaluates to 0 and therefore disables everything below,
 * including the libwebp includes.
 */
#if (0 != CONFIG_IMAGE_FORMAT_WEBP_ENABLE)

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <stddef.h>
#include <FS.h>
#include <YAGfx.h>
#include <YAGfxBitmap.h>
#include <SimpleTimer.hpp>
#include <TypedAllocator.hpp>
#include <PsAllocator.hpp>
#include <webp/demux.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Shows single image WebP files and plays animated WebP files, containing
 * several frames inside.
 *
 * Uses the libwebp library for decoding.
 *
 * Note: The complete WebP file is loaded into memory, because the libwebp
 *       library provides no streaming decoder for animated WebP files.
 *
 * Note: The LED matrix has no alpha channel. Every pixel is therefore
 *       composited against a black background, which is achieved by requesting
 *       premultiplied output (MODE_rgbA) from the libwebp decoder.
 */
class WebpImgPlayer
{
public:

    /**
     * Construct a WebP image player object.
     */
    WebpImgPlayer();

    /**
     * Construct the WebP image player by copying another player.
     *
     * Note: The copy is initialized from the image data of the given player.
     *       An animation therefore restarts with its first frame.
     *
     * @param[in] player   WebP image player, which to copy.
     */
    WebpImgPlayer(const WebpImgPlayer& player);

    /**
     * Destroy the WebP image player object.
     */
    ~WebpImgPlayer();

    /**
     * Assigns a WebP image player.
     *
     * Note: The destination is initialized from the image data of the given
     *       player. An animation therefore restarts with its first frame.
     *
     * @param[in] player   WebP image player, which to assign.
     *
     * @return WebP image player.
     */
    WebpImgPlayer& operator=(const WebpImgPlayer& player);

    /**
     * Possible return values with more information.
     */
    enum Ret
    {
        RET_OK = 0,                  /**< Successful */
        RET_FILE_NOT_FOUND,          /**< File not found. */
        RET_FILE_ALREADY_OPENED,     /**< A file is already opened. Close it first. */
        RET_FILE_FORMAT_INVALID,     /**< Invalid file format. */
        RET_FILE_FORMAT_UNSUPPORTED, /**< File format is not supported. */
        RET_IMG_TOO_BIG,             /**< Image is too big: its canvas size can not be represented in 16 bit, or its bitmap can not be allocated. */
        RET_OUT_OF_MEMORY            /**< Out of memory for an allocation which is not driven by the image size, e.g. the file buffer or a decoder structure. */
    };

    /**
     * Open a WebP file.
     * The file will be loaded into memory.
     *
     * If it fails, the player stays closed and all resources are released.
     *
     * @param[in] fs        Filesystem to use.
     * @param[in] fileName  Name of the WebP file.
     *
     * @return Status information
     */
    Ret open(FS& fs, const String& fileName);

    /**
     * Close the WebP file and release all related resources.
     */
    void close();

    /**
     * Show the image or update the image in case of an animated WebP.
     * Call it periodically to support animations. The timing of the
     * frame changes will be handled internally.
     *
     * With coordinates in the parent canvas, the image position can be
     * manipulated.
     *
     * @param[in] gfx   Graphic functions of the parent canvas.
     * @param[in] x     x-coordinate of the parent canvas.
     * @param[in] y     y-coordinate of the parent canvas.
     *
     * @return If successful, it will return true otherwise false.
     */
    bool play(YAGfx& gfx, int16_t x = 0, int16_t y = 0);

    /**
     * Is one complete frame loop cycle done?
     *
     * It is set by the play() call which showed the last frame of a cycle and
     * it is cleared by the next play() call. Animations will automatically
     * start again, depending on the loop count which is part of the WebP
     * stream.
     *
     * Note, the WebP must be opened, otherwise it will return false.
     *
     * @return If one complete frame loop cycle is done, it will return true otherwise false.
     */
    bool isAnimationFinished() const
    {
        return m_isFinished;
    }

    /**
     * Get image width.
     * Note, the WebP must be opened, otherwise it will return 0.
     *
     * @return Image width in pixels
     */
    int16_t getWidth() const
    {
        return m_width;
    }

    /**
     * Get image height.
     * Note, the WebP must be opened, otherwise it will return 0.
     *
     * @return Image height in pixels
     */
    int16_t getHeight() const
    {
        return m_height;
    }

    /**
     * Is the image an animation?
     * Note, the WebP must be opened, otherwise it will return false.
     *
     * @return If animation, it will return true otherwise false.
     */
    bool isAnimation() const
    {
        return m_isAnimation;
    }

    /**
     * Set infinite animation mode.
     *
     * If enabled, the animation will run infinite, regardless of the loop count
     * which is part of the WebP stream.
     *
     * This general setting has effect only for animated WebP images.
     * There must no WebP be opened to set it.
     *
     * @param[in] enable    Enable/disable infinite animation mode.
     */
    void setInfiniteAnimation(bool enable)
    {
        m_isInfiniteLoop = enable;
    }

private:

    /**
     * Number of bytes per pixel in the decoded libwebp output buffer.
     */
    static const size_t BYTES_PER_PIXEL = 4U;

    /**
     * Data allocator type definition.
     */
    typedef TypedAllocator<uint8_t, PsAllocator> DataAllocator;

    DataAllocator                                m_dataAllocator;  /**< uint8_t allocator. */
    uint8_t*                                     m_webpData;       /**< WebP file data in memory. */
    size_t                                       m_webpDataSize;   /**< Size of WebP file data in bytes. */
    WebPAnimDecoder*                             m_animDecoder;    /**< WebP animation decoder. Only used for animations. */
    YAGfxDynamicBitmap                           m_bitmap;         /**< The bitmap contains the current frame. */
    int16_t                                      m_width;          /**< Image width in pixels. */
    int16_t                                      m_height;         /**< Image height in pixels. */
    bool                                         m_isAnimation;    /**< WebP contains several frames which to animate. */
    bool                                         m_isFinished;     /**< Animation cycle is finished. */
    bool                                         m_isStopped;      /**< Animation stopped for good, only the last frame is redrawn. */
    uint32_t                                     m_loopCount;      /**< Number of animation repeats. 0 means infinite. */
    uint32_t                                     m_currentLoop;    /**< Number of already finished animation cycles. */
    bool                                         m_isInfiniteLoop; /**< Animation is in infinite loop mode. It will overrule any loop count. */
    SimpleTimer                                  m_timer;          /**< Timer used for animations. */
    int32_t                                      m_lastTimestamp;  /**< Timestamp of the last decoded frame in ms. */
    bool                                         m_hasMoreFrames;  /**< Indicates if more frames are available. */

    /**
     * Clean-up and release all allocated memory. The player is reset to the
     * state right after construction.
     */
    void cleanup();

    /**
     * Take over the image of the given player by copying its file data and
     * initializing the own decoder with it.
     *
     * If anything fails, the player will be left closed.
     *
     * @param[in] player   WebP image player, which image to take over.
     */
    void copyFrom(const WebpImgPlayer& player);

    /**
     * Read the complete file into memory.
     *
     * On success m_webpData and m_webpDataSize are valid, otherwise the
     * clean-up is up to the caller.
     *
     * @param[in] fs        Filesystem to use.
     * @param[in] fileName  Name of the WebP file.
     *
     * @return If successful, it will return RET_OK otherwise error code.
     */
    Ret readFileIntoMemory(FS& fs, const String& fileName);

    /**
     * Process WebP data after loading from file.
     * Validates the format and initializes the appropriate decoder.
     *
     * @return If successful, it will return RET_OK otherwise error code.
     */
    Ret processWebPData();

    /**
     * Initialize static WebP image.
     *
     * @return If successful, it will return RET_OK otherwise error code.
     */
    Ret initStaticImage();

    /**
     * Initialize animated WebP image.
     *
     * @return If successful, it will return RET_OK otherwise error code.
     */
    Ret initAnimatedImage();

    /**
     * Show the next animation frame or finish the current loop cycle, if the
     * last frame was shown.
     */
    void advanceAnimation();

    /**
     * Decode next animation frame and start the timer with its duration.
     *
     * @return If successful, it will return true otherwise false.
     */
    bool decodeNextFrame();

    /**
     * Restart the animation with its first frame.
     */
    void restartAnimation();

    /**
     * Copy the decoded premultiplied RGBA pixels to the bitmap. The alpha
     * channel is dropped, because the pixels are already composited against
     * black by the decoder.
     *
     * @param[in] rgba      Decoded premultiplied RGBA pixels. Must not be nullptr.
     * @param[in] stride    Stride in bytes from one scanline to the next.
     */
    void copyPixelsToBitmap(const uint8_t* rgba, size_t stride);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* (0 != CONFIG_IMAGE_FORMAT_WEBP_ENABLE) */

#endif /* WEBP_IMG_PLAYER_H */

/** @} */
