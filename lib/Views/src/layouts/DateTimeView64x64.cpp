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

/** Center X-coordinate of analog clock. */
static const int16_t ANALOG_CENTER_X      = 32;

/** Center Y-coordinate of analog clock.  */
static const int16_t ANALOG_CENTER_Y      = 31;

/** Anaolog Clock radius. */
static const int16_t ANALOG_RADIUS        = 31;

/** Factor by which sinus/cosinus values are scaled to use integer math.  */
static const int16_t SINUS_VAL_SCALE      = 10000;

/** Angle difference between two minute marks on analog clock ring. */
static const int16_t MINUTES_ANGLE_DELTA  = 6;

/** Each hour mark spawns 5 minutes. */
static const int16_t MINUTE_HOUR_DELTA    = 5;

/** Number of minute marks on clock. */
static const int16_t MINUTE_MARKS_COUNT   = 12;

/** Angle difference between 2 hour marks on analog clock ring. */
static const int16_t HOURS_ANGLE_DELTA    = MINUTE_HOUR_DELTA * MINUTES_ANGLE_DELTA;

/** Pixel length of hour marks on analog clock ring. */
static const int16_t HOUR_MARK_LENGTH     = ANALOG_RADIUS - 4;

/** Pixel length of hour hand.*/
static const int16_t HOUR_HAND_LENGTH     = ANALOG_RADIUS - 13;

/** Pixel length of minute hand.*/
static const int16_t MINUTE_HAND_LENGTH   = ANALOG_RADIUS - 6;

/** Pixel length of second hand.*/
static const int16_t SECOND_HAND_LENGTH   = ANALOG_RADIUS - 2;

/** Clock hand distance from clock center. */
static const int16_t HAND_CENTER_DISTANCE = 3;

/******************************************************************************
 * Types and classes
 *****************************************************************************/

/******************************************************************************
 * Prototypes
 *****************************************************************************/

/**
 * @brief Get the Minute Sinus value
 *
 * @param[in] angle Minute angle, must be multiple of 6°  (360 °/ 60 minutes)
 * @return sinus value for angle (scaled by 10.000)
 */
static int16_t getMinuteSinus(uint16_t angle);

/**
 * @brief Get the Minute Cosinus value
 *
 * @param[in] angle Minute angle, must be multiple of 6° (360 °/ 60 minutes)
 * @return cosinus value for angle (scaled by 10.000)
 */
static int16_t getMinuteCosinus(uint16_t angle);

/**
 * @brief Get hour hand destination minute mark.
 *
 * The hour hand direction also depends on the elapsed minutes of
 * the actual hour to avoid long jumps at hour change.
 *
 * Example: At 12:30, it should point to the middle between 12 and 1.
 *
 * @param[in] current hour value
 * @param[in] current minute value
 *
 * @return minute value the hour hand should point to.
 */
