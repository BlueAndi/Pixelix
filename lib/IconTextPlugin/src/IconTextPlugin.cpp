/* MIT License
 *
 * Copyright (c) 2019 - 2024 Andreas Merkle <web@blue-andi.de>
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

#include <FileSystem.h>
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
const char* IconTextPlugin::TOPIC_TEXT      = "/text";

/* Initialize plugin topic. */
const char* IconTextPlugin::TOPIC_ICON      = "/bitmap";

/* Initialize file extension for temporary files. */
const char* IconTextPlugin::FILE_EXT_TMP    = ".tmp";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

bool IconTextPlugin::isEnabled() const
{
    bool isEnabled = false;

    /* The plugin shall only be scheduled if its enabled and text is set. */
    if ((true == m_isEnabled) &&
        (false == m_view.getText().isEmpty()))
    {
        isEnabled = true;
    }

    return isEnabled;
}

void IconTextPlugin::getTopics(JsonArray& topics) const
{
    JsonObject  jsonText    = topics.createNestedObject();
    JsonObject  jsonIcon    = topics.createNestedObject();

    jsonText["name"]    = TOPIC_TEXT;

    /* Home Assistant support of MQTT discovery (https://www.home-assistant.io/integrations/mqtt) */
    jsonText["ha"]["component"]             = "text";                           /* MQTT integration */
    jsonText["ha"]["discovery"]["name"]     = "MQTT text";                      /* Application that is the origin the discovered MQTT. */
    jsonText["ha"]["discovery"]["cmd_tpl"]  = "{\"text\": \"{{ value }}\" }";   /* Command template */
    jsonText["ha"]["discovery"]["val_tpl"]  = "{{ value_json.text }}";          /* Value template */
    jsonText["ha"]["discovery"]["ic"]       = "mdi:form-textbox";               /* Icon (MaterialDesignIcons.com) */

    jsonIcon["name"]    = TOPIC_ICON;
    jsonIcon["access"]  = "w"; /* Only icon upload is supported. */
}

bool IconTextPlugin::getTopic(const String& topic, JsonObject& value) const
{
    bool isSuccessful = false;

    if (0U != topic.equals(TOPIC_TEXT))
    {
        String  formattedText   = getText();
        String  iconFullPath;

        getIconFilePath(iconFullPath);

        value["text"]           = formattedText;
        value["iconFullPath"]   = iconFullPath;

        isSuccessful = true;
    }

    return isSuccessful;
}

bool IconTextPlugin::setTopic(const String& topic, const JsonObjectConst& value)
{
    bool isSuccessful = false;

    if (0U != topic.equals(TOPIC_TEXT))
    {
        JsonVariantConst    jsonText            = value["text"];
        JsonVariantConst    jsonIconFullPath    = value["iconFullPath"];

        if (false == jsonText.isNull())
        {
            String text = jsonText.as<String>();

            setText(text);

            isSuccessful = true;
        }

        if (false == jsonIconFullPath.isNull())
        {
            String iconFullPath = jsonIconFullPath.as<String>();

            /* Clear always the icon indpended whether its requested by user.
             * In case of an uploaded new icon, clearing will close the image
             * file and makes it possible to overwrite the file.
             */
            clearIcon();

            if (false == iconFullPath.isEmpty())
            {
                /* Rename uploaded icon by removing the file extension for temporary files. */
                String iconFullPathWithoutTmp = iconFullPath.substring(0, iconFullPath.length() - strlen(FILE_EXT_TMP));

                FILESYSTEM.rename(iconFullPath, iconFullPathWithoutTmp);

                isSuccessful = loadIcon(iconFullPathWithoutTmp);
            }
            else
            {
                isSuccessful = true;
            }
        }
    }
    else if (0U != topic.equals(TOPIC_ICON))
    {
        JsonVariantConst jsonFullPath = value["fullPath"];

        /* File upload? */
        if (false == jsonFullPath.isNull())
        {
            String fullPath = jsonFullPath.as<String>();

            isSuccessful = loadIcon(fullPath);
        }
    }
    else
    {
        ;
    }

    return isSuccessful;
}

bool IconTextPlugin::hasTopicChanged(const String& topic)
{
    bool hasTopicChanged = false;

    if (0U != topic.equals(TOPIC_TEXT))
    {
        MutexGuard<MutexRecursive> guard(m_mutex);

        hasTopicChanged = m_hasTopicChanged;

        m_hasTopicChanged = false;
    }

    return hasTopicChanged;
}

