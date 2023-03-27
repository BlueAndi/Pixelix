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
 * @brief  Color definitions
 * @author Andreas Merkle <web@blue-andi.de>
 * 
 * @addtogroup gfx
 *
 * @{
 */

#ifndef COLORDEF_HPP
#define COLORDEF_HPP

/*******************************************************************************
    INCLUDES
*******************************************************************************/
#include <stdint.h>

/*******************************************************************************
    COMPILER SWITCHES
*******************************************************************************/

/*******************************************************************************
    CONSTANTS
*******************************************************************************/

/** Color definitions */
namespace ColorDef
{
    /** Color definition for BLACK. */
    static const uint32_t   BLACK                   = 0x000000;

    /** Color definition for NAVY. */
    static const uint32_t   NAVY                    = 0x000080;

    /** Color definition for DARKBLUE. */
    static const uint32_t   DARKBLUE                = 0x00008B;

    /** Color definition for MEDIUMBLUE. */
    static const uint32_t   MEDIUMBLUE              = 0x0000CD;

    /** Color definition for BLUE. */
    static const uint32_t   BLUE                    = 0x0000FF;

    /** Color definition for DARKGREEN. */
    static const uint32_t   DARKGREEN               = 0x006400;

    /** Color definition for GREEN. */
    static const uint32_t   GREEN                   = 0x008000;

    /** Color definition for TEAL. */
    static const uint32_t   TEAL                    = 0x008080;

    /** Color definition for DARKCYAN. */
    static const uint32_t   DARKCYAN                = 0x008B8B;

    /** Color definition for DEEPSKYBLUE. */
    static const uint32_t   DEEPSKYBLUE             = 0x00BFFF;

    /** Color definition for DARKTURQUOISE. */
    static const uint32_t   DARKTURQUOISE           = 0x00CED1;

    /** Color definition for MEDIUMSPRINGGREEN. */
    static const uint32_t   MEDIUMSPRINGGREEN       = 0x00FA9A;

    /** Color definition for LIME. */
    static const uint32_t   LIME                    = 0x00FF00;

    /** Color definition for SPRINGGREEN. */
    static const uint32_t   SPRINGGREEN             = 0x00FF7F;

    /** Color definition for AQUA. */
    static const uint32_t   AQUA                    = 0x00FFFF;

    /** Color definition for CYAN. */
    static const uint32_t   CYAN                    = 0x00FFFF;

    /** Color definition for MIDNIGHTBLUE. */
    static const uint32_t   MIDNIGHTBLUE            = 0x191970;

    /** Color definition for DODGERBLUE. */
    static const uint32_t   DODGERBLUE              = 0x1E90FF;

    /** Color definition for LIGHTSEAGREEN. */
    static const uint32_t   LIGHTSEAGREEN           = 0x20B2AA;

    /** Color definition for FORESTGREEN. */
    static const uint32_t   FORESTGREEN             = 0x228B22;

    /** Color definition for SEAGREEN. */
    static const uint32_t   SEAGREEN                = 0x2E8B57;

    /** Color definition for DARKSLATEGRAY. */
    static const uint32_t   DARKSLATEGRAY           = 0x2F4F4F;

    /** Color definition for LIMEGREEN. */
    static const uint32_t   LIMEGREEN               = 0x32CD32;

    /** Color definition for MEDIUMSEAGREEN. */
    static const uint32_t   MEDIUMSEAGREEN          = 0x3CB371;

    /** Color definition for ULTRADARKGRAY. */
    static const uint32_t   ULTRADARKGRAY           = 0x404040;

    /** Color definition for TURQUOISE. */
    static const uint32_t   TURQUOISE               = 0x40E0D0;

    /** Color definition for ROYALBLUE. */
    static const uint32_t   ROYALBLUE               = 0x4169E1;

    /** Color definition for STEELBLUE. */
    static const uint32_t   STEELBLUE               = 0x4682B4;

    /** Color definition for DARKSLATEBLUE. */
    static const uint32_t   DARKSLATEBLUE           = 0x483D8B;

    /** Color definition for MEDIUMTURQUOISE. */
    static const uint32_t   MEDIUMTURQUOISE         = 0x48D1CC;

    /** Color definition for INDIGO. */
    static const uint32_t   INDIGO                  = 0x4B0082;

    /** Color definition for DARKOLIVEGREEN. */
    static const uint32_t   DARKOLIVEGREEN          = 0x556B2F;

    /** Color definition for CADETBLUE. */
    static const uint32_t   CADETBLUE               = 0x5F9EA0;

    /** Color definition for CORNFLOWERBLUE. */
    static const uint32_t   CORNFLOWERBLUE          = 0x6495ED;

    /** Color definition for MEDIUMAQUAMARINE. */
    static const uint32_t   MEDIUMAQUAMARINE        = 0x66CDAA;

    /** Color definition for DIMGRAY. */
    static const uint32_t   DIMGRAY                 = 0x696969;

    /** Color definition for SLATEBLUE. */
    static const uint32_t   SLATEBLUE               = 0x6A5ACD;

    /** Color definition for OLIVEDRAB. */
    static const uint32_t   OLIVEDRAB               = 0x6B8E23;

    /** Color definition for SLATEGRAY. */
    static const uint32_t   SLATEGRAY               = 0x708090;

    /** Color definition for LIGHTSLATEGRAY. */
    static const uint32_t   LIGHTSLATEGRAY          = 0x778899;

    /** Color definition for MEDIUMSLATEBLUE. */
    static const uint32_t   MEDIUMSLATEBLUE         = 0x7B68EE;

    /** Color definition for LAWNGREEN. */
    static const uint32_t   LAWNGREEN               = 0x7CFC00;

    /** Color definition for CHARTREUSE. */
    static const uint32_t   CHARTREUSE              = 0x7FFF00;

    /** Color definition for AQUAMARINE. */
    static const uint32_t   AQUAMARINE              = 0x7FFFD4;

    /** Color definition for MAROON. */
    static const uint32_t   MAROON                  = 0x800000;

    /** Color definition for PURPLE. */
    static const uint32_t   PURPLE                  = 0x800080;

    /** Color definition for OLIVE. */
    static const uint32_t   OLIVE                   = 0x808000;

    /** Color definition for GRAY. */
    static const uint32_t   GRAY                    = 0x808080;

    /** Color definition for SKYBLUE. */
    static const uint32_t   SKYBLUE                 = 0x87CEEB;

    /** Color definition for LIGHTSKYBLUE. */
    static const uint32_t   LIGHTSKYBLUE            = 0x87CEFA;

    /** Color definition for BLUEVIOLET. */
    static const uint32_t   BLUEVIOLET              = 0x8A2BE2;

    /** Color definition for DARKRED. */
    static const uint32_t   DARKRED                 = 0x8B0000;

    /** Color definition for DARKMAGENTA. */
    static const uint32_t   DARKMAGENTA             = 0x8B008B;

    /** Color definition for SADDLEBROWN. */
    static const uint32_t   SADDLEBROWN             = 0x8B4513;

    /** Color definition for DARKSEAGREEN. */
    static const uint32_t   DARKSEAGREEN            = 0x8FBC8F;

    /** Color definition for LIGHTGREEN. */
    static const uint32_t   LIGHTGREEN              = 0x90EE90;

    /** Color definition for MEDIUMPURPLE. */
    static const uint32_t   MEDIUMPURPLE            = 0x9370DB;

    /** Color definition for DARKVIOLET. */
    static const uint32_t   DARKVIOLET              = 0x9400D3;

    /** Color definition for PALEGREEN. */
    static const uint32_t   PALEGREEN               = 0x98FB98;

    /** Color definition for DARKORCHID. */
    static const uint32_t   DARKORCHID              = 0x9932CC;

    /** Color definition for YELLOWGREEN. */
    static const uint32_t   YELLOWGREEN             = 0x9ACD32;

    /** Color definition for SIENNA. */
    static const uint32_t   SIENNA                  = 0xA0522D;

    /** Color definition for BROWN. */
    static const uint32_t   BROWN                   = 0xA52A2A;

    /** Color definition for DARKGRAY. */
    static const uint32_t   DARKGRAY                = 0xA9A9A9;

    /** Color definition for LIGHTBLUE. */
    static const uint32_t   LIGHTBLUE               = 0xADD8E6;

    /** Color definition for GREENYELLOW. */
    static const uint32_t   GREENYELLOW             = 0xADFF2F;

    /** Color definition for PALETURQUOISE. */
    static const uint32_t   PALETURQUOISE           = 0xAFEEEE;

    /** Color definition for LIGHTSTEELBLUE. */
    static const uint32_t   LIGHTSTEELBLUE          = 0xB0C4DE;

    /** Color definition for POWDERBLUE. */
    static const uint32_t   POWDERBLUE              = 0xB0E0E6;

    /** Color definition for FIREBRICK. */
    static const uint32_t   FIREBRICK               = 0xB22222;

    /** Color definition for DARKGOLDENROD. */
    static const uint32_t   DARKGOLDENROD           = 0xB8860B;

    /** Color definition for MEDIUMORCHID. */
    static const uint32_t   MEDIUMORCHID            = 0xBA55D3;

    /** Color definition for ROSYBROWN. */
    static const uint32_t   ROSYBROWN               = 0xBC8F8F;

    /** Color definition for DARKKHAKI. */
    static const uint32_t   DARKKHAKI               = 0xBDB76B;

    /** Color definition for SILVER. */
    static const uint32_t   SILVER                  = 0xC0C0C0;

    /** Color definition for MEDIUMVIOLETRED. */
    static const uint32_t   MEDIUMVIOLETRED         = 0xC71585;

    /** Color definition for INDIANRED. */
    static const uint32_t   INDIANRED               = 0xCD5C5C;

    /** Color definition for PERU. */
    static const uint32_t   PERU                    = 0xCD853F;

    /** Color definition for CHOCOLATE. */
    static const uint32_t   CHOCOLATE               = 0xD2691E;

    /** Color definition for TAN. */
    static const uint32_t   TAN                     = 0xD2B48C;

    /** Color definition for LIGHTGRAY. */
    static const uint32_t   LIGHTGRAY               = 0xD3D3D3;

    /** Color definition for THISTLE. */
    static const uint32_t   THISTLE                 = 0xD8BFD8;

    /** Color definition for ORCHID. */
    static const uint32_t   ORCHID                  = 0xDA70D6;

    /** Color definition for GOLDENROD. */
    static const uint32_t   GOLDENROD               = 0xDAA520;

    /** Color definition for PALEVIOLETRED. */
    static const uint32_t   PALEVIOLETRED           = 0xDB7093;

    /** Color definition for CRIMSON. */
    static const uint32_t   CRIMSON                 = 0xDC143C;

    /** Color definition for GAINSBORO. */
    static const uint32_t   GAINSBORO               = 0xDCDCDC;

    /** Color definition for PLUM. */
    static const uint32_t   PLUM                    = 0xDDA0DD;

    /** Color definition for BURLYWOOD. */
    static const uint32_t   BURLYWOOD               = 0xDEB887;

    /** Color definition for LIGHTCYAN. */
    static const uint32_t   LIGHTCYAN               = 0xE0FFFF;

    /** Color definition for LAVENDER. */
    static const uint32_t   LAVENDER                = 0xE6E6FA;

    /** Color definition for DARKSALMON. */
    static const uint32_t   DARKSALMON              = 0xE9967A;

    /** Color definition for VIOLET. */
    static const uint32_t   VIOLET                  = 0xEE82EE;

    /** Color definition for PALEGOLDENROD. */
    static const uint32_t   PALEGOLDENROD           = 0xEEE8AA;

    /** Color definition for LIGHTCORAL. */
    static const uint32_t   LIGHTCORAL              = 0xF08080;

    /** Color definition for KHAKI. */
    static const uint32_t   KHAKI                   = 0xF0E68C;

    /** Color definition for ALICEBLUE. */
    static const uint32_t   ALICEBLUE               = 0xF0F8FF;

    /** Color definition for HONEYDEW. */
    static const uint32_t   HONEYDEW                = 0xF0FFF0;

    /** Color definition for AZURE. */
    static const uint32_t   AZURE                   = 0xF0FFFF;

    /** Color definition for SANDYBROWN. */
    static const uint32_t   SANDYBROWN              = 0xF4A460;

    /** Color definition for WHEAT. */
    static const uint32_t   WHEAT                   = 0xF5DEB3;

    /** Color definition for BEIGE. */
    static const uint32_t   BEIGE                   = 0xF5F5DC;

    /** Color definition for WHITESMOKE. */
    static const uint32_t   WHITESMOKE              = 0xF5F5F5;

    /** Color definition for MINTCREAM. */
    static const uint32_t   MINTCREAM               = 0xF5FFFA;

    /** Color definition for GHOSTWHITE. */
    static const uint32_t   GHOSTWHITE              = 0xF8F8FF;

    /** Color definition for SALMON. */
    static const uint32_t   SALMON                  = 0xFA8072;

    /** Color definition for ANTIQUEWHITE. */
    static const uint32_t   ANTIQUEWHITE            = 0xFAEBD7;

    /** Color definition for LINEN. */
    static const uint32_t   LINEN                   = 0xFAF0E6;

    /** Color definition for LIGHTGOLDENRODYELLOW. */
    static const uint32_t   LIGHTGOLDENRODYELLOW    = 0xFAFAD2;

    /** Color definition for OLDLACE. */
    static const uint32_t   OLDLACE                 = 0xFDF5E6;

    /** Color definition for RED. */
    static const uint32_t   RED                     = 0xFF0000;

    /** Color definition for FUCHSIA. */
    static const uint32_t   FUCHSIA                 = 0xFF00FF;

    /** Color definition for MAGENTA. */
    static const uint32_t   MAGENTA                 = 0xFF00FF;

    /** Color definition for DEEPPINK. */
    static const uint32_t   DEEPPINK                = 0xFF1493;

    /** Color definition for ORANGERED. */
    static const uint32_t   ORANGERED               = 0xFF4500;

    /** Color definition for TOMATO. */
    static const uint32_t   TOMATO                  = 0xFF6347;

    /** Color definition for HOTPINK. */
    static const uint32_t   HOTPINK                 = 0xFF69B4;

    /** Color definition for CORAL. */
    static const uint32_t   CORAL                   = 0xFF7F50;

    /** Color definition for DARKORANGE. */
    static const uint32_t   DARKORANGE              = 0xFF8C00;

    /** Color definition for LIGHTSALMON. */
    static const uint32_t   LIGHTSALMON             = 0xFFA07A;

    /** Color definition for ORANGE. */
    static const uint32_t   ORANGE                  = 0xFFA500;

    /** Color definition for LIGHTPINK. */
    static const uint32_t   LIGHTPINK               = 0xFFB6C1;

    /** Color definition for PINK. */
    static const uint32_t   PINK                    = 0xFFC0CB;

    /** Color definition for GOLD. */
    static const uint32_t   GOLD                    = 0xFFD700;

    /** Color definition for PEACHPUFF. */
    static const uint32_t   PEACHPUFF               = 0xFFDAB9;

    /** Color definition for NAVAJOWHITE. */
    static const uint32_t   NAVAJOWHITE             = 0xFFDEAD;

    /** Color definition for MOCCASIN. */
    static const uint32_t   MOCCASIN                = 0xFFE4B5;

    /** Color definition for BISQUE. */
    static const uint32_t   BISQUE                  = 0xFFE4C4;

    /** Color definition for MISTYROSE. */
    static const uint32_t   MISTYROSE               = 0xFFE4E1;

    /** Color definition for BLANCHEDALMOND. */
    static const uint32_t   BLANCHEDALMOND          = 0xFFEBCD;

    /** Color definition for PAPAYAWHIP. */
    static const uint32_t   PAPAYAWHIP              = 0xFFEFD5;

    /** Color definition for LAVENDERBLUSH. */
    static const uint32_t   LAVENDERBLUSH           = 0xFFF0F5;

    /** Color definition for SEASHELL. */
    static const uint32_t   SEASHELL                = 0xFFF5EE;

    /** Color definition for CORNSILK. */
    static const uint32_t   CORNSILK                = 0xFFF8DC;

    /** Color definition for LEMONCHIFFON. */
    static const uint32_t   LEMONCHIFFON            = 0xFFFACD;

    /** Color definition for FLORALWHITE. */
    static const uint32_t   FLORALWHITE             = 0xFFFAF0;

    /** Color definition for SNOW. */
    static const uint32_t   SNOW                    = 0xFFFAFA;

    /** Color definition for YELLOW. */
    static const uint32_t   YELLOW                  = 0xFFFF00;

    /** Color definition for LIGHTYELLOW. */
    static const uint32_t   LIGHTYELLOW             = 0xFFFFE0;

    /** Color definition for IVORY. */
    static const uint32_t   IVORY                   = 0xFFFFF0;

    /** Color definition for WHITE. */
    static const uint32_t   WHITE                   = 0xFFFFFF;
    
}

/*******************************************************************************
    MACROS
*******************************************************************************/

/*******************************************************************************
    CLASSES, TYPES AND STRUCTURES
*******************************************************************************/

/*******************************************************************************
    VARIABLES
*******************************************************************************/

/*******************************************************************************
    FUNCTIONS
*******************************************************************************/

/**
 * Color definitions
 */
namespace ColorDef
{    
    /**
     * Get red value from 32 bit color definition.
     *
     * @param[in] value 32 bit color definition
     *
     * @return Red value
     */
    inline uint8_t getRed(uint32_t value)
    {
        return static_cast<uint8_t>((value >> 16U) & 0xffU);
    }
    
    /**
     * Get green value from 32 bit color definition.
     *
     * @param[in] value 32 bit color definition
     *
     * @return Green value
     */
    inline uint8_t getGreen(uint32_t value)
    {
        return static_cast<uint8_t>((value >> 8U) & 0xffU);
    }
    
    /**
     * Get blue value from 32 bit color definition.
     *
     * @param[in] value 32 bit color definition
     *
     * @return Blue value
     */
    inline uint8_t getBlue(uint32_t value)
    {
        return static_cast<uint8_t>((value >> 0U) & 0xffU);
    }
    
    /**
     * Convert color in RGB888 format to RGB565 format.
     * 
     * @param[in] value Color value in RGB888 format
     * 
     * @return Color in RGB565 format.
     */
    inline uint16_t convert888To565(uint32_t value)
    {
        const uint16_t  RED     = (value >> 16U) & 0xffU;
        const uint16_t  GREEN   = (value >>  8U) & 0xffU;
        const uint16_t  BLUE    = (value >>  0U) & 0xffU;
        const uint16_t  RED5    = RED >> 3U;
        const uint16_t  GREEN6  = GREEN >> 2U;
        const uint16_t  BLUE5   = BLUE >> 3U;

        return ((RED5 & 0x1fU) << 11U) | ((GREEN6 & 0x3fU) << 5U) | ((BLUE5 & 0x1fU) << 0U);
    }

    /**
     * Convert color in RGB565 format to RGB888 format.
     * 
     * @param[in] value Color value in RGB565 format
     * 
     * @return Color in RGB888 format.
     */
    inline uint32_t convert565To888(uint16_t value)
    {
        const uint32_t  RED5    = (value >> 11U) & 0x1fU;
        const uint32_t  GREEN6  = (value >>  5U) & 0x3fU;
        const uint32_t  BLUE5   = (value >>  0U) & 0x1fU;
        const uint32_t  RED     = RED5 << 3U;
        const uint32_t  GREEN   = GREEN6 << 2U;
        const uint32_t  BLUE    = BLUE5 << 3U;

        return (RED << 16U) | (GREEN << 8U) | (BLUE << 0U);
    }
}

#endif  /* COLORDEF_HPP */

/** @} */