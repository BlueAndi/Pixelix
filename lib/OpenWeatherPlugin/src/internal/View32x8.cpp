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
 * @brief  Plugin view for 32x8 LED matrix
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "View32x8.h"
#include <Logging.h>

using namespace _OpenWeatherPlugin;

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

/* Initialize image path for standard icon. */
const char* View::IMAGE_PATH_STD_ICON       = "/plugins/OpenWeatherPlugin/openWeather.bmp";

/* Initialize image path for uvi icon. */
const char* View::IMAGE_PATH_UVI_ICON       = "/plugins/OpenWeatherPlugin/uvi.bmp";

/* Initialize image path for humidity icon. */
const char* View::IMAGE_PATH_HUMIDITY_ICON  = "/plugins/OpenWeatherPlugin/hum.bmp";

/* Initialize image path for uvi icon. */
const char* View::IMAGE_PATH_WIND_ICON      = "/plugins/OpenWeatherPlugin/wind.bmp";

/* Initialize the icon mapping table. */
const char* View::ICON_TABLE[ICON_MAX]      =
{
    /* The order follows the order in the Icon enumeration! */
    View::IMAGE_PATH_STD_ICON,
    View::IMAGE_PATH_UVI_ICON,
    View::IMAGE_PATH_HUMIDITY_ICON,
    View::IMAGE_PATH_WIND_ICON
};

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void View::loadIcon(Icon type)
{
    const char* iconPath = IMAGE_PATH_STD_ICON;

    if (ICON_MAX > type)
    {
        iconPath = ICON_TABLE[type];
    }

    if (false == m_bitmapWidget.load(FILESYSTEM, iconPath))
    {
        LOG_WARNING("Icon doesn't exists: %s", iconPath);

        (void)m_bitmapWidget.load(FILESYSTEM, IMAGE_PATH_STD_ICON);
    }
}

void View::loadIcon(const String& fullPath)
{
    if (true == fullPath.isEmpty())
    {
        LOG_WARNING("Empty icon path, use standard icon.");

        loadIcon(ICON_STD);
    }
    else if (false == m_bitmapWidget.load(FILESYSTEM, fullPath))
    {
        LOG_WARNING("Icon doesn't exists: %s", fullPath.c_str());

        loadIcon(ICON_STD);
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
