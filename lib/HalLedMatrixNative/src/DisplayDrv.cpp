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
 * @file   DisplayDrv.cpp
 * @brief  Display driver for the native environment
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "DisplayDrv.h"

#include "SDLInterface.h"
#include "LedGridSim.h"

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
static LedGridSim theLedGridSim; /**< Simulation pixel interface  */

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/******************************************************************************
 * Public Methods
 *****************************************************************************/

DisplayDrv::DisplayDrv() :
    IDisplayDrv(),
    m_framebuffer(),
    m_brightness(UINT8_MAX),
    m_simulationInterface(&theLedGridSim)
{
}

DisplayDrv::~DisplayDrv()
{
    delete m_simulationInterface;
}

bool DisplayDrv::begin()
{
    this->clear();
    this->on();

    return true;
}

/**
 * Power the display off.
 */
void DisplayDrv::off()
{
    m_simulationInterface->setPower(false);
}

/**
 * Power the display on.
 */
void DisplayDrv::DisplayDrv::on()
{
    m_simulationInterface->setPower(true);
}

/**
 * Is the display powered on?
 *
 * @return If the display is powered on, it will return true otherwise false.
 */
bool DisplayDrv::isOn() const
{
    return m_simulationInterface->getPower();
}

void DisplayDrv::show(const YAGfxBitmap& bitmap)
{
    uint16_t x      = 0U;
    uint16_t y      = 0U;
    uint16_t width  = bitmap.getWidth();
    uint16_t height = bitmap.getHeight();

    if (CONFIG_LED_MATRIX_WIDTH < width)
    {
        width = CONFIG_LED_MATRIX_WIDTH;
    }

    if (CONFIG_LED_MATRIX_HEIGHT < height)
    {
        height = CONFIG_LED_MATRIX_HEIGHT;
    }

    uint32_t buffer[CONFIG_LED_MATRIX_HEIGHT * CONFIG_LED_MATRIX_WIDTH];

    int      I = 0;
    for (y = 0U; y < height; ++y)
    {
        for (x = 0U; x < width; ++x)
        {
            buffer[x + (y * CONFIG_LED_MATRIX_WIDTH)] = bitmap.getColor(x, y);
        }
    }

    if (!m_simulationInterface->isInitialized())
    {
        /* Initialize the simulation interface. */
        m_simulationInterface->initialize(CONFIG_LED_MATRIX_WIDTH, CONFIG_LED_MATRIX_HEIGHT);
    }

    if (m_simulationInterface->dispatchEvents())
    {
        m_simulationInterface->update(buffer);
    }
    else
    {
        exit(0); /* Window Close event received, exit the application. */
    }
}

void DisplayDrv::clear()
{
    size_t index = 0U;

    for (index = 0U; index < PIXEL_COUNT; ++index)
    {
        m_framebuffer[index] = ColorDef::BLACK;
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
