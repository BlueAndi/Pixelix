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
 * @file   PngImgLoader.cpp
 * @brief  PNG image loader
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "PngImgLoader.h"

#if (0 != CONFIG_IMAGE_FORMAT_PNG_ENABLE)

#include <Logging.h>

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and classes
 *****************************************************************************/

/**
 * Allocator for the PNGdec decoder object. The decoder object is around
 * 40 kByte, which shall not be taken from the internal heap as long as PSRAM
 * is available.
 */
typedef TypedAllocator<PNG, PsAllocator> PngAllocator;

/******************************************************************************
 * Prototypes
 *****************************************************************************/

static PngImgLoader::Ret mapDecoderError(int rc);
static uint8_t           compositeOnBlack(uint8_t channel, uint8_t alpha);
static uint8_t           getPackedSample(const uint8_t* line, int index, int bitsPerSample);
static uint8_t           scaleSampleTo8Bit(uint8_t sample, int bitsPerSample);
static int               calcLinePitch(int width, int bitsPerSample, int pixelType);

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/** Largest image dimension in pixels which YAGfxDynamicBitmap::create() and
 * the int16_t pixel coordinates of the bitmap can represent. This is a
 * representability limit of the types, not a size policy. How large an image
 * may really be, is decided by the allocation of its bitmap alone.
 */
static const int DIM_MAX_REPRESENTABLE = static_cast<int>(INT16_MAX);

/** Opaque alpha value. */
static const uint8_t ALPHA_OPAQUE      = 255U;

/** Number of bits in a byte, which is the number of bits per unpacked sample.
 *
 * A PNG carries 1, 2, 4, 8 or 16 bits per sample. PNGdec refuses 16 bits on
 * its own with PNG_UNSUPPORTED_FEATURE, the smaller depths arrive packed
 * several samples to a byte and are unpacked here.
 */
static const int BITS_PER_BYTE         = 8;

/** Number of bytes which PNGdec spends on alignment inside its line buffer.
 *
 * It starts the first line at offset 15 and rounds the start of the second
 * line up to the next multiple of 16 minus 1, which costs at most another 16
 * bytes. See the layout at png.inl:775-782.
 */
static const int ALIGNMENT_OVERHEAD    = 32;

/** Largest de-filtered image line in bytes which PNGdec can really buffer.
 *
 * PNGdec keeps the current and the previous image line in one single buffer
 * of PNG_MAX_BUFFERED_PIXELS bytes, PNGIMAGE::ucPixels at PNGdec.h:161. Its
 * own check however only refuses a line of PNG_MAX_BUFFERED_PIXELS bytes or
 * more, png.inl:643, which is nearly twice what the buffer holds. Two lines of
 * pitch + 1 bytes plus the alignment need 2 * pitch + 32 bytes, so everything
 * above (PNG_MAX_BUFFERED_PIXELS - 32) / 2 is written past the buffer: first
 * over the decoder's own file buffer, which follows it, and then past the
 * whole decoder object. The loader therefore refuses such an image before it
 * lets PNGdec decode it.
 *
 * The bound is derived from the macro instead of being written out, because
 * the macro is #ifndef guarded, PNGdec.h:54-55, and may be overridden from the
 * build. With the default of 2562 it evaluates to 1265.
 */
static const int PITCH_MAX             = (PNG_MAX_BUFFERED_PIXELS - ALIGNMENT_OVERHEAD) / 2;

/** Smallest file which can carry a PNG signature and an IHDR chunk, in bytes.
 *
 * It is the number of bytes PNGdec reads to parse the image information,
 * png.inl:595. Anything smaller is no PNG file and is refused before a buffer
 * is allocated for it.
 */
static const size_t FILE_SIZE_MIN      = 33U;

/** Largest file which can be handed to the decoder, in bytes.
 *
 * PNG::openRAM() takes the buffer size as a signed int.
 */
