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
 * @file   ChicagoBusTrackerView32x8.cpp
 * @brief  ChicagoBusTracker view for 32x8 LED matrix
 * @author Julie Hill <queenkjuul@pm.me>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "ChicagoBusTrackerView32x8.h"
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
 * |                     | |                                         |
 * |                     | |                                         |
 * |   Route Info        | |              Arrivals Info              |
 * |   12x8              | |                 19x8                    |
 * |                     | |                                         |
 * |                     | |                                         |
 * |                     | |                                         |
 * +-----------------------------------------------------------------+
 */

/**
 * Route Info section width in pixels.
 */
static const uint16_t RTE_SECTION_WIDTH  = 12U;

/**
 * Route Info section height in pixels.
 */
static const uint16_t RTE_SECTION_HEIGHT = 8U;

/**
 * Text widget x-coordinate in pixels.
 * Left aligned, after icon.
 */
static const int16_t RTE_SECTION_X       = 0;

/**
 * Text widget y-coordinate in pixels.
 */
static const int16_t RTE_SECTION_Y       = 0;

/**
 * Text width in pixels.
 */
static const uint16_t ARR_SECTION_WIDTH  = CONFIG_LED_MATRIX_WIDTH - RTE_SECTION_WIDTH - 1U;

/**
 * Text height in pixels.
 */
static const uint16_t ARR_SECTION_HEIGHT = 8U;

/**
 * Text widget x-coordinate in pixels.
 * Left aligned, after icon.
 */
static const int16_t ARR_SECTION_X       = RTE_SECTION_WIDTH + 1;

/**
 * Text widget y-coordinate in pixels.
 */
static const int16_t ARR_SECTION_Y       = 0;

/******************************************************************************
 * Public Methods
 *****************************************************************************/

ChicagoBusTrackerView32x8::ChicagoBusTrackerView32x8() :
    IChicagoBusTrackerView(),
    m_fontType(Fonts::FONT_TYPE_DEFAULT),
    m_routeWidget(RTE_SECTION_WIDTH, RTE_SECTION_HEIGHT, RTE_SECTION_X, RTE_SECTION_Y),
    m_routeScroll(m_routeWidget),
    m_arrivalsWidget(ARR_SECTION_WIDTH, ARR_SECTION_HEIGHT, ARR_SECTION_X, ARR_SECTION_Y),
    m_arrivalsScroll(m_arrivalsWidget),
    m_routeNumberText(""),
    m_routeInfoText(""),
    m_firstArrivalText(""),
    m_secondArrivalText(""),
    m_thirdArrivalText(""),
    m_arrivalsInfoText("")
{
    m_routeWidget.setVerticalAlignment(Alignment::Vertical::VERTICAL_CENTER);
    m_routeWidget.setHorizontalAlignment(Alignment::Horizontal::HORIZONTAL_LEFT);

    m_arrivalsWidget.setVerticalAlignment(Alignment::Vertical::VERTICAL_CENTER);
    m_arrivalsWidget.setHorizontalAlignment(Alignment::Horizontal::HORIZONTAL_LEFT);

    (void)m_routeNumberText.reserve(16U);
    (void)m_routeInfoText.reserve(128U);
    (void)m_arrivalsInfoText.reserve(64U);
    (void)m_firstArrivalText.reserve(18U);
    (void)m_secondArrivalText.reserve(18U);
    (void)m_thirdArrivalText.reserve(18U);

    m_routeNumberText  = " - ";
    m_firstArrivalText = " NO DATA ";
}

void ChicagoBusTrackerView32x8::update(YAGfx& gfx)
{
    gfx.fillScreen(ColorDef::BLACK);
    m_routeScroll.update(gfx, m_routeWidget);
    m_arrivalsScroll.update(gfx, m_arrivalsWidget);
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
