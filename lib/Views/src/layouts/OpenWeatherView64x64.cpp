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
 * @brief  OpenWeather view with icon and text for 64x64 LED matrix
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "OpenWeatherView64x64.h"
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

/** The epsilon is used to compare floats. */
static const float  EPSILON = 0.0001F;

/* Initialize image path for the weather condition icons. */
const char* OpenWeatherView64x64::IMAGE_PATH                        = "/plugins/OpenWeatherPlugin/";

/* Initialize image path for standard icon. */
const char* OpenWeatherView64x64::IMAGE_PATH_STD_ICON               = "/plugins/OpenWeatherPlugin/openWeather.bmp";

/* Initialize image path for standard icon. */
const char* OpenWeatherView64x64::IMAGE_PATH_STD_ICON_16X16         = "/plugins/OpenWeatherPlugin/openWeather_16x16.gif";

/* Initialize image path for uvi icon. */
const char* OpenWeatherView64x64::IMAGE_PATH_UVI_ICON_16X16         = "/plugins/OpenWeatherPlugin/uvi_16x16.gif";

/* Initialize image path for humidity icon. */
const char* OpenWeatherView64x64::IMAGE_PATH_HUMIDITY_ICON_16X16    = "/plugins/OpenWeatherPlugin/hum_16x16.gif";

/* Initialize image path for uvi icon. */
const char* OpenWeatherView64x64::IMAGE_PATH_WIND_ICON_16X16        = "/plugins/OpenWeatherPlugin/wind_16x16.gif";

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

OpenWeatherView64x64::OpenWeatherView64x64() :
    IOpenWeatherView(),
    m_fontType(Fonts::FONT_TYPE_DEFAULT),
    m_weatherIconCurrent(BITMAP_WIDTH, BITMAP_HEIGHT, BITMAP_X, BITMAP_Y),
    m_weatherInfoCurrentText(TEXT_WIDTH, TEXT_HEIGHT, TEXT_X, TEXT_Y),
    m_forecastIcons{
        { 12U, 16U, 0 * 12U + 2U, 32U },
        { 12U, 16U, 1 * 12U + 2U, 32U },
        { 12U, 16U, 2 * 12U + 2U, 32U },
        { 12U, 16U, 3 * 12U + 2U, 32U },
        { 12U, 16U, 4 * 12U + 2U, 32U }
    },
    m_forecastTemperatues{
        { 12U, 16U, 0 * 12U + 2U, 48U },
        { 12U, 16U, 1 * 12U + 2U, 48U },
        { 12U, 16U, 2 * 12U + 2U, 48U },
        { 12U, 16U, 3 * 12U + 2U, 48U },
        { 12U, 16U, 4 * 12U + 2U, 48U }
    },
    m_viewDurationTimer(),
    m_viewDuration(0U),
    m_units("metric"),
    m_weatherInfo(),
    m_weatherInfoId(0U),
    m_weatherInfoCurrent(),
    m_weatherInfoForecast{},
    m_isWeatherInfoCurrentUpdated(false),
    m_isWeatherIconCurrentUpdated(false),
    m_isWeatherInfoForecastUpdated(false),
    m_isWeatherIconForecastUpdated{}
{
    uint8_t day;

    m_weatherIconCurrent.setVerticalAlignment(Alignment::Vertical::VERTICAL_CENTER);
    m_weatherIconCurrent.setHorizontalAlignment(Alignment::Horizontal::HORIZONTAL_CENTER);
    
    m_weatherInfoCurrentText.setVerticalAlignment(Alignment::Vertical::VERTICAL_CENTER);
    m_weatherInfoCurrentText.setHorizontalAlignment(Alignment::Horizontal::HORIZONTAL_CENTER);

    for(day = 0U; day < FORECAST_DAYS; ++day)
    {
        m_forecastIcons[day].setVerticalAlignment(Alignment::Vertical::VERTICAL_CENTER);
        m_forecastIcons[day].setHorizontalAlignment(Alignment::Horizontal::HORIZONTAL_CENTER);

        m_forecastTemperatues[day].setVerticalAlignment(Alignment::Vertical::VERTICAL_CENTER);
        m_forecastTemperatues[day].setHorizontalAlignment(Alignment::Horizontal::HORIZONTAL_CENTER);
    }
}

void OpenWeatherView64x64::update(YAGfx& gfx)
{
    uint8_t day;

    handleWeatherInfo();

    gfx.fillScreen(ColorDef::BLACK);
    m_weatherIconCurrent.update(gfx);
    m_weatherInfoCurrentText.update(gfx);

    for(day = 0U; day < FORECAST_DAYS; ++day)
    {
        m_forecastIcons[day].update(gfx);
        m_forecastTemperatues[day].update(gfx);
    }
}

void OpenWeatherView64x64::setWeatherInfoCurrent(const WeatherInfoCurrent& info)
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

