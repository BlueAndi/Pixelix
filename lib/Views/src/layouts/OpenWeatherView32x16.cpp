/* MIT License
 *
 * Copyright (c) 2019 - 2025 Andreas Merkle <web@blue-andi.de>
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
 * @brief  OpenWeather view with icon and text for 32x16 LED matrix
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "OpenWeatherView32x16.h"
#include <FileSystem.h>

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and classes
 *****************************************************************************/

/** UV-index element */
typedef struct
{
    uint8_t     lower;  /**< Lower UV-index value */
    uint8_t     upper;  /**< Upper UV-index value */
    const char* color;  /**< Color to show in this UV-index range */

} UvIndexElem;

/******************************************************************************
 * Prototypes
 *****************************************************************************/

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/* Layout
 *
 * +-----------------------------------------------------------------+
 * |                |                                                |
 * |                |                                                |
 * |                |                                                |
 * |   Icon         |                   Text                         |
 * |   8x16         |                   24x16                        |
 * |                |                                                |
 * |                |                                                |
 * |                |                                                |
 * +-----------------------------------------------------------------+
 */

/**
 * Weather icon of current weather width in pixels.
 */
static const uint16_t   WEATHER_ICON_CURRENT_WIDTH              = 8U;

/**
 * Weather icon of current weather height in pixels.
 */
static const uint16_t   WEATHER_ICON_CURRENT_HEIGHT             = CONFIG_LED_MATRIX_HEIGHT;

/**
 * Weather icon of current weather widget x-coordinate in pixels.
 * Left aligned.
 */
static const int16_t    WEATHER_ICON_CURRENT_X                  = 0;

/**
 * Weather icon of current weather widget y-coordinate in pixels.
 * Top aligned.
 */
static const int16_t    WEATHER_ICON_CURRENT_Y                  = 0;

/**
 * Text width in pixels.
 */
static const uint16_t   WEATHER_INFO_TEXT_CURRENT_WIDTH         = CONFIG_LED_MATRIX_WIDTH - WEATHER_ICON_CURRENT_WIDTH;

/**
 * Text height in pixels.
 */
static const uint16_t   WEATHER_INFO_TEXT_CURRENT_HEIGHT        = CONFIG_LED_MATRIX_HEIGHT;

/**
 * Text widget x-coordinate in pixels.
 * Left aligned, after icon.
 */
static const int16_t    WEATHER_INFO_TEXT_CURRENT_X             = WEATHER_ICON_CURRENT_WIDTH;

/**
 * Text widget y-coordinate in pixels.
 */
static const int16_t    WEATHER_INFO_TEXT_CURRENT_Y             = WEATHER_ICON_CURRENT_Y;

/** The epsilon is used to compare floats. */
static const float  EPSILON = 0.0001F;

/* Initialize image path for the weather condition icons. */
const char* OpenWeatherView32x16::IMAGE_PATH                 = "/plugins/OpenWeatherPlugin/";

/* Initialize image path for standard icon. */
const char* OpenWeatherView32x16::IMAGE_PATH_STD_ICON        = "/plugins/OpenWeatherPlugin/openWeather.bmp";

/* Initialize image path for uvi icon. */
const char* OpenWeatherView32x16::IMAGE_PATH_UVI_ICON        = "/plugins/OpenWeatherPlugin/uvi.bmp";

/* Initialize image path for humidity icon. */
const char* OpenWeatherView32x16::IMAGE_PATH_HUMIDITY_ICON   = "/plugins/OpenWeatherPlugin/hum.bmp";

/* Initialize image path for uvi icon. */
const char* OpenWeatherView32x16::IMAGE_PATH_WIND_ICON       = "/plugins/OpenWeatherPlugin/wind.bmp";

/** UV-index table */
static const UvIndexElem uvIndexTable[] =
{
    { 0U,   3U,     "{#c0ffa0}" },
    { 3U,   6U,     "{#f8f140}" },
    { 6U,   8U,     "{#f77820}" },
    { 8U,   11U,    "{#d80020}" }
};

/******************************************************************************
 * Public Methods
 *****************************************************************************/

OpenWeatherView32x16::OpenWeatherView32x16() :
    IOpenWeatherView(),
    m_fontType(Fonts::FONT_TYPE_DEFAULT),
    m_weatherIconCurrent(WEATHER_ICON_CURRENT_WIDTH, WEATHER_ICON_CURRENT_HEIGHT, WEATHER_ICON_CURRENT_X, WEATHER_ICON_CURRENT_Y),
    m_weatherInfoCurrentText(WEATHER_INFO_TEXT_CURRENT_WIDTH, WEATHER_INFO_TEXT_CURRENT_HEIGHT, WEATHER_INFO_TEXT_CURRENT_X, WEATHER_INFO_TEXT_CURRENT_Y),
    m_viewDurationTimer(),
    m_viewDuration(0U),
    m_units("metric"),
    m_weatherInfo(WEATHER_INFO_ALL),
    m_weatherInfoId(0U),
    m_weatherInfoCurrent(),
    m_isWeatherInfoCurrentUpdated(false),
    m_isWeatherIconCurrentUpdated(false)
{
    m_weatherIconCurrent.setVerticalAlignment(Alignment::Vertical::VERTICAL_CENTER);
    m_weatherIconCurrent.setHorizontalAlignment(Alignment::Horizontal::HORIZONTAL_CENTER);
    
    m_weatherInfoCurrentText.setVerticalAlignment(Alignment::Vertical::VERTICAL_CENTER);
    m_weatherInfoCurrentText.setHorizontalAlignment(Alignment::Horizontal::HORIZONTAL_CENTER);
}

