/* MIT License
 *
 * Copyright (c) 2019 - 2024 Andreas Merkle <web@blue-andi.de>
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
 * @brief  GIF image player
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup GFX
 *
 * @{
 */

#ifndef GIF_IMG_PLAYER_H
#define GIF_IMG_PLAYER_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <YAGfx.h>
#include <YAGfxCanvas.h>
#include <YAGfxBitmap.h>
#include <SimpleTimer.hpp>
#include <IGifLoader.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/* Forward declarations */
typedef struct _GifFileHeader GifFileHeader;
typedef struct _ApplicationExtension ApplicationExtension;

/**
 * Shows single image GIF files and plays animated GIF files, containing
 * several images inside.
 * 
 * Specification: https://www.w3.org/Graphics/GIF/spec-gif89a.txt
 */
class GifImgPlayer
{
public:

    /**
     * Construct a GIF image player object.
     */
    GifImgPlayer() :
        m_bitmap(),
        m_gifLoader(nullptr),
        m_canvas(&m_bitmap),
        m_bgColorIndex(0U),
        m_globalColorTable(nullptr),
        m_globalColorTableLength(0U),
        m_localColorTable(nullptr),
        m_localColorTableLength(0U),
        m_disposalMethod(DISPOSAL_METHOD_NO_ACTION),
        m_imageDataBlock(nullptr),
        m_imageDataBlockLength(0U),
        m_imageDataBlockIdx(0U),
        m_posX(0),
        m_posY(0),
        m_isTransparencyEnabled(false),
        m_transparentColorIndex(0U),
        m_isTrailerFound(false),
        m_restartFilePos(0U),
        m_loopCount(0U),
        m_delay(0U),
        m_timer(),
        m_isAnimation(false),
        m_isFinished(false)
    {
    }

    /**
     * Destroy the GIF image player object.
     */
    ~GifImgPlayer()
    {
        cleanup();
    }

    /**
     * Possible return values with more information.
     */
    enum Ret
    {
        RET_OK = 0,                     /**< Successful */
        RET_FILE_NOT_FOUND,             /**< File not found. */
        RET_FILE_ALREADY_OPENED,        /**< A file is already opened. Close it first. */
        RET_FILE_FORMAT_INVALID,        /**< Invalid file format. */
        RET_FILE_FORMAT_UNSUPPORTED,    /**< File format is not supported. */
        RET_IMG_TOO_BIG                 /**< Image size is too big. */
    };

    /**
     * Open a GIF file.
     * 
     * @param[in] fs        Filesystem to use
     * @param[in] fileName  Name of the GIF file.
     * @param[in] toMem     If true, the GIF file will be loaded to memory and the file is closed immediately.
     * 
     * @return Status informtion
     */
    Ret open(FS& fs, const String& fileName, bool toMem = false);

    /**
     * Close the GIF file.
     */
    void close();

    /**
     * Show the image or update the image in case of an animated GIF.
     * Call it periodically to support animations. The timing of the
     * image changes will be handled internally.
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
     * If the trailer is found in the GIF data stream, it means its the end.
     * Animations will automatically start again, depended on the loop counter value,
     * which is part of the GIF stream.
     * 
     * @return If one complete frame loop cycle is done, it will return true otherwise false.
     */
    bool isTrailerFound() const
    {
        return m_isTrailerFound;
    }

    /**
     * Get image width.
     * Note, the GIF must be opened, otherwise it will return 0.
     * 
     * @return Image width in pixels 
     */
    int16_t getWidth() const
    {
        return m_bitmap.getWidth();
    }

    /**
     * Get image height.
     * Note, the GIF must be opened, otherwise it will return 0.
     * 
     * @return Image height in pixels 
     */
    int16_t getHeight() const
    {
        return m_bitmap.getHeight();
    }

private:

    /**
     * Image data block size in byte.
     */
    static const size_t IMAGE_DATA_BLOCK_SIZE = 256U;

    /**
     * A palette color used for the global color table.
     */
    typedef struct _PaletteColor
    {
        uint8_t red;    /**< Red */
        uint8_t green;  /**< Green */
        uint8_t blue;   /**< Blue */

    } __attribute__ ((packed)) PaletteColor;

    /**
     * The disposal method indicates the way in which the graphic is to
     * be treated after being displayed.
     */
    enum DisposalMethod
    {
        DISPOSAL_METHOD_NO_ACTION = 0,          /**< No disposal specified. The decoder is not required to take any action. */
        DISPOSAL_METHOD_NO_DISPOSE,             /**< Do not dispose. The graphic is to be left in place. */
        DISPOSAL_METHOD_RESTORE_TO_BACKGROUND,  /**< Restore to background color. The area used by the graphic must be restored to the background color. */
        DISPOSAL_METHOD_RESTORE_TO_PREVIOUS     /**< Restore to previous. The decoder is required to restore the area overwritten by the graphic with what was there prior to rendering the graphic. */
    };