static const size_t FILE_SIZE_MAX      = static_cast<size_t>(INT32_MAX);

/** Offset of the transparency information inside the PNGdec palette in bytes.
 *
 * PNGdec keeps the 256 RGB triplets of the PLTE chunk first and the 256 alpha
 * values of the tRNS chunk behind them.
 */
static const int PALETTE_ALPHA_OFFSET  = 768;

/******************************************************************************
 * Public Methods
 *****************************************************************************/

PngImgLoader::Ret PngImgLoader::load(FS& fs, const String& fileName, YAGfxDynamicBitmap& bitmap)
{
    uint8_t* fileData = nullptr;
    size_t   fileSize = 0U;
    Ret      ret      = readFileIntoMemory(fs, fileName, fileData, fileSize);

    if (RET_OK == ret)
    {
        PngAllocator pngAllocator;
        PNG*         png = pngAllocator.allocate();

        if (nullptr == png)
        {
            LOG_ERROR("Out of memory for the PNG decoder.");
            ret = RET_OUT_OF_MEMORY;
        }
        else
        {
            ret = decodeImage(*png, fileData, fileSize, fileName, bitmap);

            pngAllocator.deallocate(png);
        }
    }

    if (nullptr != fileData)
    {
        DataAllocator dataAllocator;

        dataAllocator.deallocateArray(fileData);
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

PngImgLoader::Ret PngImgLoader::readFileIntoMemory(FS& fs, const String& fileName, uint8_t*& data, size_t& size)
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

            if (FILE_SIZE_MIN > fileSize)
            {
                LOG_ERROR("File %s is too small for a PNG file.", fileName.c_str());
                ret = RET_FILE_FORMAT_INVALID;
            }
            /* The decoder takes the buffer size as a signed int, therefore a
             * larger file can not be handed over at all.
             */
            else if (FILE_SIZE_MAX < fileSize)
            {
                LOG_ERROR("File %s is too large to be held in memory.", fileName.c_str());
                ret = RET_OUT_OF_MEMORY;
            }
            else
            {
                DataAllocator dataAllocator;
                uint8_t*      fileData = dataAllocator.allocateArray(fileSize);

                if (nullptr == fileData)
                {
                    LOG_ERROR("Out of memory for PNG file %s.", fileName.c_str());
                    ret = RET_OUT_OF_MEMORY;
                }
                else if (fileSize != file.read(fileData, fileSize))
                {
                    LOG_ERROR("Failed to read PNG file %s.", fileName.c_str());
                    dataAllocator.deallocateArray(fileData);
                    ret = RET_FILE_FORMAT_INVALID;
                }
                else
                {
                    data = fileData;
                    size = fileSize;
                }
            }

            file.close();
        }
    }

    return ret;
}

