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
 * @brief  Date plugin
 * @author Yann Le Glaz <yann_le@web.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "DatePlugin.h"
#include "ClockDrv.h"

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/**
 * Size of formatted date string in the form of DD.MM. / MM/DD
 *
 *      "\\calign"      = 8  (Alignment center )
 *      "Day/Month"     = 2
 *      "separator"     = 1
 *      "Month/Day"     = 2
 *      "separator"     = 1
 *      "\0"            = 1
 *  ------------------------
 *                      = 15
 */
#define SIZE_OF_FORMATED_DATE_STRING (15U)

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

void DatePlugin::start(uint16_t width, uint16_t height)
{
    MutexGuard<Mutex> guard(m_mutex);

    if (nullptr == m_textCanvas)
    {
        m_textCanvas = new Canvas(width, height - 2U, 0, 0);

        if (nullptr != m_textCanvas)
        {
            (void)m_textCanvas->addWidget(m_textWidget);
        }
    }

    if (nullptr == m_lampCanvas)
    {
        m_lampCanvas = new Canvas(width, 1U, 1, height - 1);

        if (nullptr != m_lampCanvas)
        {
            uint8_t index = 0U;

            for(index = 0U; index < MAX_LAMPS; ++index)
            {
                /* One space at the begin, two spaces between the lamps. */
                int16_t x = (CUSTOM_LAMP_WIDTH + 1) * index + 1;

                m_lampWidgets[index].setColorOn(ColorDef::LIGHTGRAY);
                m_lampWidgets[index].setColorOff(ColorDef::ULTRADARKGRAY);
                m_lampWidgets[index].setWidth(CUSTOM_LAMP_WIDTH);

                (void)m_lampCanvas->addWidget(m_lampWidgets[index]);
                m_lampWidgets[index].move(x, 0);
            }
        }
    }

    return;
}

void DatePlugin::stop()
{
    MutexGuard<Mutex> guard(m_mutex);

    if (nullptr != m_textCanvas)
    {
        delete m_textCanvas;
        m_textCanvas = nullptr;
    }

    if (nullptr != m_lampCanvas)
    {
        delete m_lampCanvas;
        m_lampCanvas = nullptr;
    }

    return;
}

void DatePlugin::process()
{
    MutexGuard<Mutex> guard(m_mutex);

    if ((true == m_checkDateUpdateTimer.isTimerRunning()) &&
        (true == m_checkDateUpdateTimer.isTimeout()))
    {
        updateDate(false);

        m_checkDateUpdateTimer.restart();
    }

    return;
}

void DatePlugin::active(YAGfx& gfx)
{
    MutexGuard<Mutex> guard(m_mutex);

    UTIL_NOT_USED(gfx);

    /* Force immediate date update on activation */
    updateDate(true);

    /* Force drawing on display in the update() method for the very first time
     * after activation.
     */
    m_isUpdateAvailable = true;
    m_checkDateUpdateTimer.start(CHECK_DATE_UPDATE_PERIOD);
}

void DatePlugin::inactive()
{
    MutexGuard<Mutex> guard(m_mutex);

    m_checkDateUpdateTimer.stop();

    return;
}

void DatePlugin::update(YAGfx& gfx)
{
    MutexGuard<Mutex> guard(m_mutex);

    if (false != m_isUpdateAvailable)
    {
        gfx.fillScreen(ColorDef::BLACK);

        if (nullptr != m_textCanvas)
        {
            m_textCanvas->update(gfx);
        }

        if (nullptr != m_lampCanvas)
        {
            m_lampCanvas->update(gfx);
        }

        m_isUpdateAvailable = false;
    }

    return;
}

void DatePlugin::setText(const String& formatText)
{
    MutexGuard<Mutex> guard(m_mutex);

    m_textWidget.setFormatStr(formatText);

    return;
}

void DatePlugin::setLamp(uint8_t lampId, bool state)
{
    if (MAX_LAMPS > lampId)
    {
        MutexGuard<Mutex> guard(m_mutex);

        m_lampWidgets[lampId].setOnState(state);
    }

    return;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void DatePlugin::updateDate(bool force)
{
    struct tm   timeinfo = {0};

    if (true == ClockDrv::getInstance().getTime(&timeinfo))
    {
        if ((m_currentDay != timeinfo.tm_mday) ||
            (true == force))
        {
            char dateBuffer [SIZE_OF_FORMATED_DATE_STRING];

            /* tm_wday starts at sunday, first lamp indicates monday.*/
            uint8_t activeLamp = (0U < timeinfo.tm_wday) ? (timeinfo.tm_wday - 1U) : (MAX_LAMPS - 1U);

            /* Last active lamp has to be deactivated. */
            uint8_t lampToDeactivate = (0U < activeLamp) ? (activeLamp - 1U) : (MAX_LAMPS - 1U);

            /* Consider date format. */
            const char* formattedDateString = ClockDrv::getInstance().getDateFormat() ? "\\calign%d.%m.":"\\calign%m/%d";

            setLamp(activeLamp, true);
            setLamp(lampToDeactivate, false);

            strftime(dateBuffer, sizeof(dateBuffer), formattedDateString, &timeinfo);
            setText(dateBuffer);

            m_currentDay = timeinfo.tm_mday;
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