static int16_t getHourHandDestination(int hour, int minute);

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
        uint8_t idx;

        gfx.fillScreen(ColorDef::BLACK);

        for (idx = 0; MAX_LAMPS > idx; ++idx)
        {
            m_lampWidgets[idx].update(gfx);
        }

        if ((ViewMode::DIGITAL_AND_ANALOG == m_mode) || (ViewMode::ANALOG_ONLY == m_mode))
        {
            uint32_t centerRingCol = m_analogClockCfg.m_colors[ANA_CLK_COL_HAND_MIN];

            /* Draw analog clock minute circle. */
            drawAnalogClockBackground(gfx);

            /* Draw analog clock hands. */
            drawAnalogClockHand(
                gfx,
                m_now.tm_min,
                MINUTE_HAND_LENGTH,
                m_analogClockCfg.m_colors[ANA_CLK_COL_HAND_MIN]);

            drawAnalogClockHand(gfx,
                getHourHandDestination(m_now.tm_hour, m_now.tm_min),
                HOUR_HAND_LENGTH,
                m_analogClockCfg.m_colors[ANA_CLK_COL_HAND_HOUR]);

            if (0U != (m_analogClockCfg.m_secondsMode & SECOND_DISP_HAND))
            {
                /* Use second hand color also for the middle ring if this hand is enabled. */
                centerRingCol = m_analogClockCfg.m_colors[ANA_CLK_COL_HAND_SEC];
                drawAnalogClockHand(
                    gfx,
                    m_now.tm_sec,
                    SECOND_HAND_LENGTH,
                    centerRingCol);
            }

            /* Draw analog clock hand center
             * As it is small, we aproximate the circle with a four pixel rectangle
             * and make the four corners black.
             */
            gfx.drawRectangle(ANALOG_CENTER_X - 2, ANALOG_CENTER_Y - 2, 5, 5, centerRingCol);
            gfx.drawPixel(ANALOG_CENTER_X - 2, ANALOG_CENTER_Y - 2, ColorDef::BLACK);
            gfx.drawPixel(ANALOG_CENTER_X - 2, ANALOG_CENTER_Y + 2, ColorDef::BLACK);
            gfx.drawPixel(ANALOG_CENTER_X + 2, ANALOG_CENTER_Y - 2, ColorDef::BLACK);
            gfx.drawPixel(ANALOG_CENTER_X + 2, ANALOG_CENTER_Y + 2, ColorDef::BLACK);
        }

        if ((ViewMode::DIGITAL_AND_ANALOG == m_mode) || (ViewMode::DIGITAL_ONLY == m_mode))
        {
            m_textWidget.update(gfx);
        }

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
    /* Draw minute ring starting at 270°, which draws towards the top.
     * X-direction is using cos, and cos(270) is 0 (straight up or down).
     * Y-direction is using sin, and sin(270) is -1 (going up).
     */
    const uint16_t secondAngle = 270U + m_now.tm_sec * MINUTES_ANGLE_DELTA;

    for (uint16_t angle = 270U; angle < (270U + 360U); angle += MINUTES_ANGLE_DELTA)
    {
        const int16_t dx = getMinuteCosinus(angle);
        const int16_t dy = getMinuteSinus(angle);

        const int16_t xs = ANALOG_CENTER_X + (dx * ANALOG_RADIUS) / SINUS_VAL_SCALE;
        const int16_t ys = ANALOG_CENTER_Y + (dy * ANALOG_RADIUS) / SINUS_VAL_SCALE;

        if (0U == (angle % HOURS_ANGLE_DELTA)) /* Draw stronger marks at hour angles (each 30°). */
        {
            const int16_t xe = ANALOG_CENTER_X + (dx * HOUR_MARK_LENGTH) / SINUS_VAL_SCALE;
            const int16_t ye = ANALOG_CENTER_Y + (dy * HOUR_MARK_LENGTH) / SINUS_VAL_SCALE;

            gfx.drawLine(xs, ys, xe, ye, m_analogClockCfg.m_colors[ANA_CLK_COL_RING_MIN5_MARK]);
        }

        Color tickMarkCol = m_analogClockCfg.m_colors[ANA_CLK_COL_RING_MIN_DOT];
        if ((0U != (SECOND_DISP_RING & m_analogClockCfg.m_secondsMode)) && (angle <= secondAngle))
        {
            /* Draw minute tick marks with passed seconds highlighting. */
            tickMarkCol = m_analogClockCfg.m_colors[ANA_CLK_COL_HAND_SEC];
        }
        gfx.drawPixel(xs, ys, tickMarkCol);
    }
}

void DateTimeView64x64::drawAnalogClockHand(YAGfx& gfx, int16_t minute, int16_t radius, const Color& col)
{
    /* Convert minute to angle starting at 270°, which draws towards the top of the clock. */
    minute           %= 60;
    minute            = 270 + minute * MINUTES_ANGLE_DELTA;

    const int16_t dx  = getMinuteCosinus(minute);
    const int16_t dy  = getMinuteSinus(minute);

    gfx.drawLine(
        ANALOG_CENTER_X + (HAND_CENTER_DISTANCE * dx) / SINUS_VAL_SCALE,
        ANALOG_CENTER_Y + (HAND_CENTER_DISTANCE * dy) / SINUS_VAL_SCALE,
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
    /*
     * Lookup table only stores 1st quadrant sinus values.
     * Others are calculated based on sinus curve symetries.
     */
    int16_t sinus  = 0;

    angle         %= 360U;

    if (90U >= angle) /* quadrant 1 */
    {
        sinus = MINUTE_SIN_TAB[angle / MINUTES_ANGLE_DELTA];
    }
    else if (180U >= angle) /* quadrant 2 is symetric to quadrant 1*/
    {
        sinus = MINUTE_SIN_TAB[(180U - angle) / MINUTES_ANGLE_DELTA];
    }
    else if (270U >= angle) /* quadrant 3 is point symetric to 2 */
    {
        sinus = (-1) * MINUTE_SIN_TAB[(angle - 180U) / MINUTES_ANGLE_DELTA];
    }
    else /* quadrant 4 is symetric to 3 */
    {
        sinus = (-1) * MINUTE_SIN_TAB[(360 - angle) / MINUTES_ANGLE_DELTA];
    }

    return sinus;
}

static int16_t getMinuteCosinus(uint16_t angle)
{
    return getMinuteSinus(angle + 90U);
}

static int16_t getHourHandDestination(int hour, int minute)
{
    return (hour * MINUTE_HOUR_DELTA) + (minute / MINUTE_MARKS_COUNT);
}