PngImgLoader::Ret PngImgLoader::decodeImage(PNG& png, uint8_t* data, size_t size, const String& fileName, YAGfxDynamicBitmap& bitmap)
{
    Ret       ret = RET_OK;
    const int rc  = png.openRAM(data, static_cast<int>(size), drawCallback);

    if (PNG_SUCCESS != rc)
    {
        ret = mapDecoderError(rc);
    }
    else
    {
        const int width  = png.getWidth();
        const int height = png.getHeight();

        /* An image without any pixels is of no use and would ask for a
         * bitmap of zero size.
         */
        if ((0 >= width) ||
            (0 >= height))
        {
            LOG_ERROR("PNG image %s has no pixels.", fileName.c_str());
            ret = RET_FILE_FORMAT_INVALID;
        }
        /* The IHDR width and height are 31 bit wide and reach far past the
         * bitmap dimensions. An image which does not fit must be rejected
         * here, because narrowing it would silently wrap and create a small
         * bitmap which then reports success for a huge image.
         */
        else if ((DIM_MAX_REPRESENTABLE < width) ||
                 (DIM_MAX_REPRESENTABLE < height))
        {
            LOG_ERROR("PNG image of %dx%d can not be represented.", width, height);
            ret = RET_IMG_TOO_BIG;
        }
        /* PNGdec accepts images whose de-filtered line does not fit into its
         * own two line buffer and then writes past it, see PITCH_MAX. Refuse
         * them here, before decode() is called.
         */
        else if (PITCH_MAX < calcLinePitch(width, png.getBpp(), png.getPixelType()))
        {
            LOG_ERROR("PNG image of %dx%d exceeds the decoder's line buffer.", width, height);
            ret = RET_IMG_TOO_BIG;
        }
        else
        {
            bitmap.release();

            if (false == bitmap.create(static_cast<uint16_t>(width), static_cast<uint16_t>(height)))
            {
                LOG_ERROR("Failed to allocate a bitmap of %dx%d.", width, height);
                ret = RET_IMG_TOO_BIG;
            }
            else
            {
                /* The draw callback is called for every decoded line and
                 * receives the destination bitmap as its user pointer.
                 */
                const int rcDecode = png.decode(&bitmap, 0);

                if (PNG_SUCCESS != rcDecode)
                {
                    LOG_ERROR("Failed to decode PNG image %s.", fileName.c_str());
                    ret = mapDecoderError(rcDecode);
                }
            }
        }
    }

    /* openRAM() installs no close callback, so this is a no-op today. It is
     * called anyway to keep the open/close pairing of the decoder API intact.
     */
    png.close();

    return ret;
}

void PngImgLoader::drawLine(YAGfxDynamicBitmap& bitmap, const PNGDRAW& draw)
{
    if (nullptr == draw.pPixels)
    {
        /* Guard path: no decoded pixels available. */
    }
    else if ((PNG_PIXEL_TRUECOLOR == draw.iPixelType) ||
             (PNG_PIXEL_TRUECOLOR_ALPHA == draw.iPixelType))
    {
        drawTruecolorLine(bitmap, draw);
    }
    else if (PNG_PIXEL_INDEXED == draw.iPixelType)
    {
        drawIndexedLine(bitmap, draw);
    }
    else if ((PNG_PIXEL_GRAYSCALE == draw.iPixelType) ||
             (PNG_PIXEL_GRAY_ALPHA == draw.iPixelType))
    {
        drawGrayscaleLine(bitmap, draw);
    }
    else
    {
        /* Guard path: unsupported pixel type. */
        ;
    }
}

void PngImgLoader::drawTruecolorLine(YAGfxDynamicBitmap& bitmap, const PNGDRAW& draw)
{
    const bool    hasAlpha      = (PNG_PIXEL_TRUECOLOR_ALPHA == draw.iPixelType);
    const int     bytesPerPixel = (false == hasAlpha) ? 3 : 4;
    const int16_t y             = static_cast<int16_t>(draw.y);
    int           x;

    for (x = 0; x < draw.iWidth; ++x)
    {
        const uint8_t* pixel = &draw.pPixels[x * bytesPerPixel];
        const uint8_t  alpha = (false == hasAlpha) ? ALPHA_OPAQUE : pixel[3];
        const Color    color(compositeOnBlack(pixel[0], alpha),
            compositeOnBlack(pixel[1], alpha),
            compositeOnBlack(pixel[2], alpha));

        bitmap.drawPixel(static_cast<int16_t>(x), y, color);
    }
}

