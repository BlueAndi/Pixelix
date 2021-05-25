/* MIT License
 *
 * Copyright (c) 2019 - 2021 Andreas Merkle <web@blue-andi.de>
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
 *      "\\calign"      = 8  (Alignment center )
 *      "H"             = 2
 *      "separator"     = 1
 *      "M"             = 2
 *      " "             = 1
 *      "AM/PM"         = 2
 *      "\0"            = 1
 *  ------------------------
 *                      = 17
 */
#define SIZE_OF_FORMATED_TIME_STRING_HHMM (17U)

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

void TimePlugin::process()
{
    lock();

    if ((true == m_checkTimeUpdateTimer.isTimerRunning()) &&
        (true == m_checkTimeUpdateTimer.isTimeout()))
    {
        updateTime(false);

        m_checkTimeUpdateTimer.restart();
    }

    unlock();

    return;
}

void TimePlugin::active(YAGfx& gfx)
{
    UTIL_NOT_USED(gfx);

    lock();

    /* Force immediate time update to avoid displaying
     * an old time for one TIME_UPDATE_PERIOD.
     */
    updateTime(true);

    /* Force drawing on display in the update() method for the very first time
     * after activation.
     */
    m_isUpdateAvailable = true;
    m_checkTimeUpdateTimer.start(CHECK_TIME_UPDATE_PERIOD);

    unlock();
}

void TimePlugin::inactive()
{
    lock();

    m_checkTimeUpdateTimer.stop();

    unlock();

    return;
}

void TimePlugin::update(YAGfx& gfx)
{
    lock();

    if (false != m_isUpdateAvailable)
    {
        gfx.fillScreen(ColorDef::BLACK);
        m_textWidget.update(gfx);

        m_isUpdateAvailable = false;
    }

    unlock();

    return;
}

void TimePlugin::setText(const String& formatText)
{
    lock();

    m_textWidget.setFormatStr(formatText);
    
    unlock();

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
            const char* formattedTimeString = ClockDrv::getInstance().getDateFormat() ? "\\calign%H:%M":"\\calign%I:%M %p";

            strftime(timeBuffer, sizeof(timeBuffer), formattedTimeString, &timeinfo);
            setText(timeBuffer);

            m_currentMinute = timeinfo.tm_min;
            m_isUpdateAvailable = true;
        }
    }
}

void TimePlugin::lock() const
{
    if (nullptr != m_xMutex)
    {
        (void)xSemaphoreTakeRecursive(m_xMutex, portMAX_DELAY);
    }

    return;
}

void TimePlugin::unlock() const
{
    if (nullptr != m_xMutex)
    {
        (void)xSemaphoreGiveRecursive(m_xMutex);
    }

    return;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
