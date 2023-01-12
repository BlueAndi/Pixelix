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
 * @brief  Sensor plugin
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "SensorPlugin.h"
#include "FileSystem.h"

#include <Logging.h>
#include <ArduinoJson.h>
#include <JsonFile.h>
#include <SensorDataProvider.h>
#include <SensorChannelType.hpp>

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
const char* SensorPlugin::TOPIC_CHANNEL = "/channel";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void SensorPlugin::getTopics(JsonArray& topics) const
{
    (void)topics.add(TOPIC_CHANNEL);
}

bool SensorPlugin::getTopic(const String& topic, JsonObject& value) const
{
    bool isSuccessful = false;

    if (0U != topic.equals(TOPIC_CHANNEL))
    {
        uint8_t sensorIdx   = 0U;
        uint8_t channelIdx  = 0U;
        bool    isAvailable = getSensorChannel(sensorIdx, channelIdx);

        value["sensorIndex"]    = sensorIdx;
        value["channelIndex"]   = channelIdx;
        value["isAvailable"]    = isAvailable;

        isSuccessful = true;
    }
    else
    {
        ;
    }

    return isSuccessful;
}

bool SensorPlugin::setTopic(const String& topic, const JsonObject& value)
{
    bool isSuccessful = false;

    if (0U != topic.equals(TOPIC_CHANNEL))
    {
        JsonVariantConst    jsonSensorIndex     = value["sensorIndex"];
        JsonVariantConst    jsonChannelIndex    = value["channelIndex"];

        if ((false == jsonSensorIndex.isNull()) &&
            (false == jsonChannelIndex.isNull()))
        {
            uint8_t sensorIdx   = jsonSensorIndex.as<uint8_t>();
            uint8_t channelIdx  = jsonChannelIndex.as<uint8_t>();
        
            (void)setSensorChannel(sensorIdx, channelIdx);

            isSuccessful = true;
        }
    }
    else
    {
        ;
    }

    return isSuccessful;
}

void SensorPlugin::start(uint16_t width, uint16_t height)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    UTIL_NOT_USED(width);

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

    /* Try to load configuration. If there is no configuration available, a default configuration
     * will be created.
     */
    if (false == loadConfiguration())
    {
        if (false == saveConfiguration())
        {
            LOG_WARNING("Failed to create initial configuration file %s.", getFullPathToConfiguration().c_str());
        }
    }

    m_sensorChannel = getChannel(m_sensorIdx, m_channelIdx);

    /* Force update. */
    m_updateTimer.start(0U);

    return;
}

void SensorPlugin::stop()
{
    String                      configurationFilename = getFullPathToConfiguration();
    MutexGuard<MutexRecursive>  guard(m_mutex);

    if (false != FILESYSTEM.remove(configurationFilename))
    {
        LOG_INFO("File %s removed", configurationFilename.c_str());
    }

    return;
}

void SensorPlugin::active(YAGfx& gfx)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    UTIL_NOT_USED(gfx);

    /* Load configuration, because it may be changed by web request
     * or direct editing.
     */
    if (true == loadConfiguration())
    {
        m_sensorChannel = getChannel(m_sensorIdx, m_channelIdx);
    }

    return;
}

void SensorPlugin::inactive()
{
    /* Nothing to do. */
    return;
}

void SensorPlugin::update(YAGfx& gfx)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    if (true == m_updateTimer.isTimeout())
    {
        update();
        m_updateTimer.start(UPDATE_PERIOD);
    }

    gfx.fillScreen(ColorDef::BLACK);
    m_textWidget.update(gfx);

    return;
}

bool SensorPlugin::getSensorChannel(uint8_t& sensorIdx, uint8_t& channelIdx) const
{
    bool                        isAvailable = false;
    MutexGuard<MutexRecursive>  guard(m_mutex);

    sensorIdx   = m_sensorIdx;
    channelIdx  = m_channelIdx;

    if (nullptr != m_sensorChannel)
    {
        isAvailable = true;
    }

    return isAvailable;
}

