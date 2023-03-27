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
 * @brief  Conways Game of Life plugin
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "GameOfLifePlugin.h"

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

void GameOfLifePlugin::start(uint16_t width, uint16_t height)
{
    m_width     = width;
    m_height    = height;
    m_gridSize  = ((m_width * m_height) + (BITS - 1U)) / BITS;

    (void)createGrids();
}

void GameOfLifePlugin::stop()
{
    destroyGrids();
}

void GameOfLifePlugin::active(YAGfx& gfx)
{
    uint8_t index   = 0U;
    bool    isInit  = true;

    while((GRIDS > index) && (true == isInit))
    {
        if (nullptr == m_grids[index])
        {
            isInit = false;
        }

        ++index;
    }

    if (true == isInit)
    {
        /* It may happen that the slot duration is lower than the force restart period.
         * To avoid that the game of life doesn't change anymore, a new pattern shall
         * be generated every time the plugin is activated.
         */
        generateInitialPattern(m_activeGrid);
    }

    /* Show generated initial cell grid. */
    gfx.fillScreen(ColorDef::BLACK);
    update(gfx, m_activeGrid);

    m_displayTimer.start(DISPLAY_PERIOD);
    m_forceRestartTimer.start(FORCE_RESTART_PERIOD);
}

void GameOfLifePlugin::inactive()
{
    m_forceRestartTimer.stop();
    m_restartTimer.stop();
    m_displayTimer.stop();
}

