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
 * @brief  Chicago Icon and Text Plugin. Displays information remiscent of a
 *  real life Chciago city bus. Not meant to be customizable beyond text.
 *  For use with other Chicago plugins. 
 * @author Julie Hill <queenkjuul@pm.me>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "ChicagoIconTextPlugin.h"
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
const char* ChicagoIconTextPlugin::TOPIC_TEXT              = "/text";

/* Initialize default message string */
const char* ChicagoIconTextPlugin::DEFAULT_TEXT_STRING     
    = "\\#FF5500Smoking, gambling, and littering are prohibited on CTA vehicles.";

/* Initialize default icon filename */
const char* ChicagoIconTextPlugin::DEFAULT_ICON_PATH       
    = "/plugins/ChicagoIconTextPlugin/chi-icon.bmp";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

bool ChicagoIconTextPlugin::isEnabled() const
{
    bool isEnabled = false;

    /* The plugin shall only be scheduled if its enabled and text is set. */
    if ((true == m_isEnabled) &&
        (false == m_textWidget.getStr().isEmpty()))
    {
        isEnabled = true;
    }

    return isEnabled;
}

void ChicagoIconTextPlugin::getTopics(JsonArray& topics) const
{
    JsonObject  jsonText    = topics.createNestedObject();

    jsonText["name"]            = TOPIC_TEXT;

    /* Home Assistant support of MQTT discovery */
    jsonText["ha"]["component"]         = "text";
    jsonText["ha"]["commandTemplate"]   = "{\"text\": \"{{ value }}\" }";
    jsonText["ha"]["valueTemplate"]     = "{{ value_json.text }}";
    jsonText["ha"]["icon"]              = "mdi:form-textbox";
}

bool ChicagoIconTextPlugin::getTopic(const String& topic, JsonObject& value) const
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

bool ChicagoIconTextPlugin::setTopic(const String& topic, const JsonObject& value)
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
        } else {
            text = DEFAULT_TEXT_STRING;
            isSuccessful = true;
        }

        if (true == isSuccessful)
        {
            setText(text);
        }
    }
    else
    {
        ;
    }

    return isSuccessful;
}

bool ChicagoIconTextPlugin::hasTopicChanged(const String& topic)
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

void ChicagoIconTextPlugin::start(uint16_t width, uint16_t height)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    m_iconCanvas.setPosAndSize(0, 0, ICON_WIDTH, ICON_HEIGHT);
    (void)m_iconCanvas.addWidget(m_bitmapWidget);

    /**
     * we're only loading the default icon. Use a different plugin if you 
     * want a different icon.
    */
    (void)m_bitmapWidget.load(FILESYSTEM, DEFAULT_ICON_PATH);

    /* The text canvas is left aligned to the icon canvas and it spans over
     * the whole display height.
     */
    m_textCanvas.setPosAndSize(ICON_WIDTH, 0, width - ICON_WIDTH, height);
    (void)m_textCanvas.addWidget(m_textWidget);

    /* Choose font. */
    m_textWidget.setFont(Fonts::getFontByType(m_fontType));
    
    /* The text widget inside the text canvas is left aligned on x-axis and
     * aligned to the center of y-axis.
     */
    if (height > m_textWidget.getFont().getHeight())
    {
        uint16_t diffY = height - m_textWidget.getFont().getHeight();
        uint16_t offsY = diffY / 2U;

        m_textWidget.move(0, offsY);
    }

    if (false != m_textWidget.getStr().isEmpty()) {
        m_textWidget.setFormatStr(DEFAULT_TEXT_STRING);
    }
}

void ChicagoIconTextPlugin::stop()
{
    MutexGuard<MutexRecursive> guard(m_mutex);
}

void ChicagoIconTextPlugin::update(YAGfx& gfx)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    gfx.fillScreen(ColorDef::BLACK);
    m_iconCanvas.update(gfx);
    m_textCanvas.update(gfx);
}

String ChicagoIconTextPlugin::getText() const
{
    String                      formattedText;
    MutexGuard<MutexRecursive>  guard(m_mutex);

    formattedText = m_textWidget.getFormatStr();

    return formattedText;
}

void ChicagoIconTextPlugin::setText(const String& formatText)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    if (m_textWidget.getFormatStr() != formatText)
    {
        m_textWidget.setFormatStr(formatText);

        m_hasTopicChanged = true;
    }
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
