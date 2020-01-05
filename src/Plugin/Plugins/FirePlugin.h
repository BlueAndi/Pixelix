/* MIT License
 *
 * Copyright (c) 2019 - 2020 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Fire demo plugin
 * @author Andreas Merkle <web@blue-andi.de>
 * 
 * @addtogroup plugin
 *
 * @{
 */

#ifndef __FIREPLUGIN_H__
#define __FIREPLUGIN_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include "Plugin.hpp"
#include <SimpleTimer.hpp>
#include <Color.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Fire simulation plugin.
 * 
 * This basic one-dimensional 'fire' simulation works roughly as follows:
 * There's a underlying array of 'heat' cells, that model the temperature
 * at each point along the line.  Every cycle through the simulation, 
 * four steps are performed:
 * 
 * 1) All cells cool down a little bit, losing heat to the air
 * 2) The heat from each cell drifts 'up' and diffuses a little
 * 3) Sometimes randomly new 'sparks' of heat are added at the bottom
 * 4) The heat from each cell is rendered as a color into the leds array
 * 
 * The heat-to-color mapping uses a black-body radiation approximation.
 * 
 * It was ported from https://github.com/FastLED/FastLED/blob/master/examples/Fire2012/Fire2012.ino
 */
class FirePlugin : public Plugin
{
public:

    /**
     * Constructs the plugin.
     */
    FirePlugin() :
        Plugin(),
        m_heat(NULL),
        m_heatSize(0u)
    {
    }

    /**
     * Destroys the plugin.
     */
    ~FirePlugin()
    {
        if (NULL != m_heat)
        {
            delete[] m_heat;
            m_heat = NULL;
        }
    }

    /**
     * Get the plugin name.
     *
     * @return Name of the plugin.
     */
    const char* getName(void) const
    {
        return "FirePlugin";
    }

    /**
     * This method will be called in case the plugin is set active, which means
     * it will be shown on the display in the next step.
     * 
     * @param[in] gfx   Display graphics interface
     */
    void active(IGfx& gfx);

    /**
     * This method will be called in case the plugin is set inactive, which means
     * it won't be shown on the display anymore.
     */
    void inactive(void);

    /**
     * Update the display.
     * The scheduler will call this method periodically.
     * 
     * @param[in] gfx   Display graphics interface
     */
    void update(IGfx& gfx);

private:

    uint8_t*    m_heat;     /**< Heat temperature [0; 255] */
    size_t      m_heatSize; /**< Number of heat temperatures */

    /**
     * Cooling: How much does the air cool as it rises?
     * Less cooling => taller flames.
     * More cooling => shorter flames.
     */
    static const uint8_t    COOLING     = 60u;

    /**
     * Sparking: What chance (out of 255) is there that a new spark will be lit?
     * Higher chance = more roaring fire.  Lower chance = more flickery fire.
     */
    static const uint8_t    SPARKING    = 120u;

    /**
     * Approximates a 'black body radiation' spectrum for a given 'heat' level.
     * This is useful for animations of 'fire'.
     * Heat is specified as an arbitrary scale from 0 (cool) to 255 (hot).
     * This is NOT a chromatically correct 'black body radiation'
     * spectrum, but it's surprisingly close, and it's fast and small.
     */
    Color heatColor(uint8_t temperature);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __FIREPLUGIN_H__ */

/** @} */