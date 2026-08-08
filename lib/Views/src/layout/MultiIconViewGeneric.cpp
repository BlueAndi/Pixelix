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
 * @file   MultiIconViewGeneric.cpp
 * @brief  Generic view with multiple icons for LED matrix
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "MultiIconViewGeneric.h"
#include <FileSystem.h>
#include <Logging.h>

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

/******************************************************************************
 * Public Methods
 *****************************************************************************/

MultiIconViewGeneric::MultiIconViewGeneric() :
    IMultiIconView(),
    m_bitmapWidgets{
        { BITMAP_WIDTH, BITMAP_HEIGHT, BITMAP_0_X, BITMAP_Y },
        { BITMAP_WIDTH, BITMAP_HEIGHT, BITMAP_1_X, BITMAP_Y },
        { BITMAP_WIDTH, BITMAP_HEIGHT, BITMAP_2_X, BITMAP_Y }
    }
{
    uint8_t slot = 0U;

    while (MAX_ICON_SLOTS > slot)
    {
        m_bitmapWidgets[slot].setHorizontalAlignment(Alignment::Horizontal::HORIZONTAL_CENTER);
        m_bitmapWidgets[slot].setVerticalAlignment(Alignment::Vertical::VERTICAL_CENTER);

        ++slot;
    }
}

void MultiIconViewGeneric::update(YAGfx& gfx)
{
    uint8_t idx = 0U;

    gfx.fillScreen(ColorDef::BLACK);

    while (MAX_ICON_SLOTS > idx)
    {
        m_bitmapWidgets[idx].update(gfx);
        ++idx;
    }
}

bool MultiIconViewGeneric::loadIcon(uint8_t slotId, const String& filename, FS& fs)
{
    if (MAX_ICON_SLOTS <= slotId)
    {
        slotId = 0U;
    }

    return m_bitmapWidgets[slotId].load(filename, fs);
}

void MultiIconViewGeneric::clearIcon(uint8_t slotId)
{
    if (MAX_ICON_SLOTS <= slotId)
    {
        slotId = 0U;
    }

    m_bitmapWidgets[slotId].clear(ColorDef::BLACK);
}

bool MultiIconViewGeneric::isIconSlotEmpty(uint8_t slotId) const
{
    if (MAX_ICON_SLOTS <= slotId)
    {
        slotId = 0U;
    }

    return m_bitmapWidgets[slotId].isEmpty();
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
