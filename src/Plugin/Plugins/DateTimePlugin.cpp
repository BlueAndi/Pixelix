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
 * @brief  DateTime plugin
 * @author Yann Le Glaz <yann_le@web.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "DateTimePlugin.h"
#include "ClockDrv.h"

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

 /** Size of formatted date string in the form of DD:MM / MM:DD / HH:MM
 *
 *      "\\calign"          = 8  (Alignment center )
 *      "Day/Month/Hour"    = 2
 *      "separator"         = 1
 *      "Month/Day"/Minute  = 2
 *      "separator"         = 1  (only valid for date)
 *      "AM/PM"             = 2 (only at time strings)
 *      "\0"                = 1
 *      ------------------------
 *                          = 17
 */
#define SIZE_OF_FORMATED_DATE_TIME_STRING       (17U)

/** Divider to convert ms in s */
#define MS_TO_SEC_DIVIDER                       (1000U)

/** Toggle counter value to switch between date and time
* if DURATION_INFINITE was set for the plugin.
*/
#define MAX_COUNTER_VALUE_FOR_DURATION_INFINITE (15U)
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

void DateTimePlugin::setSlot(const ISlotPlugin* slotInterf)
{
    m_slotInterf = slotInterf;
    return;
}

void DateTimePlugin::active(IGfx& gfx)
{
    if (nullptr == m_textCanvas)
    {
        m_textCanvas = new Canvas(gfx.width(), gfx.height() - 2, 0, 0);

        if (nullptr != m_textCanvas)
        {
            m_textCanvas->addWidget(m_textWidget);
        }
    }

    if (nullptr == m_lampCanvas)
    {
        m_lampCanvas = new Canvas(gfx.width(), 1, 1, gfx.height() - 1);

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

                m_lampCanvas->addWidget(m_lampWidgets[index]);
                m_lampWidgets[index].move(x, 0);
            }
        }
    }

    m_isUpdateAvailable = true;

    m_durationCounter = 0U;

    m_checkUpdateTimer.start(DateTimePlugin::CHECK_UPDATE_PERIOD);

    /* Force immediate date/time update on activation */
    updateDateTime(true);
}

void DateTimePlugin::inactive()
{
    m_checkUpdateTimer.stop();

    return;
}

void DateTimePlugin::update(IGfx& gfx)
{
    if (false != m_isUpdateAvailable)
    {
        gfx.fillScreen(ColorDef::convert888To565(ColorDef::BLACK));

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

void DateTimePlugin::setText(const String& formatText)
{
    m_textWidget.setFormatStr(formatText);

    return;
}

void DateTimePlugin::setLamp(uint8_t lampId, bool state)
{
    if (MAX_LAMPS > lampId)
    {
        m_lampWidgets[lampId].setOnState(state);
    }

    return;
}

void DateTimePlugin::process()
{
    if ((true == m_checkUpdateTimer.isTimerRunning()) &&
        (true == m_checkUpdateTimer.isTimeout()))
    {
        updateDateTime(false);

        m_checkUpdateTimer.restart();
    }

    return;
}
/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void DateTimePlugin::updateDateTime(bool force)
{
    struct tm   timeinfo = { 0 };
    bool        showTime = ((0 == m_durationCounter) ? true : false);
    bool        showDate = false;
    uint32_t    duration = (nullptr == m_slotInterf) ? 0U : m_slotInterf->getDuration();

    /* If infinite duration was set switch every 15s between time and date. */
    if (0U == duration)
    {
        showDate = ((MAX_COUNTER_VALUE_FOR_DURATION_INFINITE == m_durationCounter) ? true : false);
    }
    else
    {
        showDate = ((duration / (2 * MS_TO_SEC_DIVIDER) == m_durationCounter) ? true : false);
    }

    m_durationCounter++;

    if (true == ClockDrv::getInstance().getTime(&timeinfo))
    {
        if ((false != showTime) || (true == force))
        {
            char timeBuffer [SIZE_OF_FORMATED_DATE_TIME_STRING];
            const char* formattedTimeString = ClockDrv::getInstance().getTimeFormat() ? "\\calign%H:%M":"\\calign%I:%M %p";

            setWeekdayIndicator(timeinfo);

            strftime(timeBuffer, sizeof(timeBuffer), formattedTimeString, &timeinfo);
            setText(timeBuffer);
            m_isUpdateAvailable = true;
        }

        if (false != showDate)
        {
            char dateBuffer [SIZE_OF_FORMATED_DATE_TIME_STRING];
            const char* formattedDateString = ClockDrv::getInstance().getDateFormat() ? "\\calign%d.%m.":"\\calign%m/%d";

            setWeekdayIndicator(timeinfo);

            strftime(dateBuffer, sizeof(dateBuffer), formattedDateString, &timeinfo);
            setText(dateBuffer);

            m_isUpdateAvailable = true;
        }

        /* If infinite duration was switch every 15s between time and date. */
        if (0U == duration)
        {
            if ((2 * MAX_COUNTER_VALUE_FOR_DURATION_INFINITE) == m_durationCounter)
            {
                m_durationCounter = 0U;
            }
        }
        else
        {
            if ((duration / MS_TO_SEC_DIVIDER) == m_durationCounter)
            {
                m_durationCounter = 0U;
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

void DateTimePlugin::setWeekdayIndicator(tm timeinfo)
{
    /* tm_wday starts at sunday, first lamp indicates monday.*/
    uint8_t activeLamp = (0U < timeinfo.tm_wday) ? (timeinfo.tm_wday - 1U) : (DateTimePlugin::MAX_LAMPS - 1U);

    /* Last active lamp has to be deactivated. */
    uint8_t lampToDeactivate = (0U < activeLamp) ? (activeLamp - 1U) : (DateTimePlugin::MAX_LAMPS - 1U);

    setLamp(activeLamp, true);
    setLamp(lampToDeactivate, false);
}
