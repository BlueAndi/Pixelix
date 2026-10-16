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
 * @file   PixelixLogoWidget.h
 * @brief  Animated Pixelix logo widget
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup GFX
 *
 * @{
 */

#ifndef PIXELIX_LOGO_WIDGET_H
#define PIXELIX_LOGO_WIDGET_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <Widget.hpp>
#include <YAColor.h>
#include <YAGfxBrush.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * The animated Pixelix logo, drawn completely by graphic primitives.
 *
 * It shows the same logo as the plain text variant ".:PIXELIX:." does, but
 * animated. Because it is calculated instead of decoded from an image file,
 * it needs neither filesystem space nor the RAM of an image decoder.
 *
 * The animation runs through three phases and stops afterwards with the
 * static logo. Its total length is DURATION.
 *
 * 1. The letters slide in from the top, one after another.
 * 2. The dots left and right of the letters twinkle and stay white.
 * 3. A glint sweeps from left to right over the letters.
 *
 * The logo is designed for a 32x8 area with the TomThumb font. On a larger
 * display it is centered, on a smaller one it is clipped.
 */
class PixelixLogoWidget : public Widget
{
public:

    /** Widget type string */
    static const char* WIDGET_TYPE;

    /** Total animation length in ms, from restart() until the logo is static. */
    static const uint32_t DURATION = 2690U;

    /**
     * Constructs a Pixelix logo widget.
     *
     * The animation starts right away, see restart() to begin it again.
     *
     * @param[in] width     Widget width in pixel.
     * @param[in] height    Widget height in pixel.
     * @param[in] x         Upper left corner (x-coordinate) of the widget in a canvas.
     * @param[in] y         Upper left corner (y-coordinate) of the widget in a canvas.
     */
    PixelixLogoWidget(uint16_t width = 0U, uint16_t height = 0U, int16_t x = 0, int16_t y = 0);

    /**
     * Constructs a Pixelix logo widget, by copying another one.
     *
     * @param[in] widget    Pixelix logo widget, which to copy.
     */
    PixelixLogoWidget(const PixelixLogoWidget& widget);

    /**
     * Destroys the Pixelix logo widget.
     */
    ~PixelixLogoWidget();

    /**
     * Assigns the content of another Pixelix logo widget.
     *
     * @param[in] widget    Pixelix logo widget, which to assign.
     *
     * @return Pixelix logo widget
     */
    PixelixLogoWidget& operator=(const PixelixLogoWidget& widget);

    /**
     * Start the animation from the beginning.
     */
    void restart();

    /**
     * Is the animation over and the logo shown static?
     *
     * @return If the animation is over, it will return true otherwise false.
     */
    bool isFinished() const;

protected:

    /**
     * Paint the widget with the given graphics interface.
     *
     * @param[in] gfx   Graphics interface
     */
    void paint(YAGfx& gfx) override;

private:

    /** Logo width in pixel. The TomThumb advances of ".:PIXELIX:." add up to it. */
    static const uint16_t LOGO_WIDTH  = 32U;

    /** Logo height in pixel. */
    static const uint16_t LOGO_HEIGHT = 8U;

    /** The logo letters, without the dots left and right of them. */
    static const char* LOGO_TEXT;

    /** Number of letters in LOGO_TEXT. */
    static const uint8_t LETTER_CNT                = 7U;

    /** x-coordinate of the first letter, right behind the left dots. */
    static const int16_t LETTER_X                  = 4;

    /** Baseline y-coordinate of a letter which reached its final position. */
    static const int16_t LETTER_BASELINE_Y         = 7;

    /** Baseline y-coordinate of a letter which just started to slide in. */
    static const int16_t LETTER_SLIDE_IN_Y         = 1;

    /** Distance in pixel a sliding letter moves per frame. */
    static const int16_t LETTER_SLIDE_IN_STEP      = 2;

    /** Number of frames a letter needs to slide in, the last one lands it. */
    static const uint32_t LETTER_SLIDE_IN_STEP_CNT = 4U;

    /** Baseline y-coordinate of a letter while it bounces off its landing. */
    static const int16_t LETTER_BOUNCE_Y           = LETTER_BASELINE_Y - 1;

    /** Frame of a letter it bounces at, counted from the start of its slide in. */
    static const uint32_t LETTER_BOUNCE_STEP       = LETTER_SLIDE_IN_STEP_CNT;

    /** Number of frames between the start of two consecutive letters. */
    static const uint32_t LETTER_START_DELTA       = 2U;

    /** Frame the first letter starts to slide in at. */
    static const uint32_t LETTER_START_FIRST       = 1U;

    /** Number of dots left and right of the letters. */
    static const uint8_t DOT_CNT                   = 6U;

    /** Number of dot groups which twinkle one after another. */
    static const uint8_t DOT_GROUP_CNT             = 3U;

    /** Number of brightness levels a letter is lightened up to by the glint. */
    static const uint8_t GLINT_LEVEL_CNT           = 4U;

    /** Width of the glint band in pixel. */
    static const uint8_t GLINT_WIDTH               = 4U;

    /** Duration of one slide in frame in ms. */
    static const uint32_t SLIDE_IN_FRAME_DURATION  = 60U;

    /** Number of slide in frames. */
    static const uint32_t SLIDE_IN_FRAME_CNT       = 18U;

    /** Duration of one dot twinkle frame in ms. */
    static const uint32_t DOTS_FRAME_DURATION      = 80U;

    /** Number of dot twinkle frames. */
    static const uint32_t DOTS_FRAME_CNT           = 5U;

    /** Duration of one glint frame in ms. */
    static const uint32_t GLINT_FRAME_DURATION     = 30U;

    /** Number of glint frames. It covers the letters plus the glint band. */
    static const uint32_t GLINT_FRAME_CNT          = 27U;

    /** Pause in ms between two phases. */
    static const uint32_t PAUSE_DURATION           = 200U;

    /** Timestamp in ms the slide in phase ends at. It starts at 0. */
    static const uint32_t SLIDE_IN_END             = SLIDE_IN_FRAME_CNT * SLIDE_IN_FRAME_DURATION;

    /** Timestamp in ms the dot twinkle phase starts at. */
    static const uint32_t DOTS_BEGIN               = SLIDE_IN_END + PAUSE_DURATION;

    /** Timestamp in ms the dot twinkle phase ends at. */
    static const uint32_t DOTS_END                 = DOTS_BEGIN + DOTS_FRAME_CNT * DOTS_FRAME_DURATION;

    /** Timestamp in ms the glint phase starts at. */
    static const uint32_t GLINT_BEGIN              = DOTS_END + PAUSE_DURATION;

    /** Timestamp in ms the glint phase ends at. */
    static const uint32_t GLINT_END                = GLINT_BEGIN + GLINT_FRAME_CNT * GLINT_FRAME_DURATION;

    /* The caller plans with DURATION, therefore it must never drift away from the phases. */
    static_assert(DURATION == GLINT_END, "Animation duration does not match the phase timing.");

    /**
     * A single dot left or right of the letters.
     */
    struct Dot
    {
        int16_t x;     /**< x-coordinate inside the logo. */
        int16_t y;     /**< y-coordinate inside the logo. */
        uint8_t group; /**< Twinkle group [0; DOT_GROUP_CNT - 1] the dot belongs to. */
    };

    /** The dots left and right of the letters. */
    static const Dot DOTS[DOT_CNT];

    /** Base color of every letter, in the order of LOGO_TEXT. */
    static const uint32_t LETTER_COLORS[LETTER_CNT];

    /** Brightness of a twinkling dot, one entry per step of its group. */
    static const uint8_t DOT_BRIGHTNESS[DOT_GROUP_CNT];

    /** Glint level per pixel of the glint band, from its trailing to its leading edge. */
    static const uint8_t GLINT_LEVEL[GLINT_WIDTH];

    /**
     * The glint lightens a letter by raising its dark color channels.
     * Index is the glint level, value the channel value to raise them to.
     */
    static const uint8_t GLINT_CHANNEL[GLINT_LEVEL_CNT];

    /**
     * Brush which lightens a base color depending on the glint position.
     *
     * The letters are drawn with the font, which asks the brush for the color
     * of every single pixel. This is the only way to let the glint sweep over
     * a letter instead of over the logo as a whole.
     */
    class GlintBrush : public YAGfxBrush
    {
    public:

        /**
         * Constructs a glint brush.
         *
         * @param[in] color Base color of the letter.
         * @param[in] step  Current glint frame. A negative value means no glint at all.
         */
        GlintBrush(const Color& color, int32_t step) :
            YAGfxBrush(),
            m_color(color),
            m_step(step),
            m_intensity(Color::MAX_BRIGHT)
        {
        }

        /**
         * Destroys the glint brush.
         */
        ~GlintBrush()
        {
        }

        /**
         * Get color at given position.
         *
         * @param[in] x x-coordinate inside the logo.
         * @param[in] y y-coordinate inside the logo.
         *
         * @return Color at given position.
         */
        Color getColor(int16_t x, int16_t y) const override;

        /**
         * Get brush intensity.
         *
         * @return Brush intensity [0; 255] - 0: min. bright / 255: max. bright.
         */
        uint8_t getIntensity() const override
        {
            return m_intensity;
        }

        /**
         * Set brush intensity.
         *
         * @param[in] intensity Brush intensity [0; 255] - 0: min. bright / 255: max. bright.
         */
        void setIntensity(uint8_t intensity) override
        {
            m_intensity = intensity;
        }

    private:

        Color   m_color;     /**< Base color of the letter. */
        int32_t m_step;      /**< Current glint frame. A negative value means no glint at all. */
        uint8_t m_intensity; /**< Brush intensity [0; 255]. */

        GlintBrush();
    };

    uint32_t m_startTimestamp; /**< Timestamp in ms the animation started at. */

    /**
     * Get the baseline y-coordinate of a letter while it slides in.
     *
     * @param[in]   letterIdx   Index of the letter [0; LETTER_CNT - 1].
     * @param[in]   frame       Current slide in frame.
     * @param[out]  baselineY   Baseline y-coordinate inside the logo.
     *
     * @return If the letter is visible already, it will return true otherwise false.
     */
    static bool getSlideInBaselineY(uint8_t letterIdx, uint32_t frame, int16_t& baselineY);

    /**
     * Draw the letters.
     *
     * @param[in] gfx       Graphics interface of the logo area.
     * @param[in] elapsed   Elapsed time in ms since the animation started.
     */
    void paintLetters(YAGfx& gfx, uint32_t elapsed);

    /**
     * Draw the dots left and right of the letters.
     *
     * @param[in] gfx       Graphics interface of the logo area.
     * @param[in] elapsed   Elapsed time in ms since the animation started.
     */
    void paintDots(YAGfx& gfx, uint32_t elapsed);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* PIXELIX_LOGO_WIDGET_H */

/** @} */