void OpenWeatherView32x16::update(YAGfx& gfx)
{
    handleWeatherInfo();

    gfx.fillScreen(ColorDef::BLACK);
    m_weatherIconCurrent.update(gfx);
    m_weatherInfoCurrentText.update(gfx);
}

void OpenWeatherView32x16::setWeatherInfoCurrent(const WeatherInfoCurrent& info)
{
    if ((m_weatherInfoCurrent.iconId != info.iconId) ||
        (EPSILON < fabsf(m_weatherInfoCurrent.temperature - info.temperature)) ||
        (m_weatherInfoCurrent.humidity != info.humidity) ||
        (EPSILON < fabsf(m_weatherInfoCurrent.uvIndex - info.uvIndex)) ||
        (EPSILON < fabsf(m_weatherInfoCurrent.windSpeed - info.windSpeed)))
    {
        if (m_weatherInfoCurrent.iconId != info.iconId)
        {
            m_isWeatherIconCurrentUpdated = true;
        }

        m_weatherInfoCurrent            = info;
        m_isWeatherInfoCurrentUpdated   = true;
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

uint8_t OpenWeatherView32x16::getWeatherInfoCount() const
{
    uint8_t count = 0U;
    uint8_t id;

    for(id = 0U; id < 8U; ++id)
    {
        if (0U < (m_weatherInfo & (1U << id)))
        {
            ++count;
        }
    }

    return count;
}

void OpenWeatherView32x16::nextWeatherInfo()
{
    if (0U < m_weatherInfo)
    {
        /* Search for the next enabled weather info. */
        do
        {
            ++m_weatherInfoId;
            if (8U <= m_weatherInfoId)
            {
                m_weatherInfoId = 0U;
            }

        }
        while (0U == (m_weatherInfo & (1U << m_weatherInfoId)));
    }
}

OpenWeatherView32x16::WeatherInfo OpenWeatherView32x16::getActiveWeatherInfo() const
{
    return static_cast<WeatherInfo>(m_weatherInfo & (1U << m_weatherInfoId));
}

void OpenWeatherView32x16::updateWeatherInfoCurrentOnView()
{
    String iconFullPath;
    String text;

    switch(getActiveWeatherInfo())
    {
    case WEATHER_INFO_EMPTY:
        iconFullPath = IMAGE_PATH_STD_ICON;
        text = "-";
        break;

    case WEATHER_INFO_TEMPERATURE:
        getIconPathByWeatherIconId(iconFullPath, m_weatherInfoCurrent.iconId, "");

        if (true == iconFullPath.isEmpty())
        {
            iconFullPath = IMAGE_PATH_STD_ICON;
        }

        appendTemperature(text, m_weatherInfoCurrent.temperature);
        break;
        
    case WEATHER_INFO_HUMIDITY:
        iconFullPath = IMAGE_PATH_HUMIDITY_ICON;

        appendHumidity(text, m_weatherInfoCurrent.humidity);
        break;
        
    case WEATHER_INFO_WIND_SPEED:
        iconFullPath = IMAGE_PATH_WIND_ICON;

        appendWindSpeed(text, m_weatherInfoCurrent.windSpeed);
        break;
        
    case WEATHER_INFO_UV_INDEX:
        iconFullPath = IMAGE_PATH_UVI_ICON;

        appendUvIndex(text, m_weatherInfoCurrent.uvIndex);
        break;
        
    default:
        iconFullPath = IMAGE_PATH_STD_ICON;
        break;
    }

    /* Change icon only if its really necessary to avoid restarting animated icon. */
    if (true == m_isWeatherIconCurrentUpdated)
    {
        (void)m_weatherIconCurrent.load(FILESYSTEM, iconFullPath);
        m_isWeatherIconCurrentUpdated = false;
    }

    m_weatherInfoCurrentText.setFormatStr(text);
}

void OpenWeatherView32x16::handleWeatherInfo()
{
    /* First time of weather info handling? */
    if (false == m_viewDurationTimer.isTimerRunning())
    {
        uint32_t duration           = (0U == m_viewDuration) ? VIEW_DURATION_DEFAULT : m_viewDuration;
        uint8_t  weatherInfoCount   = getWeatherInfoCount();
        uint32_t minDuration        = (0U == weatherInfoCount) ? VIEW_DURATION_MIN : duration / weatherInfoCount;

        if (VIEW_DURATION_MIN > minDuration)
        {
            minDuration = VIEW_DURATION_MIN;
        }

        /* Update icons the first time and every time a reset of the
         * weather info was triggered.
         */
        m_isWeatherIconCurrentUpdated = true;

        updateWeatherInfoCurrentOnView();
        m_viewDurationTimer.start(minDuration);
    }
    /* Update weather info periodically. */
    else if (true == m_viewDurationTimer.isTimeout())
    {
        WeatherInfo oldWeatherInfo = getActiveWeatherInfo();
        WeatherInfo newWeatherInfo = WEATHER_INFO_EMPTY;

        nextWeatherInfo();

        /* The view will only be updated if different weather info is required
         * or the weather info itself was updated.
         */
        if (oldWeatherInfo != getActiveWeatherInfo())
        {
            m_isWeatherInfoCurrentUpdated = true;
            m_isWeatherIconCurrentUpdated = true; /* The icon will change depended on kind of weather information. */
        }

        m_viewDurationTimer.restart();
    }
    else
    {
        /* Nothing to do. */
    }

    if (true == m_isWeatherInfoCurrentUpdated)
    {
        updateWeatherInfoCurrentOnView();
        m_isWeatherInfoCurrentUpdated = false;
    }
}

void OpenWeatherView32x16::getIconPathByWeatherIconId(String& fullPath, const String& weatherIconId, const String&addition) const
{
    fullPath.clear();

    if (false == weatherIconId.isEmpty())
    {
        String fullPathWithoutExt   = IMAGE_PATH + weatherIconId;
        String fullPathToIcon       = fullPathWithoutExt + addition + BitmapWidget::FILE_EXT_BITMAP;

        /* No specific bitmap icon available? */
        if (false == FILESYSTEM.exists(fullPathToIcon))
        {
            /* No specific GIF icon available? */
            fullPathToIcon = fullPathWithoutExt + addition + BitmapWidget::FILE_EXT_GIF;
            if (false == FILESYSTEM.exists(fullPathToIcon))
            {
                fullPathWithoutExt  = IMAGE_PATH + weatherIconId.substring(0U, weatherIconId.length() - 1U);
                fullPathToIcon      = fullPathWithoutExt + addition + BitmapWidget::FILE_EXT_BITMAP;

                /* No generic bitmap icon available? */
                if (false == FILESYSTEM.exists(fullPathToIcon))
                {
                    fullPathToIcon = fullPathWithoutExt + addition + BitmapWidget::FILE_EXT_GIF;

                    /* No generic GIF icon available? */
                    if (true == FILESYSTEM.exists(fullPathToIcon))
                    {
                        fullPath = fullPathToIcon;
                    }
                }
                else
                {
                    fullPath = fullPathToIcon;
                }
            }
            else
            {
                fullPath = fullPathToIcon;
            }
        }
        else
        {
            fullPath = fullPathToIcon;
        }
    }
}

const char* OpenWeatherView32x16::uvIndexToColor(uint8_t uvIndex)
{
    uint8_t     idx     = 0U;
    const char* color   = "{#a80081}"; /* Default color */

    while(UTIL_ARRAY_NUM(uvIndexTable) > idx)
    {
        if ((uvIndexTable[idx].lower <= uvIndex) &&
            (uvIndexTable[idx].upper > uvIndex))
        {
            color = uvIndexTable[idx].color;
            break;
        }

        ++idx;
    }

    return color;
}

void OpenWeatherView32x16::appendTemperature(String& dst, float temperature, bool noFraction, bool noUnit)
{
    if (false == std::isnan(temperature))
    {
        const char* reducePrecision         = (false == noFraction) ? (temperature < -9.9F) ? "%.0f" : "%.1f" : "%.0f";
        char        tempReducedPrecison[6]  = { 0 };

        /* Generate temperature string with reduced precision and add unit �C/�F. */
        (void)snprintf(tempReducedPrecison, sizeof(tempReducedPrecison), reducePrecision, temperature);

        dst += tempReducedPrecison;

        if (false == noUnit)
        {
            dst += "\x8E";

            if (m_units == "default")
            {
                dst += "K";
            }
            else if (m_units == "metric")
            {
                dst += "C";
            }
            else
            {
                dst += "F";
            }
        }
    }
    else
    {
        dst += "-";
    }
}

void OpenWeatherView32x16::appendHumidity(String& dst, uint8_t humidity)
{
    dst += humidity;
    dst += "%";
}

void OpenWeatherView32x16::appendWindSpeed(String& dst, float windSpeed)
{
    if (false == std::isnan(windSpeed))
    {
        char windReducedPrecison[5] = { 0 };

        (void)snprintf(windReducedPrecison, sizeof(windReducedPrecison), "%.1f", windSpeed);

        dst += windReducedPrecison;

        if (m_units == "default")
        {
            dst += "m/s";
        }
        else if (m_units == "metric")
        {
            dst += "m/s";
        }
        else
        {
            dst += "mph";
        }
    }
    else
    {
        dst += "-";
    }
}

void OpenWeatherView32x16::appendUvIndex(String& dst, float uvIndex)
{
    if (false == std::isnan(uvIndex))
    {
        dst += uvIndexToColor(static_cast<uint8_t>(uvIndex));
        dst += uvIndex;
    }
    else
    {
        dst += "-";
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
