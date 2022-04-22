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
 * @brief  Icon and text plugin
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "IconTextPlugin.h"
#include "RestApi.h"
#include "FileSystem.h"

#include <Logging.h>
#include <ArduinoJson.h>

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

/* Initialize plugin topic. */
const char* IconTextPlugin::TOPIC_TEXT              = "/text";

/* Initialize plugin topic. */
const char* IconTextPlugin::TOPIC_ICON              = "/bitmap";

/* Initialize bitmap image filename extension. */
const char* IconTextPlugin::FILE_EXT_BITMAP         = ".bmp";

/* Initialize sprite sheet parameter filename extension. */
const char* IconTextPlugin::FILE_EXT_SPRITE_SHEET   = ".sprite";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void IconTextPlugin::getTopics(JsonArray& topics) const
{
    (void)topics.add(TOPIC_TEXT);
    (void)topics.add(TOPIC_ICON);
}

bool IconTextPlugin::getTopic(const String& topic, JsonObject& value) const
{
    bool isSuccessful = false;

    if (0U != topic.equals(TOPIC_TEXT))
    {
        String  formattedText   = getText();

        value["text"] = formattedText;

        isSuccessful = true;
    }

    return isSuccessful;
}

bool IconTextPlugin::setTopic(const String& topic, const JsonObject& value)
{
    bool isSuccessful = false;

    if (0U != topic.equals(TOPIC_TEXT))
    {
        String      text;
        JsonVariant jsonShow    = value["show"];

        if (false == jsonShow.isNull())
        {
            text = jsonShow.as<String>();
            isSuccessful = true;
        }

        if (true == isSuccessful)
        {
            setText(text);
        }
    }
    else if (0U != topic.equals(TOPIC_ICON))
    {
        JsonVariant jsonFullPath = value["fullPath"];

        if (false == jsonFullPath.isNull())
        {
            String fullPath = jsonFullPath.as<String>();

            isSuccessful = loadBitmap(fullPath);
        }
    }
    else
    {
        ;
    }

    return isSuccessful;
}

bool IconTextPlugin::isUploadAccepted(const String& topic, const String& srcFilename, String& dstFilename)
{
    bool isAccepted = false;

    if (0U != topic.equals(TOPIC_ICON))
    {
        /* Accept upload of bitmap file. */
        if (0U != srcFilename.endsWith(FILE_EXT_BITMAP))
        {
            dstFilename = getFileName(FILE_EXT_BITMAP);

            isAccepted = true;
        }
        /* Accept upload of a sprite sheet file. */
        else if (0U != srcFilename.endsWith(FILE_EXT_SPRITE_SHEET))
        {
            dstFilename = getFileName(FILE_EXT_SPRITE_SHEET);

            isAccepted = true;
        }
        else
        {
            /* Not accepted. */
            ;
        }
    }

    return isAccepted;
}

void IconTextPlugin::start(uint16_t width, uint16_t height)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_iconCanvas.setPosAndSize(0, 0, ICON_WIDTH, ICON_HEIGHT);
    (void)m_iconCanvas.addWidget(m_bitmapWidget);

    /* If there is already an icon in the filesystem, it will be loaded.
     * First check whether it is a animated sprite sheet and if not, try
     * to load just a bitmap image.
     */
    if (false == m_bitmapWidget.loadSpriteSheet(FILESYSTEM, getFileName(FILE_EXT_SPRITE_SHEET), getFileName(FILE_EXT_BITMAP)))
    {
        (void)m_bitmapWidget.load(FILESYSTEM, getFileName(FILE_EXT_BITMAP));
    }

    m_textCanvas.setPosAndSize(ICON_WIDTH, 0, width - ICON_WIDTH, height);
    (void)m_textCanvas.addWidget(m_textWidget);

    return;
}

void IconTextPlugin::stop()
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    if (false != FILESYSTEM.remove(getFileName(FILE_EXT_BITMAP)))
    {
        LOG_INFO("File %s removed", getFileName(FILE_EXT_BITMAP).c_str());
    }

    if (false != FILESYSTEM.remove(getFileName(FILE_EXT_SPRITE_SHEET)))
    {
        LOG_INFO("File %s removed", getFileName(FILE_EXT_SPRITE_SHEET).c_str());
    }

    return;
}

void IconTextPlugin::update(YAGfx& gfx)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    gfx.fillScreen(ColorDef::BLACK);
    m_iconCanvas.update(gfx);
    m_textCanvas.update(gfx);

    return;
}

String IconTextPlugin::getText() const
{
    String                      formattedText;
    MutexGuard<MutexRecursive>  guard(m_mutex);

    formattedText = m_textWidget.getFormatStr();

    return formattedText;
}

void IconTextPlugin::setText(const String& formatText)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_textWidget.setFormatStr(formatText);

    return;
}

bool IconTextPlugin::loadBitmap(const String& filename)
{
    bool                        status = false;
    MutexGuard<MutexRecursive>  guard(m_mutex);

    if (0U != filename.endsWith(FILE_EXT_BITMAP))
    {
        status = m_bitmapWidget.load(FILESYSTEM, filename);

        /* Ensure that only the bitmap image file exists in the filesystem,
         * otherwise after a restart, the obsolete sprite sheet will
         * be loaded.
         */
        if (true == status)
        {
            (void)FILESYSTEM.remove(getFileName(FILE_EXT_SPRITE_SHEET));
        }
    }
    else if (0U != filename.endsWith(FILE_EXT_SPRITE_SHEET))
    {
        String bmpFilename = filename;

        bmpFilename.replace(FILE_EXT_SPRITE_SHEET, FILE_EXT_BITMAP);

        status = m_bitmapWidget.loadSpriteSheet(FILESYSTEM, filename,  bmpFilename);
    }
    else
    {
        /* Not supported. */
        ;
    }

    return status;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

String IconTextPlugin::getFileName(const String& ext)
{
    return generateFullPath(ext);
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
