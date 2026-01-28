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
 * @file   View.cpp
 * @brief  MakapixPlugin view
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "View.h"

using namespace _MakapixPlugin;

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

void View::process()
{
    if ((true == m_actionTimer.isTimerRunning()) &&
        (true == m_actionTimer.isTimeout()))
    {
        clearActionIcon();
    }
}

void View::showActionIconNext()
{
    clearActionIcon();
    m_canvasWidget.drawLine(
        ACTION_ICON_POS_X,
        ACTION_ICON_POS_Y + ACTION_ICON_SIZE - 1U,
        ACTION_ICON_POS_X + ACTION_ICON_SIZE - 1U,
        ACTION_ICON_POS_Y + ACTION_ICON_SIZE / 2U,
        ColorDef::LIGHTGREEN);
    m_canvasWidget.drawLine(
        ACTION_ICON_POS_X + ACTION_ICON_SIZE - 1U,
        ACTION_ICON_POS_Y + ACTION_ICON_SIZE / 2U,
        ACTION_ICON_POS_X,
        ACTION_ICON_POS_Y,
        ColorDef::LIGHTGREEN);

    m_actionTimer.start(ACTION_ICON_DISPLAY_TIME);
}

void View::showActionIconPrev()
{
    clearActionIcon();
    m_canvasWidget.drawLine(
        ACTION_ICON_POS_X + ACTION_ICON_SIZE - 1U,
        ACTION_ICON_POS_Y + ACTION_ICON_SIZE - 1U,
        ACTION_ICON_POS_X,
        ACTION_ICON_POS_Y + ACTION_ICON_SIZE / 2U,
        ColorDef::LIGHTGREEN);
    m_canvasWidget.drawLine(
        ACTION_ICON_POS_X,
        ACTION_ICON_POS_Y + ACTION_ICON_SIZE / 2U,
        ACTION_ICON_POS_X + ACTION_ICON_SIZE - 1U,
        ACTION_ICON_POS_Y,
        ColorDef::LIGHTGREEN);

    m_actionTimer.start(ACTION_ICON_DISPLAY_TIME);
}

void View::showActionIconPlay()
{
    clearActionIcon();
    m_canvasWidget.drawVLine(
        ACTION_ICON_POS_X,
        ACTION_ICON_POS_Y,
        ACTION_ICON_SIZE,
        ColorDef::LIGHTGREEN);
    m_canvasWidget.drawLine(
        ACTION_ICON_POS_X,
        ACTION_ICON_POS_Y + ACTION_ICON_SIZE - 1U,
        ACTION_ICON_POS_X + ACTION_ICON_SIZE - 1U,
        ACTION_ICON_POS_Y + ACTION_ICON_SIZE / 2U,
        ColorDef::LIGHTGREEN);
    m_canvasWidget.drawLine(
        ACTION_ICON_POS_X + ACTION_ICON_SIZE - 1U,
        ACTION_ICON_POS_Y + ACTION_ICON_SIZE / 2U,
        ACTION_ICON_POS_X,
        ACTION_ICON_POS_Y,
        ColorDef::LIGHTGREEN);

    m_actionTimer.start(ACTION_ICON_DISPLAY_TIME);
}

void View::showActionIconPause()
{
    clearActionIcon();
    m_canvasWidget.drawVLine(
        ACTION_ICON_POS_X,
        ACTION_ICON_POS_Y,
        ACTION_ICON_SIZE,
        ColorDef::LIGHTGREEN);
    m_canvasWidget.drawVLine(
        ACTION_ICON_POS_X + ACTION_ICON_SIZE - 1U,
        ACTION_ICON_POS_Y,
        ACTION_ICON_SIZE,
        ColorDef::LIGHTGREEN);

    m_actionTimer.start(ACTION_ICON_DISPLAY_TIME);
}

void View::showActionIconFail()
{
    clearActionIcon();
    m_canvasWidget.drawLine(
        ACTION_ICON_POS_X,
        ACTION_ICON_POS_Y,
        ACTION_ICON_POS_X + ACTION_ICON_SIZE - 1U,
        ACTION_ICON_POS_Y + ACTION_ICON_SIZE - 1U,
        ColorDef::RED);
    m_canvasWidget.drawLine(
        ACTION_ICON_POS_X + ACTION_ICON_SIZE - 1U,
        ACTION_ICON_POS_Y,
        ACTION_ICON_POS_X,
        ACTION_ICON_POS_Y + ACTION_ICON_SIZE - 1U,
        ColorDef::RED);

    m_actionTimer.start(ACTION_ICON_DISPLAY_TIME);
}

void View::clearActionIcon()
{
    m_canvasWidget.fillRect(
        ACTION_ICON_POS_X,
        ACTION_ICON_POS_Y,
        ACTION_ICON_SIZE,
        ACTION_ICON_SIZE,
        ColorDef::BLACK);

    m_actionTimer.stop();
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
