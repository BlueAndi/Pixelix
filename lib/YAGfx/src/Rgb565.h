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
 * The base colors are internal stored in RGB565 format.
 * Additional one byte is used for color intensity, used for non-destructive
 * fading.
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
        m_color565(0U),
        m_intensity(MAX_BRIGHT)
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
     * The color intensity will be set to max. bright.
     *
     * @param[in] red   Red value
     * @param[in] green Green value
     * @param[in] blue  Blue value
     */
    Rgb565(uint8_t red, uint8_t green, uint8_t blue) :
        m_color565(ColorUtil::to565(red, green, blue)),
        m_intensity(MAX_BRIGHT)
    {
    }

    /**
     * Specialized constructor, used in case every base color (RGB) and
     * the intensity is given.
     *
     * @param[in] red       Red value
     * @param[in] green     Green value
     * @param[in] blue      Blue value
     * @param[in] intensity Color intensity [0; 255]
     */
    Rgb565(uint8_t red, uint8_t green, uint8_t blue, uint8_t intensity) :
        m_color565(ColorUtil::to565(red, green, blue)),
        m_intensity(intensity)
    {
    }

    /**
     * Specialized constructor, used in case a color value (RGB) is given as uint32 type.
     * Color intensity will be set to max. bright.
     *
     * @param[in] value Color value in 24 bit format
     */
    Rgb565(uint32_t value) :
        m_color565(ColorUtil::to565(value)),
        m_intensity(MAX_BRIGHT)
    {
    }

    /**
     * Copy the given color.
     *
     * @param[in] color Color, which to copy
     */
    Rgb565(const Rgb565& color) :
        m_color565(color.m_color565),
        m_intensity(color.m_intensity)
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
            m_color565  = color.m_color565;
            m_intensity = color.m_intensity;
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
        return (m_color565 == other.m_color565) && (m_intensity == other.m_intensity);
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
        return (m_color565 != other.m_color565) || (m_intensity != other.m_intensity);
    }

    /**
     * Convert to RGB24 uint32_t value with respect to current intensity.
     */
    operator uint32_t() const
    {
        return ColorUtil::to888(getRed(), getGreen(), getBlue());
    }

    /**
     * Get base color information with respect to current intensity.
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
     * Intensity is not changed.
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
     * Set base color information, incl. intensity.
     *
     * @param[in] red       Red value
     * @param[in] green     Green value
     * @param[in] blue      Blue value
     * @param[in] intensity Color intensity [0; 255]
     */
    void set(uint8_t red, uint8_t green, uint8_t blue, uint8_t intensity)
    {
        m_color565  = ColorUtil::to565(red, green, blue);
        m_intensity = intensity;
    }

    /**
     * Set new color information by RGB24 value.
     * The intensity won't change.
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
        return applyIntensity(ColorUtil::rgb565Red(m_color565));
    }

    /**
     * Get green color value.
     *
     * @return Green value
     */
    uint8_t getGreen() const
    {
        return applyIntensity(ColorUtil::rgb565Green(m_color565));
    }

    /**
     * Get blue color value.
     *
     * @return Blue value
     */
    uint8_t getBlue() const
    {
        return applyIntensity(ColorUtil::rgb565Blue(m_color565));
    }

    /**
     * Get color intensity.
     *
     * @return Color intensity [0; 255] - 0: min. bright / 255: max. bright
     */
    uint8_t getIntensity() const
    {
        return m_intensity;
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
     * Set color intensity.
     *
     * @param[in] intensity Color intensity [0; 255] - 0: min. bright / 255: max. bright
     */
    void setIntensity(uint8_t intensity)
    {
        m_intensity = intensity;
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
     * The intensity won't change.
     *
     * @param[in] value Color value (RGB) in 16 bit format
     */
    void fromRgb565(uint16_t value)
    {
        m_color565 = value;
    }

private:

    uint16_t m_color565;  /**< Color value in 5-6-5 RGB format */
    uint8_t  m_intensity; /**< Color intensity [0; 255] - 0: min. bright / 255: max. bright */

    /**
     * Calculate the base color with respect to the current intensity.
     *
     * @return Base color with considered intensity.
     */
    inline uint8_t applyIntensity(uint8_t baseColor) const
    {
        return (static_cast<uint16_t>(baseColor) * static_cast<uint16_t>(m_intensity)) / MAX_BRIGHT;
    }
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* RGB565_H */

/** @} */