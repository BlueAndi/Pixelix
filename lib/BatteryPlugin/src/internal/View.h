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
 * @brief  BatteryPlugin view
 * @author Andreas Merkle <web@blue-andi.de>
 * @addtogroup plugin
 *
 * @{
 */

#ifndef BATTERY_PLUGIN_VIEW_H
#define BATTERY_PLUGIN_VIEW_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <CanvasViewBase.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/**
 * Divide and round at compile time.
 */
#define DIVIDE_AND_ROUND(_dividend, _divisor)   (((_dividend) + (_divisor) / 2U) / (_divisor))

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** Internal plugin functionality. */
namespace _BatteryPlugin
{

/**
 * BatteryPlugin view.
 */
class View : public CanvasViewBase
{
public:

    /**
     * Construct the view.
     */
    View() :
        CanvasViewBase(),
        m_stateOfCharge(0U)
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
        CanvasViewBase::init(width, height);

        /* Draw battery symbol body. */
        m_canvasWidget.drawRectangle(
            SPACE_LEFT_AND_RIGHT + BATTERY_POLE_WIDTH,
            SPACE_TOP_AND_BOTTOM,
            BATTERY_WIDTH,
            BATTERY_HEIGHT,
            ColorDef::WHITE);
        m_canvasWidget.fillRect(
            SPACE_LEFT_AND_RIGHT,
            SPACE_TOP_AND_BOTTOM + 2U * BATTERY_BORDER,
            BATTERY_POLE_WIDTH,
            BATTERY_POLE_HEIGHT,
            ColorDef::WHITE);
    }

    /**
     * Update the underlying canvas.
     * 
     * @param[in] gfx   Graphic functionality to draw on the underlying canvas.
     */
    void update(YAGfx& gfx) final
    {
        CanvasViewBase::update(gfx);
        drawStateOfCharge(gfx);
    }

    /**
     * Set the state-of-charge in percent.
     * 
     * @param[in] soc   State-of-charge in percent
     */
    void setStateOfCharge(uint8_t soc)
    {
        m_stateOfCharge = soc;
    }

private:

    /**
     * Bitmap width in pixels.
     */
    static const uint16_t   BATTERY_SYMBOL_WIDTH    = CONFIG_LED_MATRIX_WIDTH;

    /**
     * Bitmap height in pixels.
     */
    static const uint16_t   BATTERY_SYMBOL_HEIGHT   = CONFIG_LED_MATRIX_HEIGHT;

    /**
     * Space left and right to the border in pixel.
     * It shall be 12.5 % of the widget width.
     */
    static const uint16_t   SPACE_LEFT_AND_RIGHT    = DIVIDE_AND_ROUND(BATTERY_SYMBOL_WIDTH, 8U);

    /**
     * Space top and bottom to the border in pixel.
     * It shall be 12.5 % of the widget width.
     */
    static const uint16_t   SPACE_TOP_AND_BOTTOM    = DIVIDE_AND_ROUND(BATTERY_SYMBOL_HEIGHT, 8U);

    /**
     * Battery border in pixel.
     */
    static const uint16_t   BATTERY_BORDER          = 1U;

    /**
     * Battery pole width in pixel.
     * It shall be 5 % of the widget width.
     */
    static const uint16_t   BATTERY_POLE_WIDTH      = DIVIDE_AND_ROUND(BATTERY_SYMBOL_WIDTH, 20U);

    /**
     * Battery pole height in pixel.
     */
    static const uint16_t   BATTERY_POLE_HEIGHT     = BATTERY_SYMBOL_HEIGHT - 2U * (SPACE_TOP_AND_BOTTOM + 2U * BATTERY_BORDER);

    /**
     * Battery width in pixel.
     */
    static const uint16_t   BATTERY_WIDTH           = BATTERY_SYMBOL_WIDTH - 2U * SPACE_LEFT_AND_RIGHT - BATTERY_POLE_WIDTH;

    /**
     * Battery height in pixel.
     */
    static const uint16_t   BATTERY_HEIGHT          = BATTERY_SYMBOL_HEIGHT - 2U * SPACE_TOP_AND_BOTTOM;

    /**
     * State of capacity bar x-coordinate in pixel.
     */
    static const int16_t    SOC_BAR_X               = SPACE_LEFT_AND_RIGHT + BATTERY_POLE_WIDTH + BATTERY_BORDER;

    /**
     * State of capacity bar y-coordinate in pixel.
     */
    static const int16_t    SOC_BAR_Y               = SPACE_TOP_AND_BOTTOM + BATTERY_BORDER;

    /**
     * State of capacity bar width in pixel.
     */
    static const uint16_t   SOC_BAR_WIDTH           = BATTERY_WIDTH - 2U * BATTERY_BORDER;

    /**
     * State of capacity bar height in pixel.
     */
    static const uint16_t   SOC_BAR_HEIGHT          = BATTERY_HEIGHT - 2U * BATTERY_BORDER;

    uint8_t m_stateOfCharge;    /**< Battery state of charge in %. */

    View(const View& other);
    View& operator=(const View& other);

    /**
     * Draw the state of charge on the display.
     *
     * @param[in] gfx   Display graphics interface
     */
    void drawStateOfCharge(YAGfx& gfx) const;
};

} /* _BatteryPlugin */

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* BATTERY_PLUGIN_VIEW_H */

/** @} */
