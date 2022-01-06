/* MIT License
 *
 * Copyright (c) 2019 - 2022 Andreas Merkle <web@blue-andi.de>
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
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "SpriteSheet.h"

#include <ArduinoJson.h>
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

/******************************************************************************
 * Public Methods
 *****************************************************************************/

SpriteSheet& SpriteSheet::operator=(const SpriteSheet& spriteSheet)
{
    if (this != (&spriteSheet))
    {
        m_texture       = spriteSheet.m_texture;
        m_textureMap    = spriteSheet.m_textureMap;
        m_frame         = spriteSheet.m_frame;
        m_frameCnt      = spriteSheet.m_frameCnt;
        m_fps           = spriteSheet.m_fps;
        m_framesX       = spriteSheet.m_framesX;
        m_framesY       = spriteSheet.m_framesY;
        m_currentFrameX = spriteSheet.m_currentFrameX;
        m_currentFrameY = spriteSheet.m_currentFrameY;
    }

    return *this;
}

bool SpriteSheet::loadTexture(FS& fs, const String& fileName, uint16_t frameWidth, uint16_t frameHeight, uint8_t frameCnt, uint8_t fps)
{
    bool isSuccessful = false;

    /* The frame size must be given, otherwise the texture can not be
     * cut to single frames.
     */
    if ((0U < frameWidth) &&
        (0U < frameHeight))
    {
        BmpImgLoader loader;

        if (BmpImgLoader::RET_OK == loader.load(fs, fileName, m_texture))
        {
            /* The frame size must be lower or equal to the texture size. */
            if ((m_texture.getWidth() >= frameWidth) &&
                (m_texture.getHeight() >= frameHeight))
            {
                m_framesX   = m_texture.getWidth() / frameWidth;
                m_framesY   = m_texture.getHeight() / frameHeight;

                /* A 0 number of frames requests the automatic frame count calculation.
                 * This assumes that there will be no frame gaps in the texture image.
                 */
                if (0U == frameCnt)
                {
                    m_frameCnt = m_framesX * m_framesY;
                }
                else
                {
                    m_frameCnt = frameCnt;
                }

                m_textureMap.setOffsetX(0);
                m_textureMap.setOffsetY(0);
                m_textureMap.setWidth(frameWidth);
                m_textureMap.setHeight(frameHeight);

                m_fps           = fps;
                m_currentFrameX = 0U;
                m_currentFrameY = 0U;

                isSuccessful = true;
            }
            else
            {
                m_texture.release();
            }
        }
    }

    return isSuccessful;
}

bool SpriteSheet::load(FS& fs, const String& spriteSheetFileName, const String& textureFileName)
{
    bool    isSuccessful    = false;
    File    fd              = fs.open(spriteSheetFileName);

    if (true == fd)
    {
        const size_t            JSON_DOC_SIZE   = 1024U;
        DynamicJsonDocument     jsonDoc(JSON_DOC_SIZE);
        DeserializationError    error   = deserializeJson(jsonDoc, fd);

        fd.close();

        if (DeserializationError::Ok == error.code())
        {
            JsonVariant jsonFrameWidth  = jsonDoc["texture"]["frame"]["width"];
            JsonVariant jsonFrameHeight = jsonDoc["texture"]["frame"]["height"];
            JsonVariant jsonFrameCnt    = jsonDoc["texture"]["frames"];
            JsonVariant jsonFps         = jsonDoc["texture"]["fps"];

            if ((false == jsonFrameWidth.isNull()) &&
                (false == jsonFrameHeight.isNull()) &&
                (false == jsonFps.isNull()))
            {
                uint8_t     frameCnt    = 0U;
                uint16_t    frameWidth  = jsonFrameWidth.as<uint16_t>();
                uint16_t    frameHeight = jsonFrameHeight.as<uint16_t>();
                uint8_t     fps         = jsonFps.as<uint8_t>();

                if (false == jsonFrameCnt.isNull())
                {
                    frameCnt = jsonFrameCnt.as<uint8_t>();
                }

                isSuccessful = loadTexture( fs, 
                                            textureFileName, 
                                            frameWidth,
                                            frameHeight,
                                            frameCnt,
                                            fps);
            }
        }
    }

    return isSuccessful;
}

void SpriteSheet::next()
{
    ++m_currentFrameX;

    if (m_framesX <= m_currentFrameX)
    {
        m_currentFrameX = 0U;

        ++m_currentFrameY;

        if (m_framesY <= m_currentFrameY)
        {
            m_currentFrameY = 0U;
        }
    }

    /* The texture may be not complete filled with frames.
     * This will be considered here.
     */
    if (m_frameCnt < (m_currentFrameX + m_currentFrameY * m_framesX))
    {
        m_currentFrameX = 0U;
        m_currentFrameY = 0U;
    }

    /* Calculate and set the frame offset in the texture image. */
    m_textureMap.setOffsetX(m_currentFrameX * m_frame.getWidth());
    m_textureMap.setOffsetY(m_currentFrameY * m_frame.getHeight());
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