bool SensorPlugin::setSensorChannel(uint8_t sensorIdx, uint8_t channelIdx)
{
    ISensorChannel*             channel     = nullptr;
    bool                        isAvailable = false;
    MutexGuard<MutexRecursive>  guard(m_mutex);

    /* Anything changed? */
    if ((sensorIdx != m_sensorIdx) ||
        (channelIdx != m_channelIdx))
    {
        channel = getChannel(sensorIdx, channelIdx);

        m_sensorIdx     = sensorIdx;
        m_channelIdx    = channelIdx;
        m_sensorChannel = channel;

        if (nullptr != channel)
        {
            isAvailable = true;
        }

        if (false == saveConfiguration())
        {
            LOG_WARNING("Couldn't save configuration.");
        }
    }
    /* Nothing changed and sensor is available? */
    else if (nullptr != m_sensorChannel)
    {
        isAvailable = true;
    }
    else
    {
        /* Nothing changed, sensor is not available. */
        ;
    }

    return isAvailable;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void SensorPlugin::update()
{
    String          text;
    const uint32_t  PRECISION   = 2U;

    if (nullptr == m_sensorChannel)
    {
        text = "\\calign-";
    }
    else
    {
        /* If text is updated and keeps static shown (no scrolling), right
         * aligned looks better, because the unit stays at the same
         * position and only the number seems to be updated.
         */
        text  = "\\ralign";
        text += m_sensorChannel->getValueAsString(PRECISION);
        text += " ";
        text += ISensorChannel::channelTypeToUnit(m_sensorChannel->getType());
    }

    m_textWidget.setFormatStr(text);
}

ISensorChannel* SensorPlugin::getChannel(uint8_t sensorIdx, uint8_t channelIdx)
{
    ISensor*        sensor  = SensorDataProvider::getInstance().getSensor(sensorIdx);
    ISensorChannel* channel = nullptr;

    if ((nullptr != sensor) &&
        (true == sensor->isAvailable()))
    {
        channel = sensor->getChannel(channelIdx);
    }

    return channel;
}

bool SensorPlugin::saveConfiguration() const
{
    bool                status                  = true;
    JsonFile            jsonFile(FILESYSTEM);
    const size_t        JSON_DOC_SIZE           = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    String              configurationFilename   = getFullPathToConfiguration();

    jsonDoc["sensorIndex"]  = m_sensorIdx;
    jsonDoc["channelIndex"] = m_channelIdx;
    
    if (false == jsonFile.save(configurationFilename, jsonDoc))
    {
        LOG_WARNING("Failed to save file %s.", configurationFilename.c_str());
        status = false;
    }
    else
    {
        LOG_INFO("File %s saved.", configurationFilename.c_str());
    }

    return status;
}

bool SensorPlugin::loadConfiguration()
{
    bool                status                  = true;
    JsonFile            jsonFile(FILESYSTEM);
    const size_t        JSON_DOC_SIZE           = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
    String              configurationFilename   = getFullPathToConfiguration();

    if (false == jsonFile.load(configurationFilename, jsonDoc))
    {
        LOG_WARNING("Failed to load file %s.", configurationFilename.c_str());
        status = false;
    }
    else
    {
        JsonVariantConst    jsonSensorIndex     = jsonDoc["sensorIndex"];
        JsonVariantConst    jsonChannelIndex    = jsonDoc["channelIndex"];

        if (false == jsonSensorIndex.is<uint8_t>())
        {
            LOG_WARNING("Sensor index not found or invalid type.");
            status = false;
        }
        else if (false == jsonChannelIndex.is<uint8_t>())
        {
            LOG_WARNING("Channel index not found or invalid type.");
            status = false;
        }
        else
        {
            m_sensorIdx     = jsonSensorIndex.as<uint8_t>();
            m_channelIdx    = jsonChannelIndex.as<uint8_t>();
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
