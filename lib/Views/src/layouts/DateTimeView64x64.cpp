/* MIT License
 *
 * Copyright (c) 2019 - 2024 Andreas Merkle <web@blue-andi.de>
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
 * @brief  View for 64x64 LED matrix with date and time.
 *
 * @author Andreas Merkle <web@blue-andi.de>
 * @author Norbert Schulz <github@schulznorbert.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "DateTimeView64x64.h"

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/** Factor by which sinus/cosinus values are scaled to use integer math.  */
static const int16_t SINUS_VAL_SCALE = 10000;

/******************************************************************************
 * Types and classes
 *****************************************************************************/

/******************************************************************************
 * Prototypes
 *****************************************************************************/

/**
* @brief Get the Minute Siunus value
*
* @param angle Minute angle, must be multiple of 6°  (360 °/ 60 minutes)
* @return sinus value for angle (scaled by 10.000)
*/
static int16_t getMinuteSinus(uint16_t angle);

/**
 * @brief Get the Minute Cosinus value
 *
 * @param angle Minute angle, must be multiple of 6° (360 °/ 60 minutes)
 * @return cosinus value for angle (scaled by 10.000)
 */
static int16_t getMinuteCosinus(uint16_t angle);

/******************************************************************************
 * Local Variables
 *****************************************************************************/

/**
 * Sinus lookup table for analog clock drawing
 *
 * Holds sinus values for the minutes 0 .. 15 angles in quadarant 0.
 * Other quadrants and cosinus values get derived from these values
 * to avoid recalculations.
 *
 * Sinus value are stored as integer scaled by 10.000.
 */
static const int16_t MINUTE_SIN_TAB[16U] = {
    0,    /* sin(0°)   */
    1045, /* sin(6°)   */
    2079, /* sin(12°)  */
    3090, /* sin(18°)  */
    4067, /* sin(24°)  */
    4999, /* sin(30°)  */
    5877, /* sin(36°)  */
    6691, /* sin(42°)  */
    7431, /* sin(48°)  */
    8090, /* sin(54°)  */
    8660, /* sin(60°)  */
    9135, /* sin(66°)  */
    9510, /* sin(72°)  */
    9781, /* sin(78°)  */
    9945, /* sin(84°)  */
    10000 /* sin(90°)  */
};

/******************************************************************************
 * Public Methods
 *****************************************************************************/

/**
 * Update the underlying canvas.
 *
 * @param[in] gfx   Graphic functionality to draw on the underlying canvas.
 */
