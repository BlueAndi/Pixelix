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
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "GifImgPlayer.h"
#include "LzwDecoder.h"

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and classes
 *****************************************************************************/

/** GIF signature */
static const char*  GIF_SIGNATURE   = "GIF"; /* 3 byte signature, incl. string termination. */

/** Supported GIF version */
static const char*  GIF_VERSION     = "89a"; /* 3 byte version, incl. string termination. */

/**
 * The main block ids.
 */
typedef enum : uint8_t
{
    BLOCK_ID_EXTENSION          = 0x21U,    /**< Extension */
    BLOCK_ID_IMAGE_DESCRIPTOR   = 0x2CU,    /**< Image descriptor */
    BLOCK_ID_TRAILER            = 0x3BU     /**< Trailer */

} BlockId;

/**
 * Supported extension labels which gives the GIF parser the information about
 * the kind of extension.
 */
typedef enum : uint8_t
{
    LABEL_PLAIN_TEXT_EXTENSION      = 0x01U,    /**< Plain text extension */
    LABEL_GRAPHIC_CONTROL_EXTENSION = 0xF9U,    /**< Graphic control extension */
    LABEL_COMMENT_EXTENSION         = 0xFEU,    /**< Comment extension */
    LABEL_APPLICATION_EXTENSION     = 0xFFU     /**< Application extension */

} ExtensionLabel;

/**
 * To store general information about the GIF image file.
 * Not needed after the file is loaded in memory.
 */
typedef struct _GifFileHeader
{
    uint8_t signature[3U];  /**< Signature */
    uint8_t version[3U];    /**< Version */

} __attribute__ ((packed)) GifFileHeader;

/**
 * Packed field out of the logical screen descriptor.
 */
typedef struct _LsdPackedField
{
    uint8_t globalColorTableSizeExp : 3;    /**< Size exponent (2^(N+1)) of global color table */
    uint8_t sortFlag : 1;                   /**< Sort flag */
    uint8_t colorResolution : 3;            /**< Color resolution */
    uint8_t globalColorTableFlag : 1;       /**< Global color table flag */

} __attribute__ ((packed)) LsdPackedField;

/**
 * The logical screen descriptor.
 */
typedef struct _LogicalScreenDescriptor
{
    uint16_t        canvasWidth;        /**< Canvas width in pixel */
    uint16_t        canvasHeight;       /**< Canvas height in pixel */
    LsdPackedField  packedField;        /**< Packed field */
    uint8_t         bgColorIndex;       /**< Background color index */
    uint8_t         pixelAspectRatio;   /**< Pixel aspect ratio */

} __attribute__ ((packed)) LogicalScreenDescriptor;

/**
 * Packed field out of the graphic control extension.
 */
typedef struct _GcePackedField
{
    uint8_t transparentColorFlag : 1;   /**< Transparent color flag */
    uint8_t userInputFlag : 1;          /**< User input flag */
    uint8_t disposalMethod : 3;         /**< Disposal method */
    uint8_t reserved : 3;               /**< Reserved for future use */

} __attribute__ ((packed)) GcePackedField;

/**
 * The graphic control extension.
 */
typedef struct _GraphicControlExtension
{
    GcePackedField  packedField;            /**< Packed field */
    uint16_t        delayTime;              /**< Delay time */
    uint8_t         transparentColorIndex;  /**< Transparent color index */

} __attribute__ ((packed)) GraphicControlExtension;

/**
 * Packed field out of the image descriptor.
 */
typedef struct _IdPackedField
{
    uint8_t localColorTableSizeExp : 3;     /**< Size exponent (2^(N+1)) of local color table */
    uint8_t reserved : 2;                   /**< Reserved for future use */
    uint8_t sortFlag : 1;                   /**< Sort flag */
    uint8_t interlaceFlag : 1;              /**< Interlace flag */
    uint8_t localColorTableFlag : 1;        /**< Local color table flag */

} __attribute__ ((packed)) IdPackedField;

/**
 * The image descriptor.
 */
