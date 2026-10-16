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
 * @file   PixelixLogoWidget.cpp
 * @brief  Animated Pixelix logo widget
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "PixelixLogoWidget.h"

#include <Arduino.h>
#include <ColorDef.hpp>
#include <Util.h>
#include <YAFont.h>
#include <YAGfxCanvas.h>

/* The font header needs the glyph types of YAFont.h, therefore it comes last. */
#include <TomThumb.h>

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

/* Initialize Pixelix logo widget type. */
const char* PixelixLogoWidget::WIDGET_TYPE                                     = "pixelixLogo";

/* The letters of the logo. The dots around them are drawn as single pixels. */
const char* PixelixLogoWidget::LOGO_TEXT                                       = "PIXELIX";

/* Base color of every letter, the same ones the plain text logo uses. */
const uint32_t PixelixLogoWidget::LETTER_COLORS[PixelixLogoWidget::LETTER_CNT] = {
    0xFF0000U, /* P */
    0xFFFF00U, /* I */
    0x00FF00U, /* X */
    0x00FFFFU, /* E */
    0x0000FFU, /* L */
    0xFF00FFU, /* I */
    0xFF0000U  /* X */
};

/* The dots which frame the letters. Every group twinkles with its own phase. */
const PixelixLogoWidget::Dot PixelixLogoWidget::DOTS[PixelixLogoWidget::DOT_CNT] = {
    { 0, 6, 0U },  /* Left outer dot */
    { 30, 6, 0U }, /* Right outer dot */
    { 2, 4, 1U },  /* Left colon, upper dot */
    { 2, 6, 1U },  /* Left colon, lower dot */
    { 28, 4, 2U }, /* Right colon, upper dot */
    { 28, 6, 2U }  /* Right colon, lower dot */
};

/* Brightness of a twinkling dot, from dark to bright. */
const uint8_t PixelixLogoWidget::DOT_BRIGHTNESS[PixelixLogoWidget::DOT_GROUP_CNT]  = { 0x33U, 0x80U, 0xFFU };

/* The glint band is brightest in its middle, which gives it a soft edge. */
const uint8_t PixelixLogoWidget::GLINT_LEVEL[PixelixLogoWidget::GLINT_WIDTH]       = { 1U, 3U, 3U, 1U };

/* Value a dark color channel is raised to, per glint level. */
const uint8_t PixelixLogoWidget::GLINT_CHANNEL[PixelixLogoWidget::GLINT_LEVEL_CNT] = { 0x00U, 0x55U, 0xAAU, 0xFFU };

/******************************************************************************
 * Public Methods
 *****************************************************************************/

PixelixLogoWidget::PixelixLogoWidget(uint16_t width, uint16_t height, int16_t x, int16_t y) :
    Widget(WIDGET_TYPE, width, height, x, y),
    m_startTimestamp(millis())
{
}

PixelixLogoWidget::PixelixLogoWidget(const PixelixLogoWidget& widget) :
    Widget(widget),
    m_startTimestamp(widget.m_startTimestamp)
{
}

PixelixLogoWidget::~PixelixLogoWidget()
{
}

PixelixLogoWidget& PixelixLogoWidget::operator=(const PixelixLogoWidget& widget)
{
    if (&widget != this)
    {
        Widget::operator=(widget);

        m_startTimestamp = widget.m_startTimestamp;
    }

    return *this;
}

void PixelixLogoWidget::restart()
{
    m_startTimestamp = millis();
}

