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
 * @brief  Time plugin
 * @author Yann Le Glaz <yann_le@web.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "TimePlugin.h"

#include "ClockDrv.h"

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

 /** Size of formatted timestring in the form of HH:MM
 *
 *  "\\calign" = 8  (Alignment center )
 *         "H" = 2
 *         ":" = 1
 *         "M" = 2
 *        "\0" = 1
 *       ---------
 *           = 14
 */
#define SIZE_OF_FORMATED_TIME_STRING_HHMM (14U)

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

void TimePlugin::active(IGfx& gfx)
{
    m_isUpdateAvailable = true;

    m_checkTimeUpdateTimer.start(CHECK_TIME_UPDATE_PERIOD);

    /* Force immediate time update to avoid displaying
     * an old time for one TIME_UPDATE_PERIOD.
     */
    updateTime(true);
}

void TimePlugin::inactive()
{
    m_checkTimeUpdateTimer.stop();

    return;
}

void TimePlugin::update(IGfx& gfx)
{
    if (false != m_isUpdateAvailable)
    {
        gfx.fillScreen(ColorDef::convert888To565(ColorDef::BLACK));
        m_textWidget.update(gfx);

        m_isUpdateAvailable = false;
    }

    return;
}

void TimePlugin::setText(const String& formatText)
{
    m_textWidget.setFormatStr(formatText);

    return;
}

void TimePlugin::process()
{
    if ((true == m_checkTimeUpdateTimer.isTimerRunning()) &&
        (true == m_checkTimeUpdateTimer.isTimeout()))
    {
        updateTime(false);

        m_checkTimeUpdateTimer.restart();
    }

    return;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void TimePlugin::updateTime(bool force)
{
    struct tm timeinfo = { 0 };

    if (true == ClockDrv::getInstance().getTime(&timeinfo))
    {
        if ((m_currentMinute != timeinfo.tm_min) ||
            (true == force))
        {
            char timeBuffer [SIZE_OF_FORMATED_TIME_STRING_HHMM];

            strftime(timeBuffer, sizeof(timeBuffer), "\\calign%H:%M", &timeinfo);
            setText(timeBuffer);

            m_currentMinute = timeinfo.tm_min;
            m_isUpdateAvailable = true;
        }
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
