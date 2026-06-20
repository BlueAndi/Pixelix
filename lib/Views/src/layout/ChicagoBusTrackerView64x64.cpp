/* MIT License
 *
 * Copyright (c) 2019 - 2026 Andreas Merkle <web@blue-andi.de>
 * Copyright (c) 2026        Julie Hill <queenkjuul@pm.me>
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
 * @file   ChicagoBusTrackerView64x64.cpp
 * @brief  ChicagoBusTracker view for 64x64 LED matrix
 * @author Julie Hill <queenkjuul@pm.me>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "ChicagoBusTrackerView64x64.h"
#include <FileSystem.h>

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

/* Layout
 *
 * +-----------------------------------------------------------------+
 * |                     | |                                         |
 * |   Route Number      | |              Widget 1                   |
 * |   12x8              | |          Route Info if present.         |
 * |                     | |          first arrival if not.          |
 * |                     | |                 51x8                    |
 * |                     | |                                         |
 * +-----------------------------------------------------------------+
 * |                         Widget 2                                |
 * |                     First or Second Arrival                     |
 * |                          64x8                                   |
 * +-----------------------------------------------------------------+
 * |                         Widget 3                                |
 * |                     Second or Third Arrival                     |
 * |                          64x8                                   |
 * +-----------------------------------------------------------------+
 * |                        Widget 4                                 |
 * |               Blank or third arrival if route info present      |
 * |                            64x8                                 |
 * +-----------------------------------------------------------------+
 */

/**
 * Route Number section width in pixels.
 */
static const uint16_t RTE_NUMBER_WIDTH  = 12U;

/**
 * Route Number section height in pixels.
 */
static const uint16_t RTE_NUMBER_HEIGHT = 8U;

/**
 * Route Number x-coordinate in pixels.
 */
static const int16_t RTE_NUMBER_X       = 0;

/**
 * Route Number y-coordinate in pixels.
 */
static const int16_t RTE_NUMBER_Y       = 0;

/**
 * Route Info section width in pixels.
 */
static const int16_t WIDGET_1_WIDTH     = CONFIG_LED_MATRIX_WIDTH - RTE_NUMBER_WIDTH - 1U;

/**
 * Route Info section height in pixels.
 */
static const int16_t WIDGET_HEIGHT      = 8U;

/**
 * Route Info x-coordinate in pixels.
 */
static const int16_t WIDGET_1_X         = RTE_NUMBER_WIDTH + 1U;

/**
 * Widget 1 y-coordinate in pixels.
 */
static const int16_t WIDGET_1_Y         = 0;

/**
 * Text width in pixels.
 */
static const uint16_t DISPLAY_WIDTH     = CONFIG_LED_MATRIX_WIDTH;

/**
 * Text widget x-coordinate in pixels.
 */
static const int16_t WIDGET_X           = 0;

/**
 * Widget 2 y-coordinate in pixels.
 */
static const int16_t WIDGET_2_Y         = 8U;

/**
 * Widget 3 y-coordinate in pixels
 */
static const int16_t WIDGET_3_Y         = 16U;

/**
 * Widget 4 y-coordinate in pixels
 */
static const int16_t WIDGET_4_Y         = 24U;

/******************************************************************************
 * Public Methods
 *****************************************************************************/

ChicagoBusTrackerView64x64::ChicagoBusTrackerView64x64() :
    IChicagoBusTrackerView(),
    m_fontType(Fonts::FONT_TYPE_DEFAULT),
    m_routeNumberWidget(RTE_NUMBER_WIDTH, RTE_NUMBER_HEIGHT, RTE_NUMBER_X, RTE_NUMBER_Y),
    m_infoWidget1(WIDGET_1_WIDTH, WIDGET_HEIGHT, WIDGET_1_X, WIDGET_1_Y),
    m_infoWidget2(DISPLAY_WIDTH, WIDGET_HEIGHT, WIDGET_X, WIDGET_2_Y),
    m_infoWidget3(DISPLAY_WIDTH, WIDGET_HEIGHT, WIDGET_X, WIDGET_3_Y),
    m_infoWidget4(DISPLAY_WIDTH, WIDGET_HEIGHT, WIDGET_X, WIDGET_4_Y),
    m_routeNumberText(" - "),
    m_routeInfoText(""),
    m_firstArrivalText(" NO DATA "),
    m_secondArrivalText(""),
    m_thirdArrivalText("")
{
    m_routeNumberWidget.setVerticalAlignment(Alignment::Vertical::VERTICAL_CENTER);
    m_routeNumberWidget.setHorizontalAlignment(Alignment::Horizontal::HORIZONTAL_LEFT);

    m_infoWidget1.setVerticalAlignment(Alignment::Vertical::VERTICAL_CENTER);
    m_infoWidget1.setHorizontalAlignment(Alignment::Horizontal::HORIZONTAL_RIGHT);

    m_infoWidget2.setVerticalAlignment(Alignment::Vertical::VERTICAL_CENTER);
    m_infoWidget2.setHorizontalAlignment(Alignment::Horizontal::HORIZONTAL_RIGHT);

    m_infoWidget3.setVerticalAlignment(Alignment::Vertical::VERTICAL_CENTER);
    m_infoWidget3.setHorizontalAlignment(Alignment::Horizontal::HORIZONTAL_RIGHT);

    m_infoWidget4.setVerticalAlignment(Alignment::Vertical::VERTICAL_CENTER);
    m_infoWidget4.setHorizontalAlignment(Alignment::Horizontal::HORIZONTAL_RIGHT);
}

void ChicagoBusTrackerView64x64::update(YAGfx& gfx)
{
    gfx.fillScreen(ColorDef::BLACK);
    m_routeNumberWidget.update(gfx);
    m_infoWidget1.update(gfx);
    m_infoWidget2.update(gfx);
    m_infoWidget3.update(gfx);
    m_infoWidget4.update(gfx);
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