bool PixelixLogoWidget::isFinished() const
{
    uint32_t elapsed = millis() - m_startTimestamp;

    return (DURATION <= elapsed);
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

void PixelixLogoWidget::paint(YAGfx& gfx)
{
    uint32_t elapsed = millis() - m_startTimestamp;
    uint16_t width   = gfx.getWidth();
    uint16_t height  = gfx.getHeight();
    int16_t  originX = 0;
    int16_t  originY = 0;

    /* Center the logo, which keeps it in shape on a display larger than it. */
    if (LOGO_WIDTH < width)
    {
        originX = static_cast<int16_t>((width - LOGO_WIDTH) / 2U);
    }

    if (LOGO_HEIGHT < height)
    {
        originY = static_cast<int16_t>((height - LOGO_HEIGHT) / 2U);
    }

    gfx.fillScreen(ColorDef::BLACK);

    /* The canvas clips everything outside the logo area. The letters rely on
     * it, because they slide in from above the logo.
     */
    YAGfxCanvas logoCanvas(&gfx, originX, originY, LOGO_WIDTH, LOGO_HEIGHT);

    paintLetters(logoCanvas, elapsed);
    paintDots(logoCanvas, elapsed);
}

/******************************************************************************
 * Private Methods
 *****************************************************************************/

bool PixelixLogoWidget::getSlideInBaselineY(uint8_t letterIdx, uint32_t frame, int16_t& baselineY)
{
    uint32_t startFrame = LETTER_START_FIRST + LETTER_START_DELTA * static_cast<uint32_t>(letterIdx);
    bool     isVisible  = false;

    if (startFrame > frame)
    {
        /* Guard path: the letter did not start to slide in yet. */
    }
    else
    {
        uint32_t step = frame - startFrame;

        if (LETTER_SLIDE_IN_STEP_CNT > step)
        {
            int32_t posY = static_cast<int32_t>(LETTER_SLIDE_IN_Y) + static_cast<int32_t>(LETTER_SLIDE_IN_STEP) * static_cast<int32_t>(step);

            baselineY    = static_cast<int16_t>(posY);
        }
        else if (LETTER_BOUNCE_STEP == step)
        {
            /* The letter bounces one pixel up right after it landed, which
             * makes the landing look like an impact.
             */
            baselineY = LETTER_BOUNCE_Y;
        }
        else
        {
            baselineY = LETTER_BASELINE_Y;
        }

        isVisible = true;
    }

    return isVisible;
}

void PixelixLogoWidget::paintLetters(YAGfx& gfx, uint32_t elapsed)
{
    YAFont   font(&TomThumb);
    int16_t  cursorX      = LETTER_X;
    uint8_t  letterIdx    = 0U;
    bool     isSlidingIn  = (SLIDE_IN_END > elapsed);
    uint32_t slideInFrame = 0U;
    int32_t  glintStep    = -1;

    if (true == isSlidingIn)
    {
        slideInFrame = elapsed / SLIDE_IN_FRAME_DURATION;
    }

    if ((GLINT_BEGIN <= elapsed) &&
        (GLINT_END > elapsed))
    {
        glintStep = static_cast<int32_t>((elapsed - GLINT_BEGIN) / GLINT_FRAME_DURATION);
    }

    for (letterIdx = 0U; letterIdx < LETTER_CNT; ++letterIdx)
    {
        char     letter     = LOGO_TEXT[letterIdx];
        int16_t  baselineY  = LETTER_BASELINE_Y;
        bool     isVisible  = true;
        uint16_t charWidth  = 0U;
        uint16_t charHeight = 0U;

        if (true == isSlidingIn)
        {
            isVisible = getSlideInBaselineY(letterIdx, slideInFrame, baselineY);
        }

        if (true == isVisible)
        {
            GlintBrush brush(Color(LETTER_COLORS[letterIdx]), glintStep);
            int16_t    charX = cursorX;

            font.drawChar(gfx, charX, baselineY, letter, brush);
        }

        /* The cursor must advance even for a letter which is not visible yet,
         * otherwise the letters would not keep their place.
         */
        if (false == font.getCharBoundingBox(letter, charWidth, charHeight))
        {
            /* Guard path: the font does not know the letter, keep the cursor. */
        }
        else
        {
            cursorX += static_cast<int16_t>(charWidth);
        }
    }
}

void PixelixLogoWidget::paintDots(YAGfx& gfx, uint32_t elapsed)
{
    uint8_t dotIdx = 0U;

    if (DOTS_BEGIN > elapsed)
    {
        /* Guard path: the dots appear together with their twinkle phase. */
    }
    else
    {
        bool     isTwinkling = (DOTS_END > elapsed);
        uint32_t step        = 0U;

        if (true == isTwinkling)
        {
            step = (elapsed - DOTS_BEGIN) / DOTS_FRAME_DURATION;
        }

        for (dotIdx = 0U; dotIdx < DOT_CNT; ++dotIdx)
        {
            const Dot& dot        = DOTS[dotIdx];
            uint8_t    brightness = UINT8_MAX;

            if (true == isTwinkling)
            {
                /* Every group runs through the brightness ramp backwards and
                 * starts at its own position, which lets the twinkle rotate.
                 */
                uint32_t rampIdx = (1U + 2U * (static_cast<uint32_t>(dot.group) + step)) % static_cast<uint32_t>(DOT_GROUP_CNT);

                brightness       = DOT_BRIGHTNESS[rampIdx];
            }

            gfx.drawPixel(dot.x, dot.y, Color(brightness, brightness, brightness));
        }
    }
}

Color PixelixLogoWidget::GlintBrush::getColor(int16_t x, int16_t y) const
{
    Color   color    = m_color;
    int32_t distance = m_step - (static_cast<int32_t>(x) - static_cast<int32_t>(LETTER_X));
    uint8_t level    = 0U;

    UTIL_NOT_USED(y);

    if ((0 <= m_step) &&
        (0 <= distance) &&
        (static_cast<int32_t>(GLINT_WIDTH) > distance))
    {
        level = GLINT_LEVEL[static_cast<size_t>(distance)];
    }

    if (0U == level)
    {
        /* Guard path: the glint does not reach this pixel. */
    }
    else
    {
        /* A base color channel is either off or fully on. Raising the off ones
         * lightens the color towards white without changing its hue.
         */
        uint8_t channel = GLINT_CHANNEL[level];

        if (0U == color.getRed())
        {
            color.setRed(channel);
        }

        if (0U == color.getGreen())
        {
            color.setGreen(channel);
        }

        if (0U == color.getBlue())
        {
            color.setBlue(channel);
        }
    }

    color.dim(m_intensity);

    return color;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