void OpenWeatherView64x64::setWeatherInfoForecast(uint8_t day, const WeatherInfoForecast& info)
{
    if (FORECAST_DAYS > day)
    {
        if ((m_weatherInfoForecast[day].iconId != info.iconId) ||
            (EPSILON < fabsf(m_weatherInfoForecast[day].temperatureMin - info.temperatureMin)) ||
            (EPSILON < fabsf(m_weatherInfoForecast[day].temperatureMax - info.temperatureMax)))
        {
            if (m_weatherInfoForecast[day].iconId != info.iconId)
            {
                m_isWeatherIconForecastUpdated[day] = true;
            }

            m_weatherInfoForecast[day]      = info;
            m_isWeatherInfoForecastUpdated  = true;
        }
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

uint8_t OpenWeatherView64x64::getWeatherInfoCount() const
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

void OpenWeatherView64x64::nextWeatherInfo()
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

OpenWeatherView64x64::WeatherInfo OpenWeatherView64x64::getActiveWeatherInfo() const
{
    return static_cast<WeatherInfo>(m_weatherInfo & (1U << m_weatherInfoId));
}

void OpenWeatherView64x64::updateWeatherInfoCurrentOnView()
{
    String iconFullPath;
    String text;

    switch(getActiveWeatherInfo())
    {
    case WEATHER_INFO_EMPTY:
        iconFullPath = IMAGE_PATH_STD_ICON_16X16;
        text = "-";
        break;

    case WEATHER_INFO_TEMPERATURE:
        getIconPathByWeatherIconId(iconFullPath, m_weatherInfoCurrent.iconId, "_16x16");

        if (true == iconFullPath.isEmpty())
        {
            iconFullPath = IMAGE_PATH_STD_ICON_16X16;
        }

        appendTemperature(text, m_weatherInfoCurrent.temperature);
        break;
        
    case WEATHER_INFO_HUMIDITY:
        iconFullPath = IMAGE_PATH_HUMIDITY_ICON_16X16;

        appendHumidity(text, m_weatherInfoCurrent.humidity);
        break;
        
    case WEATHER_INFO_WIND_SPEED:
        iconFullPath = IMAGE_PATH_WIND_ICON_16X16;

        appendWindSpeed(text, m_weatherInfoCurrent.windSpeed);
        break;
        
    case WEATHER_INFO_UV_INDEX:
        iconFullPath = IMAGE_PATH_UVI_ICON_16X16;

        appendUvIndex(text, m_weatherInfoCurrent.uvIndex);
        break;
        
    default:
        iconFullPath = IMAGE_PATH_STD_ICON_16X16;
        text = "-";
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

void OpenWeatherView64x64::updateWeatherInfoForecastOnView()
{
    int8_t day;

    for(day = 0U; day < FORECAST_DAYS; ++day)
    {
        String temperatures;

        if (true == m_isWeatherIconForecastUpdated[day])
        {
            String iconFullPath;

            getIconPathByWeatherIconId(iconFullPath, m_weatherInfoForecast[day].iconId, "");

            if (true == iconFullPath.isEmpty())
            {
                iconFullPath = IMAGE_PATH_STD_ICON;
            }

            (void)m_forecastIcons[day].load(FILESYSTEM, iconFullPath);
            m_isWeatherIconForecastUpdated[day] = false;
        }

        appendTemperature(temperatures, m_weatherInfoForecast[day].temperatureMin, true, true);
        temperatures += "\n";
        appendTemperature(temperatures, m_weatherInfoForecast[day].temperatureMax, true, true);

        m_forecastTemperatues[day].setFormatStr(temperatures);
    }
}

void OpenWeatherView64x64::handleWeatherInfo()
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
        for(uint8_t day = 0U; day < FORECAST_DAYS; ++day)
        {
            m_isWeatherIconForecastUpdated[day] = true;
        }

        updateWeatherInfoCurrentOnView();
        updateWeatherInfoForecastOnView();
        
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

    if (true == m_isWeatherInfoForecastUpdated)
    {
        updateWeatherInfoForecastOnView();
        m_isWeatherInfoForecastUpdated = false;
    }
}

void OpenWeatherView64x64::getIconPathByWeatherIconId(String& fullPath, const String& weatherIconId, const String&addition) const
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

const char* OpenWeatherView64x64::uvIndexToColor(uint8_t uvIndex)
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

void OpenWeatherView64x64::appendTemperature(String& dst, float temperature, bool noFraction, bool noUnit)
{
    if (false == std::isnan(temperature))
    {
        const char* reducePrecision         = (false == noFraction) ? (temperature < -9.9F) ? "%.0f" : "%.1f" : "%.0f";
        char        tempReducedPrecison[6]  = { 0 };

        /* Generate temperature string with reduced precision and add unit °C/°F. */
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

void OpenWeatherView64x64::appendHumidity(String& dst, uint8_t humidity)
{
    dst += humidity;
    dst += "%";
}

void OpenWeatherView64x64::appendWindSpeed(String& dst, float windSpeed)
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

void OpenWeatherView64x64::appendUvIndex(String& dst, float uvIndex)
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
