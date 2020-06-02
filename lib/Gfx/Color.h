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
 * @brief  Color
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup gfx
 *
 * @{
 */

#ifndef __COLOR_H__
#define __COLOR_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <ColorDef.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Color, which is based on the three base colors red, green and blue.
 */
class Color
{
public:

    /**
     * Constructs the color black.
     */
    Color() :
        m_red(ColorDef::getRed(ColorDef::BLACK)),
        m_green(ColorDef::getGreen(ColorDef::BLACK)),
        m_blue(ColorDef::getBlue(ColorDef::BLACK))
    {
    }

    /**
     * Destroys the color.
     */
    ~Color()
    {
    }

    /**
     * Specialized constructor, used in case every base color (RGB) is given.
     *
     * @param[in] red   Red value
     * @param[in] green Green value
     * @param[in] blue  Blue value
     */
    Color(uint8_t red, uint8_t green, uint8_t blue) :
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
    Color(uint32_t value) :
        m_red(ColorDef::getRed(value)),
        m_green(ColorDef::getGreen(value)),
        m_blue(ColorDef::getBlue(value))
    {
    }

    /**
     * Copy the given color.
     *
     * @param[in] color Color, which to copy
     */
    Color(const Color& color) :
        m_red(color.m_red),
        m_green(color.m_green),
        m_blue(color.m_blue)
    {
        return;
    }

    /**
     * Assign RGB color.
     *
     * @param[in] color Color, which to assign
     */
    Color& operator=(const Color& color)
    {
        m_red   = color.m_red;
        m_green = color.m_green;
        m_blue  = color.m_blue;

        return *this;
    }

    /**
     * Convert to RGB888 uint32_t value.
     */
    operator uint32_t() const
    {
        uint32_t color888 = m_red;

        color888 <<= 8;
        color888 |= m_green;
        color888 <<= 8;
        color888 |= m_blue;

        return color888;
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
        red		= m_red;
        green	= m_green;
        blue	= m_blue;
        return;
    }

    /**
     * Set base color information.
     *
     * @param[in] red   Red value
     * @param[in] green Green value
     * @param[in] blue  Blue value
     */
    void set(const uint8_t& red, const uint8_t& green, const uint8_t& blue)
    {
        m_red	= red;
        m_green	= green;
        m_blue	= blue;

        return;
    }

    /**
     * Set new color information.
     *
     * @param[in] value Color value (RGB) in 24 bit format
     */
    void set(const uint32_t& value)
    {
        m_red	= ColorDef::getRed(value);
        m_green	= ColorDef::getGreen(value);
        m_blue	= ColorDef::getBlue(value);

        return;
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
    void setRed(const uint8_t& value)
    {
        m_red = value;

        return;
    }

    /**
     * Set green color value.
     *
     * @param[in] value Green value
     */
    void setGreen(const uint8_t& value)
    {
        m_green = value;

        return;
    }

    /**
     * Set blue color value.
     *
     * @param[in] value Blue value
     */
    void setBlue(const uint8_t& value)
    {
        m_blue = value;

        return;
    }

    /**
     * Get color in 5-6-5 RGB format.
     *
     * @return Color in 5-6-5 RGB format
     */
    uint16_t to565() const
    {
        const uint16_t  RED     = m_red;
        const uint16_t  GREEN   = m_green;
        const uint16_t  BLUE    = m_blue;
        const uint16_t  RED5    = RED >> 3U;
        const uint16_t  GREEN6  = GREEN >> 2U;
        const uint16_t  BLUE5   = BLUE >> 3U;

        return ((RED5 & 0x1fU) << 11U) | ((GREEN6 & 0x3fU) << 5U) | ((BLUE5 & 0x1fU) << 0U);
    }

    /**
     * Set color according to the position in the color wheel.
     * It provides typical rainbow colors, which means a color is based on
     * only two base colors.
     *
     * @param[in] wheelPos  Color wheel position
     */
    void turnColorWheel(uint8_t wheelPos);

protected:

private:

    uint8_t m_red;      /**< Red intensity value */
    uint8_t m_green;    /**< Green intensity value */
    uint8_t m_blue;     /**< Blue intensity value */

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __COLOR_H__ */

/** @} */