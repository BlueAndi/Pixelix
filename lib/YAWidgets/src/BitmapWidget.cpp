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
 * @file   BitmapWidget.cpp
 * @brief  Bitmap Widget
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "BitmapWidget.h"
#include "BmpImgLoader.h"

/* The PNG image loader is only available if the build switch is enabled. Its
 * header pulls in the PNGdec header, therefore the include must stay inside
 * the guard.
 */
#if (0 != CONFIG_IMAGE_FORMAT_PNG_ENABLE)
#include "PngImgLoader.h"
#endif /* (0 != CONFIG_IMAGE_FORMAT_PNG_ENABLE) */

#include <YAColor.h>
#include <Logging.h>
#include <FileUtil.h>

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

/* Initialize bitmap widget type. */
const char* BitmapWidget::WIDGET_TYPE     = "bitmap";

/* Initialize bitmap image filename extension. */
const char* BitmapWidget::FILE_EXT_BITMAP = "bmp";

/* Initialize GIF image filename extension. */
const char* BitmapWidget::FILE_EXT_GIF    = "gif";

#if (0 != CONFIG_IMAGE_FORMAT_PNG_ENABLE)

/* Initialize PNG image filename extension. */
const char* BitmapWidget::FILE_EXT_PNG = "png";

#endif /* (0 != CONFIG_IMAGE_FORMAT_PNG_ENABLE) */

#if (0 != CONFIG_IMAGE_FORMAT_WEBP_ENABLE)

/* Initialize WebP image filename extension. */
const char* BitmapWidget::FILE_EXT_WEBP = "webp";

#endif /* (0 != CONFIG_IMAGE_FORMAT_WEBP_ENABLE) */

/* Initialize supported image file extensions. */
const char* BitmapWidget::IMAGE_FILE_EXTENSIONS[] = {
    FILE_EXT_BITMAP,
    FILE_EXT_GIF,
#if (0 != CONFIG_IMAGE_FORMAT_PNG_ENABLE)
    FILE_EXT_PNG,
#endif /* (0 != CONFIG_IMAGE_FORMAT_PNG_ENABLE) */
#if (0 != CONFIG_IMAGE_FORMAT_WEBP_ENABLE)
    FILE_EXT_WEBP,
#endif /* (0 != CONFIG_IMAGE_FORMAT_WEBP_ENABLE) */
};

/* Initialize number of supported image file extensions. */
const size_t BitmapWidget::IMAGE_FILE_EXTENSIONS_COUNT = sizeof(BitmapWidget::IMAGE_FILE_EXTENSIONS) / sizeof(BitmapWidget::IMAGE_FILE_EXTENSIONS[0U]);

/******************************************************************************
 * Public Methods
 *****************************************************************************/

BitmapWidget& BitmapWidget::operator=(const BitmapWidget& widget)
{
    if (&widget != this)
    {
        Widget::operator=(widget);

        m_imgType       = widget.m_imgType;
        m_bitmap        = widget.m_bitmap;
        m_gifPlayer     = widget.m_gifPlayer;
        m_hAlign        = widget.m_hAlign;
        m_vAlign        = widget.m_vAlign;
        m_hAlignPosX    = widget.m_hAlignPosX;
        m_vAlignPosY    = widget.m_vAlignPosY;
        m_gifFileLoader = widget.m_gifFileLoader;

#if (0 != CONFIG_IMAGE_FORMAT_WEBP_ENABLE)
        m_webpPlayer = widget.m_webpPlayer;
#endif /* (0 != CONFIG_IMAGE_FORMAT_WEBP_ENABLE) */
    }

    return *this;
}

void BitmapWidget::set(const YAGfxBitmap& bitmap)
{
    /* Release unused memory. */
    m_bitmap.release();
    m_gifPlayer.close();
    releaseWebp();

    if (true == m_bitmap.create(bitmap.getWidth(), bitmap.getHeight()))
    {
        m_bitmap.copy(bitmap);
        m_imgType = IMG_TYPE_BMP;

        alignWidget();
    }
}

