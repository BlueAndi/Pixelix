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
 * @file   Rgb565.h
 * @brief  Color in RGB565 format
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup GFX
 *
 * @{
 */

#ifndef RGB565_H
#define RGB565_H

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
 * The base colors are internal stored in RGB565 format, so a single pixel needs
 * only two bytes.
 *
 * The color intensity is not stored per pixel. Dimming is applied destructively
 * via dim().
 */
class Rgb565
{
public:

    /** Max. color intensity */
    static const uint8_t MAX_BRIGHT = UINT8_MAX;

    /** Min. color intensity */
    static const uint8_t MIN_BRIGHT = 0U;

    /**
     * Constructs the color black.
     */
    Rgb565() :
        m_color565(0U)
    {
    }

    /**
     * Destroys the color.
     */
    ~Rgb565()
    {
    }

    /**
     * Specialized constructor, used in case every base color (RGB) is given.
     *
     * @param[in] red   Red value
     * @param[in] green Green value
     * @param[in] blue  Blue value
     */
    Rgb565(uint8_t red, uint8_t green, uint8_t blue) :
        m_color565(ColorUtil::to565(red, green, blue))
    {
    }

    /**
     * Specialized constructor, used in case a color value (RGB) is given as uint32 type.
     *
     * @param[in] value Color value in 24 bit format
     */
    Rgb565(uint32_t value) :
        m_color565(ColorUtil::to565(value))
    {
    }

    /**
     * Copy the given color.
     *
     * @param[in] color Color, which to copy
     */
    Rgb565(const Rgb565& color) :
        m_color565(color.m_color565)
    {
    }

    /**
     * Assign RGB color.
     *
     * @param[in] color Color, which to assign
     *
     * @return RGB Color
     */
    Rgb565& operator=(const Rgb565& color)
    {
        if (this != &color)
        {
            m_color565 = color.m_color565;
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
    bool operator==(const Rgb565& other) const
    {
        return (m_color565 == other.m_color565);
    }

    /**
     * Compare color for non-equality.
     *
     * @param[in] other Other color to compare with.
     *
     * @return If both colors are not equal, it will return true otherwise false.
     */
    bool operator!=(const Rgb565& other) const
    {
        return (m_color565 != other.m_color565);
    }

    /**
     * Convert to RGB24 uint32_t value.
     */
    operator uint32_t() const
    {
        return ColorUtil::to888(getRed(), getGreen(), getBlue());
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
        red   = getRed();
        green = getGreen();
        blue  = getBlue();
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
        m_color565 = ColorUtil::to565(red, green, blue);
    }

    /**
     * Set new color information by RGB24 value.
     *
     * @param[in] value Color value (RGB) in 24 bit format
     */
    void set(const uint32_t& value)
    {
        m_color565 = ColorUtil::to565(value);
    }

    /**
     * Get red color value.
     *
     * @return Red value
     */
    uint8_t getRed() const
    {
        return ColorUtil::rgb565Red(m_color565);
    }

    /**
     * Get green color value.
     *
     * @return Green value
     */
    uint8_t getGreen() const
    {
        return ColorUtil::rgb565Green(m_color565);
    }

    /**
     * Get blue color value.
     *
     * @return Blue value
     */
    uint8_t getBlue() const
    {
        return ColorUtil::rgb565Blue(m_color565);
    }

    /**
     * Set red color value.
     *
     * @param[in] value Red value
     */
    void setRed(uint8_t value)
    {
        m_color565 = ColorUtil::to565(value, getGreen(), getBlue());
    }

    /**
     * Set green color value.
     *
     * @param[in] value Green value
     */
    void setGreen(uint8_t value)
    {
        m_color565 = ColorUtil::to565(getRed(), value, getBlue());
    }

    /**
     * Set blue color value.
     *
     * @param[in] value Blue value
     */
    void setBlue(uint8_t value)
    {
        m_color565 = ColorUtil::to565(getRed(), getGreen(), value);
    }

    /**
     * Dim the color by the given brightness level. This scales the base colors
     * in place, so it is a destructive operation.
     *
     * @param[in] level Brightness level [0; 255] - 0: black / 255: no change.
     */
    void dim(uint8_t level)
    {
        uint8_t red   = static_cast<uint8_t>((static_cast<uint16_t>(getRed()) * level) / MAX_BRIGHT);
        uint8_t green = static_cast<uint8_t>((static_cast<uint16_t>(getGreen()) * level) / MAX_BRIGHT);
        uint8_t blue  = static_cast<uint8_t>((static_cast<uint16_t>(getBlue()) * level) / MAX_BRIGHT);

        m_color565    = ColorUtil::to565(red, green, blue);
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
        return m_color565;
    }

    /**
     * Set new color information by RGB565 value.
     *
     * @param[in] value Color value (RGB) in 16 bit format
     */
    void fromRgb565(uint16_t value)
    {
        m_color565 = value;
    }

private:

    uint16_t m_color565; /**< Color value in 5-6-5 RGB format */
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* RGB565_H */

/** @} */
