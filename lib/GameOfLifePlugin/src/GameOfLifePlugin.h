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
 *
 * @addtogroup plugin
 *
 * @{
 */

#ifndef GAMEOFLIFEPLUGIN_H
#define GAMEOFLIFEPLUGIN_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include "Plugin.hpp"
#include <SimpleTimer.hpp>

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * Conways Game of Life plugin.
 *
 * The universe of the Game of Life is an infinite, two-dimensional orthogonal
 * grid of square cells, each of which is in one of two possible states, alive
 * or dead, (or populated and unpopulated, respectively). Every cell interacts
 * with its eight neighbours, which are the cells that are horizontally,
 * vertically, or diagonally adjacent. At each step in time, the following
 * transitions occur:
 * 1. Any live cell with fewer than two live neighbours dies, as if by underpopulation.
 * 2. Any live cell with two or three live neighbours lives on to the next generation.
 * 3. Any live cell with more than three live neighbours dies, as if by overpopulation.
 * 4. Any dead cell with exactly three live neighbours becomes a live cell, as if by reproduction.
 *
 * See https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life
 */
class GameOfLifePlugin : public Plugin
{
public:

    /**
     * Constructs the plugin.
     *
     * @param[in] name  Plugin name
     * @param[in] uid   Unique id
     */
    GameOfLifePlugin(const String& name, uint16_t uid) :
        Plugin(name, uid),
        m_activeGrid(0U),
        m_gridSize(0U),
        m_grids(),
        m_width(0U),
        m_height(0U),
        m_displayTimer(),
        m_restartTimer(),
        m_forceRestartTimer()
    {
        uint8_t index = 0U;

        while(GRIDS > index)
        {
            m_grids[index] = nullptr;
            ++index;
        }
    }

    /**
     * Destroys the plugin.
     */
    ~GameOfLifePlugin()
    {
        destroyGrids();
    }

    /**
     * Plugin creation method, used to register on the plugin manager.
     *
     * @param[in] name  Plugin name
     * @param[in] uid   Unique id
     *
     * @return If successful, it will return the pointer to the plugin instance, otherwise nullptr.
     */
    static IPluginMaintenance* create(const String& name, uint16_t uid)
    {
        return new(std::nothrow)GameOfLifePlugin(name, uid);
    }

    /**
     * Start the plugin. This is called only once during plugin lifetime.
     * It can be used as deferred initialization (after the constructor)
     * and provides the canvas size.
     * 
     * If your display layout depends on canvas or font size, calculate it
     * here.
     * 
     * Overwrite it if your plugin needs to know that it was installed.
     * 
     * @param[in] width     Display width in pixel
     * @param[in] height    Display height in pixel
     */
    void start(uint16_t width, uint16_t height) final;
    
   /**
     * Stop the plugin. This is called only once during plugin lifetime.
     * It can be used as a first clean-up, before the plugin will be destroyed.
     * 
     * Overwrite it if your plugin needs to know that it will be uninstalled.
     */
    void stop() final;

    /**
     * This method will be called in case the plugin is set active, which means
     * it will be shown on the display in the next step.
     *
     * @param[in] gfx   Display graphics interface
     */
    void active(YAGfx& gfx) final;

    /**
     * This method will be called in case the plugin is set inactive, which means
     * it won't be shown on the display anymore.
     */
    void inactive() final;

    /**
     * Update the display.
     * The scheduler will call this method periodically.
     *
     * @param[in] gfx   Display graphics interface
     */
    void update(YAGfx& gfx) final;

private:

    /** Number of grids */
    static const uint8_t    GRIDS                   = 2U;

    /** Bits per grid data, derived from uint32_t. */
    static const uint8_t    BITS                    = 32U;

    /** Display update period in ms */
    static const uint32_t   DISPLAY_PERIOD          = 250U;

    /** Restart period in ms after grid is stable. */
    static const uint32_t   RESTART_PERIOD          = SIMPLE_TIMER_SECONDS(1U);

    /** Force restart period in ms. */
    static const uint32_t   FORCE_RESTART_PERIOD    = SIMPLE_TIMER_SECONDS(10U);

    uint8_t     m_activeGrid;           /**< Current active grid */
    uint32_t    m_gridSize;             /**< Size of one grid in number of elements */
    uint32_t*   m_grids[GRIDS];         /**< Two grids as playfields. */
    uint16_t    m_width;                /**< Grid width */
    uint16_t    m_height;               /**< Grid height */
    SimpleTimer m_displayTimer;         /**< Timer, used for cyclic display update. */
    SimpleTimer m_restartTimer;         /**< Timer, used to restart the whole game of life if grid is stable. */
    SimpleTimer m_forceRestartTimer;    /**< Timer, used to force a restart of the whole game of life. */

    /**
     * Create all grids.
     *
     * @return If successful, it will return true otherwise false.
     */
    bool createGrids();

    /**
     * Destroys all grids.
     */
    void destroyGrids();

    /**
     * Generate a random initial pattern.
     *
     * @param[in] gridId    Id of grid, where to generate.
     */
    void generateInitialPattern(uint8_t gridId);

    /**
     * Get cell state.
     *
     * @param[in] gridId    Id of grid, where the cell lives
     * @param[in] x         x-coordinate of cell
     * @param[in] y         y-coordinate of cell
     *
     * @return Alive (true) or dead (false).
     */
    bool getCellState(uint8_t gridId, int16_t x, int16_t y);

    /**
     * Set cell state.
     *
     * @param[in] gridId    Id of grid, where the cell lives
     * @param[in] x         x-coordinate of cell
     * @param[in] y         y-coordinate of cell
     * @param[in] state     Alive (true) or dead (false).
     */
    void setCellState(uint8_t gridId, int16_t x, int16_t y, bool state);

    /**
     * Count all alive neighbours of cell.
     *
     * @param[in] gridId    Id of grid, where the cell lives
     * @param[in] x         x-coordinate of cell
     * @param[in] y         y-coordinate of cell
     *
     * @return Number of alive neighbours.
     */
    uint8_t countAliveNeighbours(uint8_t gridId, int16_t x, int16_t y);

    /**
     * Update the display with the grid.
     *
     * @param[in] gfx       Graphics interface
     * @param[in] gridId    Grid id
     */
    void update(YAGfx& gfx, uint8_t gridId);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* GAMEOFLIFEPLUGIN_H */

/** @} */