void BitmapWidget::clear(const Color& color)
{
    (void)color; /* Not used. */

    if (IMG_TYPE_BMP == m_imgType)
    {
        m_bitmap.release();
    }
#if (0 != CONFIG_IMAGE_FORMAT_PNG_ENABLE)
    /* A PNG image is decoded into the same bitmap as a BMP image. */
    else if (IMG_TYPE_PNG == m_imgType)
    {
        m_bitmap.release();
    }
#endif /* (0 != CONFIG_IMAGE_FORMAT_PNG_ENABLE) */
    else if (IMG_TYPE_GIF == m_imgType)
    {
        m_gifPlayer.close();
    }
#if (0 != CONFIG_IMAGE_FORMAT_WEBP_ENABLE)
    else if (IMG_TYPE_WEBP == m_imgType)
    {
        m_webpPlayer.close();
    }
#endif /* (0 != CONFIG_IMAGE_FORMAT_WEBP_ENABLE) */
    else
    {
        ;
    }

    m_imgType = IMG_TYPE_NO_IMAGE;
}

bool BitmapWidget::load(const String& filename, FS& fs)
{
    bool isSuccessful = false;

    if (false == fs.exists(filename))
    {
        LOG_WARNING("File %s doesn't exists.", filename.c_str());
    }
    else
    {
        String fileExtension = FileUtil::getFileExtension(filename);

        /* File extension found? */
        if (false == fileExtension.isEmpty())
        {
            /* BMP image? */
            if (true == fileExtension.equalsIgnoreCase(FILE_EXT_BITMAP))
            {
                isSuccessful = loadBMP(fs, filename);
            }
            /* GIF image? */
            else if (true == fileExtension.equalsIgnoreCase(FILE_EXT_GIF))
            {
                isSuccessful = loadGIF(fs, filename);
            }
#if (0 != CONFIG_IMAGE_FORMAT_PNG_ENABLE)
            /* PNG image? */
            else if (true == fileExtension.equalsIgnoreCase(FILE_EXT_PNG))
            {
                isSuccessful = loadPNG(fs, filename);
            }
#endif /* (0 != CONFIG_IMAGE_FORMAT_PNG_ENABLE) */
#if (0 != CONFIG_IMAGE_FORMAT_WEBP_ENABLE)
            /* WebP image? */
            else if (true == fileExtension.equalsIgnoreCase(FILE_EXT_WEBP))
            {
                isSuccessful = loadWEBP(fs, filename);
            }
#endif /* (0 != CONFIG_IMAGE_FORMAT_WEBP_ENABLE) */
            else
            {
                /* Not supported. */
                ;
            }

            if (true == isSuccessful)
            {
                alignWidget();
            }
        }
    }

    return isSuccessful;
}

