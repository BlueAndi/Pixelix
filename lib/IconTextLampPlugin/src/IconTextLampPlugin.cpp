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
 * @brief  Icon, text and lamp plugin
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "IconTextLampPlugin.h"
#include "FileSystem.h"

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
const char* IconTextLampPlugin::TOPIC_TEXT              = "/text";

/* Initialize plugin topic. */
const char* IconTextLampPlugin::TOPIC_LAMPS             = "/lamps";

/* Initialize plugin topic. */
const char* IconTextLampPlugin::TOPIC_LAMP              = "/lamp";

/* Initialize plugin topic. */
const char* IconTextLampPlugin::TOPIC_ICON              = "/bitmap";

/* Initialize bitmap image filename extension. */
const char* IconTextLampPlugin::FILE_EXT_BITMAP         = ".bmp";

/* Initialize sprite sheet parameter filename extension. */
const char* IconTextLampPlugin::FILE_EXT_SPRITE_SHEET   = ".sprite";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void IconTextLampPlugin::getTopics(JsonArray& topics) const
{
    uint8_t     lampId      = 0U;
    JsonObject  jsonText    = topics.createNestedObject();
    JsonObject  jsonIcon    = topics.createNestedObject();
    JsonObject  jsonLamps   = topics.createNestedObject();

    jsonText["name"]            = TOPIC_TEXT;

    /* Home Assistant support of MQTT discovery */
    jsonText["ha"]["component"]         = "text";
    jsonText["ha"]["commandTemplate"]   = "{\"text\": \"{{ value }}\" }";
    jsonText["ha"]["valueTemplate"]     = "{{ value_json.text }}";

    jsonIcon["name"]            = TOPIC_ICON;
    jsonIcon["access"]          = "w"; /* Only icon upload is supported. */

    jsonLamps["name"]           = TOPIC_LAMPS;
    jsonLamps["access"]         = "r"; /* Only read access allowed. */

    for(lampId = 0U; lampId < MAX_LAMPS; ++lampId)
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

        value["text"] = formattedText;

        isSuccessful = true;
    }
    else if (0U != topic.equals(TOPIC_LAMPS))
    {
        JsonArray   lampArray   = value.createNestedArray("lamps");
        uint8_t     lampId      = 0U;

        for(lampId = 0U; lampId < MAX_LAMPS; ++lampId)
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
        uint8_t     lampId              = MAX_LAMPS;
        bool        status              = Util::strToUInt8(lampIdStr, lampId);

        if ((true == status) &&
            (MAX_LAMPS > lampId))
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

bool IconTextLampPlugin::setTopic(const String& topic, const JsonObject& value)
{
    bool isSuccessful = false;

    if (0U != topic.equals(TOPIC_TEXT))
    {
        String              text;
        JsonVariantConst    jsonShow    = value["text"];

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
    else if (0U != topic.startsWith(String(TOPIC_LAMP) + "/"))
    {
        uint32_t            indexBeginLampId    = topic.lastIndexOf("/") + 1U;
        String              lampIdStr           = topic.substring(indexBeginLampId);
        uint8_t             lampId              = MAX_LAMPS;
        bool                status              = Util::strToUInt8(lampIdStr, lampId);
        JsonVariantConst    jsonSet             = value["state"];

        if ((true == status) &&
            (MAX_LAMPS > lampId) &&
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
        uint8_t     lampId              = MAX_LAMPS;
        bool        status              = Util::strToUInt8(lampIdStr, lampId);

        if ((true == status) &&
            (MAX_LAMPS > lampId))
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

void IconTextLampPlugin::start(uint16_t width, uint16_t height)
{
    uint16_t                    tcHeight        = 0U;
    uint16_t                    lampWidth       = 0U;
    uint16_t                    lampDistance    = 0U;
    const uint16_t              minDistance     = 1U;   /* Min. distance between lamps. */
    const uint16_t              minBorder       = 1U;   /* Min. border left and right of all lamps. */
    const uint16_t              canvasWidth     = width - ICON_WIDTH;
    MutexGuard<MutexRecursive>  guard(m_mutex);

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

    /* The text canvas is left aligned to the icon canvas and aligned to the
     * top. Consider that below the text canvas the lamps are shown.
     */
    tcHeight = height - 2U;
    m_textCanvas.setPosAndSize(ICON_WIDTH, 0, width - ICON_WIDTH, tcHeight);
    (void)m_textCanvas.addWidget(m_textWidget);

    /* The text widget inside the text canvas is left aligned on x-axis and
     * aligned to the center of y-axis.
     */
    if (tcHeight > m_textWidget.getFont().getHeight())
    {
        uint16_t diffY = height - m_textWidget.getFont().getHeight();
        uint16_t offsY = diffY / 2U;

        m_textWidget.move(0, offsY);
    }

    m_lampCanvas.setPosAndSize(ICON_WIDTH, height - 1, canvasWidth, 1U);

    if (true == calcLayout(canvasWidth, MAX_LAMPS, minDistance, minBorder, lampWidth, lampDistance))
    {
        /* Calculate the border to have the lamps shown aligned to center. */
        uint16_t    border  = ((canvasWidth - (MAX_LAMPS * lampWidth)) - ((MAX_LAMPS - 1U) * lampDistance)) / 2U;
        uint8_t     index   = 0U;

        for(index = 0U; index < MAX_LAMPS; ++index)
        {
            /* One space at the begin, two spaces between the lamps. */
            int16_t x = (lampWidth + lampDistance) * index + border;

            m_lampWidgets[index].setWidth(lampWidth);

            (void)m_lampCanvas.addWidget(m_lampWidgets[index]);
            m_lampWidgets[index].move(x, 0);
        }
    }
}

void IconTextLampPlugin::stop()
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
}

void IconTextLampPlugin::update(YAGfx& gfx)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    gfx.fillScreen(ColorDef::BLACK);
    m_iconCanvas.update(gfx);
    m_textCanvas.update(gfx);
    m_lampCanvas.update(gfx);
}

String IconTextLampPlugin::getText() const
{
    String                      formattedText;
    MutexGuard<MutexRecursive>  guard(m_mutex);

    formattedText = m_textWidget.getFormatStr();

    return formattedText;
}

void IconTextLampPlugin::setText(const String& formatText)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    if (m_textWidget.getFormatStr() != formatText)
    {
        m_textWidget.setFormatStr(formatText);

        m_hasTopicTextChanged = true;
    }
}

bool IconTextLampPlugin::loadBitmap(const String& filename)
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

bool IconTextLampPlugin::getLamp(uint8_t lampId) const
{
    bool lampState = false;

    if (MAX_LAMPS > lampId)
    {
        MutexGuard<MutexRecursive> guard(m_mutex);

        lampState = m_lampWidgets[lampId].getOnState();
    }

    return lampState;
}

void IconTextLampPlugin::setLamp(uint8_t lampId, bool state)
{
    if (MAX_LAMPS > lampId)
    {
        MutexGuard<MutexRecursive> guard(m_mutex);

        if (state != m_lampWidgets[lampId].getOnState())
        {
            m_lampWidgets[lampId].setOnState(state);

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
    return PluginConfigFsHandler::generateFullPath(getUID(), ext);
}

bool IconTextLampPlugin::calcLayout(uint16_t width, uint16_t cnt, uint16_t minDistance, uint16_t minBorder, uint16_t& elementWidth, uint16_t& elementDistance)
{
    bool    status  = false;

    /* The min. border (left and right) must not be greater than the given width. */
    if (width > (2U * minBorder))
    {
        uint16_t    availableWidth  = width - (2U * minBorder); /* The available width is calculated considering the min. borders. */

        /* The available width must be greater than the number of elements, including the min. element distance. */
        if (availableWidth > (cnt + ((cnt - 1U) * minDistance)))
        {
            uint16_t    maxElementWidth                     = (availableWidth - ((cnt - 1U) * minDistance)) / cnt; /* Max. element width, considering the given limitation. */
            uint16_t    elementWidthToAvailWidthRatio       = 8U;   /* 1 / N */
            uint16_t    elementDistanceToElementWidthRatio  = 4U;   /* 1 / N */
            uint16_t    elementWidthConsideringRatio        = availableWidth / elementWidthToAvailWidthRatio;

            /* Consider the ratio between element width to available width and
             * ratio between element distance to element width.
             * This is just to have a nice look.
             */
            if (maxElementWidth > elementWidthConsideringRatio)
            {
                uint16_t    elementDistanceConsideringRatio = elementWidthConsideringRatio / elementDistanceToElementWidthRatio;

                if (0U == elementDistanceConsideringRatio)
                {
                    if (0U == minDistance)
                    {
                        elementDistance = 0U;
                    }
                    else
                    {
                        elementWidth    = maxElementWidth;
                        elementDistance = (availableWidth - (cnt * maxElementWidth)) / (cnt - 1U);
                    }
                }
                else
                {
                    elementWidth    = elementWidthConsideringRatio - elementDistanceConsideringRatio;
                    elementDistance = elementDistanceConsideringRatio;
                }
            }
            else
            {
                elementWidth    = maxElementWidth;
                elementDistance = minDistance;
            }

            status = true;
        }
    }

    return status;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
