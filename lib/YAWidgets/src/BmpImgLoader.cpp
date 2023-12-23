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
 * @brief  Bitmap image loader
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "BmpImgLoader.h"

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and classes
 *****************************************************************************/

/** Bitmap format signature "BM" */
static const uint16_t BMP_SIGNATURE = 0x4D42;

/**
 * To store general information about the bitmap image file.
 * Not needed after the file is loaded in memory.
 */
typedef struct _BmpFileHeader
{
    uint16_t    signature; /**< Bitmap signature for file format identification. */
    uint32_t    fileSize;  /**< The size of the BMP file in bytes. */
    uint16_t    reserved1; /**< Reserved */
    uint16_t    reserved2; /**< Reserved */
    uint32_t    offset;    /**< The offset, i.e. starting address, of the byte where the bitmap image data (pixel array) can be found. */

} __attribute__ ((packed)) BmpFileHeader;

/**
 * Device independent header (DIB): The bitmap info header (size: 40 bytes).
 */
typedef struct _BmpInfoHeader
{
    uint32_t    headerSize;     /**< The size of this header. */
    int32_t     imageWidth;     /**< The bitmap width in pixels. */
    int32_t     imageHeight;    /**< The bitmap height in pixels. */
    uint16_t    planes;         /**< The number of color planes, must be 1. */
    uint16_t    bpp;            /**< The number of bits per pixel, which is the color depth of the image. Typical values are 1, 4, 8, 16 24 and 32. */
    uint32_t    compression;    /**< The compression method being used. */
    uint32_t    imageSize;      /**< The image size. This is the size of the raw bitmap data; a dummy 0 can be given for BI_RGB bitmaps. */
    uint32_t    horizonalRes;   /**< The horizontal resolution of the image. (pixel per metre, signed integer) */
    uint32_t	verticalRes;    /**< The vertical resolution of the image. (pixel per metre, signed integer) */
    uint32_t    paletteColors;  /**< The number of colors in the color palette, or 0 to default to 2^n */
    uint32_t    importantColors;/**< The number of important colors used, or 0 when every color is important; generally ignored. */

} __attribute__ ((packed)) BmpInfoHeader;

/**
 * Device independent header (DIB): The bitmap v5 header (size: 124 bytes).
 */
typedef struct _BmpV5Header
{
    BmpInfoHeader   infoHeader;             /**< Bitmap info header. */
    uint32_t        redChannelBitmask;      /**< Read channel bitmask. */
    uint32_t        greenChannelBitmask;    /**< Read channel bitmask. */
    uint32_t        blueChannelBitmask;     /**< Read channel bitmask. */
    uint32_t        alphaChannelBitmask;    /**< Alpha channel bitmask. */
    uint32_t        colorSpaceType;         /**< Color space type. */
    uint32_t        colorSpaceEndpoints;    /**< Color space endpoints. */
    uint32_t        gammaRedChannel;        /**< Gamma for red channel. */
    uint32_t        gammaGreenChannel;      /**< Gamma for green channel. */
    uint32_t        gammaBlueChannel;       /**< Gamma for blue channel. */
    uint32_t        intent;                 /**< Intent. */
    uint32_t        iccProfileData;         /**< ICC profile data. */
    uint32_t        iccProfileSize;         /**< ICC profile size. */
    uint32_t        reserved;               /**< Reserved. */

} __attribute__ ((packed)) BmpV5Header;

typedef enum
{
    COMPRESSION_METHOD_RGB          = 0,    /**< None */
    COMPRESSION_METHOD_RLE8         = 1,    /**< RLE 8-bit/pixel */
    COMPRESSION_METHOD_RLE4         = 2,    /**< RLE 4-bit/pixel */
    COMPRESSION_METHOD_BITFIELDS    = 3,    /**< Bitmasks indicate where to get the base colors */
    COMPRESSION_METHOD_JPEG         = 4,    /**< RLE-24 */
    COMPRESSION_METHOD_PNG          = 5,    /**< ? */
    COMPRESSION_METHOD_ALPHA        = 6,    /**< RGBA bit field masks */
    COMPRESSION_METHOD_CMYK         = 11,   /**< None */
    COMPRESSION_METHOD_CMYK_RLE8    = 12,   /**< RLE-8 */
    COMPRESSION_METHOD_CMYK_RLE4    = 13    /**< RLE-4 */

} CompressionMethod;

/******************************************************************************
 * Prototypes
 *****************************************************************************/

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/******************************************************************************
 * Public Methods
 *****************************************************************************/