bool BitmapWidget::isImageTypeSupported(const String& path)
{
    bool isSupported = false;

    for (size_t idx = 0U; idx < sizeof(IMAGE_FILE_EXTENSIONS) / sizeof(IMAGE_FILE_EXTENSIONS[0U]); ++idx)
    {
        const char* ext = IMAGE_FILE_EXTENSIONS[idx];

        if (true == FileUtil::getFileExtension(path).equalsIgnoreCase(ext))
        {
            isSupported = true;
            break;
        }
    }

    return isSupported;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void BitmapWidget::alignWidget()
{
    uint16_t imageWidth  = 0U;
    uint16_t imageHeight = 0U;

    switch (m_imgType)
    {
    case IMG_TYPE_NO_IMAGE:
        break;

    case IMG_TYPE_BMP:
        imageWidth  = m_bitmap.getWidth();
        imageHeight = m_bitmap.getHeight();
        break;

    case IMG_TYPE_GIF:
        imageWidth  = m_gifPlayer.getWidth();
        imageHeight = m_gifPlayer.getHeight();
        break;

#if (0 != CONFIG_IMAGE_FORMAT_PNG_ENABLE)
    /* A PNG image is decoded into the same bitmap as a BMP image. */
    case IMG_TYPE_PNG:
        imageWidth  = m_bitmap.getWidth();
        imageHeight = m_bitmap.getHeight();
        break;
#endif /* (0 != CONFIG_IMAGE_FORMAT_PNG_ENABLE) */

#if (0 != CONFIG_IMAGE_FORMAT_WEBP_ENABLE)
    case IMG_TYPE_WEBP:
        imageWidth  = static_cast<uint16_t>(m_webpPlayer.getWidth());
        imageHeight = static_cast<uint16_t>(m_webpPlayer.getHeight());
        break;
#endif /* (0 != CONFIG_IMAGE_FORMAT_WEBP_ENABLE) */

    default:
        break;
    }

    /* The alignment is calculated signed on purpose. If the image is larger
     * than the widget, the position gets negative and the image will be
     * cropped instead of being moved out of the visible area.
     */
    const int32_t canvasWidth  = static_cast<int32_t>(m_canvas.getWidth());
    const int32_t canvasHeight = static_cast<int32_t>(m_canvas.getHeight());
    const int32_t imgWidth     = static_cast<int32_t>(imageWidth);
    const int32_t imgHeight    = static_cast<int32_t>(imageHeight);

    switch (m_hAlign)
    {
    case Alignment::Horizontal::HORIZONTAL_LEFT:
        m_hAlignPosX = 0;
        break;

    case Alignment::Horizontal::HORIZONTAL_CENTER:
        m_hAlignPosX = static_cast<int16_t>((canvasWidth - imgWidth) / 2);
        break;

    case Alignment::Horizontal::HORIZONTAL_RIGHT:
        m_hAlignPosX = static_cast<int16_t>(canvasWidth - imgWidth);
        break;

    default:
        break;
    }

    switch (m_vAlign)
    {
    case Alignment::Vertical::VERTICAL_TOP:
        m_vAlignPosY = 0;
        break;

    case Alignment::Vertical::VERTICAL_CENTER:
        m_vAlignPosY = static_cast<int16_t>((canvasHeight - imgHeight) / 2);
        break;

    case Alignment::Vertical::VERTICAL_BOTTOM:
        m_vAlignPosY = static_cast<int16_t>(canvasHeight - imgHeight);
        break;

    default:
        break;
    }
}

bool BitmapWidget::loadBMP(FS& fs, const String& filename)
{
    bool              isSuccessful = false;
    BmpImgLoader      loader;
    BmpImgLoader::Ret ret = loader.load(fs, filename, m_bitmap);

    if (BmpImgLoader::RET_OK != ret)
    {
        if (BmpImgLoader::RET_FILE_NOT_FOUND == ret)
        {
            LOG_ERROR("Failed to open file %s.", filename.c_str());
        }
        else if (BmpImgLoader::RET_FILE_FORMAT_INVALID == ret)
        {
            LOG_ERROR("File %s has invalid format.", filename.c_str());
        }
        else if (BmpImgLoader::RET_FILE_FORMAT_UNSUPPORTED == ret)
        {
            LOG_ERROR("File %s has unsupported format.", filename.c_str());
        }
        else if (BmpImgLoader::RET_IMG_TOO_BIG == ret)
        {
            LOG_ERROR("File %s is too big.", filename.c_str());
        }
        else
        {
            LOG_ERROR("Failed to load %s because of internal error.", filename.c_str());
        }
    }
    else
    {
        /* Release unused memory. */
        m_gifPlayer.close();
        releaseWebp();

        /* Select image type. */
        m_imgType    = IMG_TYPE_BMP;

        isSuccessful = true;
    }

    return isSuccessful;
}

bool BitmapWidget::loadGIF(FS& fs, const String& filename)
{
    bool              isSuccessful = false;
    GifImgPlayer::Ret ret;

    /* A already opened GIF image shall be closed first. */
    m_gifPlayer.close();

    /* Open GIF image and keep it opened as long its shown. */
    ret = m_gifPlayer.open(fs, filename, m_gifFileLoader);

    if (GifImgPlayer::RET_OK != ret)
    {
        if (GifImgPlayer::RET_FILE_NOT_FOUND == ret)
        {
            LOG_ERROR("Failed to open file %s.", filename.c_str());
        }
        else if (GifImgPlayer::RET_FILE_ALREADY_OPENED == ret)
        {
            LOG_ERROR("File %s already opened.", filename.c_str());
        }
        else if (GifImgPlayer::RET_FILE_FORMAT_INVALID == ret)
        {
            LOG_ERROR("File %s has invalid format.", filename.c_str());
        }
        else if (GifImgPlayer::RET_FILE_FORMAT_UNSUPPORTED == ret)
        {
            LOG_ERROR("File %s has unsupported format.", filename.c_str());
        }
        else if (GifImgPlayer::RET_IMG_TOO_BIG == ret)
        {
            LOG_ERROR("File %s is too big.", filename.c_str());
        }
        else
        {
            LOG_ERROR("Failed to load %s because of internal error.", filename.c_str());
        }
    }
    else
    {
        /* Release unused memory. */
        m_bitmap.release();
        releaseWebp();

        /* Select image type. */
        m_imgType    = IMG_TYPE_GIF;

        isSuccessful = true;
    }

    return isSuccessful;
}

#if (0 != CONFIG_IMAGE_FORMAT_PNG_ENABLE)

bool BitmapWidget::loadPNG(FS& fs, const String& filename)
{
    bool              isSuccessful = false;
    PngImgLoader      loader;
    PngImgLoader::Ret ret = loader.load(fs, filename, m_bitmap);

    if (PngImgLoader::RET_OK != ret)
    {
        if (PngImgLoader::RET_FILE_NOT_FOUND == ret)
        {
            LOG_ERROR("Failed to open file %s.", filename.c_str());
        }
        else if (PngImgLoader::RET_FILE_FORMAT_INVALID == ret)
        {
            LOG_ERROR("File %s has invalid format.", filename.c_str());
        }
        else if (PngImgLoader::RET_FILE_FORMAT_UNSUPPORTED == ret)
        {
            /* The decoder refuses 16 bit per sample and interlaced (Adam7)
             * images. Interlacing is a common export option, therefore it is
             * named explicit.
             */
            LOG_ERROR("File %s has unsupported format. 16 bit per sample and interlaced PNG images are not supported.", filename.c_str());
        }
        else if (PngImgLoader::RET_IMG_TOO_BIG == ret)
        {
            /* Either the image size can not be represented, the image is wider
             * than the decoder line buffer can hold or the bitmap could not be
             * allocated.
             */
            LOG_ERROR("File %s is too big: image size not representable, image too wide for the decoder or no memory for the bitmap.", filename.c_str());
        }
        else if (PngImgLoader::RET_OUT_OF_MEMORY == ret)
        {
            LOG_ERROR("Failed to load %s because of out of memory.", filename.c_str());
        }
        else
        {
            LOG_ERROR("Failed to load %s because of internal error.", filename.c_str());
        }
    }
    else
    {
        /* Release unused memory. */
        m_gifPlayer.close();
        releaseWebp();

        /* Select image type. */
        m_imgType    = IMG_TYPE_PNG;

        isSuccessful = true;
    }

    return isSuccessful;
}

#endif /* (0 != CONFIG_IMAGE_FORMAT_PNG_ENABLE) */

#if (0 != CONFIG_IMAGE_FORMAT_WEBP_ENABLE)

bool BitmapWidget::loadWEBP(FS& fs, const String& filename)
{
    bool               isSuccessful = false;
    WebpImgPlayer::Ret ret;

    /* A already opened WebP image shall be closed first. */
    m_webpPlayer.close();

    /* Open WebP image and keep it opened as long its shown. */
    ret = m_webpPlayer.open(fs, filename);

    if (WebpImgPlayer::RET_OK != ret)
    {
        if (WebpImgPlayer::RET_FILE_NOT_FOUND == ret)
        {
            LOG_ERROR("Failed to open file %s.", filename.c_str());
        }
        else if (WebpImgPlayer::RET_FILE_ALREADY_OPENED == ret)
        {
            LOG_ERROR("File %s already opened.", filename.c_str());
        }
        else if (WebpImgPlayer::RET_FILE_FORMAT_INVALID == ret)
        {
            LOG_ERROR("File %s has invalid format.", filename.c_str());
        }
        else if (WebpImgPlayer::RET_FILE_FORMAT_UNSUPPORTED == ret)
        {
            LOG_ERROR("File %s has unsupported format.", filename.c_str());
        }
        else if (WebpImgPlayer::RET_IMG_TOO_BIG == ret)
        {
            /* Either the canvas size can not be represented or the bitmap
             * could not be allocated.
             */
            LOG_ERROR("File %s is too big: canvas size not representable or no memory for the bitmap.", filename.c_str());
        }
        else if (WebpImgPlayer::RET_OUT_OF_MEMORY == ret)
        {
            LOG_ERROR("Failed to load %s because of out of memory.", filename.c_str());
        }
        else
        {
            LOG_ERROR("Failed to load %s because of internal error.", filename.c_str());
        }
    }
    else
    {
        /* Release unused memory. */
        m_bitmap.release();
        m_gifPlayer.close();

        /* Select image type. */
        m_imgType    = IMG_TYPE_WEBP;

        isSuccessful = true;
    }

    return isSuccessful;
}

#endif /* (0 != CONFIG_IMAGE_FORMAT_WEBP_ENABLE) */

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