    YAGfxDynamicBitmap  m_bitmap;                   /**< The bitmap contains the last drawn scene. */
    IGifLoader*         m_gifLoader;                /**< GIF file loader used to read the file. */
    YAGfxCanvas         m_canvas;                   /**< Canvas used for drawing each scene. Its position and size follows the image descriptor. */
    uint8_t             m_bgColorIndex;             /**< Background color index. Used by disposal method. */
    PaletteColor*       m_globalColorTable;         /**< Global color table. */
    size_t              m_globalColorTableLength;   /**< Number of palette colors in the global color table. */
    PaletteColor*       m_localColorTable;          /**< Local color table. */
    size_t              m_localColorTableLength;    /**< Number of palette colors in the local color table. */
    DisposalMethod      m_disposalMethod;           /**< Disposal method of the last graphic control extension block.  */
    uint8_t*            m_imageDataBlock;           /**< Image data block buffer. See IMAGE_DATA_BLOCK_SIZE for fixed size in byte. */
    size_t              m_imageDataBlockLength;     /**< Image data block length in bytes (fill level). */
    size_t              m_imageDataBlockIdx;        /**< Read index to the image data block. */
    int16_t             m_posX;                     /**< Current x-coordinate in the canvas, used inside the LZW decoder callback. */
    int16_t             m_posY;                     /**< Current y-coordinate in the canvas, used inside the LZW decoder callback. */
    bool                m_isTransparencyEnabled;    /**< Is transparency enabled or not? */
    uint8_t             m_transparentColorIndex;    /**< Index of the transparent color. */
    bool                m_isTrailerFound;           /**< Is the trailer found? */
    
    /* Only animation relevant variables. */
    size_t          m_restartFilePos;           /**< File position used to restart the animation. */
    uint16_t        m_loopCount;                /**< Number of animation repeats. 0 means infinite. */
    uint32_t        m_delay;                    /**< Delay in ms used between animation scenes. */
    SimpleTimer     m_timer;                    /**< Timer used for animations. */
    bool            m_isAnimation;              /**< GIF contais several scenes which to animate. */
    bool            m_isFinished;               /**< Scenes are finished. */

    /**
     * Clean-up and release all allocated memory.
     */
    void cleanup();

    /**
     * Verifies whether the file format is supported or not.
     * 
     * @param[in] header    GIF file header
     * 
     * @return If GIF format is supported, it will return true otherwise false.
     */
    bool isFileSupported(const GifFileHeader& header) const;

    /**
     * Calculate the color table size in bytes from the size exponent.
     * 
     * @param[in] sizeExp   Size exponent
     * 
     * @return Color table size in bytes.
     */
    size_t calcColorTableSize(uint8_t sizeExp) const;

    /**
     * Parse extemsopm.
     * 
     * @return If successful, it will return true otherwise false.
     */
    bool parseExtension();

    /**
     * Parse image descriptor.
     * 
     * @return If successful, it will return true otherwise false.
     */
    bool parseImageDescriptor();

    /**
     * Apply the disposal method.
     */
    void applyDisposalMethod();

    /**
     * Parse graphic control extension.
     * 
     * @return If successful, it will return true otherwise false.
     */

    bool parseGraphicControlExentsion();

    /**
     * Parse application extension.
     * 
     * @return If successful, it will return true otherwise false.
     */
    bool parseApplicationExtension();

    /**
     * Parse NETSCAPE 2.0 sub-blocks.
     * 
     * @return If successful, it will return true otherwise false.
     */
    bool parseNetscape20subBlocks();

    /**
     * Skips the current block in the file.
     * Note, the file descriptor must be just before the block size!
     * 
     * @return If successful, it will return true otherwise false.
     */
    bool skipBlock();

    /**
     * Load a single image data block.
     * 
     * @param[out]  block   Block buffer
     * @param[in]   size    Block buffer size (at least 256 bytes)
     * 
     * @return Read number of data bytes.
     */
    size_t loadImageDataBlock(uint8_t* block, size_t size);

    /**
     * Callback used by LZW decoder to read data from the code stream.
     * 
     * @param[out] data Code stream byte
     * 
     * @return If successful read, it will return true otherwise false.
     */
    bool readFromCodeStream(uint8_t& data);

    /**
     * Callback used by LZW decoder to write data to the index stream.
     * It will update the canvas accordingly.
     * 
     * @param[in] data  Index for color table
     * 
     * @return If successful written, it will return true otherwise false.
     */
    bool writeToIndexStream(uint8_t data);

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* GIF_IMG_PLAYER_H */

/** @} */