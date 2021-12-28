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

#include <YAColor.h>
#include <Logging.h>
#include <BmpImg.h>

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
        Widget::operator=(widget);
        
        m_image = widget.m_image;
    }

    return *this;
}

void BitmapWidget::set(const Color* bitmap, uint16_t width, uint16_t height)
{
    m_image.copy(bitmap, width, height);

    return;
}

bool BitmapWidget::load(FS& fs, const String& filename)
{
    bool    status  = false;

    if (false == fs.exists(filename))
    {
        LOG_WARNING("File %s doesn't exists.", filename.c_str());
    }
    else
    {
        BmpImg::Ret ret = m_image.load(fs, filename);

        if (BmpImg::RET_OK != ret)
        {
            if (BmpImg::RET_FILE_NOT_FOUND == ret)
            {
                LOG_ERROR("Failed to open file %s.", filename.c_str());
            }
            else if (BmpImg::RET_FILE_FORMAT_INVALID == ret)
            {
                LOG_ERROR("File %s has invalid format.", filename.c_str());
            }
            else if (BmpImg::RET_FILE_FORMAT_UNSUPPORTED == ret)
            {
                LOG_ERROR("File %s has unsupported format.", filename.c_str());
            }
            else if (BmpImg::RET_FILE_FORMAT_UNSUPPORTED == ret)
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
            status = true;
        }
    }

    return status;
}

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
