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
 * @brief  VolumioPlugin view
 * @author Andreas Merkle <web@blue-andi.de>
 * @addtogroup PLUGIN
 *
 * @{
 */

#ifndef VOLUMIO_PLUGIN_VIEW_H
#define VOLUMIO_PLUGIN_VIEW_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <PlayerViewBase.hpp>
#include <ProgressBar.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** Internal plugin functionality. */
namespace _VolumioPlugin
{

/**
 * VolumioPlugin view.
 */
class View : public PlayerViewBase
{
public:

    /**
     * Supported icons.
     */
    enum Icon
    {
        ICON_STD = 0,   /**< Standard Volumio icon. */
        ICON_STOP,      /**< Stop icon. */
        ICON_PLAY,      /**< Play icon. */
        ICON_PAUSE,     /**< Pause icon. */
        ICON_MAX        /**< Max. number of icons. */
    };

    /**
     * Construct the view.
     */
    View() :
        PlayerViewBase()
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
     * 
     * @param[in] width     Display width in pixel.
     * @param[in] height    Display height in pixel.
     */
    void init(uint16_t width, uint16_t height) final
    {
        PlayerViewBase::init(width, height);

        setFormatText("{hc}?");
        (void)loadIconByType(ICON_STD);
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
    static const char*  IMAGE_PATH_STD_ICON;

    /**
     * Image path within the filesystem to "stop" icon.
     */
    static const char*  IMAGE_PATH_STOP_ICON;

    /**
     * Image path within the filesystem to "play" icon.
     */
    static const char*  IMAGE_PATH_PLAY_ICON;

    /**
     * Image path within the filesystem to "pause" icon.
     */
    static const char*  IMAGE_PATH_PAUSE_ICON;

    /**
     * Table of icons mapped according to the icon enumeration.
     */
    static const char*  ICON_TABLE[ICON_MAX];

    View(const View& other);
    View& operator=(const View& other);
};

} /* _VolumioPlugin */

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* VOLUMIO_PLUGIN_VIEW_H */

/** @} */
