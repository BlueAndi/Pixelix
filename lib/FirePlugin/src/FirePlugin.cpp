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
 * @file   FirePlugin.cpp
 * @brief  Fire demo plugin
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "FirePlugin.h"

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

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void FirePlugin::start(uint16_t width, uint16_t height)
{
    if (nullptr == m_heat)
    {
        m_heatSize = width * height;
        m_heat     = new (std::nothrow) uint8_t[m_heatSize];

        if (nullptr == m_heat)
        {
            m_heatSize = 0U;
        }
    }
}

void FirePlugin::stop()
{
    if (nullptr != m_heat)
    {
        delete[] m_heat;
        m_heat = nullptr;
    }
}

void FirePlugin::active(YAGfx& gfx)
{
    /* Clear display */
    gfx.fillScreen(ColorDef::BLACK);
}

void FirePlugin::inactive()
{
    /* Nothing to do. */
}

void FirePlugin::update(YAGfx& gfx)
{
    if (nullptr != m_heat)
    {
        uint16_t width  = gfx.getWidth();
        uint16_t height = gfx.getHeight();

        coolDown(width, height);
        heatUpCellDriftAndDiffuse(width, height);
        randomNewSparks(width, height);
        draw(gfx);
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

Color FirePlugin::heatColor(uint8_t temperature)
{
    Color heatColor;

    /* Scale 'heat' down from 0-255 to 0-191, which can then be easily divided
     * into three equal 'thirds' of 64 units each.
     */
    uint8_t t192       = (static_cast<uint16_t>(temperature) * 191U) >> 8U;

    /* Calculate a value that ramps up from zero to 255 in each 'third' of the scale. */
    uint8_t heatRamp   = t192 & 0x3fU; /* 0..63 */

    /* Scale up to 0..252 */
    heatRamp         <<= 2;

    /* Now figure out which third of the spectrum we're in. */
    if (t192 & 0x80U)
    {
        /* We're in the hottest third */
        heatColor.setRed(255U);      /* Full red */
        heatColor.setGreen(255U);    /* Full green */
        heatColor.setBlue(heatRamp); /* Ramp up blue */
    }
    else if (t192 & 0x40U)
    {
        /* We're in the middle third */
        heatColor.setRed(255U);       /* Full red */
        heatColor.setGreen(heatRamp); /* Ramp up green */
        heatColor.setBlue(0U);        /* No blue */
    }
    else
    {
        /* We're in the coolest third */
        heatColor.setRed(heatRamp); /* Ramp up red */
        heatColor.setGreen(0U);     /* No green */
        heatColor.setBlue(0U);      /* No blue */
    }

    return heatColor;
}

void FirePlugin::coolDown(uint16_t width, uint16_t height)
{
    uint16_t x;

    for (x = 0U; x < width; ++x)
    {
        for (uint16_t y = 0U; y < height; ++y)
        {
            uint8_t  coolDownTemperature = random(0, ((COOLING * 10U) / height) + 2U);
            uint32_t heatPos             = x + y * width;

            if (coolDownTemperature >= m_heat[heatPos])
            {
                m_heat[heatPos] = 0U;
            }
            else
            {
                m_heat[heatPos] -= coolDownTemperature;
            }
        }
    }
}

void FirePlugin::heatUpCellDriftAndDiffuse(uint16_t width, uint16_t height)
{
    uint16_t x;
    uint16_t y;

    for (x = 0U; x < width; ++x)
    {
        for (y = 0U; y < (height - 1U); ++y)
        {
            uint16_t diffusHeat = 0U;

            if ((height - 2U) > y)
            {
                diffusHeat += m_heat[x + (y + 1U) * width];
                diffusHeat += m_heat[x + (y + 1U) * width];
                diffusHeat += m_heat[x + (y + 2U) * width];
                diffusHeat /= 3U;
            }
            else
            {
                diffusHeat += m_heat[x + (y + 0U) * width];
                diffusHeat += m_heat[x + (y + 0U) * width];
                diffusHeat += m_heat[x + (y + 1U) * width];
                diffusHeat /= 3U;
            }

            m_heat[x + y * width] = diffusHeat;
        }
    }
}

void FirePlugin::randomNewSparks(uint16_t width, uint16_t height)
{
    uint16_t x;

    for (x = 0U; x < width; ++x)
    {
        if (random(0, 255) < SPARKING)
        {
            uint8_t  randValue = random(160, 255);
            uint32_t heatPos   = x + (height - 1U) * width;
            uint16_t heat      = m_heat[heatPos] + randValue;

            if (UINT8_MAX < heat)
            {
                m_heat[heatPos] = 255U;
            }
            else
            {
                m_heat[heatPos] = heat;
            }
        }
    }
}

void FirePlugin::draw(YAGfx& gfx)
{
    const uint16_t WIDTH  = gfx.getWidth();
    const uint16_t HEIGHT = gfx.getHeight();
    uint16_t       x;

    for (x = 0; x < WIDTH; ++x)
    {
        /* Step 4) Map from heat cells to LED colors */
        uint16_t dstOffset  = 0U;
        Color*   dstAddress = gfx.getFrameBufferYAddr(x, 0U, HEIGHT, dstOffset);

        if (nullptr != dstAddress)
        {
            uint16_t y = 0;

            while (HEIGHT > y)
            {
                dstAddress[y * dstOffset] = heatColor(m_heat[x + y * WIDTH]);
                ++y;
            }
        }
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
