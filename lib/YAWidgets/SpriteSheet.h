/* MIT License
 *
 * Copyright (c) 2019 - 2022 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Sprite sheet
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup gfx
 *
 * @{
 */

#ifndef __SPRITE_SHEET_H__
#define __SPRITE_SHEET_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>

#include "BmpImg.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * The sprite sheet provides sprites used for animation.
 * Each sprite in the texture image, shall have the same fixed canvas size.
 * The texture image may contain sprites on the x-axis and on the y-axis.
 * It is allowed that the texture image contains gaps, but only in the last
 * row. It is assumed that all previous rows are complete filled with sprites.
 * 
 * The order of the sprites shall follow in x-direction from 0 to N and
 * continue in the next y row and so on.
 */
class SpriteSheet
{
public:

    /**
     * Constructs a sprite sheet, without texture image.
     */
    SpriteSheet() :
        m_frames(nullptr),
        m_frameCnt(0U),
        m_frameWidth(0U),
        m_frameHeight(0U),
        m_fps(DEFAULT_FPS),
        m_currentFrameIdx(0U)
    {
    }

    /**
     * Constructs a sprite sheet by copy.
     * 
     * @param[in] spriteSheet   The sprite sheet, which to copy from.
     */
    SpriteSheet(const SpriteSheet& spriteSheet) :
        m_frames(nullptr),
        m_frameCnt(spriteSheet.m_frameCnt),
        m_frameWidth(spriteSheet.m_frameWidth),
        m_frameHeight(spriteSheet.m_frameHeight),
        m_fps(spriteSheet.m_fps),
        m_currentFrameIdx(spriteSheet.m_currentFrameIdx)
    {
        copyFrames(spriteSheet.m_frames, spriteSheet.m_frameCnt);
    }

    /**
     * Destroys the sprite sheet.
     */
    ~SpriteSheet()
    {
        releaseFrames();
    }

    /**
     * Assgins a sprite sheet.
     * Note, the referenced bitmap image won't be assigned! You have to do this
     * separately.
     * 
     * @param[in] spriteSheet   The sprite sheet, which to copy from.
     * 
     * @return The sprite sheet itself.
     */
    SpriteSheet& operator=(const SpriteSheet& spriteSheet);

    /**
     * Get animation speed.
     * 
     * @return Frame per second
     */
    uint8_t getFPS() const
    {
        return m_fps;
    }

    /**
     * Set animation speed.
     * 
     * @param[in] fps   Frames per second
     */
    void setFPS(uint8_t fps)
    {
        m_fps = fps;
    }

    /**
     * Get frame width in pixels.
     * 
     * @return Frame width in pixels
     */
    uint16_t getFrameWidth() const
    {
        return m_frameWidth;
    }

    /**
     * Get frame height in pixels.
     * 
     * @return Frame height in pixels
     */
    uint16_t getFrameHeight() const
    {
        return m_frameHeight;
    }

    /**
     * Get current selected frame as color buffer. Or in other words, get current selected sprite
     * out of the sprite sheet.
     * 
     * @return If no texture image is available, it will return nullptr otherwise the frame buffer.
     */
    const Color* getFrame() const
    {
        const Color*    frameBuffer = nullptr;

        if (nullptr != m_frames)
        {
            frameBuffer = m_frames[m_currentFrameIdx].get();
        }

        return frameBuffer;
    }

    /**
     * Load sprite sheet texture image (.bmp) from the filesystem.
     * 
     * @param[in] fs            File system
     * @param[in] fileName      Name of the bitmap file in the filesystem
     * @param[in] frameWidth    Canvas frame width in pixels
     * @param[in] frameHeight   Canvas frame height in pixels
     * @param[in] frameCnt      Number of frames
     * @param[in] fps           Frames per second
     * 
     * @return If successful loaded, it will return true otherwise false.
     */
    bool loadTexture(FS& fs, const String& fileName, uint16_t frameWidth, uint16_t frameHeight, uint8_t frameCnt = 0, uint8_t fps = DEFAULT_FPS);

    /**
     * Load sprite sheet file (.sprite) and texture file (.bmp) from the filesystem.
     * 
     * @param[in] fs                    The filesystem
     * @param[in] spriteSheetFileName   Name of the sprite sheet file in the filesystem
     * @param[in] textureFileName       Name of the texture image file in the filesystem
     *
     * @return If successful loaded, it will return true otherwise false.
     */
    bool load(FS& fs, const String& spriteSheetFileName, const String& textureFileName);

    /**
     * Move frame to the next sprite.
     */
    void next();

    /**
     * Release the internal frame buffers.
     */
    void release()
    {
        releaseFrames();
    }

    /**
     * Use this function to determine whether a sprite sheet is loaded or not.
     * 
     * @return If no sprite sheet is loaded, it will return true otherwise false.
     */
    bool isEmpty() const
    {
        return (nullptr == m_frames);
    }

private:

    /**
     * Default frames per seconds (FPS).
     */
    static const uint8_t    DEFAULT_FPS = 12U;

    BmpImg*     m_frames;           /**< Array of frames, derived from a texture image. */
    uint8_t     m_frameCnt;         /**< Number of frames in the texture. */
    uint16_t    m_frameWidth;       /**< Width in pixels of a frame, which contains one sprite. */
    uint16_t    m_frameHeight;      /**< Height in pixels of a frame, which contains one sprite. */
    uint8_t     m_fps;              /**< Number of frames per second. */
    uint8_t     m_currentFrameIdx;  /**< Index of the current selected frame. */
    
    /**
     * Copy all source frames.
     * 
     * @param[in] frames    Source frames
     * @param[in] cnt       Number of frames
     */
    void copyFrames(BmpImg* frames, uint8_t cnt)
    {
        if (true == allocateFrames(cnt))
        {
            uint8_t idx = 0U;

            while(cnt > idx)
            {
                m_frames[idx] = frames[idx];

                ++idx;
            }
        }
    }

    /**
     * Allocate frames. If frames already allocated, they will be released.
     * 
     * @param[in] frameCnt  Number of frames to allocate.
     * 
     * @return If successful, it will return true otherwise false.
     */
    bool allocateFrames(uint8_t frameCnt)
    {
        bool isSuccessful = false;

        releaseFrames();

        m_frames = new BmpImg[frameCnt];

        if (nullptr != m_frames)
        {
            m_frameCnt = frameCnt;
            isSuccessful = true;
        }

        return isSuccessful;
    }

    /**
     * Release all frames.
     * If no frames are allocated, nothing will happen.
     */
    void releaseFrames()
    {
        if (nullptr != m_frames)
        {
            delete[] m_frames;

            m_frames    = nullptr;
            m_frameCnt  = 0U;
        }
    }
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __SPRITE_SHEET_H__ */

/** @} */