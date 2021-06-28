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
const char* IconTextPlugin::TOPIC_TEXT  = "/text";

/* Initialize plugin topic. */
const char* IconTextPlugin::TOPIC_ICON  = "/bitmap";

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
        String  text;

        if (false == value["show"].isNull())
        {
            text = value["show"].as<String>();
            isSuccessful = true;
        }

        if (true == isSuccessful)
        {
            setText(text);
        }
    }
    else if (0U != topic.equals(TOPIC_ICON))
    {
        if (false == value["fullPath"].isNull())
        {
            String fullPath = value["fullPath"].as<String>();

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
        if (0U != srcFilename.endsWith(".bmp"))
        {
            dstFilename = getFileName();

            isAccepted = true;
        }
    }

    return isAccepted;
}

void IconTextPlugin::start(uint16_t width, uint16_t height)
{
    MutexGuard<Mutex> guard(m_mutex);

    if (nullptr == m_iconCanvas)
    {
        m_iconCanvas = new Canvas(ICON_WIDTH, ICON_HEIGHT, 0, 0);

        if (nullptr != m_iconCanvas)
        {
            (void)m_iconCanvas->addWidget(m_bitmapWidget);

            /* If there is already an icon in the filesystem, load it. */
            (void)m_bitmapWidget.load(FILESYSTEM, getFileName());
        }
    }

    if (nullptr == m_textCanvas)
    {
        m_textCanvas = new Canvas(width - ICON_WIDTH, height, ICON_WIDTH, 0);

        if (nullptr != m_textCanvas)
        {
            (void)m_textCanvas->addWidget(m_textWidget);

            /* Move the text widget one line lower for better look. */
            m_textWidget.move(0, 1);
        }
    }

    return;
}

void IconTextPlugin::stop()
{
    MutexGuard<Mutex> guard(m_mutex);

    if (false != FILESYSTEM.remove(getFileName()))
    {
        LOG_INFO("File %s removed", getFileName().c_str());
    }

    if (nullptr != m_iconCanvas)
    {
        delete m_iconCanvas;
        m_iconCanvas = nullptr;
    }

    if (nullptr != m_textCanvas)
    {
        delete m_textCanvas;
        m_textCanvas = nullptr;
    }

    return;
}

void IconTextPlugin::update(YAGfx& gfx)
{
    MutexGuard<Mutex> guard(m_mutex);

    gfx.fillScreen(ColorDef::BLACK);

    if (nullptr != m_iconCanvas)
    {
        m_iconCanvas->update(gfx);
    }

    if (nullptr != m_textCanvas)
    {
        m_textCanvas->update(gfx);
    }

    return;
}

String IconTextPlugin::getText() const
{
    String              formattedText;
    MutexGuard<Mutex>   guard(m_mutex);

    formattedText = m_textWidget.getFormatStr();

    return formattedText;
}

void IconTextPlugin::setText(const String& formatText)
{
    MutexGuard<Mutex> guard(m_mutex);

    m_textWidget.setFormatStr(formatText);

    return;
}

void IconTextPlugin::setBitmap(const Color* bitmap, uint16_t width, uint16_t height)
{
    if ((nullptr != bitmap) &&
        (ICON_WIDTH >= width) &&
        (ICON_HEIGHT >= height))
    {
        MutexGuard<Mutex> guard(m_mutex);

        m_bitmapWidget.set(bitmap, width, height);
    }

    return;
}

bool IconTextPlugin::loadBitmap(const String& filename)
{
    bool                status = false;
    MutexGuard<Mutex>   guard(m_mutex);

    status = m_bitmapWidget.load(FILESYSTEM, filename);

    return status;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

String IconTextPlugin::getFileName()
{
    return generateFullPath(".bmp");
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
