/* MIT License
 *
 * Copyright (c) 2019 - 2025 Andreas Merkle <web@blue-andi.de>
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
 * @brief  View with multiple icons for 64x64 LED matrix
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "MultiIconView64x64.h"

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

uint8_t MultiIconView64x64::getActiveIconSlots()
{
    uint8_t cnt = 0U;
    uint8_t idx = 0U;

    while(MAX_ICON_SLOTS > idx)
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

void MultiIconView64x64::reorder()
{
    uint8_t cnt = getActiveIconSlots();

    switch(cnt)
    {
    case 0U:
        /* Nothing to do. */
        break;
    
    case 1U:
        /*      +-------+
         *      |       |
         *      |   0   |
         *      |       |
         *      +-------+
         */
        applyLayout1();
        break;

    case 2U:
        /*      +-------+
         *      |   0   |
         *      |       |
         *      |   1   |
         *      +-------+
         */
        applyLayout2();
        break;

    case 3U:
        /*      +-------+
         *      | 0   1 |
         *      |       |
         *      |   2   |
         *      +-------+
         */
        applyLayout3();
        break;

    case 4U:
        /*      +-------+
         *      | 0   1 |
         *      |       |
         *      | 2   3 |
         *      +-------+
         */
        applyLayout4();
        break;

    default:
        break;
    }
}

void MultiIconView64x64::applyLayout1()
{
    uint8_t cnt = 0U;
    uint8_t idx = 0U;

    while((MAX_ICON_SLOTS > idx) && (1U > cnt))
    {
        BitmapWidget& bitmapWidget = m_bitmapWidgets[idx];

        if (false == bitmapWidget.isEmpty())
        {
            bitmapWidget.move(0, 0);
            bitmapWidget.setWidth(CONFIG_LED_MATRIX_WIDTH);
            bitmapWidget.setHeight(CONFIG_LED_MATRIX_HEIGHT);

            ++cnt;
        }

        ++idx;
    }
}

void MultiIconView64x64::applyLayout2()
{
    uint8_t cnt = 0U;
    uint8_t idx = 0U;

    while((MAX_ICON_SLOTS > idx) && (2U > cnt))
    {
        BitmapWidget& bitmapWidget = m_bitmapWidgets[idx];

        if (false == bitmapWidget.isEmpty())
        {
            if (0U == cnt)
            {
                bitmapWidget.move(0, 0);
                bitmapWidget.setWidth(CONFIG_LED_MATRIX_WIDTH);
                bitmapWidget.setHeight(CONFIG_LED_MATRIX_HEIGHT / 2U);
            }
            else
            {
                bitmapWidget.move(0, CONFIG_LED_MATRIX_HEIGHT / 2U);
                bitmapWidget.setWidth(CONFIG_LED_MATRIX_WIDTH);
                bitmapWidget.setHeight(CONFIG_LED_MATRIX_HEIGHT / 2U);
            }

            ++cnt;
        }

        ++idx;
    }
}

void MultiIconView64x64::applyLayout3()
{
    uint8_t cnt = 0U;
    uint8_t idx = 0U;

    while((MAX_ICON_SLOTS > idx) && (3U > cnt))
    {
        BitmapWidget& bitmapWidget = m_bitmapWidgets[idx];

        if (false == bitmapWidget.isEmpty())
        {
            if (0U == cnt)
            {
                bitmapWidget.move(0, 0);
                bitmapWidget.setWidth(CONFIG_LED_MATRIX_WIDTH / 2U);
                bitmapWidget.setHeight(CONFIG_LED_MATRIX_HEIGHT / 2U);
            }
            else if (1U == cnt)
            {
                bitmapWidget.move(CONFIG_LED_MATRIX_WIDTH / 2U, 0);
                bitmapWidget.setWidth(CONFIG_LED_MATRIX_WIDTH / 2U);
                bitmapWidget.setHeight(CONFIG_LED_MATRIX_HEIGHT / 2U);
            }
            else
            {
                bitmapWidget.move(0, CONFIG_LED_MATRIX_HEIGHT / 2U);
                bitmapWidget.setWidth(CONFIG_LED_MATRIX_WIDTH);
                bitmapWidget.setHeight(CONFIG_LED_MATRIX_HEIGHT / 2U);
            }

            ++cnt;
        }

        ++idx;
    }
}

void MultiIconView64x64::applyLayout4()
{
    uint8_t cnt = 0U;
    uint8_t idx = 0U;

    while((MAX_ICON_SLOTS > idx) && (4U > cnt))
    {
        BitmapWidget& bitmapWidget = m_bitmapWidgets[idx];

        if (false == bitmapWidget.isEmpty())
        {
            if (0U == cnt)
            {
                bitmapWidget.move(0, 0);
                bitmapWidget.setWidth(CONFIG_LED_MATRIX_WIDTH / 2U);
                bitmapWidget.setHeight(CONFIG_LED_MATRIX_HEIGHT / 2U);
            }
            else if (1U == cnt)
            {
                bitmapWidget.move(CONFIG_LED_MATRIX_WIDTH / 2U, 0);
                bitmapWidget.setWidth(CONFIG_LED_MATRIX_WIDTH / 2U);
                bitmapWidget.setHeight(CONFIG_LED_MATRIX_HEIGHT / 2U);
            }
            else if (2U == cnt)
            {
                bitmapWidget.move(0, CONFIG_LED_MATRIX_HEIGHT / 2U);
                bitmapWidget.setWidth(CONFIG_LED_MATRIX_WIDTH / 2U);
                bitmapWidget.setHeight(CONFIG_LED_MATRIX_HEIGHT / 2U);
            }
            else
            {
                bitmapWidget.move(CONFIG_LED_MATRIX_WIDTH / 2U, CONFIG_LED_MATRIX_HEIGHT / 2U);
                bitmapWidget.setWidth(CONFIG_LED_MATRIX_WIDTH / 2U);
                bitmapWidget.setHeight(CONFIG_LED_MATRIX_HEIGHT / 2U);
            }

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
