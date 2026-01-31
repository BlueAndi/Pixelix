/* MIT License
 *
 * Copyright (c) 2019 - 2026 Andreas Merkle <web@blue-andi.de>
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
 * @file   IndicatorViewGeneric.h
 * @brief  Generic view with indicators in each display corner.
 * @author Andreas Merkle <web@blue-andi.de>
 * @addtogroup PLUGIN
 *
 * @{
 */

#ifndef INDICATOR_VIEW_GENERIC_H
#define INDICATOR_VIEW_GENERIC_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <YAGfx.h>
#include <LampWidget.h>
#include <Util.h>

#include "../interface/IIndicatorView.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Generic view for LED matrix with bitmap and canvas.
 */
class IndicatorViewGeneric : public IIndicatorView
{
public:

    /**
     * Construct the view.
     */
    IndicatorViewGeneric() :
        IIndicatorView(),
        m_lampWidgets{
            { LAMP_WIDTH, LAMP_HEIGHT, LAMP_0_X, LAMP_0_Y }, /* Top left */
            { LAMP_WIDTH, LAMP_HEIGHT, LAMP_1_X, LAMP_1_Y }, /* Top right */
            { LAMP_WIDTH, LAMP_HEIGHT, LAMP_2_X, LAMP_2_Y }, /* Bottom right */
            { LAMP_WIDTH, LAMP_HEIGHT, LAMP_3_X, LAMP_3_Y }  /* Bottom left */
        }
    {
        size_t idx;

        /* Set on state color to yellow. */
        for (idx = 0U; idx < MAX_LAMPS; ++idx)
        {
            m_lampWidgets[idx].setColorOn(ColorDef::YELLOW);
        }
    }

    /**
     * Destroy the view.
     */
    virtual ~IndicatorViewGeneric()
    {
    }

    /**
     * Initialize view, which will prepare the widgets and the default values.
     *
     * @param[in] width     Display width in pixel.
     * @param[in] height    Display height in pixel.
     */
    void init(uint16_t width, uint16_t height) override
    {
        UTIL_NOT_USED(width);
        UTIL_NOT_USED(height);
    }

    /**
     * Update the underlying canvas.
     *
     * @param[in] gfx   Graphic functionality to draw on the underlying canvas.
     */
    void update(YAGfx& gfx) override;

    /**
     * Set the indicator at given position to on/off state.
     * If the indicator id is invalid, it will do nothing.
     *
     * The indicator id 255 will be used to turn on/off all indicators at once.
     *
     * @param[in] indicatorId   Id of the indicator, which to set.
     * @param[in] isOn          If true, the indicator will be set to on state, otherwise off.
     */
    void setIndicator(uint8_t indicatorId, bool isOn) override;

    /**
     * Get the indicator state at given position.
     * If the indicator id is invalid, it will return false.
     *
     * @param[in]  indicatorId   Id of the indicator, which to set.
     *
     * @return If the indicator is on, it will return true otherwise false.
     */
    bool isIndicatorOn(uint8_t indicatorId) const override;

    /**
     *  Indicator id for all indicators.
     */
    static const uint8_t INDICATOR_ID_ALL = 255U;

    /**
     * Max. number of lamps.
     */
    static const uint8_t MAX_LAMPS        = 4U;

protected:

    /**
     * Lamp width in pixel.
     */
    static const uint8_t LAMP_WIDTH  = 1U;

    /**
     * Lamp height in pixel.
     */
    static const uint8_t LAMP_HEIGHT = 1U;

    /**
     * Lamp 0 x-coordinate in pixel.
     */
    static const int16_t LAMP_0_X    = 0;

    /**
     * Lamp 0 y-coordinate in pixel.
     */
    static const int16_t LAMP_0_Y    = 0;

    /**
     * Lamp 1 x-coordinate in pixel.
     */
    static const int16_t LAMP_1_X    = CONFIG_LED_MATRIX_WIDTH - 1;

    /**
     * Lamp 1 y-coordinate in pixel.
     */
    static const int16_t LAMP_1_Y    = 0;

    /**
     * Lamp 2 x-coordinate in pixel.
     */
    static const int16_t LAMP_2_X    = CONFIG_LED_MATRIX_WIDTH - 1;

    /**
     * Lamp 2 y-coordinate in pixel.
     */
    static const int16_t LAMP_2_Y    = CONFIG_LED_MATRIX_HEIGHT - 1;

    /**
     * Lamp 3 x-coordinate in pixel.
     */
    static const int16_t LAMP_3_X    = 0;

    /**
     * Lamp 3 y-coordinate in pixel.
     */
    static const int16_t LAMP_3_Y    = CONFIG_LED_MATRIX_HEIGHT - 1;

    LampWidget           m_lampWidgets[MAX_LAMPS]; /**< Lamp widgets, used to signal different things. */

private:

    IndicatorViewGeneric(const IndicatorViewGeneric& other);
    IndicatorViewGeneric& operator=(const IndicatorViewGeneric& other);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* INDICATOR_VIEW_GENERIC_H */

/** @} */
