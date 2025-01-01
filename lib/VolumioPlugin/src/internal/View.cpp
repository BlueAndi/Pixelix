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
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "View.h"

using namespace _VolumioPlugin;

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
const char* View::IMAGE_PATH_STD_ICON   = "/plugins/VolumioPlugin/volumio.bmp";

/* Initialize image path for "stop" icon. */
const char* View::IMAGE_PATH_STOP_ICON  = "/plugins/VolumioPlugin/volumioStop.bmp";

/* Initialize image path for "play" icon. */
const char* View::IMAGE_PATH_PLAY_ICON  = "/plugins/VolumioPlugin/volumioPlay.bmp";

/* Initialize image path for "pause" icon. */
const char* View::IMAGE_PATH_PAUSE_ICON = "/plugins/VolumioPlugin/volumioPause.bmp";

/* Initialize the icon mapping table. */
const char* View::ICON_TABLE[ICON_MAX]      =
{
    /* The order follows the order in the Icon enumeration! */
    View::IMAGE_PATH_STD_ICON,
    View::IMAGE_PATH_STOP_ICON,
    View::IMAGE_PATH_PLAY_ICON,
    View::IMAGE_PATH_PAUSE_ICON
};

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void View::loadIconByType(Icon type)
{
    const char* iconPath = IMAGE_PATH_STD_ICON;

    if (ICON_MAX > type)
    {
        iconPath = ICON_TABLE[type];
    }

    if (false == loadIcon(iconPath))
    {
        (void)loadIcon(IMAGE_PATH_STD_ICON);
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
