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

void IconTextPlugin::stop()
{
    if (false != FILESYSTEM.remove(getFileName()))
    {
        LOG_INFO("File %s removed", getFileName().c_str());
    }
}

void IconTextPlugin::active(YAGfx& gfx)
{
    lock();

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
        m_textCanvas = new Canvas(gfx.getWidth() - ICON_WIDTH, gfx.getHeight(), ICON_WIDTH, 0);

        if (nullptr != m_textCanvas)
        {
            (void)m_textCanvas->addWidget(m_textWidget);

            /* Move the text widget one line lower for better look. */
            m_textWidget.move(0, 1);
        }
    }

    unlock();

    return;
}

void IconTextPlugin::inactive()
{
    /* Nothing to do. */
    return;
}

void IconTextPlugin::update(YAGfx& gfx)
{
    lock();

    gfx.fillScreen(ColorDef::BLACK);

    if (nullptr != m_iconCanvas)
    {
        m_iconCanvas->update(gfx);
    }

    if (nullptr != m_textCanvas)
    {
        m_textCanvas->update(gfx);
    }

    unlock();

    return;
}

String IconTextPlugin::getText() const
{
    String formattedText;

    lock();
    formattedText = m_textWidget.getFormatStr();
    unlock();

    return formattedText;
}

void IconTextPlugin::setText(const String& formatText)
{
    lock();
    m_textWidget.setFormatStr(formatText);
    unlock();

    return;
}

void IconTextPlugin::setBitmap(const Color* bitmap, uint16_t width, uint16_t height)
{
    if ((nullptr != bitmap) &&
        (ICON_WIDTH >= width) &&
        (ICON_HEIGHT >= height))
    {
        lock();
        m_bitmapWidget.set(bitmap, width, height);
        unlock();
    }

    return;
}

bool IconTextPlugin::loadBitmap(const String& filename)
{
    bool status = false;

    lock();
    status = m_bitmapWidget.load(FILESYSTEM, filename);
    unlock();

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

void IconTextPlugin::lock() const
{
    if (nullptr != m_xMutex)
    {
        (void)xSemaphoreTakeRecursive(m_xMutex, portMAX_DELAY);
    }

    return;
}

void IconTextPlugin::unlock() const
{
    if (nullptr != m_xMutex)
    {
        (void)xSemaphoreGiveRecursive(m_xMutex);
    }

    return;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
