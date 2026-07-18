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
 * @file   Rgb888.h
 * @brief  Color in RGB888 format
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup GFX
 *
 * @{
 */

#ifndef RGB888_H
#define RGB888_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <ColorUtil.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Color, which is based on the three base colors red, green and blue.
 * The base colors are internal stored as 8-bit values, so in RGB888 format.
 *
 * The color intensity is not stored per pixel. Dimming is applied destructively
 * via dim(), which keeps the color as small as possible in memory.
 */
class Rgb888
{
public:

    /** Max. color intensity */
    static const uint8_t MAX_BRIGHT = UINT8_MAX;

    /** Min. color intensity */
    static const uint8_t MIN_BRIGHT = 0U;

    /**
     * Constructs the color black.
     */
    Rgb888() :
        m_red(0U),
        m_green(0U),
        m_blue(0U)
    {
    }

    /**
     * Destroys the color.
     */
    ~Rgb888()
    {
    }

    /**
     * Specialized constructor, used in case every base color (RGB) is given.
     *
     * @param[in] red   Red value
     * @param[in] green Green value
     * @param[in] blue  Blue value
     */
    Rgb888(uint8_t red, uint8_t green, uint8_t blue) :
        m_red(red),
        m_green(green),
        m_blue(blue)
    {
    }

    /**
     * Specialized constructor, used in case a color value (RGB) is given as uint32 type.
     *
     * @param[in] value Color value in 24 bit format
     */
    Rgb888(uint32_t value) :
        m_red(ColorUtil::rgb888Red(value)),
        m_green(ColorUtil::rgb888Green(value)),
        m_blue(ColorUtil::rgb888Blue(value))
    {
    }

    /**
     * Copy the given color.
     *
     * @param[in] color Color, which to copy
     */
    Rgb888(const Rgb888& color) :
        m_red(color.m_red),
        m_green(color.m_green),
        m_blue(color.m_blue)
    {
    }

    /**
     * Assign RGB color.
     *
     * @param[in] color Color, which to assign
     *
     * @return RGB Color
     */
    Rgb888& operator=(const Rgb888& color)
    {
        if (this != &color)
        {
            m_red   = color.m_red;
            m_green = color.m_green;
            m_blue  = color.m_blue;
        }

        return *this;
    }

    /**
     * Compare color for equality.
     *
     * @param[in] other Other color to compare with.
     *
     * @return If both colors are equal, it will return true otherwise false.
     */
    bool operator==(const Rgb888& other) const
    {
        return (m_red == other.m_red) && (m_green == other.m_green) && (m_blue == other.m_blue);
    }

    /**
     * Compare color for non-equality.
     *
     * @param[in] other Other color to compare with.
     *
     * @return If both colors are not equal, it will return true otherwise false.
     */
    bool operator!=(const Rgb888& other) const
    {
        return (m_red != other.m_red) || (m_green != other.m_green) || (m_blue != other.m_blue);
    }

    /**
     * Convert to RGB24 uint32_t value.
     */
    operator uint32_t() const
    {
        return ColorUtil::to888(m_red, m_green, m_blue);
    }

    /**
     * Get base color information.
     *
     * @param[out] red      Red value
     * @param[out] green    Green value
     * @param[out] blue     Blue value
     */
    void get(uint8_t& red, uint8_t& green, uint8_t& blue) const
    {
        red   = m_red;
        green = m_green;
        blue  = m_blue;
    }

    /**
     * Set base color information.
     *
     * @param[in] red   Red value
     * @param[in] green Green value
     * @param[in] blue  Blue value
     */
    void set(uint8_t red, uint8_t green, uint8_t blue)
    {
        m_red   = red;
        m_green = green;
        m_blue  = blue;
    }

    /**
     * Set new color information by RGB24 value.
     *
     * @param[in] value Color value (RGB) in 24 bit format
     */
    void set(const uint32_t& value)
    {
        m_red   = ColorUtil::rgb888Red(value);
        m_green = ColorUtil::rgb888Green(value);
        m_blue  = ColorUtil::rgb888Blue(value);
    }

    /**
     * Get red color value.
     *
     * @return Red value
     */
    uint8_t getRed() const
    {
        return m_red;
    }

    /**
     * Get green color value.
     *
     * @return Green value
     */
    uint8_t getGreen() const
    {
        return m_green;
    }

    /**
     * Get blue color value.
     *
     * @return Blue value
     */
    uint8_t getBlue() const
    {
        return m_blue;
    }

    /**
     * Set red color value.
     *
     * @param[in] value Red value
     */
    void setRed(uint8_t value)
    {
        m_red = value;
    }

    /**
     * Set green color value.
     *
     * @param[in] value Green value
     */
    void setGreen(uint8_t value)
    {
        m_green = value;
    }

    /**
     * Set blue color value.
     *
     * @param[in] value Blue value
     */
    void setBlue(uint8_t value)
    {
        m_blue = value;
    }

    /**
     * Dim the color by the given brightness level. This scales the base colors
     * in place, so it is a destructive operation.
     *
     * @param[in] level Brightness level [0; 255] - 0: black / 255: no change.
     */
    void dim(uint8_t level)
    {
        m_red   = static_cast<uint8_t>((static_cast<uint16_t>(m_red) * level) / MAX_BRIGHT);
        m_green = static_cast<uint8_t>((static_cast<uint16_t>(m_green) * level) / MAX_BRIGHT);
        m_blue  = static_cast<uint8_t>((static_cast<uint16_t>(m_blue) * level) / MAX_BRIGHT);
    }

    /**
     * Set color according to the position in the color wheel.
     * It provides typical rainbow colors, which means a color is based on
     * only two base colors.
     *
     * @param[in] wheelPos  Color wheel position
     */
    void turnColorWheel(uint8_t wheelPos);

    /**
     * Convert color information to RGB565 format.
     *
     * @return Color value (RGB) in 16 bit format
     */
    uint16_t toRgb565() const
    {
        return ColorUtil::to565(m_red, m_green, m_blue);
    }

    /**
     * Set new color information by RGB565 value.
     *
     * @param[in] value Color value (RGB) in 16 bit format
     */
    void fromRgb565(const uint16_t& value)
    {
        m_red   = ColorUtil::rgb565Red(value);
        m_green = ColorUtil::rgb565Green(value);
        m_blue  = ColorUtil::rgb565Blue(value);
    }

private:

    uint8_t m_red;   /**< Red intensity value */
    uint8_t m_green; /**< Green intensity value */
    uint8_t m_blue;  /**< Blue intensity value */
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* RGB888_H */

/** @} */