void PngImgLoader::drawIndexedLine(YAGfxDynamicBitmap& bitmap, const PNGDRAW& draw)
{
    const int16_t y = static_cast<int16_t>(draw.y);
    int           x;

    if (nullptr == draw.pPalette)
    {
        /* Guard path: no palette available. */
    }
    else
    {
        const bool hasAlpha = (0 != draw.iHasAlpha);

        for (x = 0; x < draw.iWidth; ++x)
        {
            /* The sample is a palette index, which the palette resolves to a
             * color. It is therefore never scaled, no matter how many bits it
             * was packed into.
             */
            const int      index = static_cast<int>(getPackedSample(draw.pPixels, x, draw.iBpp));
            const uint8_t* rgb   = &draw.pPalette[index * 3];
            const uint8_t  alpha = (false == hasAlpha) ? ALPHA_OPAQUE : draw.pPalette[PALETTE_ALPHA_OFFSET + index];
            const Color    color(compositeOnBlack(rgb[0], alpha),
                compositeOnBlack(rgb[1], alpha),
                compositeOnBlack(rgb[2], alpha));

            bitmap.drawPixel(static_cast<int16_t>(x), y, color);
        }
    }
}

void PngImgLoader::drawGrayscaleLine(YAGfxDynamicBitmap& bitmap, const PNGDRAW& draw)
{
    const bool    hasAlpha = (PNG_PIXEL_GRAY_ALPHA == draw.iPixelType);
    const int16_t y        = static_cast<int16_t>(draw.y);
    int           x;

    /* A grayscale image with an alpha channel always carries 8 bit samples,
     * because the PNG specification allows nothing else for it besides 16 bit,
     * which PNGdec refuses on its own. Only the plain grayscale image can
     * arrive with its samples packed.
     */
    if (true == hasAlpha)
    {
        const int BYTES_PER_PIXEL = 2;

        for (x = 0; x < draw.iWidth; ++x)
        {
            const uint8_t* pixel = &draw.pPixels[x * BYTES_PER_PIXEL];
            const uint8_t  gray  = compositeOnBlack(pixel[0], pixel[1]);
            const Color    color(gray, gray, gray);

            bitmap.drawPixel(static_cast<int16_t>(x), y, color);
        }
    }
    else
    {
        for (x = 0; x < draw.iWidth; ++x)
        {
            /* The sample is the brightness itself and has to be stretched to
             * the full 8 bit range, so that the brightest sample of a 1, 2 or
             * 4 bit image really becomes white.
             */
            const uint8_t sample = getPackedSample(draw.pPixels, x, draw.iBpp);
            const uint8_t gray   = scaleSampleTo8Bit(sample, draw.iBpp);
            const Color   color(gray, gray, gray);

            bitmap.drawPixel(static_cast<int16_t>(x), y, color);
        }
    }
}

