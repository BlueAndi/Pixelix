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
 * @brief  OpenWeather view with icon and text for 32x8 LED matrix
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "OpenWeatherView32x8.h"
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

/* Initialize image path for the weather condition icons. */
const char* OpenWeatherView32x8::IMAGE_PATH                 = "/plugins/OpenWeatherPlugin/";

/* Initialize image path for standard icon. */
const char* OpenWeatherView32x8::IMAGE_PATH_STD_ICON        = "/plugins/OpenWeatherPlugin/openWeather.bmp";

/* Initialize image path for uvi icon. */
const char* OpenWeatherView32x8::IMAGE_PATH_UVI_ICON        = "/plugins/OpenWeatherPlugin/uvi.bmp";

/* Initialize image path for humidity icon. */
const char* OpenWeatherView32x8::IMAGE_PATH_HUMIDITY_ICON   = "/plugins/OpenWeatherPlugin/hum.bmp";

/* Initialize image path for uvi icon. */
const char* OpenWeatherView32x8::IMAGE_PATH_WIND_ICON       = "/plugins/OpenWeatherPlugin/wind.bmp";

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

void OpenWeatherView32x8::update(YAGfx& gfx)
{
    handleWeatherInfo();

    gfx.fillScreen(ColorDef::BLACK);
    m_bitmapWidget.update(gfx);
    m_textWidget.update(gfx);
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

uint8_t OpenWeatherView32x8::getWeatherInfoCount() const
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

void OpenWeatherView32x8::nextWeatherInfo()
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

OpenWeatherView32x8::WeatherInfo OpenWeatherView32x8::getActiveWeatherInfo() const
{
    return static_cast<WeatherInfo>(m_weatherInfo & (1U << m_weatherInfoId));
}

void OpenWeatherView32x8::updateWeatherInfoOnView()
{
    String iconFullPath;
    String text         = "-";

    switch(getActiveWeatherInfo())
    {
    case WEATHER_INFO_EMPTY:
        iconFullPath = IMAGE_PATH_STD_ICON;
        break;

    case WEATHER_INFO_TEMPERATURE:
        getIconPathByWeatherIconId(iconFullPath, m_weatherInfoCurrent.iconId);

        if (true == iconFullPath.isEmpty())
        {
            iconFullPath = IMAGE_PATH_STD_ICON;
        }

        if (false == std::isnan(m_weatherInfoCurrent.temperature))
        {
            const char* reducePrecision         = (m_weatherInfoCurrent.temperature < -9.9F) ? "%.0f" : "%.1f";
            char        tempReducedPrecison[6]  = { 0 };

            /* Generate temperature string with reduced precision and add unit °C/°F. */
            (void)snprintf(tempReducedPrecison, sizeof(tempReducedPrecison), reducePrecision, m_weatherInfoCurrent.temperature);

            text  = tempReducedPrecison;
            text += "\x8E";

            if (m_units == "default")
            {
                text += "K";
            }
            else if (m_units == "metric")
            {
                text += "C";
            }
            else
            {
                text += "F";
            }
        }
        break;
        
    case WEATHER_INFO_HUMIDITY:
        iconFullPath = IMAGE_PATH_HUMIDITY_ICON;

        text  = m_weatherInfoCurrent.humidity;
        text += "%";
        break;
        
    case WEATHER_INFO_WIND_SPEED:
        iconFullPath = IMAGE_PATH_WIND_ICON;

        if (false == std::isnan(m_weatherInfoCurrent.windSpeed))
        {
            char windReducedPrecison[5] = { 0 };

            (void)snprintf(windReducedPrecison, sizeof(windReducedPrecison), "%.1f", m_weatherInfoCurrent.windSpeed);

            text  = windReducedPrecison;

            if (m_units == "default")
            {
                text += "m/s";
            }
            else if (m_units == "metric")
            {
                text += "m/s";
            }
            else
            {
                text += "mph";
            }
        }
        break;
        
    case WEATHER_INFO_UV_INDEX:
        iconFullPath = IMAGE_PATH_UVI_ICON;

        if (false == std::isnan(m_weatherInfoCurrent.uvIndex))
        {
            text  = uvIndexToColor(static_cast<uint8_t>(m_weatherInfoCurrent.uvIndex));
            text += m_weatherInfoCurrent.uvIndex;
        }
        break;
        
    default:
        iconFullPath = IMAGE_PATH_STD_ICON;
        break;
    }

    /* Change icon only if its really necessary to avoid restarting animated
     * icon.
     */
    if (m_iconFullPath != iconFullPath)
    {
        (void)m_bitmapWidget.load(FILESYSTEM, iconFullPath);
        m_iconFullPath = iconFullPath;
    }

    m_textWidget.setFormatStr(text);
}

void OpenWeatherView32x8::handleWeatherInfo()
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

        updateWeatherInfoOnView();
        m_viewDurationTimer.start(minDuration);
    }
    /* Update weather info periodically or in case the weather info is updated, immediate. */
    else if ((true == m_viewDurationTimer.isTimeout()) || (true == m_isWeatherInfoUpdated))
    {
        WeatherInfo oldWeatherInfo = getActiveWeatherInfo();
        WeatherInfo newWeatherInfo = WEATHER_INFO_EMPTY;

        nextWeatherInfo();

        /* The view will only be updated if different weather info is required
         * or the weather info itself was updated.
         */
        if ((oldWeatherInfo != getActiveWeatherInfo()) || (true == m_isWeatherInfoUpdated))
        {
            updateWeatherInfoOnView();
        }

        m_viewDurationTimer.restart();
        m_isWeatherInfoUpdated = false;
    }
    else
    {
        /* Nothing to do. */
    }
}

void OpenWeatherView32x8::getIconPathByWeatherIconId(String& fullPath, const String& weatherIconId) const
{
    fullPath.clear();

    if (false == weatherIconId.isEmpty())
    {
        String fullPathWithoutExt   = IMAGE_PATH + weatherIconId;
        String fullPathToIcon       = fullPathWithoutExt + BitmapWidget::FILE_EXT_BITMAP;

        /* No specific bitmap icon available? */
        if (false == FILESYSTEM.exists(fullPathToIcon))
        {
            /* No specific GIF icon available? */
            fullPathToIcon = fullPathWithoutExt + BitmapWidget::FILE_EXT_GIF;
            if (false == FILESYSTEM.exists(fullPathToIcon))
            {
                fullPathWithoutExt  = IMAGE_PATH + weatherIconId.substring(0U, weatherIconId.length() - 1U);
                fullPathToIcon      = fullPathWithoutExt + BitmapWidget::FILE_EXT_BITMAP;

                /* No generic bitmap icon available? */
                if (false == FILESYSTEM.exists(fullPathToIcon))
                {
                    fullPathToIcon = fullPathWithoutExt + BitmapWidget::FILE_EXT_GIF;

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

const char* OpenWeatherView32x8::uvIndexToColor(uint8_t uvIndex)
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

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
