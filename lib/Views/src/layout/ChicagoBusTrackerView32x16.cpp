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
 * @file   ChicagoBusTrackerView32x16.cpp
 * @brief  ChicagoBusTracker view for 32x16 LED matrix
 * @author Julie Hill <queenkjuul@pm.me>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "ChicagoBusTrackerView32x16.h"
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
 * |   Route Number      | |              Top Widget                 |
 * |   12x8              | |          Route Info if present.         |
 * |                     | |          first arrival if not.          |
 * |                     | |                 19x8                    |
 * |                     | |                                         |
 * +-----------------------------------------------------------------+
 * |                                                                 |
 * |                                                                 |
 * |                                                                 |
 * |                        Bottom Widget                            |
 * |           All arrivals if route info present, second if not     |
 * |                            32x8                                 |
 * |                                                                 |
 * +-----------------------------------------------------------------+
 */

/**
 * Route Number section width in pixels.
 */
static const uint16_t RTE_NUMBER_WIDTH   = 12U;

/**
 * Route Number section height in pixels.
 */
static const uint16_t RTE_NUMBER_HEIGHT  = 8U;

/**
 * Route Number x-coordinate in pixels.
 */
static const int16_t RTE_NUMBER_X        = 0;

/**
 * Route Number y-coordinate in pixels.
 */
static const int16_t RTE_NUMBER_Y        = 0;

/**
 * Route Info section width in pixels.
 */
static const int16_t RTE_INFO_WIDTH      = CONFIG_LED_MATRIX_WIDTH - RTE_NUMBER_WIDTH - 1U;

/**
 * Route Info section height in pixels.
 */
static const int16_t RTE_INFO_HEIGHT     = 8U;

/**
 * Route Info x-coordinate in pixels.
 */
static const int16_t RTE_INFO_X          = RTE_NUMBER_WIDTH + 1U;

/**
 * Route Info y-coordinate in pixels.
 */
static const int16_t RTE_INFO_Y          = 0;

/**
 * Text width in pixels.
 */
static const uint16_t ARR_SECTION_WIDTH  = CONFIG_LED_MATRIX_WIDTH;

/**
 * Text height in pixels.
 */
static const uint16_t ARR_SECTION_HEIGHT = 8U;

/**
 * Text widget x-coordinate in pixels.
 */
static const int16_t ARR_SECTION_X       = 0;

/**
 * Text widget y-coordinate in pixels.
 */
static const int16_t ARR_SECTION_Y       = 8U;

/******************************************************************************
 * Public Methods
 *****************************************************************************/

ChicagoBusTrackerView32x16::ChicagoBusTrackerView32x16() :
    IChicagoBusTrackerView(),
    m_fontType(Fonts::FONT_TYPE_DEFAULT),
    m_routeNumberWidget(RTE_NUMBER_WIDTH, RTE_NUMBER_HEIGHT, RTE_NUMBER_X, RTE_NUMBER_Y),
    m_topInfoWidget(RTE_INFO_WIDTH, RTE_INFO_HEIGHT, RTE_INFO_X, RTE_INFO_Y),
    m_bottomInfoWidget(ARR_SECTION_WIDTH, ARR_SECTION_HEIGHT, ARR_SECTION_X, ARR_SECTION_Y),
    m_routeNumberText(""),
    m_routeInfoText(""),
    m_firstArrivalText(""),
    m_secondArrivalText(""),
    m_thirdArrivalText("")
{
    m_routeNumberWidget.setVerticalAlignment(Alignment::Vertical::VERTICAL_CENTER);
    m_routeNumberWidget.setHorizontalAlignment(Alignment::Horizontal::HORIZONTAL_LEFT);
    m_routeNumberWidget.disableFadeEffect();

    m_topInfoWidget.setVerticalAlignment(Alignment::Vertical::VERTICAL_CENTER);
    m_topInfoWidget.setHorizontalAlignment(Alignment::Horizontal::HORIZONTAL_LEFT);
    m_topInfoWidget.disableFadeEffect();

    m_bottomInfoWidget.setVerticalAlignment(Alignment::Vertical::VERTICAL_CENTER);
    m_bottomInfoWidget.setHorizontalAlignment(Alignment::Horizontal::HORIZONTAL_RIGHT);
    m_bottomInfoWidget.disableFadeEffect();

    (void)m_routeNumberText.reserve(16U);
    (void)m_routeInfoText.reserve(128U);
    (void)m_arrivalsInfoText.reserve(64U);
    (void)m_firstArrivalText.reserve(18U);
    (void)m_secondArrivalText.reserve(18U);
    (void)m_thirdArrivalText.reserve(18U);

    m_routeNumberText  = " - ";
    m_firstArrivalText = " NO DATA ";
}

void ChicagoBusTrackerView32x16::update(YAGfx& gfx)
{
    gfx.fillScreen(ColorDef::BLACK);
    m_routeNumberWidget.update(gfx);
    m_topInfoWidget.update(gfx);
    m_bottomInfoWidget.update(gfx);
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
