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

#include "Logging.h"
#include "RestartMgr.h"

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

static LedGridSim theLedGridSim; /**< Simulation pixel interface  */

/******************************************************************************
 * Public Methods
 *****************************************************************************/

DisplayDrv::DisplayDrv() :
    IDisplayDrv(),
    m_simulationInterface(&theLedGridSim)
{
}

DisplayDrv::~DisplayDrv()
{
}

bool DisplayDrv::begin()
{
    this->clear();
    this->on();

    return true;
}

void DisplayDrv::off()
{
    m_simulationInterface->setPower(false);
}

void DisplayDrv::on()
{
    m_simulationInterface->setPower(true);
}

void DisplayDrv::setBrightness(uint8_t brightness)
{
    m_simulationInterface->setBrightness(brightness);
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
    static bool isFirstCall = true;

    if ((false == m_simulationInterface->isInitialized()) && (true == isFirstCall))
    {
        /* Initialize the simulation interface.
         * This is done during the first call of show(), because the SDL init must run in the same thread
         * as the later update() calls. Alternative would be to run the simulation interface in a separate thread,
         * but this would require some broader changes and doesn't seem to be necessary so far.
         */
        m_simulationInterface->initialize(CONFIG_LED_MATRIX_WIDTH, CONFIG_LED_MATRIX_HEIGHT);
        isFirstCall = false;
    }

    /* Check if the simulation interface is initialized again because the initialization might have failed.
     * This happens for example in pipelines or if running Pixelix from an ssh session. In this case we don't
     * die, but run without the UI. The webserver is still running and can be used to control the app.
     */
    if (true == m_simulationInterface->isInitialized())
    {
        if (true == m_simulationInterface->dispatchEvents())
        {
            m_simulationInterface->update(bitmap);
        }
        else
        {
            const uint32_t               RESTART_DELAY = 100U; /* ms */
            RestartMgr&                  restartMgr    = RestartMgr::getInstance();
            RestartMgr::RestartReqStatus status        = RestartMgr::RESTART_REQ_STATUS_ERR;


            if (RestartMgr::RESTART_REQ_STATUS_OK != restartMgr.reqRestart(RESTART_DELAY, false))
            {
                LOG_ERROR("Unable to perform restart.");
            }
        }
    }
}

void DisplayDrv::clear()
{
    /* No local framebuffer to clear, because the simulation interface is drawing directly to the window. */
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