void GameOfLifePlugin::update(YAGfx& gfx)
{
    uint8_t index   = 0U;
    bool    isInit  = true;

    /* Check whether grid is initialized or not. */
    while((GRIDS > index) && (true == isInit))
    {
        if (nullptr == m_grids[index])
        {
            isInit = false;
        }

        ++index;
    }

    /* Grid initialized? */
    if (false == isInit)
    {
        /* Not initialized, do nothing. */
        ;
    }
    /* Force a restart? */
    else if ((true == m_forceRestartTimer.isTimerRunning()) &&
             (true == m_forceRestartTimer.isTimeout()))
    {
        generateInitialPattern(m_activeGrid);
        m_forceRestartTimer.restart();
        m_restartTimer.stop();
    }
    /* If the grid is stable, keep it for a while and then restart. */
    else if ((true == m_restartTimer.isTimerRunning()) &&
             (true == m_restartTimer.isTimeout()))
    {
        generateInitialPattern(m_activeGrid);
        m_forceRestartTimer.restart();
        m_restartTimer.stop();
    }

    /* Let's play the game of life. */
    if ((true == isInit) &&
        (true == m_displayTimer.isTimeout()))
    {
        int32_t cellX           = 0;
        int32_t cellY           = 0;
        uint8_t inactiveGrid    = (m_activeGrid + 1U) % GRIDS;
        bool    isStable        = true;

        /* Note: The active grid is the one, where we look how the current state of
         * every cell is. This is the grid, which is shown on the display right now.
         * The next time cycle of the game will be drawn now on the inactive grid,
         * which will be then shown at the end of this routine.
         *
         * After that the active grid will be inactive and vice versa.
         */

        /* Handle active grid */
        for(cellY = 0; cellY < m_height; ++cellY)
        {
            for(cellX = 0; cellX < m_width; ++cellX)
            {
                bool    cellState       = getCellState(m_activeGrid, cellX, cellY);
                uint8_t neighboursAlive = countAliveNeighbours(m_activeGrid, cellX, cellY);
                bool    newState        = false;

                /* Rules:
                 * 1. Any live cell with fewer than two live neighbours dies, as if by underpopulation.
                 * 2. Any live cell with two or three live neighbours lives on to the next generation.
                 * 3. Any live cell with more than three live neighbours dies, as if by overpopulation.
                 * 4. Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
                 */

                /* Dead? */
                if (false == cellState)
                {
                    /* Rule 4: Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction. */
                    if (3U == neighboursAlive)
                    {
                        newState = true;
                    }
                    /* Cell stays dead. */
                    else
                    {
                        newState = false;
                    }
                }
                /* Alive */
                else
                {
                    /* Rule 1: Any live cell with fewer than two live neighbours dies, as if by underpopulation. */
                    if (2U > neighboursAlive)
                    {
                        newState = false;
                    }
                    /* Rule 2: Any live cell with two or three live neighbours lives on to the next generation. */
                    else if (4U > neighboursAlive)
                    {
                        newState = true;
                    }
                    /* Rule 3: Any live cell with more than three live neighbours dies, as if by overpopulation. */
                    else
                    {
                        newState = false;
                    }
                }

                setCellState(inactiveGrid, cellX, cellY, newState);

                if (cellState != newState)
                {
                    isStable = false;
                }
            }
        }

        update(gfx, inactiveGrid);

        /* If grid is stable, restart game after a period. */
        if ((true == isStable) &&
            (false == m_restartTimer.isTimerRunning()))
        {
            m_restartTimer.start(RESTART_PERIOD);
        }

        /* Activate next grid */
        ++m_activeGrid;
        m_activeGrid %= GRIDS;

        m_displayTimer.restart();
    }
    else
    {
        /* Nothing to do. */
        ;
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

bool GameOfLifePlugin::createGrids()
{
    bool    status  = true;
    uint8_t index   = 0U;

    while((GRIDS > index) && (true == status))
    {
        m_grids[index] = new(std::nothrow) uint32_t[m_gridSize];

        if (nullptr == m_grids[index])
        {
            status = false;
        }

        ++index;
    }

    if (false == status)
    {
        destroyGrids();
    }

    return status;
}

void GameOfLifePlugin::destroyGrids()
{
    uint8_t index = 0U;

    while(GRIDS > index)
    {
        if (nullptr != m_grids[index])
        {
            delete[] m_grids[index];
            m_grids[index] = nullptr;
        }

        ++index;
    }
}

void GameOfLifePlugin::generateInitialPattern(uint8_t gridId)
{
    uint32_t gridDataIndex = 0U;

    randomSeed(ESP.getCycleCount());

    while(m_gridSize > gridDataIndex)
    {
        m_grids[gridId][gridDataIndex] = random(INT32_MAX);
        m_grids[gridId][gridDataIndex] |= (0 == random(2)) ? 0x00000000 : 0x80000000;

        ++gridDataIndex;
    }
}

bool GameOfLifePlugin::getCellState(uint8_t gridId, int16_t x, int16_t y)
{
    bool isAlive = false;

    if (0 > x)
    {
        x = m_width + x;
    }
    else if (m_width <= x)
    {
        x -= m_width;
    }
    else
    {
        ;
    }

    if (0 > y)
    {
        y = m_height + y;
    }
    else if (m_height <= y)
    {
        y -= m_height;
    }
    else
    {
        ;
    }

    if ((0 <= x) &&
        (0 <= y) &&
        (m_width > x) &&
        (m_height > y))
    {
        uint32_t    cellIndex       = x + m_width * y;
        uint32_t    cellDataIndex   = cellIndex / BITS;
        uint8_t     bitIndex        = cellIndex - (cellDataIndex * BITS);

        isAlive = (0 != (m_grids[gridId][cellDataIndex] & (1 << bitIndex)));
    }

    return isAlive;
}

void GameOfLifePlugin::setCellState(uint8_t gridId, int16_t x, int16_t y, bool state)
{
    if (0 > x)
    {
        x = m_width + x;
    }
    else if (m_width <= x)
    {
        x -= m_width;
    }
    else
    {
        ;
    }

    if (0 > y)
    {
        y = m_height + y;
    }
    else if (m_height <= y)
    {
        y -= m_height;
    }
    else
    {
        ;
    }

    if ((0 <= x) &&
        (0 <= y) &&
        (m_width > x) &&
        (m_height > y))
    {
        uint32_t    cellIndex       = x + m_width * y;
        uint32_t    cellDataIndex   = cellIndex / BITS;
        uint8_t     bitIndex        = cellIndex - (cellDataIndex * BITS);

        if (false == state)
        {
            m_grids[gridId][cellDataIndex] &= ~(1 << bitIndex);
        }
        else
        {
            m_grids[gridId][cellDataIndex] |= 1 << bitIndex;
        }
    }
}

uint8_t GameOfLifePlugin::countAliveNeighbours(uint8_t gridId, int16_t x, int16_t y)
{
    const int16_t   MAX_X   = 3;    /* 3 cells per row */
    const int16_t   MAX_Y   = 3;    /* 3 cells per column */
    uint8_t         count   = 0U;
    int16_t         dX      = 0;
    int16_t         dY      = 0;

    /* Cell neighbours, marked with a '#':
     * ###
     * #x#
     * ###
     */

    for(dY = 0; dY < MAX_Y; ++dY)
    {
        int16_t neighbourY = (y - 1) + dY;

        for(dX = 0; dX < MAX_X; ++dX)
        {
            int16_t neighbourX = (x - 1) + dX;

            if ((x != neighbourX) ||
                (y != neighbourY))
            {
                if (true == getCellState(gridId, neighbourX, neighbourY))
                {
                    ++count;
                }
            }
        }
    }

    return count;
}

void GameOfLifePlugin::update(YAGfx& gfx, uint8_t gridId)
{
    int16_t x   = 0;
    int16_t y   = 0;

    for(y = 0; y < m_height; ++y)
    {
        for(x = 0; x < m_width; ++x)
        {
            if (false == getCellState(gridId, x, y))
            {
                gfx.drawPixel(x, y, ColorDef::BLACK);
            }
            else
            {
                gfx.drawPixel(x, y, ColorDef::BLUE);
            }
        }
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