typedef struct _ImageDescriptor
{
    uint16_t        imageLeft;      /**< x-coordinate inside the canvas. */
    uint16_t        imageTop;       /**< y-coordinate inside the canvas. */
    uint16_t        imageWidth;     /**< Image width in pixel. */
    uint16_t        imageHeight;    /**< Image height in pixel. */
    IdPackedField   packedField;    /**< Packed field */

} __attribute__ ((packed)) ImageDescriptor;

/**
 * The application extension.
 */
typedef struct _ApplicationExtension
{
    uint8_t identifier[8U];         /**< Application identifier */
    uint8_t authenticationCode[3U]; /**< Application authentication code */

}  __attribute__ ((packed)) ApplicationExtension;

/******************************************************************************
 * Prototypes
 *****************************************************************************/

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/******************************************************************************
 * Public Methods
 *****************************************************************************/

GifImgPlayer::Ret GifImgPlayer::open(FS& fs, const String& fileName)
{
    Ret ret = RET_OK;
    
    /* File already opened? */
    if (true == m_fd)
    {
        ret = RET_FILE_ALREADY_OPENED;
    }
    /* Open file */
    else
    {
        m_fd = fs.open(fileName);

        /* File not found? */
        if (false == m_fd)
        {
            ret = RET_FILE_NOT_FOUND;
        }
    }

    /* If file is opened, the parsing will be started. */
    if (RET_OK == ret)
    {
        GifFileHeader           gifFileHeader;
        LogicalScreenDescriptor logicalScreenDescriptor;

        /* Read the GIF file header. */
        if (false == read(m_fd, &gifFileHeader, sizeof(gifFileHeader)))
        {
            ret = RET_FILE_FORMAT_INVALID;
        }
        /* Is it not a supported GIF file? */
        else if (false == isFileSupported(gifFileHeader))
        {
            ret = RET_FILE_FORMAT_UNSUPPORTED;
        }
        /* Read the logical screen descriptor. */
        else if (false == read(m_fd, &logicalScreenDescriptor, sizeof(logicalScreenDescriptor)))
        {
            ret = RET_FILE_FORMAT_INVALID;
        }
        else
        {
            /* Reset */
            m_loopCount             = 0U;
            m_delay                 = 0U;
            m_isTransparencyEnabled = false;
            m_isAnimation           = false;
            m_timer.stop();

            /* Store the image width and height.
             * Might be used later to fill the image with a background color.
             * See disposal method.
             */
            m_width     = logicalScreenDescriptor.canvasWidth;
            m_height    = logicalScreenDescriptor.canvasHeight;

            /* Global color table available? */
            if (0U != logicalScreenDescriptor.packedField.globalColorTableFlag)
            {
                size_t globalColorTableSize = calcColorTableSize(logicalScreenDescriptor.packedField.globalColorTableSizeExp);
                
                m_globalColorTableLength    =  globalColorTableSize / sizeof(PaletteColor);
                m_globalColorTable          = new(std::nothrow) PaletteColor[m_globalColorTableLength];

                /* Out of memory? */
                if (nullptr == m_globalColorTable)
                {
                    m_globalColorTableLength = 0U;
                    ret = RET_IMG_TOO_BIG;
                }
                /* Read global color table. */
                else if (false == read(m_fd, m_globalColorTable, globalColorTableSize))
                {
                    ret = RET_FILE_FORMAT_INVALID;
                }
                else
                {
                    ;
                }
            }
        }
    }

    /* Clean up in case a error happended. */
    if (RET_OK != ret)
    {
        m_fd.close();

        if (nullptr != m_globalColorTable)
        {
            delete[] m_globalColorTable;
            
            m_globalColorTable          = nullptr;
            m_globalColorTableLength    = 0U;
        }
    }

    return ret;
}

void GifImgPlayer::close()
{
    m_fd.close();

    if (nullptr != m_globalColorTable)
    {
        delete[] m_globalColorTable;
        
        m_globalColorTable          = nullptr;
        m_globalColorTableLength    = 0U;
    }

    if (nullptr != m_localColorTable)
    {
        delete[] m_localColorTable;
        
        m_localColorTable       = nullptr;
        m_localColorTableLength = 0U;
    }
}

