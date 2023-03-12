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
 * @brief  Bitmap Widget
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "BitmapWidget.h"

#include <YAColor.h>
#include <Logging.h>
#include <BmpImgLoader.h>

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
        
        m_bitmap        = widget.m_bitmap;
        m_spriteSheet   = widget.m_spriteSheet;
        m_timer         = widget.m_timer;
        m_duration      = widget.m_duration;
    }

    return *this;
}

void BitmapWidget::clear(const Color& color)
{
    if (true == m_spriteSheet.isEmpty())
    {
        m_bitmap.fillScreen(color);
    }
    else
    {
        m_spriteSheet.release();
        m_timer.stop();
    }
}

bool BitmapWidget::load(FS& fs, const String& filename)
{
    bool isSuccessful = false;

    if (false == fs.exists(filename))
    {
        LOG_WARNING("File %s doesn't exists.", filename.c_str());
    }
    else
    {
        BmpImgLoader        loader;
        BmpImgLoader::Ret   ret = loader.load(fs, filename, m_bitmap);

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
            /* Avoid wasting memory. Additional this is important to detect whether the sprite sheet
             * shall be shown or the single bitmap image.
             */
            m_spriteSheet.release();
            m_timer.stop();

            isSuccessful = true;
        }
    }

    return isSuccessful;
}

bool BitmapWidget::loadSpriteSheet(FS& fs, const String& spriteSheetFileName, const String& textureFileName)
{
    bool isSuccessful = false;

    if (false == fs.exists(spriteSheetFileName))
    {
        LOG_WARNING("File %s doesn't exists.", spriteSheetFileName.c_str());
    }
    else if (false == fs.exists(textureFileName))
    {
        LOG_WARNING("File %s doesn't exists.", textureFileName.c_str());
    }
    else if (true == m_spriteSheet.load(fs, spriteSheetFileName, textureFileName))
    {
        /* Calculate duration per frame. */
        m_duration = 1000U / m_spriteSheet.getFPS();

        /* Avoid wasting memory. Additional this is important to detect whether the sprite sheet
         * shall be shown or the single bitmap image.
         */
        m_bitmap.release();        

        isSuccessful = true;
    }

    return isSuccessful;
}

bool BitmapWidget::isSpriteSheetForward() const
{
    return m_spriteSheet.isForward();
}

void BitmapWidget::setSpriteSheetForward(bool forward)
{
    m_spriteSheet.setForward(forward);
}

bool BitmapWidget::isSpriteSheetRepeatInfinite() const
{
    return m_spriteSheet.isRepeatedInfinite();
}

void BitmapWidget::setSpriteSheetRepeatInfinite(bool repeat)
{
    m_spriteSheet.repeatInfinite(repeat);
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
