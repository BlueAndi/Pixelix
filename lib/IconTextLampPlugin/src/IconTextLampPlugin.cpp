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
 * @brief  Icon, text and lamp plugin
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "IconTextLampPlugin.h"

#include <FileSystem.h>
#include <Logging.h>
#include <ArduinoJson.h>
#include <Util.h>

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
const char* IconTextLampPlugin::TOPIC_TEXT      = "/text";

/* Initialize plugin topic. */
const char* IconTextLampPlugin::TOPIC_LAMPS     = "/lamps";

/* Initialize plugin topic. */
const char* IconTextLampPlugin::TOPIC_LAMP      = "/lamp";

/* Initialize plugin topic. */
const char* IconTextLampPlugin::TOPIC_ICON      = "/bitmap";

/* Initialize file extension for temporary files. */
const char* IconTextLampPlugin::FILE_EXT_TMP    = ".tmp";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void IconTextLampPlugin::getTopics(JsonArray& topics) const
{
    uint8_t     lampId      = 0U;
    JsonObject  jsonText    = topics.createNestedObject();
    JsonObject  jsonIcon    = topics.createNestedObject();
    JsonObject  jsonLamps   = topics.createNestedObject();

    jsonText["name"]    = TOPIC_TEXT;

    /* Home Assistant support of MQTT discovery (https://www.home-assistant.io/integrations/mqtt) */
    jsonText["ha"]["component"]             = "text";                           /* MQTT integration */
    jsonText["ha"]["discovery"]["name"]     = "MQTT text";                      /* Application that is the origin the discovered MQTT. */
    jsonText["ha"]["discovery"]["cmd_tpl"]  = "{\"text\": \"{{ value }}\" }";   /* Command template */
    jsonText["ha"]["discovery"]["val_tpl"]  = "{{ value_json.text }}";          /* Value template */
    jsonText["ha"]["discovery"]["ic"]       = "mdi:form-textbox";               /* Icon (MaterialDesignIcons.com) */

    jsonIcon["name"]    = TOPIC_ICON;
    jsonIcon["access"]  = "w"; /* Only icon upload is supported. */

    jsonLamps["name"]   = TOPIC_LAMPS;
    jsonLamps["access"] = "r"; /* Only read access allowed. */

    for(lampId = 0U; lampId < _IconTextLampPlugin::View::MAX_LAMPS; ++lampId)
    {
        (void)topics.add(String(TOPIC_LAMP) + "/" + lampId);
    }
}

bool IconTextLampPlugin::getTopic(const String& topic, JsonObject& value) const
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
    else if (0U != topic.equals(TOPIC_LAMPS))
    {
        JsonArray   lampArray   = value.createNestedArray("lamps");
        uint8_t     lampId      = 0U;

        for(lampId = 0U; lampId < _IconTextLampPlugin::View::MAX_LAMPS; ++lampId)
        {
            bool        lampOnState = getLamp(lampId);
            JsonObject  lampObj     = lampArray.createNestedObject();

            lampObj["id"]       = lampId;
            lampObj["state"]    = (false == lampOnState) ? String("off") : String("on");
        }

        isSuccessful = true;
    }
    else if (0U != topic.startsWith(String(TOPIC_LAMP) + "/"))
    {
        uint32_t    indexBeginLampId    = topic.lastIndexOf("/") + 1U;
        String      lampIdStr           = topic.substring(indexBeginLampId);
        uint8_t     lampId              = _IconTextLampPlugin::View::MAX_LAMPS;
        bool        status              = Util::strToUInt8(lampIdStr, lampId);

        if ((true == status) &&
            (_IconTextLampPlugin::View::MAX_LAMPS > lampId))
        {
            bool    lampState       = getLamp(lampId);
            String  lampStateStr    = (false == lampState) ? "off" : "on";

            value["id"]     = lampId;
            value["state"]  = lampStateStr;

            isSuccessful = true;
        }
    }
    else
    {
        ;
    }

    return isSuccessful;
}

