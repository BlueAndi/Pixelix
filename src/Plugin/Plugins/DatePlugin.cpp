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
 * @brief  Date plugin
 * @author Yann Le Glaz <yann_le@web.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "DatePlugin.h"

#include "ClockDrv.h"
#include <Logging.h>

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

 /** Size of formatted timestring in the form of DD:MM
 *
 *  "\\calign" = 8  (Alignment center )
 *         "Day/Month" = 2
 *         ":" = 1
 *         "Month/Day" = 2
 *        "\0" = 1
 *       ---------
 *           = 14
 */
#define SIZE_OF_FORMATED_DATE_STRING (14U)

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

void DatePlugin::active(IGfx& gfx)
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
        m_lampCanvas = new Canvas(gfx.width(), 1, 2, gfx.height()-1);
        if (nullptr != m_lampCanvas)
        {
            uint8_t index = 0U;

            for(index = 0U; index < MAX_LAMPS; ++index)
            {
                /* One space at the begin, two spaces between the lamps. */
                int16_t x = (3 + 1) * index + 1;

                m_lampWidgets[index].setColorOn(ColorDef::LIGHTGRAY);
                m_lampWidgets[index].setColorOff(ColorDef::DARKSLATEGRAY);
                m_lampWidgets[index].setWidth(3u);
                
                m_lampCanvas->addWidget(m_lampWidgets[index]);
                m_lampWidgets[index].move(x, 0);
            }
        }
    }

      /* Force immediate date update on activation */
    updateDate();
}

void DatePlugin::inactive()
{
    setLamp(0, false);
    setLamp(1, false);
    setLamp(2, false);
    setLamp(3, false);
    setLamp(4, false);
    setLamp(5, false);
    setLamp(6, false);

    return;
}

void DatePlugin::update(IGfx& gfx)
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

    return;
}

void DatePlugin::setText(const String& formatText)
{
    m_textWidget.setFormatStr(formatText);

    return;
}

void DatePlugin::setLamp(uint8_t lampId, bool state)
{
    if (MAX_LAMPS > lampId)
    {
        m_lampWidgets[lampId].setOnState(state);
    }

    return;
}

void DatePlugin::updateDate()
{
    struct tm   timeinfo = {0};

    if (true == ClockDrv::getInstance().getTime(&timeinfo))
    {
        char dateBuffer [SIZE_OF_FORMATED_DATE_STRING];
        /* tm_wday starts at sunday, first lamp indicates monday.*/
        uint8_t activeLamp = (timeinfo.tm_wday > 0) ? (timeinfo.tm_wday -1) : 0u;
        setLamp(activeLamp, true);
        strftime(dateBuffer, sizeof(dateBuffer), "\\calign%d.%m", &timeinfo);
        setText(dateBuffer);
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

