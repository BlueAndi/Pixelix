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
#include <Logging.h>
#include <FileSystem.h>

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

PngImgLoader::Ret PngImgLoader::load(FS& fs, const String& fileName, YAGfxDynamicBitmap& bitmap)
{
    Ret ret = RET_OK;

    if (false == fs.exists(fileName))
    {
        ret = RET_FILE_NOT_FOUND;
    }
    else
    {
        PNG* png = new (std::nothrow) PNG(); /* Place it on heap to avoid stack overflow. */

        if (nullptr == png)
        {
            ret = RET_IMG_TOO_BIG;
        }
        else
        {
            int rc;

            m_bitmap = &bitmap;

            /* Open PNG from file using custom callbacks */
            rc       = png->open(fileName.c_str(), openCallback, closeCallback, readCallback, seekCallback, drawCallback);

            if (PNG_SUCCESS != rc)
            {
                if (PNG_INVALID_FILE == rc)
                {
                    ret = RET_FILE_FORMAT_INVALID;
                }
                else if (PNG_UNSUPPORTED_FEATURE == rc)
                {
                    ret = RET_FILE_FORMAT_UNSUPPORTED;
                }
                else
                {
                    ret = RET_FILE_FORMAT_INVALID;
                }
            }
            else
            {
                int width  = png->getWidth();
                int height = png->getHeight();

                /* Check for invalid dimensions */
                if ((0 > width) || (0 > height))
                {
                    ret = RET_FILE_FORMAT_INVALID;
                }
                /* Check image size limits */
                else if ((UINT16_MAX < width) || (UINT16_MAX < height))
                {
                    ret = RET_IMG_TOO_BIG;
                }
                else
                {
                    bitmap.release();

                    if (false == bitmap.create(static_cast<uint16_t>(width), static_cast<uint16_t>(height)))
                    {
                        ret = RET_IMG_TOO_BIG;
                    }
                    else
                    {
                        /* Decode PNG image - the drawCallback will be called for each line */
                        rc = png->decode(this, 0);

                        if (PNG_SUCCESS != rc)
                        {
                            ret = RET_FILE_FORMAT_INVALID;
                        }
                    }
                }

                png->close();
            }

            delete png;
        }
        m_bitmap = nullptr;
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

int PngImgLoader::drawCallback(PNGDRAW* pDraw)
{
    PngImgLoader* loader = static_cast<PngImgLoader*>(pDraw->pUser);

    if ((nullptr != loader) && (nullptr != loader->m_bitmap))
    {
        /* Is it truecolor? */
        if (PNG_PIXEL_TRUECOLOR == pDraw->iPixelType || PNG_PIXEL_TRUECOLOR_ALPHA == pDraw->iPixelType)
        {
            uint8_t* pSrc          = pDraw->pPixels;
            int      bytesPerPixel = (PNG_PIXEL_TRUECOLOR == pDraw->iPixelType) ? 3 : 4;

            for (int x = 0; x < pDraw->iWidth; ++x)
            {
                uint8_t red   = pSrc[x * bytesPerPixel];
                uint8_t green = pSrc[x * bytesPerPixel + 1];
                uint8_t blue  = pSrc[x * bytesPerPixel + 2];

                loader->m_bitmap->drawPixel(x, pDraw->y, Color(red, green, blue));
            }
        }
        /* Indexed color? */
        else if (PNG_PIXEL_INDEXED == pDraw->iPixelType)
        {
            /* For indexed color, use the fast palette if available */
            if (nullptr != pDraw->pFastPalette)
            {
                /* Note, the fast palette is in RGB565 format */
                for (int x = 0; x < pDraw->iWidth; ++x)
                {
                    uint8_t  index    = pDraw->pPixels[x];
                    uint16_t pixel565 = pDraw->pFastPalette[index];
                    Color    color;

                    color.fromRgb565(pixel565);
                    loader->m_bitmap->drawPixel(x, pDraw->y, color);
                }
            }
            else if (nullptr != pDraw->pPalette)
            {
                for (int x = 0; x < pDraw->iWidth; ++x)
                {
                    uint8_t index = pDraw->pPixels[x];
                    uint8_t r     = pDraw->pPalette[index * 3];
                    uint8_t g     = pDraw->pPalette[index * 3 + 1];
                    uint8_t b     = pDraw->pPalette[index * 3 + 2];

                    loader->m_bitmap->drawPixel(x, pDraw->y, Color(r, g, b));
                }
            }
        }
        /* Grayscale color? */
        else if (PNG_PIXEL_GRAYSCALE == pDraw->iPixelType || PNG_PIXEL_GRAY_ALPHA == pDraw->iPixelType)
        {
            int bytesPerPixel = (PNG_PIXEL_GRAYSCALE == pDraw->iPixelType) ? 1 : 2;

            for (int x = 0; x < pDraw->iWidth; x++)
            {
                uint8_t gray = pDraw->pPixels[x * bytesPerPixel];

                loader->m_bitmap->drawPixel(x, pDraw->y, Color(gray, gray, gray));
            }
        }
        else
        {
            /* Unsupported pixel type, do nothing. */
            ;
        }
    }

    return 1; /* Continue decoding */
}

void* PngImgLoader::openCallback(const char* szFilename, int32_t* pFileSize)
{
    File* pFile = new (std::nothrow) File();

    if (nullptr != pFile)
    {
        *pFile = FILESYSTEM.open(szFilename, "r");

        if (false == *pFile)
        {
            delete pFile;
            pFile = nullptr;
        }
        else
        {
            *pFileSize = pFile->size();
        }
    }

    return pFile;
}

void PngImgLoader::closeCallback(void* pHandle)
{
    if (nullptr != pHandle)
    {
        File* pFile = static_cast<File*>(pHandle);
        pFile->close();
        delete pFile;
    }
}

int32_t PngImgLoader::readCallback(PNGFILE* pFile, uint8_t* pBuf, int32_t iLen)
{
    int32_t bytesRead = 0;

    if ((nullptr != pFile) && (nullptr != pFile->fHandle))
    {
        File* pFileHandle  = static_cast<File*>(pFile->fHandle);
        bytesRead          = pFileHandle->read(pBuf, iLen);
        pFile->iPos       += bytesRead;
    }

    return bytesRead;
}

int32_t PngImgLoader::seekCallback(PNGFILE* pFile, int32_t iPosition)
{
    int32_t newPos = -1;

    if ((nullptr != pFile) && (nullptr != pFile->fHandle))
    {
        File* pFileHandle = static_cast<File*>(pFile->fHandle);

        if (true == pFileHandle->seek(iPosition, SeekSet))
        {
            newPos      = iPosition;
            pFile->iPos = newPos;
        }
    }

    return newPos;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
