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
 * @file   BaseGfxBrush.hpp
 * @brief  Base graphics brush
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup GFX
 *
 * @{
 */

#ifndef BASE_GFX_BRUSH_HPP
#define BASE_GFX_BRUSH_HPP

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Base graphics brush used for drawing text.
 *
 * @tparam TColor Color type, which is used by the brush.
 */
template < typename TColor >
class BaseGfxBrush
{
public:

    /**
     * Destroys a graphics brush.
     */
    virtual ~BaseGfxBrush()
    {
    }

    /**
     * Get color at given position, used for drawing text.
     *
     * @param[in] x x-coordinate
     * @param[in] y y-coordinate
     *
     * @return Color at given position
     */
    virtual TColor getColor(int16_t x, int16_t y) const = 0;

    /**
     * Get brush intensity.
     *
     * @return Brush intensity [0; 255] - 0: min. bright / 255: max. bright.
     */
    virtual uint8_t getIntensity() const                = 0;

    /**
     * Set brush intensity.
     *
     * @param[in] intensity Brush intensity [0; 255] - 0: min. bright / 255: max. bright.
     */
    virtual void setIntensity(uint8_t intensity)        = 0;

protected:

    /**
     * Constructs a base graphics brush.
     */
    BaseGfxBrush()
    {
    }
};

/**
 * A solid graphics brush, providing a single color.
 *
 * @tparam TColor Color type, which is used by the brush.
 */
template < typename TColor >
class BaseGfxSolidBrush : public BaseGfxBrush<TColor>
{
public:

    /**
     * Constructs a solid graphics brush with default color.
     */
    BaseGfxSolidBrush() :
        BaseGfxBrush<TColor>(),
        m_color()
    {
    }

    /**
     * Constructs a solid graphics brush.
     *
     * @param[in] color Color of the brush
     */
    BaseGfxSolidBrush(const TColor& color) :
        BaseGfxBrush<TColor>(),
        m_color(color)
    {
    }

    /**
     * Destroys a solid graphics brush.
     */
    virtual ~BaseGfxSolidBrush()
    {
    }

    /**
     * Get color at given position, used for drawing text.
     *
     * @param[in] x x-coordinate
     * @param[in] y y-coordinate
     *
     * @return Color at given position.
     */
    TColor getColor(int16_t x, int16_t y) const override
    {
        (void)x;
        (void)y;
        return m_color;
    }

    /**
     * Get brush intensity.
     *
     * @return Brush intensity [0; 255] - 0: min. bright / 255: max. bright.
     */
    uint8_t getIntensity() const override
    {
        return m_color.getIntensity();
    }

    /**
     * Set brush intensity.
     *
     * @param[in] intensity Brush intensity [0; 255] - 0: min. bright / 255: max. bright.
     */
    void setIntensity(uint8_t intensity) override
    {
        m_color.setIntensity(intensity);
    }

    /**
     * Get the color of the brush.
     *
     * @return Color of the brush.
     */
    const TColor& getColor() const
    {
        return m_color;
    }

    /**
     * Set the color of the brush.
     *
     * @param[in] color Color of the brush
     */
    void setColor(const TColor& color)
    {
        m_color = color;
    }

private:

    TColor m_color; /**< Color of the brush. */
};

/**
 * A graphics brush, providing a linear gradient color.
 *
 * @tparam TColor Color type, which is used by the brush.
 */
template < typename TColor >
class BaseGfxLinearGradientBrush : public BaseGfxBrush<TColor>
{
public:

    /**
     * Constructs a gradient graphics brush with default colors.
     */
    BaseGfxLinearGradientBrush() :
        BaseGfxBrush<TColor>(),
        m_color(),
        m_startColor(),
        m_endColor(),
        m_offset(0),
        m_gradientLength(32U),    /* Default gradient length in pixels. */
        m_verticalGradient(false) /* Default horizontal gradient. */
    {
    }

    /**
     * Constructs a gradient graphics brush.
     *
     * @param[in] startColor        Start color of the gradient.
     * @param[in] endColor          End color of the gradient.
     * @param[in] offset            Offset in pixels of the gradient start color.
     * @param[in] gradientLength    Length of the gradient in pixels.
     * @param[in] verticalGradient  Flag for vertical gradient.
     */
    BaseGfxLinearGradientBrush(const TColor& startColor, const TColor& endColor, int16_t offset, uint16_t gradientLength, bool verticalGradient) :
        BaseGfxBrush<TColor>(),
        m_color(),
        m_startColor(startColor),
        m_endColor(endColor),
        m_offset(offset),
        m_gradientLength(gradientLength),
        m_verticalGradient(verticalGradient)
    {
    }

    /**
     * Destroys a gradient graphics brush.
     */
    virtual ~BaseGfxLinearGradientBrush()
    {
    }

    /**
     * Get color at given position, used for drawing text.
     *
     * @param[in] x x-coordinate
     * @param[in] y y-coordinate
     *
     * @return Color at given position.
     */
    TColor getColor(int16_t x, int16_t y) const override
    {
        uint8_t ratio = 0U;

        /* Simple linear interpolation between start and end colors. */
        if (0U < m_gradientLength)
        {
            int32_t pos = (false == m_verticalGradient) ? x : y;

            if (m_offset >= pos)
            {
                pos = 0;
            }
            else if (m_gradientLength < pos)
            {
                pos = m_gradientLength;
            }
            else
            {
                ;
            }

            ratio = static_cast<uint8_t>((pos * 255) / m_gradientLength);
        }

        return blendColors(m_startColor, m_endColor, ratio);
    }

    /**
     * Get brush intensity.
     *
     * @return Brush intensity [0; 255] - 0: min. bright / 255: max. bright.
     */
    uint8_t getIntensity() const override
    {
        return m_startColor.getIntensity();
    }

    /**
     * Set brush intensity.
     *
     * @param[in] intensity Brush intensity [0; 255] - 0: min. bright / 255: max. bright.
     */
    void setIntensity(uint8_t intensity) override
    {
        m_startColor.setIntensity(intensity);
        m_endColor.setIntensity(intensity);
    }

    /**
     * Get the start color of the gradient.
     *
     * @return Start color of the gradient.
     */
    const TColor& getStartColor() const
    {
        return m_startColor;
    }

    /**
     * Get the end color of the gradient.
     *
     * @return End color of the gradient.
     */
    const TColor& getEndColor() const
    {
        return m_endColor;
    }

    /**
     * Set the start color of the gradient.
     *
     * @param[in] color Start color of the gradient
     */
    void setStartColor(const TColor& color)
    {
        m_startColor = color;
    }

    /**
     * Set the end color of the gradient.
     *
     * @param[in] color End color of the gradient
     */
    void setEndColor(const TColor& color)
    {
        m_endColor = color;
    }

    /**
     * Get the offset of the gradient.
     *
     * @return Offset in pixels of the gradient start color.
     */
    int16_t getOffset() const
    {
        return m_offset;
    }

    /**
     * Set the offset of the gradient.
     *
     * @param[in] offset Offset in pixels of the gradient start color.
     */
    void setOffset(int16_t offset)
    {
        m_offset = offset;
    }

    /**
     * Get the length of the gradient.
     *
     * @return Length of the gradient in pixels.
     */
    uint16_t getLength() const
    {
        return m_gradientLength;
    }

    /**
     * Set the length of the gradient.
     *
     * @param[in] length Length of the gradient in pixels.
     */
    void setLength(uint16_t length)
    {
        m_gradientLength = length;
    }

    /**
     * Get the direction of the gradient.
     *
     * @return If vertical gradient, it will return true otherwise false.
     */
    bool isVertical() const
    {
        return m_verticalGradient;
    }

    /**
     * Set the direction of the gradient.
     *
     * @param[in] vertical Flag for vertical gradient.
     */
    void setDirection(bool vertical)
    {
        m_verticalGradient = vertical;
    }

private:

    TColor   m_color;            /**< Color of the brush. */
    TColor   m_startColor;       /**< Start color of the gradient. */
    TColor   m_endColor;         /**< End color of the gradient. */
    int16_t  m_offset;           /**< Offset in pixels of the gradient start color. */
    uint16_t m_gradientLength;   /**< Length of the gradient in pixels. */
    bool     m_verticalGradient; /**< Flag for vertical gradient. */

    /**
     * Blend two colors based on a ratio (integer version).
     *
     * @param[in] color1 First color
     * @param[in] color2 Second color
     * @param[in] ratio  Ratio for blending [0; 255]
     *
     * @return Blended color.
     */
    TColor inline blendColors(const TColor& color1, const TColor& color2, uint8_t ratio) const
    {
        uint16_t red1   = color1.getRed();
        uint16_t green1 = color1.getGreen();
        uint16_t blue1  = color1.getBlue();

        uint16_t red2   = color2.getRed();
        uint16_t green2 = color2.getGreen();
        uint16_t blue2  = color2.getBlue();

        /* Blend each channel using integer math. */
        uint8_t red     = static_cast<uint8_t>((red1 * (255U - ratio) + red2 * ratio) / 255U);
        uint8_t green   = static_cast<uint8_t>((green1 * (255U - ratio) + green2 * ratio) / 255U);
        uint8_t blue    = static_cast<uint8_t>((blue1 * (255U - ratio) + blue2 * ratio) / 255U);

        return TColor(red, green, blue);
    }
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* BASE_GFX_BRUSH_HPP */

/** @} */