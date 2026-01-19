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

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <YAGfx.h>
#include <YAGfxBitmap.h>
#include <SimpleTimer.hpp>
#include <TypedAllocator.hpp>
#include <PsAllocator.hpp>
#include <FS.h>
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
 * Note: For animated WebP images its required to have the complete
 *       WebP file in memory. Because there is no streaming decoder for animated
 *       WebP files available in the libwebp library.
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
        RET_IMG_TOO_BIG,             /**< Image size is too big. */
        RET_OUT_OF_MEMORY            /**< Out of memory. */
    };

    /**
     * Open a WebP file.
     * The file will be loaded into memory.
     *
     * @param[in] fs        Filesystem to use
     * @param[in] fileName  Name of the WebP file.
     *
     * @return Status information
     */
    Ret open(FS& fs, const String& fileName);

    /**
     * Close the WebP file.
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
     * Animations will automatically start again, depending on the loop counter value,
     * which is part of the WebP stream.
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
     *
     * @return If animation, it will return true otherwise false.
     */
    bool isAnimation() const
    {
        return m_isAnimation;
    }

private:

    /**
     * Data allocator type definition.
     */
    typedef TypedAllocator<uint8_t, PsAllocator> DataAllocator;

    DataAllocator                                m_dataAllocator; /**< uint8_t allocator. */
    uint8_t*                                     m_webpData;      /**< WebP file data in memory. */
    size_t                                       m_webpDataSize;  /**< Size of WebP file data in bytes. */
    WebPDemuxer*                                 m_demuxer;       /**< WebP demuxer. */
    WebPAnimDecoder*                             m_animDecoder;   /**< WebP animation decoder. */
    YAGfxDynamicBitmap                           m_bitmap;        /**< The bitmap contains the current frame. */
    int16_t                                      m_width;         /**< Image width in pixels. */
    int16_t                                      m_height;        /**< Image height in pixels. */
    bool                                         m_isAnimation;   /**< WebP contains several frames which to animate. */
    bool                                         m_isFinished;    /**< Animation cycle is finished. */
    uint32_t                                     m_loopCount;     /**< Number of animation repeats. 0 means infinite. */
    uint32_t                                     m_currentLoop;   /**< Current loop iteration. */
    SimpleTimer                                  m_timer;         /**< Timer used for animations. */
    int32_t                                      m_lastTimestamp; /**< Last frame timestamp. */
    bool                                         m_hasMoreFrames; /**< Indicates if more frames are available. */

    /**
     * Clean-up and release all allocated memory.
     */
    void cleanup();

    /**
     * Process WebP data after loading from file.
     * Validates format and initializes appropriate decoder.
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
     * Decode next animation frame.
     *
     * @return If successful, it will return true otherwise false.
     */
    bool decodeNextFrame();
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* WEBP_IMG_PLAYER_H */

/** @} */
