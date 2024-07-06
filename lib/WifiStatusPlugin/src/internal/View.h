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
 * @brief  CanvasTextPlugin view
 * @author Andreas Merkle <web@blue-andi.de>
 * @addtogroup plugin
 *
 * @{
 */

#ifndef CANVAS_TEXT_PLUGIN_VIEW_H
#define CANVAS_TEXT_PLUGIN_VIEW_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <CanvasTextViewBase.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** Internal plugin functionality. */
namespace _WifiStatusPlugin
{

/**
 * CanvasTextPlugin view.
 */
class View : public CanvasTextViewBase
{
public:

    /**
     * Construct the view.
     */
    View() :
        CanvasTextViewBase()
    {
    }

    /**
     * Destroy the view.
     */
    ~View()
    {
    }

    /**
     * Update wifi status.
     *
     * @param[in] quality   Signal quality in percent [0; 100].
     */
    void updateWifiStatus(uint8_t quality);

private:

    /**
     * Number of signal signal strength bars.
     */
    static const uint8_t    WIFI_BARS               = 4U;

    /**
     * Number of spaces between bars.
     */
    static const uint8_t    WIFI_BAR_SPACES         = WIFI_BARS - 1U;

    /**
     * Width in pixel of a single signal strength bar.
     */
    static const uint16_t   WIFI_BAR_WIDTH          = (2U * CANVAS_WIDTH) / (2U * WIFI_BARS + WIFI_BAR_SPACES);

    /**
     * Width in pixel of a bar space. It shall be half of the bar width.
     */
    static const uint16_t   WIFI_BAR_SPACE_WIDTH    = WIFI_BAR_WIDTH / 2U;

    /**
     * Height in pixel of the lowest signal strength bar.
     */
    static const uint16_t   WIFI_BAR_HEIGHT         = CANVAS_HEIGHT / 4U;

    View(const View& other);
    View& operator=(const View& other);
};

} /* _WifiStatusPlugin */

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* CANVAS_TEXT_PLUGIN_VIEW_H */

/** @} */
