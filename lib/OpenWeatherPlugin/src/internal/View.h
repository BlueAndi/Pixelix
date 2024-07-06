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
 * @brief  OpenWeatherPlugin view
 * @author Andreas Merkle <web@blue-andi.de>
 * @addtogroup plugin
 *
 * @{
 */

#ifndef OPENWEATHER_PLUGIN_VIEW_H
#define OPENWEATHER_PLUGIN_VIEW_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <IconTextViewBase.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** Internal plugin functionality. */
namespace _OpenWeatherPlugin
{

/**
 * OpenWeatherPlugin view.
 */
class View : public IconTextViewBase
{
public:

    /**
     * Supported icons.
     */
    enum Icon
    {
        ICON_STD = 0,   /**< Standard OpenWeather icon. */
        ICON_UVI,       /**< UV-index icon. */
        ICON_HUMIDITY,  /**< Humidity icon. */
        ICON_WIND,      /**< Wind icon. */
        ICON_MAX        /**< Max. number of icons. */
    };

    /**
     * Construct the view.
     */
    View() :
        IconTextViewBase()
    {
    }

    /**
     * Destroy the view.
     */
    ~View()
    {
    }

    /**
     * Initialize view, which will prepare the widgets and the default values.
     */
    void init(uint16_t width, uint16_t height) final
    {
        IconTextViewBase::init(width, height);

        setFormatText("{hc}?");
        (void)loadIconByType(ICON_STD);
    }

    /**
     * Update the underlying canvas only with the bitmap.
     * 
     * @param[in] gfx   Graphic functionality to draw on the underlying canvas.
     */
    void updateOnlyBitmap(YAGfx& gfx)
    {
        m_bitmapWidget.update(gfx);
    }

    /**
     * Load dedicated icon.
     * 
     * @param[in] type  The icon type which to set.
     */
    void loadIconByType(Icon type);

private:

    /**
     * Image path within the filesystem to standard icon.
     */
    static const char*      IMAGE_PATH_STD_ICON;

    /**
     * Image path within the filesystem to UV index icon.
     */
    static const char*      IMAGE_PATH_UVI_ICON;

    /**
     * Image path within the filesystem to humidity icon.
     */
    static const char*      IMAGE_PATH_HUMIDITY_ICON;

    /**
     * Image path within the filesystem to windspeed icon.
     */
    static const char*      IMAGE_PATH_WIND_ICON;

    /**
     * Table of icons mapped according to the icon enumeration.
     */
    static const char*      ICON_TABLE[ICON_MAX];

    View(const View& other);
    View& operator=(const View& other);
};

} /* _OpenWeatherPlugin */

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* OPENWEATHER_PLUGIN_VIEW_H */

/** @} */
