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
 * @brief  Sprite sheet
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup gfx
 *
 * @{
 */

#ifndef SPRITE_SHEET_H
#define SPRITE_SHEET_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <YAGfxMap.h>
#include <YAGfxBitmap.h>
#include <FS.h>

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
        m_texture(),
        m_textureMap(m_texture),
        m_frame(m_textureMap),
        m_frameCnt(0U),
        m_fps(DEFAULT_FPS),
        m_repeat(true),
        m_isForward(true),
        m_framesX(0U),
        m_framesY(0U),
        m_currentFrameX(0U),
        m_currentFrameY(0U)
    {
    }

    /**
     * Constructs a sprite sheet by copy.
     * 
     * @param[in] spriteSheet   The sprite sheet, which to copy from.
     */
    SpriteSheet(const SpriteSheet& spriteSheet) :
        m_texture(spriteSheet.m_texture),
        m_textureMap(spriteSheet.m_textureMap),
        m_frame(spriteSheet.m_frame),
        m_frameCnt(spriteSheet.m_frameCnt),
        m_fps(spriteSheet.m_fps),
        m_repeat(spriteSheet.m_repeat),
        m_isForward(spriteSheet.m_isForward),
        m_framesX(spriteSheet.m_framesX),
        m_framesY(spriteSheet.m_framesY),
        m_currentFrameX(spriteSheet.m_currentFrameX),
        m_currentFrameY(spriteSheet.m_currentFrameY)
    {
    }

    /**
     * Destroys the sprite sheet.
     */
    ~SpriteSheet()
    {
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
     * Does the animation runs infinite or just once?
     * 
     * @return If the animation is continuously repeated, it will return true otherwise false.
     */
    bool isRepeatedInfinite() const
    {
        return m_repeat;
    }

    /**
     * Set whether the animation is repeated continuously or it runs just once.
     * 
     * @param[in] repeat    If set to true, the animation will run infinite.
     */
    void repeatInfinite(bool repeat)
    {
        m_repeat = repeat;
    }

    /**
     * Is the animation running forward or backward?
     * 
     * @return If the animation runs forward, it will return true otherwise false.
     */
    bool isForward() const
    {
        return m_isForward;
    }

    /**
     * Set animation direction to forward or backward.
     * 
     * @param[in] isForward If set to true, it will run forwards otherwise backwards.
     */
    void setForward(bool isForward)
    {
        m_isForward = isForward;
    }

    /**
     * Get frame width in pixels.
     * 
     * @return Frame width in pixels
     */
    uint16_t getFrameWidth() const
    {
        return m_frame.getWidth();
    }

    /**
     * Get frame height in pixels.
     * 
     * @return Frame height in pixels
     */
    uint16_t getFrameHeight() const
    {
        return m_frame.getHeight();
    }

    /**
     * @brief Get the Frame object
     * 
     * @return YAGfxBitmap& 
     */
    const YAGfxBitmap& getFrame() const
    {
        return m_frame;
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
     * If the number of frames in the texture is not available, it will be assumed
     * that the bitmap texture is filled completly.
     * 
     * If the parameter whether the animation runs infinite is not available, it will
     * be assumed that it shall run infinite.
     * 
     * The animation direction will be reset to forward.
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
     * Reset animation sequence.
     * 
     * If the animation repeats only once, this will trigger that it will be
     * repeated once again.
     */
    void reset();

    /**
     * Release the internal pixel buffer with texture.
     */
    void release()
    {
        m_texture.release();
    }

    /**
     * Use this function to determine whether a sprite sheet is loaded or not.
     * 
     * @return If no sprite sheet is loaded, it will return true otherwise false.
     */
    bool isEmpty() const
    {
        return !m_texture.isAllocated();
    }

private:

    /**
     * Default frames per seconds (FPS).
     */
    static const uint8_t    DEFAULT_FPS = 12U;

    YAGfxDynamicBitmap  m_texture;          /**< Texture image. */
    YAGfxMap            m_textureMap;       /**< Map canvas over the texture image. */
    YAGfxOverlayBitmap  m_frame;            /**< The current frame. */
    uint8_t             m_frameCnt;         /**< Number of frames in the texture. */
    uint8_t             m_fps;              /**< Number of frames per second. */
    bool                m_repeat;           /**< Repeat animation continuously or it runs just once. */
    bool                m_isForward;        /**< The animation (order of sprites) runs forwards and backwards. */
    uint8_t             m_framesX;          /**< Number of frames on texture x-axis. */
    uint8_t             m_framesY;          /**< Number of frames on texture y-axis. */
    uint8_t             m_currentFrameX;    /**< x index of current selected frame. */
    uint8_t             m_currentFrameY;    /**< y index of current selected frame. */

    /**
     * Is the current frame the very first one?
     * 
     * @return If the current frame is the very first one, it will return true otherwise false.
     */
    bool isBegin() const;

    /**
     * Is the current frame the very last one?
     * 
     * @return If the current frame is the very last one, it will return true otherwise false.
     */
    bool isEnd() const;

    /**
     * Move current frame to the begin.
     */
    void moveToBegin();

    /**
     * Move current frame to the end.
     */
    void moveToEnd();

    /**
     * Move current frame one frame forward, but only if the animation repeats infinite.
     */
    void moveForward();

    /**
     * Move current frame one frame backward, but only if the animation repeats infinite.
     */
    void moveBackward();
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* SPRITE_SHEET_H */

/** @} */