void DateTimeView64x64::update(YAGfx& gfx)
{

    if (m_lastUpdateSecondVal != m_now.tm_sec)
    {
        uint8_t idx = 0U;

        gfx.fillScreen(ColorDef::BLACK);

        m_textWidget.update(gfx);

        for (uint8_t idx = 0; MAX_LAMPS > idx; ++idx)
        {
            m_lampWidgets[idx].update(gfx);
        }

        /* Draw analog clock minute circle. */
        drawAnalogClockBackground(gfx);

        /* Draw analog clock hands. */
        drawAnalogClockHand(gfx, m_now.tm_min, ANALOG_RADIUS - 6, ColorDef::GRAY);
        drawAnalogClockHand(gfx, m_now.tm_hour * 5 + m_now.tm_min / 12 , ANALOG_RADIUS - 13, ColorDef::WHITE);

        if (0U != (m_secondsDisplayMode & SECOND_DISP_HAND))
        {
            drawAnalogClockHand(gfx, m_now.tm_sec, ANALOG_RADIUS - 1, ColorDef::YELLOW);
        }

        /* Draw analog clock hand center */
        gfx.drawRectangle(ANALOG_CENTER_X - 2, ANALOG_CENTER_Y - 2, 5, 5, ColorDef::YELLOW);
        gfx.drawPixel(ANALOG_CENTER_X, ANALOG_CENTER_Y, ColorDef::BLACK);
        gfx.drawPixel(ANALOG_CENTER_X-2, ANALOG_CENTER_Y-2, ColorDef::BLACK);
        gfx.drawPixel(ANALOG_CENTER_X-2, ANALOG_CENTER_Y+2, ColorDef::BLACK);
        gfx.drawPixel(ANALOG_CENTER_X+2, ANALOG_CENTER_Y-2, ColorDef::BLACK);
        gfx.drawPixel(ANALOG_CENTER_X+2, ANALOG_CENTER_Y+2, ColorDef::BLACK);

        m_textWidget.update(gfx);

        m_lastUpdateSecondVal = m_now.tm_sec;
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void DateTimeView64x64::drawAnalogClockBackground(YAGfx& gfx)
{
    /* draw minute ring */
    uint16_t secondAngle(270U + m_now.tm_sec * 6U);

    for (uint16_t angle = 270U; angle < (270U + 360U); angle += 6U)
    {
        int16_t dx(getMinuteCosinus(angle));
        int16_t dy(getMinuteSinus(angle));

        int16_t xs(ANALOG_CENTER_X + (dx * ANALOG_RADIUS) / SINUS_VAL_SCALE);
        int16_t ys(ANALOG_CENTER_Y + (dy * ANALOG_RADIUS) / SINUS_VAL_SCALE);

        if (0U == (angle % 30U))  /* Draw stronger marks at hour angles. */
        {
            int16_t xe(ANALOG_CENTER_X + (dx * (ANALOG_RADIUS - 4)) / SINUS_VAL_SCALE);
            int16_t ye(ANALOG_CENTER_Y + (dy * (ANALOG_RADIUS - 4)) / SINUS_VAL_SCALE);

            gfx.drawLine(xs, ys, xe, ye, ColorDef::BLUE);
        }

        Color tickMarkCol(ColorDef::DARKGRAY);
        if ((0U != (SECOND_DISP_RING & m_secondsDisplayMode)) && (angle <= secondAngle))
        {
           /* Draw minute tick marks with passed seconds highlighting. */
           tickMarkCol = ColorDef::YELLOW;
        }
        gfx.drawPixel(xs, ys, tickMarkCol);
    }
}

void DateTimeView64x64::drawAnalogClockHand(YAGfx& gfx, int16_t minute, int16_t radius, const Color& col)
{
    /* Convert minute to angle starting at 270°, which draws towards the top of the clock. */
    minute %= 60;
    minute = 270 + minute * 6;

    int16_t dx(getMinuteCosinus(minute));
    int16_t dy(getMinuteSinus(minute));

     gfx.drawLine(
        ANALOG_CENTER_X + (3 * dx) / SINUS_VAL_SCALE,
        ANALOG_CENTER_Y + (3 * dy) / SINUS_VAL_SCALE,
        ANALOG_CENTER_X + (radius * dx) / SINUS_VAL_SCALE,
        ANALOG_CENTER_Y + (radius * dy) / SINUS_VAL_SCALE,
        col);
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/

static int16_t getMinuteSinus(uint16_t angle)
{
    angle %= 360U;

    /*
     * Lookup table only stores 1st quadrant sinus values.
     * Others are calculated based on sinus curve symetries.
     */
    int16_t sinus(0);
    if (90U >= angle)  /* quadrant 1 */
    {
        sinus = MINUTE_SIN_TAB[angle / 6U];
    }
    else if (180U >= angle) /* quadrant 2 is symetric to quadrant 1*/
    {
        sinus = MINUTE_SIN_TAB[(180U - angle) / 6U];
    }
    else if ( 270U >= angle)  /* quadrant 3 is point symetric to 2 */
    {
        sinus = (-1) * MINUTE_SIN_TAB[(angle - 180U) / 6U];
    }
    else /* quadrant 4 is symetric to 3 */
    {
        sinus = (-1) * MINUTE_SIN_TAB[(360 - angle) / 6U];
    }

    return sinus;
}

int16_t getMinuteCosinus(uint16_t angle)
{
    return getMinuteSinus(angle + 90U);
}
