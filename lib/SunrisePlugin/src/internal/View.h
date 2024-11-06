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
 * @brief  SunrisePlugin view
 * @author Andreas Merkle <web@blue-andi.de>
 * @addtogroup plugin
 *
 * @{
 */

#ifndef SUNRISE_PLUGIN_VIEW_H
#define SUNRISE_PLUGIN_VIEW_H

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
namespace _SunrisePlugin
{

/**
 * SunrisePlugin view.
 */
class View : public IconTextViewBase
{
public:

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
        (void)loadIcon(ICON_PATH);
    }

private:

    /**
     * Image path within the filesystem to icon.
     */
    static const char*  ICON_PATH;

    View(const View& other);
    View& operator=(const View& other);
};

} /* _SunrisePlugin */

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* SUNRISE_PLUGIN_VIEW_H */

/** @} */