bool IconTextLampPlugin::setTopic(const String& topic, const JsonObjectConst& value)
{
    bool isSuccessful = false;

    if (0U != topic.equals(TOPIC_TEXT))
    {
        String              text;
        JsonVariantConst    jsonText            = value["text"];
        JsonVariantConst    jsonIconFullPath    = value["iconFullPath"];

        if (false == jsonText.isNull())
        {
            text = jsonText.as<String>();

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
    else if (0U != topic.startsWith(String(TOPIC_LAMP) + "/"))
    {
        uint32_t            indexBeginLampId    = topic.lastIndexOf("/") + 1U;
        String              lampIdStr           = topic.substring(indexBeginLampId);
        uint8_t             lampId              = _IconTextLampPlugin::View::MAX_LAMPS;
        bool                status              = Util::strToUInt8(lampIdStr, lampId);
        JsonVariantConst    jsonSet             = value["state"];

        if ((true == status) &&
            (_IconTextLampPlugin::View::MAX_LAMPS > lampId) &&
            (false == jsonSet.isNull()))
        {
            String state = jsonSet.as<String>();

            if (state == "off")
            {
                setLamp(lampId, false);
                isSuccessful = true;
            }
            else if (state == "on")
            {
                setLamp(lampId, true);
                isSuccessful = true;
            }
            else
            {
                ;
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

bool IconTextLampPlugin::hasTopicChanged(const String& topic)
{
    bool hasTopicChanged = false;

    if (0U != topic.equals(TOPIC_TEXT))
    {
        MutexGuard<MutexRecursive> guard(m_mutex);

        hasTopicChanged = m_hasTopicTextChanged;
        m_hasTopicTextChanged = false;
    }
    else if (0U != topic.equals(TOPIC_LAMPS))
    {
        MutexGuard<MutexRecursive> guard(m_mutex);

        hasTopicChanged = m_hasTopicLampsChanged;
        m_hasTopicLampsChanged = false;
    }
    else if (0U != topic.startsWith(String(TOPIC_LAMP) + "/"))
    {
        uint32_t    indexBeginLampId    = topic.lastIndexOf("/") + 1U;
        String      lampIdStr           = topic.substring(indexBeginLampId);
        uint8_t     lampId              = _IconTextLampPlugin::View::MAX_LAMPS;
        bool        status              = Util::strToUInt8(lampIdStr, lampId);

        if ((true == status) &&
            (_IconTextLampPlugin::View::MAX_LAMPS > lampId))
        {
            MutexGuard<MutexRecursive> guard(m_mutex);
            
            hasTopicChanged = m_hasTopicLampChanged[lampId];
            m_hasTopicLampChanged[lampId] = false;
        }
    }
    else
    {
        ;
    }

    return hasTopicChanged;
}

bool IconTextLampPlugin::isUploadAccepted(const String& topic, const String& srcFilename, String& dstFilename)
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

    return isAccepted;
}

void IconTextLampPlugin::start(uint16_t width, uint16_t height)
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

void IconTextLampPlugin::stop()
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

void IconTextLampPlugin::update(YAGfx& gfx)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_view.update(gfx);
}

String IconTextLampPlugin::getText() const
{
    MutexGuard<MutexRecursive>  guard(m_mutex);
    String                      formattedText   = m_view.getFormatText();

    return formattedText;
}

void IconTextLampPlugin::setText(const String& formatText)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    if (m_view.getFormatText() != formatText)
    {
        m_view.setFormatText(formatText);

        m_hasTopicTextChanged = true;
    }
}

bool IconTextLampPlugin::loadIcon(const String& filename)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    if (m_iconPath != filename)
    {
        m_iconPath = filename;

        m_hasTopicTextChanged = true;
    }

    /* Load the icon always again, as the path might be the same, but
     * the icon file changed.
     */
    return m_view.loadIcon(m_iconPath);
}

void IconTextLampPlugin::clearIcon()
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    if (false == m_iconPath.isEmpty())
    {
        /* Clear icon first in the view (will close file). */
        m_view.clearIcon();

        /* Remove icon from filesystem. */
        (void)FILESYSTEM.remove(m_iconPath);

        /* Clear the path to the icon. */
        m_iconPath.clear();

        m_hasTopicTextChanged = true;
    }
}

void IconTextLampPlugin::getIconFilePath(String& fullPath) const
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    fullPath = m_iconPath;
}

bool IconTextLampPlugin::getLamp(uint8_t lampId) const
{
    MutexGuard<MutexRecursive>  guard(m_mutex);
    bool                        lampState = m_view.getLamp(lampId);

    return lampState;
}

void IconTextLampPlugin::setLamp(uint8_t lampId, bool state)
{
    if (_IconTextLampPlugin::View::MAX_LAMPS > lampId)
    {
        MutexGuard<MutexRecursive> guard(m_mutex);

        if (state != m_view.getLamp(lampId))
        {
            m_view.setLamp(lampId, state);

            m_hasTopicLampsChanged = true;
            m_hasTopicLampChanged[lampId] = true;
        }
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

String IconTextLampPlugin::getFileName(const String& ext)
{
    return generateFullPath(getUID(), ext);
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