BmpImgLoader::Ret BmpImgLoader::load(FS& fs, const String& fileName, YAGfxDynamicBitmap& bitmap)
{
    Ret     ret = RET_OK;
    File    fd  = fs.open(fileName);

    if (false == fd)
    {
        ret = RET_FILE_NOT_FOUND;
    }
    else
    {
        BmpFileHeader   bmpFileHeader;
        BmpV5Header     dibHeader;

        if (false == loadBmpFileHeader(fd, bmpFileHeader))
        {
            ret = RET_FILE_FORMAT_INVALID;
        }
        /* Is it not a bitmap file? */
        else if (BMP_SIGNATURE != bmpFileHeader.signature)
        {
            ret = RET_FILE_FORMAT_UNSUPPORTED;
        }
        else if (false == loadDibHeader(fd, dibHeader))
        {
            ret = RET_FILE_FORMAT_UNSUPPORTED;
        }
        /* Planes must be 1.
         * Compression is not supported.
         * Palette colors are not supported.
         * 24 and 32 bits per pixel are supported.
         */
        else if ((1U != dibHeader.infoHeader.planes) ||
                 (COMPRESSION_METHOD_RGB != dibHeader.infoHeader.compression) ||
                 (0U < dibHeader.infoHeader.paletteColors) ||
                 ((24U != dibHeader.infoHeader.bpp) && (32U != dibHeader.infoHeader.bpp)))
        {
            ret = RET_FILE_FORMAT_UNSUPPORTED;
        }
        /* Supported image size is limited. */
        else if ((UINT16_MAX < dibHeader.infoHeader.imageWidth) ||
                 (UINT16_MAX < dibHeader.infoHeader.imageHeight))
        {
            ret = RET_IMG_TOO_BIG;
        }
        else
        {
            uint16_t width  = abs(dibHeader.infoHeader.imageWidth);
            uint16_t height = abs(dibHeader.infoHeader.imageHeight);

            bitmap.release();

            if (false == bitmap.create(width, height))
            {
                ret = RET_IMG_TOO_BIG;
            }
            else
            {
                uint32_t    pos             = 0;
                uint32_t    rowSize         = 0;
                int16_t     y               = 0;
                bool        isTopToBottom   = false;
                uint16_t    bytePerPixel    = dibHeader.infoHeader.bpp / 8;

                /* The bits representing the bitmap pixels are packed in rows.
                 * The size of each row is rounded up to a multiple of 4 bytes
                 * (a 32-bit DWORD) by padding.
                 */
                rowSize =  (dibHeader.infoHeader.bpp * bitmap.getWidth() + 31) / 32 * 4;

                /* ImageHeight is expressed as a negative number for top-down images. */
                if (0 > dibHeader.infoHeader.imageHeight)
                {
                    isTopToBottom = true;
                }

                while((bitmap.getHeight() > y) && (RET_OK == ret))
                {
                    int16_t x = 0;

                    while((bitmap.getWidth() > x) && (RET_OK == ret))
                    {
                        uint8_t lineBuffer[bytePerPixel];

                        if (false == isTopToBottom)
                        {
                            pos = x * bytePerPixel + (bitmap.getHeight() - y - 1) * rowSize;
                        }
                        else
                        {
                            pos = x * bytePerPixel + y * rowSize;
                        }

                        if (false == fd.seek(bmpFileHeader.offset + pos, SeekSet))
                        {
                            ret = RET_FILE_FORMAT_INVALID;
                        }
                        else if (sizeof(lineBuffer) != fd.read(lineBuffer, sizeof(lineBuffer)))
                        {
                            ret = RET_FILE_FORMAT_INVALID;
                        }
                        else
                        {
                            Color color(lineBuffer[2], lineBuffer[1], lineBuffer[0]);

                            bitmap.drawPixel(x, y, color);
                        }

                        ++x;
                    }

                    ++y;
                }
            }
        }

        fd.close();
    }

    if (RET_OK != ret)
    {
        bitmap.release();
    }

    return ret;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

bool BmpImgLoader::loadBmpFileHeader(File& fd, BmpFileHeader& header)
{
    bool        isSuccessful    = true;
    void*       vHeader         = &header;
    uint8_t*    u8Header        = static_cast<uint8_t*>(vHeader);

    if (sizeof(header) != fd.read(u8Header, sizeof(header)))
    {
        isSuccessful = false;
    }

    return isSuccessful;
}

bool BmpImgLoader::loadDibHeader(File& fd, BmpV5Header& header)
{
    bool        isSuccessful    = true;
    uint32_t    dibHeaderSize   = 0U;
    void*       vDibHeaderSize  = &dibHeaderSize;
    uint8_t*    u8DibHeaderSize = static_cast<uint8_t*>(vDibHeaderSize);

    memset(&header, 0, sizeof(header));

    if (sizeof(dibHeaderSize) != fd.read(u8DibHeaderSize, sizeof(dibHeaderSize)))
    {
        isSuccessful = false;
    }
    else
    {
        const uint32_t  DIB_HEADER_SIZE     = sizeof(BmpInfoHeader);
        const uint32_t  DIB_HEADER_V5_SIZE  = 124U;
        void*           vHeader             = &header;
        uint8_t*        u8Header            = static_cast<uint8_t*>(vHeader);

        if (false == fd.seek(fd.position() - sizeof(dibHeaderSize), SeekSet))
        {
            isSuccessful = false;
        }
        else if ((DIB_HEADER_SIZE != dibHeaderSize) &&
                 (DIB_HEADER_V5_SIZE != dibHeaderSize))
        {
            isSuccessful = false;
        }
        else
        {
            if (DIB_HEADER_V5_SIZE == dibHeaderSize)
            {
                dibHeaderSize = sizeof(BmpV5Header);
            }

            if (dibHeaderSize != fd.read(u8Header, dibHeaderSize))
            {
                isSuccessful = false;
            }
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
