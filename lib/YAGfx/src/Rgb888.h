/* MIT License
 *
 * Copyright (c) 2019 - 2023 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Color in RGB888 format
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup gfx
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

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Color, which is based on the three base colors red, green and blue.
 * The base colors are internal stored as 8-bit values, so in RGB888 format.
 * Additional one byte is used for color intensity, used for non-destructive
 * fading.
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
        m_blue(0U),
        m_intensity(MAX_BRIGHT)
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
     * The color intensity will be set to max. bright.
     *
     * @param[in] red   Red value
     * @param[in] green Green value
     * @param[in] blue  Blue value
     */
    Rgb888(uint8_t red, uint8_t green, uint8_t blue) :
        m_red(red),
        m_green(green),
        m_blue(blue),
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
    Rgb888(uint8_t red, uint8_t green, uint8_t blue, uint8_t intensity) :
        m_red(red),
        m_green(green),
        m_blue(blue),
        m_intensity(intensity)
    {
    }

    /**
     * Specialized constructor, used in case a color value (RGB) is given as uint32 type.
     * Color intensity will be set to max. bright.
     *
     * @param[in] value Color value in 24 bit format
     */
    Rgb888(uint32_t value) :
        m_red(extractRed(value)),
        m_green(extractGreen(value)),
        m_blue(extractBlue(value)),
        m_intensity(MAX_BRIGHT)
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
        m_blue(color.m_blue),
        m_intensity(color.m_intensity)
    {
        return;
    }

    /**
     * Assign RGB color.
     *
     * @param[in] color Color, which to assign
     */
    Rgb888& operator=(const Rgb888& color)
    {
        if (this != &color)
        {
            m_red       = color.m_red;
            m_green     = color.m_green;
            m_blue      = color.m_blue;
            m_intensity = color.m_intensity;
        }

        return *this;
    }

    /**
     * Convert to RGB24 uint32_t value.
     */
    operator uint32_t() const
    {
        uint32_t color24 = applyIntensity(m_red);

        color24 <<= 8;
        color24 |= applyIntensity(m_green);
        color24 <<= 8;
        color24 |= applyIntensity(m_blue);

        return color24;
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
        red     = applyIntensity(m_red);
        green   = applyIntensity(m_green);
        blue    = applyIntensity(m_blue);
        return;
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
        m_red   = red;
        m_green = green;
        m_blue  = blue;
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
        m_red       = red;
        m_green     = green;
        m_blue      = blue;
        m_intensity = intensity;
    }

    /**
     * Set new color information.
     * The intensity won't change.
     *
     * @param[in] value Color value (RGB) in 24 bit format
     */
    void set(const uint32_t& value)
    {
        m_red   = extractRed(value);
        m_green = extractGreen(value);
        m_blue  = extractBlue(value);
    }

    /**
     * Get red color value.
     *
     * @return Red value
     */
    uint8_t getRed() const
    {
        return applyIntensity(m_red);
    }

    /**
     * Get green color value.
     *
     * @return Green value
     */
    uint8_t getGreen() const
    {
        return applyIntensity(m_green);
    }

    /**
     * Get blue color value.
     *
     * @return Blue value
     */
    uint8_t getBlue() const
    {
        return applyIntensity(m_blue);
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
     * Set color intensity.
     * 
     * @param[in] intensity Color intensity [0; 255] - 0: min. bright / 255: max. bright
     */
    void setIntensity(uint8_t intensity)
    {
        m_intensity = intensity;
    }

    /**
     * Get color in 5-6-5 RGB format.
     *
     * @return Color in 5-6-5 RGB format
     */
    uint16_t to565() const
    {
        const uint16_t  RED     = applyIntensity(m_red);
        const uint16_t  GREEN   = applyIntensity(m_green);
        const uint16_t  BLUE    = applyIntensity(m_blue);
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

    /**
     * Extract the red base color from a RGB24 value.
     * 
     * @param[in] value Color value in RGB24 format.
     * 
     * @return Red base color
     */
    static uint8_t extractRed(uint32_t value)
    {
        return (value >> 16U) & 0xffU;
    }

    /**
     * Extract the green base color from a RGB24 value.
     * 
     * @param[in] value Color value in RGB24 format.
     * 
     * @return Green base color
     */
    static uint8_t extractGreen(uint32_t value)
    {
        return (value >> 8U) & 0xffU;
    }

    /**
     * Extract the blue base color from a RGB24 value.
     * 
     * @param[in] value Color value in RGB24 format.
     * 
     * @return Blue base color
     */
    static uint8_t extractBlue(uint32_t value)
    {
        return (value >> 0U) & 0xffU;
    }

protected:

private:

    uint8_t m_red;          /**< Red intensity value */
    uint8_t m_green;        /**< Green intensity value */
    uint8_t m_blue;         /**< Blue intensity value */
    uint8_t m_intensity;    /**< Color intensity [0; 255] - 0: min. bright / 255: max. bright */

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

#endif  /* RGB888_H */

/** @} */