int PngImgLoader::drawCallback(PNGDRAW* pDraw)
{
    YAGfxDynamicBitmap* bitmap = (nullptr == pDraw) ? nullptr : static_cast<YAGfxDynamicBitmap*>(pDraw->pUser);

    if (nullptr != bitmap)
    {
        drawLine(*bitmap, *pDraw);
    }

    return 1; /* Continue decoding. */
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

/**
 * Map a PNGdec error code to the loader result.
 *
 * @param[in] rc    PNGdec error code.
 *
 * @return Loader result.
 */
static PngImgLoader::Ret mapDecoderError(int rc)
{
    PngImgLoader::Ret ret;

    if (PNG_UNSUPPORTED_FEATURE == rc)
    {
        ret = PngImgLoader::RET_FILE_FORMAT_UNSUPPORTED;
    }
    /* PNGdec buffers two image lines internally and refuses every image whose
     * line does not fit, which is a size problem and no format problem.
     */
    else if (PNG_TOO_BIG == rc)
    {
        ret = PngImgLoader::RET_IMG_TOO_BIG;
    }
    else
    {
        ret = PngImgLoader::RET_FILE_FORMAT_INVALID;
    }

    return ret;
}

/**
 * Composite a single color channel against a black background.
 *
 * @param[in] channel   Color channel value.
 * @param[in] alpha     Alpha value. 0 is fully transparent, 255 fully opaque.
 *
 * @return Composited color channel value.
 */
static uint8_t compositeOnBlack(uint8_t channel, uint8_t alpha)
{
    const uint16_t composited = static_cast<uint16_t>(channel) * static_cast<uint16_t>(alpha);

    return static_cast<uint8_t>(composited / ALPHA_OPAQUE);
}

/**
 * Get a single sample out of a de-filtered image line.
 *
 * A PNG packs 1, 2 and 4 bit samples several to a byte, most significant bits
 * first, and pads the last byte of a line. The padding is never read, because
 * only the samples of the line are asked for.
 *
 * @param[in] line              De-filtered image line. Must not be nullptr.
 * @param[in] index             Index of the sample within the line. 0 .. width - 1.
 * @param[in] bitsPerSample     Bits per sample. 1, 2, 4 or 8.
 *
 * @return Sample value, right aligned. 0 .. 2^bitsPerSample - 1.
 */
static uint8_t getPackedSample(const uint8_t* line, int index, int bitsPerSample)
{
    uint8_t sample = 0U;

    if ((0 >= bitsPerSample) ||
        (BITS_PER_BYTE < bitsPerSample))
    {
        /* Guard path: PNGdec delivers nothing else, but do not read out of
         * bounds if it ever does.
         */
    }
    else
    {
        const int     samplesPerByte = BITS_PER_BYTE / bitsPerSample;
        const int     shift          = BITS_PER_BYTE - bitsPerSample * ((index % samplesPerByte) + 1);
        const uint8_t mask           = static_cast<uint8_t>((1U << bitsPerSample) - 1U);

        sample                       = static_cast<uint8_t>((line[index / samplesPerByte] >> shift) & mask);
    }

    return sample;
}

/**
 * Calculate the length of one de-filtered image line in bytes.
 *
 * This is PNGdec's own calculation, png.inl:629-639, repeated here because
 * PNGdec keeps the result to itself. An unknown pixel type yields 0, which is
 * what PNGdec leaves its own pitch at in that case.
 *
 * @param[in] width             Image width in pixels.
 * @param[in] bitsPerSample     Bits per sample. 1, 2, 4 or 8.
 * @param[in] pixelType         PNG pixel type.
 *
 * @return Length of one image line in bytes.
 */
static int calcLinePitch(int width, int bitsPerSample, int pixelType)
{
    int samplesPerPixel;

    switch (pixelType)
    {
    case PNG_PIXEL_GRAYSCALE:
    case PNG_PIXEL_INDEXED:
        samplesPerPixel = 1;
        break;

    case PNG_PIXEL_GRAY_ALPHA:
        samplesPerPixel = 2;
        break;

    case PNG_PIXEL_TRUECOLOR:
        samplesPerPixel = 3;
        break;

    case PNG_PIXEL_TRUECOLOR_ALPHA:
        samplesPerPixel = 4;
        break;

    default:
        samplesPerPixel = 0;
        break;
    }

    return ((samplesPerPixel * bitsPerSample) * width + 7) / 8;
}

/**
 * Stretch a sample to the full 8 bit range, so that the largest possible
 * sample value becomes 255 exactly.
 *
 * @param[in] sample            Sample value, right aligned.
 * @param[in] bitsPerSample     Bits per sample. 1, 2, 4 or 8.
 *
 * @return Sample value stretched to 0 .. 255.
 */
static uint8_t scaleSampleTo8Bit(uint8_t sample, int bitsPerSample)
{
    uint8_t scaled = sample;

    if ((0 >= bitsPerSample) ||
        (BITS_PER_BYTE <= bitsPerSample))
    {
        /* Guard path: an 8 bit sample is already scaled. */
    }
    else
    {
        const uint16_t sampleMax = static_cast<uint16_t>((1U << bitsPerSample) - 1U);

        scaled                   = static_cast<uint8_t>((static_cast<uint16_t>(sample) * 255U) / sampleMax);
    }

    return scaled;
}

#endif /* (0 != CONFIG_IMAGE_FORMAT_PNG_ENABLE) */
