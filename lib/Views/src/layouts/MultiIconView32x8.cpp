/* MIT License
 *
 * Copyright (c) 2019 - 2024 Andreas Merkle <web@blue-andi.de>
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
 * @brief  View with multiple icons for 32x8 LED matrix
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "MultiIconView32x8.h"

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

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

uint8_t MultiIconView32x8::getActiveIconSlots()
{
    uint8_t cnt = 0U;
    uint8_t idx = 0U;

    while (MAX_ICON_SLOTS > idx)
    {
        BitmapWidget& bitmapWidget = m_bitmapWidgets[idx];

        if (false == bitmapWidget.isEmpty())
        {
            ++cnt;
        }

        ++idx;
    }

    return cnt;
}

void MultiIconView32x8::reorder()
{
    uint8_t cnt = getActiveIconSlots();

    /*      +-------------------------------+
     *      |                               |
     *      |               0               |
     *      |                               |
     *      +-------------------------------+
     */

    /*      +---------------+---------------+
     *      |               |               |
     *      |       0       |       1       |
     *      |               |               |
     *      +---------------+---------------+
     */

    /*      +---------+----------+----------+
     *      |         |          |          |
     *      |    0    |     1    |    2     |
     *      |         |          |          |
     *      +---------+----------+----------+
     */

    /*      +-------+-------+-------+-------+
     *      |       |       |       |       |
     *      |   0   |   1   |   2   |   3   |
     *      |       |       |       |       |
     *      +-------+-------+-------+-------+
     */

    applyLayout(cnt);
}

void MultiIconView32x8::applyLayout(uint8_t widgetCnt)
{
    uint8_t        cnt          = 0U;
    uint8_t        idx          = 0U;
    const uint16_t WIDGET_WIDTH = (0U < widgetCnt) ? (CONFIG_LED_MATRIX_WIDTH / widgetCnt) : 0U;

    while ((MAX_ICON_SLOTS > idx) && (widgetCnt > cnt))
    {
        BitmapWidget& bitmapWidget = m_bitmapWidgets[idx];

        if (false == bitmapWidget.isEmpty())
        {
            bitmapWidget.move(cnt * WIDGET_WIDTH, 0);
            bitmapWidget.setWidth(WIDGET_WIDTH);
            bitmapWidget.setHeight(CONFIG_LED_MATRIX_HEIGHT);

            ++cnt;
        }

        ++idx;
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
