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
 * @file   ViewConfig.h
 * @brief  View configuration
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup UTILITIES
 *
 * @{
 */

#ifndef VIEW_CONFIG_H
#define VIEW_CONFIG_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <YAGfxBrush.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/** The view configuration contains the global settings for the views. */
class ViewConfig
{
public:

    /**
     * Get the view configuration instance.
     *
     * @return View configuration instance.
     */
    static ViewConfig& getInstance()
    {
        static ViewConfig instance; /* idiom */

        return instance;
    }

    /**
     * Use the solid brush.
     */
    void setSolidBrush()
    {
        m_brush = &m_solidBrush;
    }

    /**
     * Use the linear gradient brush.
     */
    void setLinearGradientBrush()
    {
        m_brush = &m_linearGradientBrush;
    }

    /**
     * Set solid brush.
     *
     * @param[in] color Color of the brush
     */
    void setSolidBrush(const Color& color)
    {
        m_solidBrush.setColor(color);
    }

    /**
     * Set linear gradient brush.
     *
     * @param[in] color1    Start color of the gradient.
     * @param[in] color2    End color of the gradient.
     * @param[in] offset    Offset in pixels of the gradient start color.
     * @param[in] length    Length of the gradient in pixels.
     * @param[in] vertical  Flag for vertical gradient.
     */
    void setLinearGradientBrush(const Color& color1, const Color& color2, uint32_t offset, uint32_t length, bool vertical)
    {
        m_linearGradientBrush.setStartColor(color1);
        m_linearGradientBrush.setEndColor(color2);
        m_linearGradientBrush.setOffset(offset);
        m_linearGradientBrush.setLength(length);
        m_linearGradientBrush.setDirection(vertical);
    }

    /**
     * Get the brush used for drawing text.
     *
     * @return Brush used for drawing text.
     */
    YAGfxBrush& getBrush()
    {
        return *m_brush;
    }

    /** Default text color */
    static const uint32_t DEFAULT_TEXT_COLOR                  = ColorDef::WHITE;

    /** Default text color gradient color 1 */
    static const uint32_t DEFAULT_TEXT_COLOR_GRADIENT_COLOR_1 = ColorDef::RED;

    /** Default text color gradient color 2 */
    static const uint32_t DEFAULT_TEXT_COLOR_GRADIENT_COLOR_2 = ColorDef::BLUE;

    /** Default text color gradient offset in pixels. */
    static const int16_t DEFAULT_TEXT_COLOR_GRADIENT_OFFSET   = 0;

    /** Default text color gradient length in pixels. */
    static const uint16_t DEFAULT_TEXT_COLOR_GRADIENT_LENGTH  = 32U;

    /** Default text color gradient direction (true = vertical, false = horizontal). */
    static const bool DEFAULT_TEXT_COLOR_GRADIENT_VERTICAL    = true;

private:

    YAGfxSolidBrush          m_solidBrush;          /**< Solid brush. */
    YAGfxLinearGradientBrush m_linearGradientBrush; /**< Linear gradient brush. */
    YAGfxBrush*              m_brush;               /**< Active brush. */

    /**
     * Construct a new view config object.
     */
    ViewConfig() :
        m_solidBrush(DEFAULT_TEXT_COLOR),
        m_linearGradientBrush(
            DEFAULT_TEXT_COLOR_GRADIENT_COLOR_1,
            DEFAULT_TEXT_COLOR_GRADIENT_COLOR_2,
            DEFAULT_TEXT_COLOR_GRADIENT_OFFSET,
            DEFAULT_TEXT_COLOR_GRADIENT_LENGTH,
            DEFAULT_TEXT_COLOR_GRADIENT_VERTICAL),
        m_brush(&m_solidBrush)
    {
    }

    /**
     * Destroy the view config object.
     */
    ~ViewConfig()
    {
    }
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* VIEW_CONFIG_H */

/** @} */