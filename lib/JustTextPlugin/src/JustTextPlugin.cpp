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
 * @brief  JustText plugin
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "JustTextPlugin.h"

#include <Logging.h>
#include <ArduinoJson.h>
#include <functional>

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
const char* JustTextPlugin::TOPIC_TEXT  = "/text";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

bool JustTextPlugin::isEnabled() const
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

void JustTextPlugin::getTopics(JsonArray& topics) const
{
    JsonObject jsonText = topics.createNestedObject();
 
    jsonText["name"] = TOPIC_TEXT;

    /* Home Assistant support of MQTT discovery (https://www.home-assistant.io/integrations/mqtt) */
    jsonText["ha"]["component"]             = "text";                           /* MQTT integration */
    jsonText["ha"]["discovery"]["name"]     = "MQTT text";                      /* Application that is the origin the discovered MQTT. */
    jsonText["ha"]["discovery"]["cmd_tpl"]  = "{\"text\": \"{{ value }}\" }";   /* Command template */
    jsonText["ha"]["discovery"]["val_tpl"]  = "{{ value_json.text }}";          /* Value template */
    jsonText["ha"]["discovery"]["ic"]       = "mdi:form-textbox";               /* Icon (MaterialDesignIcons.com) */
}

bool JustTextPlugin::getTopic(const String& topic, JsonObject& value) const
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

bool JustTextPlugin::setTopic(const String& topic, const JsonObjectConst& value)
{
    bool isSuccessful = false;

    if (0U != topic.equals(TOPIC_TEXT))
    {
        String              text;
        JsonVariantConst    jsonText    = value["text"];

        if (false == jsonText.isNull())
        {
            text = jsonText.as<String>();
            isSuccessful = true;
        }

        if (true == isSuccessful)
        {
            setText(text);
        }
    }

    return isSuccessful;
}

bool JustTextPlugin::hasTopicChanged(const String& topic)
{
    MutexGuard<MutexRecursive>  guard(m_mutex);
    bool                        hasTopicChanged = m_hasTopicChanged;

    /* Only a single topic, therefore its not necessary to check. */
    PLUGIN_NOT_USED(topic);

    m_hasTopicChanged = false;

    return hasTopicChanged;
}

void JustTextPlugin::start(uint16_t width, uint16_t height)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    PLUGIN_NOT_USED(width);

    /* Choose font. */
    m_textWidget.setFont(Fonts::getFontByType(m_fontType));

    /* The text widget is left aligned on x-axis and aligned to the center
     * of y-axis.
     */
    if (height > m_textWidget.getFont().getHeight())
    {
        uint16_t diffY = height - m_textWidget.getFont().getHeight();
        uint16_t offsY = diffY / 2U;

        m_textWidget.move(0, offsY);
    }
}

void JustTextPlugin::stop()
{
    /* Nothing to do. */
}

void JustTextPlugin::update(YAGfx& gfx)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    gfx.fillScreen(ColorDef::BLACK);
    m_textWidget.update(gfx);
}

String JustTextPlugin::getText() const
{
    String                      formattedText;
    MutexGuard<MutexRecursive>  guard(m_mutex);

    formattedText = m_textWidget.getFormatStr();

    return formattedText;
}

void JustTextPlugin::setText(const String& formatText)
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
