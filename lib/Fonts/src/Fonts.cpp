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
 * @brief  Fonts
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "Fonts.h"

#include <Arduino.h>
#include <muMatrix8ptRegular.h>
#include <TomThumb.h>
#include <string.h>

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

/**
 * 6pt font for YAGfx: TomThumb
 */
static YAFont   gFont6pt(&TomThumb);

/**
 * 8pt font for YAGfx: muHeavy8ptRegular
 */
static YAFont   gFont8pt(&muMatrix8ptRegular);

/**
 * Font type default as string.
 */
static const char*  FONT_TYPE_DEFAULT_AS_STR = "default";

/**
 * Font type normal as string.
 */
static const char*  FONT_TYPE_NORMAL_AS_STR = "normal";

/**
 * Font type large as string.
 */
static const char*  FONT_TYPE_LARGE_AS_STR = "large";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

extern const char* Fonts::fontTypeToStr(FontType type)
{
    const char* fontTypeStr = FONT_TYPE_DEFAULT_AS_STR;

    switch(type)
    {
    case FONT_TYPE_DEFAULT:
        /* Nothing to do. */
        break;

    case FONT_TYPE_NORMAL:
        fontTypeStr = FONT_TYPE_NORMAL_AS_STR;
        break;

    case FONT_TYPE_LARGE:
        fontTypeStr = FONT_TYPE_LARGE_AS_STR;
        break;

    default:
        /* Nothing to do. */
        break;
    }

    return fontTypeStr;
}

extern Fonts::FontType Fonts::strToFontType(const char* str)
{
    FontType fontType = FONT_TYPE_DEFAULT;

    if (0 == strcmp(FONT_TYPE_NORMAL_AS_STR, str))
    {
        fontType = FONT_TYPE_NORMAL;
    }
    else if (0 == strcmp(FONT_TYPE_LARGE_AS_STR, str))
    {
        fontType = FONT_TYPE_LARGE;
    }
    else
    {
        /* Nothing to do. */
        ;
    }

    return fontType;
}

extern YAFont&  Fonts::getFontByType(FontType type)
{
    YAFont* font = &gFont6pt;   /* The normal font is the default font. */

    if (FONT_TYPE_LARGE == type)
    {
        font = &gFont8pt;   /* Large font */
    }

    return *font;
}

/******************************************************************************
 * Local Functions
 *****************************************************************************/