bool IconTextPlugin::isUploadAccepted(const String& topic, const String& srcFilename, String& dstFilename)
{
    bool isAccepted = false;

    if (0U != topic.equals(TOPIC_ICON))
    {
        /* Accept upload of bitmap file. */
        if (0U != srcFilename.endsWith(BitmapWidget::FILE_EXT_BITMAP))
        {
            dstFilename = getFileName(BitmapWidget::FILE_EXT_BITMAP);

            isAccepted = true;
        }
        /* Accept upload of GIF file. */
        else if (0U != srcFilename.endsWith(BitmapWidget::FILE_EXT_GIF))
        {
            dstFilename = getFileName(BitmapWidget::FILE_EXT_GIF);

            isAccepted = true;
        }
        else
        {
            ;
        }

        if (true == isAccepted)
        {
            /* If a GIF image is loaded, the file is kept open and can not be overwritten.
             * Therefore store it first with the additional extension for temporary files.
             * It will be renamed then in the setTopic() method if upload is successful.
             */
            dstFilename += FILE_EXT_TMP;
        }
    }
    else
    {
        ;
    }

    return isAccepted;
}

void IconTextPlugin::start(uint16_t width, uint16_t height)
{
    String                      bitmapFullPath  = getFileName(BitmapWidget::FILE_EXT_BITMAP);
    String                      gifFullPath     = getFileName(BitmapWidget::FILE_EXT_GIF);
    MutexGuard<MutexRecursive>  guard(m_mutex);

    m_view.init(width, height);

    if (true == m_view.loadIcon(bitmapFullPath))
    {
        m_iconPath = bitmapFullPath;
    }
    else if (true == m_view.loadIcon(gifFullPath))
    {
        m_iconPath = gifFullPath;
    }
    else
    {
        m_iconPath.clear();
    }
}

void IconTextPlugin::stop()
{
    String                      bitmapFullPath  = getFileName(BitmapWidget::FILE_EXT_BITMAP);
    String                      gifFullPath     = getFileName(BitmapWidget::FILE_EXT_GIF);
    MutexGuard<MutexRecursive>  guard(m_mutex);

    /* Remove icon which is specific for the plugin instance. */
    if (false != FILESYSTEM.remove(bitmapFullPath))
    {
        LOG_INFO("File %s removed", bitmapFullPath.c_str());
    }

    /* Remove icon which is specific for the plugin instance. */
    if (false != FILESYSTEM.remove(gifFullPath))
    {
        LOG_INFO("File %s removed", gifFullPath.c_str());
    }
}

void IconTextPlugin::update(YAGfx& gfx)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_view.update(gfx);
}

String IconTextPlugin::getText() const
{
    MutexGuard<MutexRecursive>  guard(m_mutex);
    String                      formattedText   = m_view.getFormatText();

    return formattedText;
}

void IconTextPlugin::setText(const String& formatText)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    if (m_view.getFormatText() != formatText)
    {
        m_view.setFormatText(formatText);

        m_hasTopicChanged = true;
    }
}

bool IconTextPlugin::loadIcon(const String& filename)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    if (m_iconPath != filename)
    {
        m_iconPath = filename;

        m_hasTopicChanged = true;
    }

    /* Load the icon always again, as the path might be the same, but
     * the icon file changed.
     */
    return m_view.loadIcon(m_iconPath);
}

void IconTextPlugin::clearIcon()
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    if (false == m_iconPath.isEmpty())
    {
        /* Clear icon first in the view (will close file). */
        m_view.clearIcon();

        /* If plugin owns the file, it will be removed from filesystem. */
        if (true == isFileOwnedByPlugin(m_iconPath))
        {
            (void)FILESYSTEM.remove(m_iconPath);
        }

        /* Clear the path to the icon. */
        m_iconPath.clear();

        m_hasTopicChanged = true;
    }
}

void IconTextPlugin::getIconFilePath(String& fullPath) const
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    fullPath = m_iconPath;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

String IconTextPlugin::getFileName(const String& ext) const
{
    return generateFullPath(getUID(), ext);
}

bool IconTextPlugin::isFileOwnedByPlugin(const String& filename) const
{
    bool    isOwned     = false;
    String  bmpFileName = getFileName(BitmapWidget::FILE_EXT_BITMAP);
    String  gifFileName = getFileName(BitmapWidget::FILE_EXT_GIF);

    if ((filename == bmpFileName) ||
        (filename == gifFileName))
    {
        isOwned = true;
    }

    return isOwned;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
