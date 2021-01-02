/* MIT License
 *
 * Copyright (c) 2019 - 2021 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Bitmap Widget
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "BitmapWidget.h"

#ifndef NATIVE

#include <SPIFFS.h>
#include <NeoPixelBus.h>
#include <Color.h>
#include <Logging.h>

#endif  /* NATIVE */

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
const char* BitmapWidget::WIDGET_TYPE = "bitmap";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

BitmapWidget& BitmapWidget::operator=(const BitmapWidget& widget)
{
    if (&widget != this)
    {
        m_bufferSize    = widget.m_bufferSize;
        m_width         = widget.m_width;
        m_height        = widget.m_height;

        if (nullptr != m_buffer)
        {
            delete[] m_buffer;
            m_buffer = nullptr;
        }

        if (nullptr != widget.m_buffer)
        {
            m_buffer = new Color[m_bufferSize];

            if (nullptr == m_buffer)
            {
                m_bufferSize = 0U;
            }
            else
            {
                size_t index = 0U;

                for(index = 0U; index < m_bufferSize; ++index)
                {
                    m_buffer[index] = widget.m_buffer[index];
                }
            }
        }
    }

    return *this;
}

void BitmapWidget::set(const Color* bitmap, uint16_t width, uint16_t height)
{
    if (nullptr != bitmap)
    {
        if (nullptr != m_buffer)
        {
            delete[] m_buffer;
            m_buffer = nullptr;
        }

        m_bufferSize    = width * height;
        m_width         = width;
        m_height        = height;

        m_buffer = new Color[m_bufferSize];

        if (nullptr == m_buffer)
        {
            m_bufferSize = 0U;
        }
        else
        {
            size_t index = 0U;

            for(index = 0U; index < m_bufferSize; ++index)
            {
                m_buffer[index] = bitmap[index];
            }
        }
    }

    return;
}

#ifndef NATIVE

bool BitmapWidget::load(const String& filename)
{
    bool    status  = false;
    File    fd;

    if (false == SPIFFS.exists(filename))
    {
        LOG_WARNING("File %s doesn't exists.", filename.c_str());
    }
    else
    {
        NeoBitmapFile<NeoGrbFeature, File>  neoFile;

        fd = SPIFFS.open(filename, "r");

        if (false == fd)
        {
            LOG_ERROR("Failed to open file %s.", filename.c_str());
        }
        else
        {
            if (false == neoFile.Begin(fd))
            {
                LOG_ERROR("File %s has incompatible bitmap file format.", filename.c_str());
            }
            else
            {
                m_width         = neoFile.Width();
                m_height        = neoFile.Height();
                m_bufferSize    = m_width * m_height;

                if (nullptr != m_buffer)
                {
                    delete[] m_buffer;
                    m_buffer = nullptr;
                }

                m_buffer = new Color[m_bufferSize];

                if (nullptr != m_buffer)
                {
                    uint16_t x = 0U;
                    uint16_t y = 0U;

                    for(y = 0U; y < m_height; ++y)
                    {
                        for(x = 0U; x < m_width; ++x)
                        {
                            RgbColor rgbColor = neoFile.GetPixelColor(x, y);

                            m_buffer[x + y * m_width].set(rgbColor.R, rgbColor.G, rgbColor.B);
                        }
                    }

                    status = true;
                }
            }

            fd.close();
        }
    }

    return status;
}

#endif  /* NATIVE */

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