bool GifImgPlayer::play(YAGfx& gfx)
{
    bool isSuccessful = true;

    /* No GIF image opened? */
    if (false == m_fd)
    {
        isSuccessful = false;
    }
    /* Finished? */
    else if (true == m_isFinished)
    {
        /* Nothing to do. */
        ;
    }
    /* Delay? */
    else if ((true == m_timer.isTimerRunning()) &&
             (false == m_timer.isTimeout()))
    {
        /* Nothing to do, come back later. */
        ;
    }
    else
    {
        bool    isImageShown    = false;
        BlockId blockId         = BLOCK_ID_EXTENSION;

        m_canvas.setParentGfx(gfx);

        /* Walk through all blocks in the GIF image. */
        while((BLOCK_ID_TRAILER != blockId) && (false == isImageShown) && (true == isSuccessful))
        {
            /* Read block id. */
            if (false == read(m_fd, &blockId, sizeof(blockId)))
            {
                isSuccessful = false;
            }
            /* Is the block a extension? */
            else if (BLOCK_ID_EXTENSION == blockId)
            {
                isSuccessful = parseExtension(m_fd);
            }
            /* Is the block an image descriptor? */
            else if (BLOCK_ID_IMAGE_DESCRIPTOR == blockId)
            {
                isSuccessful = parseImageDescriptor(m_fd);

                if (true == isSuccessful)
                {
                    /* Animation? */
                    if (true == m_isAnimation)
                    {
                        m_timer.start(m_delay);
                        isImageShown = true;
                    }
                }
            }
            /* Is it the end of the image? */
            else if (BLOCK_ID_TRAILER == blockId)
            {
                /* Animation running? */
                if (true == m_isAnimation)
                {
                    /* Is animation limited to a specific number of repeats? */
                    if (0U < m_loopCount)
                    {
                        --m_loopCount;

                        /* Animation finished? */
                        if (0U == m_loopCount)
                        {
                            m_isFinished = true;
                            m_timer.stop();
                        }
                    }
                    /* Infinite animation. */
                    else
                    {
                        ;
                    }

                    if (false == m_isFinished)
                    {
                        /* Restart from begin. */
                        if (false == m_fd.seek(m_animationRestartPos, SeekSet))
                        {
                            isSuccessful = false;
                        }
                        else
                        {
                            /* Force to continuoue until a scene is shown. */
                            blockId = BLOCK_ID_EXTENSION;
                        }
                    }
                }
                else
                {
                    m_isFinished = true;
                }
            }
            /* Unknown block id? */
            else
            {
                isSuccessful = false;
            }
        }

        /* Clean-up required because of any error? */
        if (false == isSuccessful)
        {
            if (nullptr != m_localColorTable)
            {
                delete[] m_localColorTable;
                m_localColorTable       = nullptr;
                m_localColorTableLength = 0U;
            }

            close();
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

bool GifImgPlayer::read(File& fd, void* buffer, size_t size)
{
    bool        isSuccessful    = true;
    uint8_t*    u8Buffer        = static_cast<uint8_t*>(buffer);

    if (size != fd.read(u8Buffer, size))
    {
        isSuccessful = false;
    }

    return isSuccessful;
}

bool GifImgPlayer::isFileSupported(const GifFileHeader& header) const
{
    bool        isSupported     = true;
    uint8_t     index;

    for(index = 0U; index < sizeof(header.signature); ++index)
    {
        if (GIF_SIGNATURE[index] != header.signature[index])
        {
            isSupported = false;
            break;
        }
    }

    if (true == isSupported)
    {
        for(index = 0U; index < sizeof(header.version); ++index)
        {
            if (GIF_VERSION[index] != header.version[index])
            {
                isSupported = false;
                break;
            }
        }
    }

    return isSupported;
}

size_t GifImgPlayer::calcColorTableSize(uint8_t sizeExp) const
{
    uint8_t index   = 0U;
    size_t  size    = 1U;

    /* Calculation:
     * Size in byte = 3 * 2^(N + 1) 
     */

    /* Determine 2^(N + 1) */
    while(sizeExp >= index)
    {
        size *= 2U;

        ++index;
    }

    /* Consider RGB values */
    size *= 3U;

    return size;
}

bool GifImgPlayer::parseExtension(File& fd)
{
    bool            isSuccessful = true;
    ExtensionLabel  label;

        /* Load extension label. */
    if (false == read(fd, &label, sizeof(label)))
    {
        isSuccessful = false;
    }
    else
    {
        if (LABEL_GRAPHIC_CONTROL_EXTENSION == label)
        {
            isSuccessful = parseGraphicControlExentsion(fd);
        }
        else if (LABEL_APPLICATION_EXTENSION == label)
        {
            isSuccessful = parseApplicationExtension(fd);
        }
        /* Skip every other extension. */
        else
        {
            if (false == skipBlock(fd))
            {
                isSuccessful = false;
            }
        }
    }

    return isSuccessful;
}

bool GifImgPlayer::parseImageDescriptor(File& fd)
{
    bool            isSuccessful = true;
    ImageDescriptor imageDescriptor;

    if (false == read(fd, &imageDescriptor, sizeof(imageDescriptor)))
    {
        isSuccessful = false;
    }
    else
    {
        /* The image descriptor specifies the image left position and image
         * top position of where the image should begin on the canvas.
         */
        m_canvas.setOffsetX(imageDescriptor.imageLeft);
        m_canvas.setOffsetY(imageDescriptor.imageTop);
        m_canvas.setWidth(imageDescriptor.imageWidth);
        m_canvas.setHeight(imageDescriptor.imageHeight);

        /* Destroy any old local color table. */
        if (nullptr != m_localColorTable)
        {
            delete[] m_localColorTable;
            m_localColorTable       = nullptr;
            m_localColorTableLength = 0U;
        }
        
        /* Local color table available? */
        if (0U != imageDescriptor.packedField.localColorTableFlag)
        {
            size_t localColorTableSize = calcColorTableSize(imageDescriptor.packedField.localColorTableSizeExp);

            m_localColorTableLength = localColorTableSize / sizeof(PaletteColor);
            m_localColorTable       = new(std::nothrow) PaletteColor[m_localColorTableLength];

            if (nullptr == m_localColorTable)
            {
                m_localColorTableLength = 0U;

                isSuccessful = false;
            }
            else if (false == read(fd, m_localColorTable, localColorTableSize))
            {
                isSuccessful = false;
            }
            else
            {
                ;
            }
        }

        /* Process image data */
        if (true == isSuccessful)
        {
            uint8_t lzwMinCodeSize = 0U;

            if (false == read(fd, &lzwMinCodeSize, sizeof(lzwMinCodeSize)))
            {
                isSuccessful = false;
            }
            else
            {
                LzwDecoder                      lzwDecoder;
                LzwDecoder::ReadFromInStream    readFromCodeStreamFunc  =
                    [this](uint8_t& data) -> bool
                    {
                        return this->readFromCodeStream(data);
                    };
                LzwDecoder::WriteToOutStream    writeToIndexStreamFunc  =
                    [this](uint8_t data) -> bool
                    {
                        return this->writeToIndexStream(data);
                    };
                uint8_t                         blockTerminator         = 0U;

                /* Reset data block before start decoding the next block. */
                m_imageDataBlockIdx = 0U;
                m_imageDataBlockLength = 0U;

                /* Reset coordinates used for drawing. */
                m_posX = 0U;
                m_posY = 0U;

                lzwDecoder.init(lzwMinCodeSize);

                if (false == lzwDecoder.decode(readFromCodeStreamFunc, writeToIndexStreamFunc))
                {
                    isSuccessful = false;
                }

                /* After the image data, the block terminator marks the end. */
                if (sizeof(blockTerminator) != fd.read(&blockTerminator, sizeof(blockTerminator)))
                {
                    isSuccessful = false;
                }
                else if (0U != blockTerminator)
                {
                    isSuccessful = false;
                }
                else
                {
                    ;
                }
            }
        }
    }

    return isSuccessful;
}

bool GifImgPlayer::parseGraphicControlExentsion(File& fd)
{
    bool                    isSuccessful    = true;
    uint8_t                 blockSize       = 0x0U;
    uint8_t                 blockTerminator = 0x0U;
    GraphicControlExtension gce;

    if (sizeof(blockSize) != fd.read(&blockSize, sizeof(blockSize)))
    {
        isSuccessful = false;
    }
    else if (sizeof(GraphicControlExtension) != blockSize)
    {
        isSuccessful = false;
    }
    else if (false == read(fd, &gce, sizeof(gce)))
    {
        isSuccessful = false;
    }
    else if (sizeof(blockTerminator) != fd.read(&blockTerminator, sizeof(blockTerminator)))
    {
        isSuccessful = false;
    }
    else if (0U != blockTerminator)
    {
        isSuccessful = false;
    }
    else
    {
        m_delay                 = gce.delayTime;
        m_transparentColorIndex = gce.transparentColorIndex;

        if (0U == gce.packedField.transparentColorFlag)
        {
            m_isTransparencyEnabled = false;
        }
        else
        {
            m_isTransparencyEnabled = true;
        }

        /* Leave the image in place and draw the next image on top of it? */
        if (1U == gce.packedField.disposalMethod)
        {
            /* Nothing to do. */
            ;
        }
        /* The canvas should be restored to the background color?*/
        else if (2U == gce.packedField.disposalMethod)
        {
            PaletteColor*   paletteColor = &m_globalColorTable[m_bgColorIndex];
            Color           bgColor(paletteColor->red, paletteColor->green, paletteColor->blue);

            m_canvas.getParentGfx()->fillRect(0, 0, m_width, m_height, bgColor);
        }
        /* The decoder should restore the canvas to its previous state before the current image was drawn. */
        else if (3U == gce.packedField.disposalMethod)
        {
            /* Not supported. */
            ;
        }
        else
        {
            /* No disposal method is applied. */
        }
    }

    return isSuccessful;
}

bool GifImgPlayer::parseApplicationExtension(File& fd)
{
    bool                    isSuccessful    = true;
    uint8_t                 blockSize       = 0x0U;
    ApplicationExtension    appExt;

    /* Read application extension block size. */
    if (sizeof(blockSize) != fd.read(&blockSize, sizeof(blockSize)))
    {
        isSuccessful = false;
    }
    /*  Invalid? */
    else if (sizeof(appExt) != blockSize)
    {
        isSuccessful = false;
    }
    /* Read application extension. */
    else if (false == read(fd, &appExt, sizeof(appExt)))
    {
        isSuccessful = false;
    }
    else
    {
        const void* vAppIdentifier  = appExt.identifier;
        const char* appIdentifier   = static_cast<const char*>(vAppIdentifier);
        const void* vAppAuthCode    = appExt.authenticationCode;
        const char* appAuthCode     = static_cast<const char*>(vAppAuthCode);

        /* Only the NETSCAPE 2.0 application is supported for animatins. */
        if ((0 == strncmp(appIdentifier, "NETSCAPE", sizeof(appExt.identifier))) &&
            (0 == strncmp(appAuthCode, "2.0", sizeof(appExt.authenticationCode))))
        {
            isSuccessful = parseNetscape20subBlocks(fd);
        }
        else
        {
            /* Skip all sub-blocks, which are application specific. */
            isSuccessful = skipBlock(fd);
        }
    }

    return isSuccessful;
}

bool GifImgPlayer::parseNetscape20subBlocks(File& fd)
{
    bool    isSuccessful    = true;
    uint8_t subBlockSize    = 0xFFU;
    uint8_t subBlockId      = 0U;
    uint8_t blockTerminator = 0U;

    /* Read sub-block size. */
    if (sizeof(subBlockSize) != fd.read(&subBlockSize, sizeof(subBlockSize)))
    {
        isSuccessful = false;
    }
    /* The sub-block size shall contain the id and the loop counter. */
    else if (0x03U != subBlockSize)
    {
        isSuccessful = false;
    }
    /* Read the id. */
    else if (sizeof(subBlockId) != fd.read(&subBlockId, sizeof(subBlockId)))
    {
        isSuccessful = false;
    }
    /* The id shall be 0x01. */
    else if (0x01U != subBlockId)
    {
        isSuccessful = false;
    }
    /* Read the loop counter. */
    else if (false == read(fd, &m_loopCount, sizeof(m_loopCount)))
    {
        isSuccessful = false;
    }
    /* Read block terminator. */
    else if (sizeof(blockTerminator) != fd.read(&blockTerminator, sizeof(blockTerminator)))
    {
        isSuccessful = false;
    }
    /* Verify block terminator. */
    else if (0x00U != blockTerminator)
    {
        isSuccessful = false;
    }
    else
    {
        m_isAnimation = true;

        /* Store position after application extension to know where to restart
         * the animation.
         */
        m_animationRestartPos = fd.position();
    }

    return isSuccessful;
}

bool GifImgPlayer::skipBlock(File& fd)
{
    bool    isSuccessful    = true;
    uint8_t blockSize       = 0xFFU;

    while((0U < blockSize) && (true == isSuccessful))
    {
        if (sizeof(blockSize) != fd.read(&blockSize, sizeof(blockSize)))
        {
            isSuccessful = false;
        }
        else if (0U < blockSize)
        {
            if (false == fd.seek(blockSize, SeekCur))
            {
                isSuccessful = false;
            }
        }
        else
        {
            ;
        }
    }

    return isSuccessful;
}

size_t GifImgPlayer::loadImageDataBlock(File& fd, uint8_t* block, size_t size)
{
    bool        isSuccessful    = true;
    uint8_t     blockSize       = 0U;

    if (false == read(fd, &blockSize, sizeof(blockSize)))
    {
        isSuccessful = false;
    }
    else if (0U < blockSize)
    {
        if (size < blockSize)
        {
            isSuccessful = false;
        }
        else if (false == read(fd, block, blockSize))
        {
            isSuccessful = false;
        }
        else
        {
            ;
        }
    }

    if (false == isSuccessful)
    {
        blockSize = 0U;
    }

    return blockSize;
}

bool GifImgPlayer::readFromCodeStream(uint8_t& data)
{
    bool isSuccessful = true;

    if ((0U == m_imageDataBlockLength) ||
        (m_imageDataBlockLength <= m_imageDataBlockIdx))
    {
        m_imageDataBlockLength = loadImageDataBlock(m_fd, m_imageDataBlock, IMAGE_DATA_BLOCK_SIZE);

        if (0U == m_imageDataBlockLength)
        {
            isSuccessful = false;
        }
        else
        {
            m_imageDataBlockIdx = 0U;
        }
    }

    if (m_imageDataBlockLength > m_imageDataBlockIdx)
    {
        data = m_imageDataBlock[m_imageDataBlockIdx];
        ++m_imageDataBlockIdx;
    }

    return isSuccessful;
}

bool GifImgPlayer::writeToIndexStream(uint8_t data)
{
    bool            isSuccessful        = false;
    PaletteColor*   colorTable          = (nullptr != m_localColorTable) ? m_localColorTable : m_globalColorTable;
    size_t          colorTableLength    = (nullptr != m_localColorTable) ? m_localColorTableLength : m_globalColorTableLength;

    if ((nullptr != colorTable) &&
        (colorTableLength > data))
    {
        if ((false == m_isTransparencyEnabled) ||
            (   (true == m_isTransparencyEnabled) &&
                (m_transparentColorIndex != data)
            ))
        {
            PaletteColor*   paletteColor = &colorTable[data];
            Color           color(paletteColor->red, paletteColor->green, paletteColor->blue);

            m_canvas.drawPixel(m_posX, m_posY, color);
        }

        ++m_posX;
        if (m_canvas.getWidth() <= m_posX)
        {
            m_posX = 0;
            ++m_posY;
        }

        isSuccessful = true;
    }

    return isSuccessful;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
