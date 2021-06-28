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

/**
 * Size of formatted date string in the form of DD:MM / MM:DD / HH:MM
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

/**
 * Toggle counter value to switch between date and time
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

void DateTimePlugin::start(uint16_t width, uint16_t height)
{
    MutexGuard<Mutex> guard(m_mutex);

    if (nullptr == m_textCanvas)
    {
        m_textCanvas = new Canvas(width, height - 2, 0, 0);

        if (nullptr != m_textCanvas)
        {
            (void)m_textCanvas->addWidget(m_textWidget);
        }
    }

    if (nullptr == m_lampCanvas)
    {
        uint16_t        lampWidth       = 0U;
        uint16_t        lampDistance    = 0U;
        const uint16_t  minDistance     = 1U;   /* Min. distance between lamps. */
        const uint16_t  minBorder       = 1U;   /* Min. border left and right of all lamps. */
        
        if (true == calcLayout(width, MAX_LAMPS, minDistance, minBorder, lampWidth, lampDistance))
        {
            m_lampCanvas = new Canvas(width, 1U, 1, height - 1);

            if (nullptr != m_lampCanvas)
            {
                /* Calculate the border to have the days (lamps) shown aligned to center. */
                uint16_t    border  = (width - (MAX_LAMPS * (lampWidth + lampDistance))) / 2U;
                uint8_t     index   = 0U;

                for(index = 0U; index < MAX_LAMPS; ++index)
                {
                    int16_t x = (lampWidth + lampDistance) * index + border;

                    m_lampWidgets[index].setColorOn(ColorDef::LIGHTGRAY);
                    m_lampWidgets[index].setColorOff(ColorDef::ULTRADARKGRAY);
                    m_lampWidgets[index].setWidth(lampWidth);

                    (void)m_lampCanvas->addWidget(m_lampWidgets[index]);
                    m_lampWidgets[index].move(x, 0);
                }
            }
        }
    }

    return;
}

void DateTimePlugin::stop()
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

void DateTimePlugin::process()
{
    MutexGuard<Mutex> guard(m_mutex);

    if ((true == m_checkUpdateTimer.isTimerRunning()) &&
        (true == m_checkUpdateTimer.isTimeout()))
    {
        updateDateTime(false);

        m_checkUpdateTimer.restart();
    }

    return;
}

void DateTimePlugin::active(YAGfx& gfx)
{
    MutexGuard<Mutex> guard(m_mutex);

    /* Force immediate date/time update on activation */
    updateDateTime(true);

    /* Force drawing on display in the update() method for the very first time
     * after activation.
     */
    m_isUpdateAvailable = true;
    m_durationCounter = 0U;
    m_checkUpdateTimer.start(CHECK_UPDATE_PERIOD);
}

void DateTimePlugin::inactive()
{
    MutexGuard<Mutex> guard(m_mutex);

    m_checkUpdateTimer.stop();

    return;
}

void DateTimePlugin::update(YAGfx& gfx)
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

void DateTimePlugin::setText(const String& formatText)
{
    MutexGuard<Mutex> guard(m_mutex);

    m_textWidget.setFormatStr(formatText);

    return;
}

void DateTimePlugin::setLamp(uint8_t lampId, bool state)
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

void DateTimePlugin::updateDateTime(bool force)
{
    struct tm   timeinfo = { 0 };
    bool        showTime = ((0U == m_durationCounter) ? true : false);
    bool        showDate = false;
    uint32_t    duration = (nullptr == m_slotInterf) ? 0U : m_slotInterf->getDuration();

    /* If infinite duration was set switch every 15s between time and date. */
    if (0U == duration)
    {
        showDate = ((MAX_COUNTER_VALUE_FOR_DURATION_INFINITE == m_durationCounter) ? true : false);
    }
    else
    {
        showDate = ((duration / (2U * MS_TO_SEC_DIVIDER) == m_durationCounter) ? true : false);
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
            if ((2U * MAX_COUNTER_VALUE_FOR_DURATION_INFINITE) == m_durationCounter)
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

void DateTimePlugin::setWeekdayIndicator(tm timeinfo)
{
    /* tm_wday starts at sunday, first lamp indicates monday.*/
    uint8_t activeLamp = (0U < timeinfo.tm_wday) ? (timeinfo.tm_wday - 1U) : (DateTimePlugin::MAX_LAMPS - 1U);

    /* Last active lamp has to be deactivated. */
    uint8_t lampToDeactivate = (0U < activeLamp) ? (activeLamp - 1U) : (DateTimePlugin::MAX_LAMPS - 1U);

    setLamp(activeLamp, true);
    setLamp(lampToDeactivate, false);
}

bool DateTimePlugin::calcLayout(uint16_t width, uint16_t cnt, uint16_t minDistance, uint16_t minBorder, uint16_t& elementWidth, uint16_t& elementDistance)
{
    bool    status  = false;

    /* The min. border (left and right) must not be greater than the given width. */
    if (width > (2U * minBorder))
    {
        uint16_t    availableWidth  = width - (2U * minBorder); /* The available width is calculated considering the min. borders. */

        /* The available width must be greater than the number of elements, including the min. element distance. */
        if (availableWidth > (cnt + ((cnt - 1U) * minDistance)))
        {
            uint16_t    maxElementWidth                     = (availableWidth - ((cnt - 1U) * minDistance)) / cnt; /* Max. element width, considering the given limitation. */
            uint16_t    elementWidthToAvailWidthRatio       = 8U;   /* 1 / N */
            uint16_t    elementDistanceToElementWidthRatio  = 4U;   /* 1 / N */
            uint16_t    elementWidthConsideringRatio        = availableWidth / elementWidthToAvailWidthRatio;

            /* Consider the ratio between element width to available width and
             * ratio between element distance to element width.
             * This is just to have a nice look.
             */
            if (maxElementWidth > elementWidthConsideringRatio)
            {
                uint16_t    elementDistanceConsideringRatio = elementWidthConsideringRatio / elementDistanceToElementWidthRatio;

                if (0U == elementDistanceConsideringRatio)
                {
                    if (0U == minDistance)
                    {
                        elementDistance = 0U;
                    }
                    else
                    {
                        elementWidth    = maxElementWidth;
                        elementDistance = (availableWidth - (cnt * maxElementWidth)) / (cnt - 1U);
                    }
                }
                else
                {
                    elementWidth    = elementWidthConsideringRatio - elementDistanceConsideringRatio;
                    elementDistance = elementDistanceConsideringRatio;
                }
            }
            else
            {
                elementWidth    = maxElementWidth;
                elementDistance = minDistance;
            }

            status = true;
        }
    }

    return status;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
