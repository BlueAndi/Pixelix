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
 * @brief  Worm demo plugin
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup plugin
 *
 * @{
 */

#ifndef WORM_PLUGIN_H
#define WORM_PLUGIN_H

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
 * Worms are walking over the display to find some meal. If a worm eats a meal,
 * its length increases. If the worm eats too much, it will explode.
 * 
 * If a worm is blocked and can't move, its length decreases.
 * If all worms can not move anymore, they will explode.
 */
class WormPlugin : public Plugin
{
public:

    /**
     * Constructs the plugin.
     *
     * @param[in] name  Plugin name
     * @param[in] uid   Unique id
     */
    WormPlugin(const String& name, uint16_t uid) :
        Plugin(name, uid),
        m_wormMaxLen(0U),
        m_wormLen(),
        m_worms(nullptr),
        m_wormBodyColor(),
        m_meal(),
        m_mealLen(0U),
        m_timer(),
        m_isExplosion(false),
        m_explosionRadius(0U)
    {
    }

    /**
     * Destroys the plugin.
     */
    ~WormPlugin()
    {
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
        return new(std::nothrow)WormPlugin(name, uid);
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

    /**
     * Max. number of worms.
     */
    static const uint8_t    MAX_WORMS       = 4U;

    /**
     * Max. worm length in %.
     * It means the worm can have a X % length of the number of available display pixels.
     */
    static const uint8_t    MAX_WORM_LENGTH = 80U / MAX_WORMS;

    /**
     * Min. worm length of the head and one part of the body.
     */
    static const uint8_t    MIN_WORM_LENGTH = 2U;

    /**
     * Max. number of meals wich are placed at once.
     */
    static const uint8_t    MAX_MEALS       = 6U;

    /**
     * Worm velocity in ms to move one pixel forward.
     */
    static const uint32_t   WORM_VELOCITY   = 200U;

    /**
     * Color of the worm head is the same for all worms.
     */
    static const Color      WORM_HEAD_COLOR;

    /**
     * Color of a meal.
     */
    static const Color      MEAL_COLOR;

    /**
     * Pixel position on the display.
     */
    struct Pos
    {
        int16_t x;  /**< x-coordinate */
        int16_t y;  /**< y-coordinate */
    };

    size_t          m_wormMaxLen;               /**< The max. length a worm can have. */
    size_t          m_wormLen[MAX_WORMS];       /**< The length of each worm. */
    Pos*            m_worms;                    /**< The worms itself and their head and body positions. */
    Color           m_wormBodyColor[MAX_WORMS]; /**< The colors of each worm body. */
    Pos             m_meal[MAX_MEALS];          /**< The meals, a worm can eat. */
    uint8_t         m_mealLen;                  /**< Number of current available meals. */
    SimpleTimer     m_timer;                    /**< Timer used for display update. */
    bool            m_isExplosion;              /**< Explosion of a worm happended? */
    uint16_t        m_explosionRadius;          /**< Explosion radius, used for explosion animation. */

    /**
     * Place some meal random on the display.
     * 
     * @param[in] width     Display width in pixel
     * @param[in] height    Display height in pixel
     */
    void placeMealRandom(uint16_t width, uint16_t height);

    /**
     * A worm eat the meal. Remove it from the available ones.
     * 
     * @param[in]       width     Display width in pixel
     * @param[in]       height    Display height in pixel
     * @param[in,out]   mealIndex Index of the meal, which to remove.
     */
    void eatMeal(uint16_t width, uint16_t height, uint8_t mealIndex);

    /**
     * Get the worm position in the array by worm id.
     * 
     * @param[in] wormId    The worm id.
     * 
     * @return Position in worm array
     */
    size_t inline wormPosInArray(uint8_t wormId)
    {
        return wormId * m_wormMaxLen;
    }

    /**
     * Create a new worm (head + one part of the body).
     * 
     * @param[in]   wormId  The id of the worm which to create.
     * @param[in]   width   Display width in pixel
     * @param[in]   height  Display height in pixel
     */
    void createWorm(uint8_t wormId, uint16_t width, uint16_t height);

    /**
     * Create all new worms (head + one part of the body).
     * 
     * @param[in]   width   Display width in pixel
     * @param[in]   height  Display height in pixel
     */
    void createAllWorms(uint16_t width, uint16_t height);

    /**
     * Destroy a worm.
     * 
     * @param[in] wormId    The id of the worm which to destroy.
     */
    void destroyWorm(uint8_t wormId);

    /**
     * Is there a collision with any other worm?
     * 
     * @param[in]   pos Position which to check
     * 
     * @return If no collision, it will return false otherwise true.
     */
    bool isCollision(const Pos& pos);

    /**
     * Is meal found at the given position?
     * 
     * @param[in]   pos     The position which to check for meal.
     * @param[out]  idx     The index in the meal array, which was found.
     *
     * @return If meal found, it will return true otherwise false.
     */
    bool isMealFound(const Pos& pos, uint8_t& idx);

    /**
     * Move worm random.
     * 
     * @param[in]   wormId  The id of the worm which to move.
     * @param[in]   width   The display width in pixel.
     * @param[in]   height  The display height in pixel.
     * 
     * @return If movement is successful, it will return true otherwise false.
     */
    bool moveWormRandom(uint8_t wormId, uint16_t width, uint16_t height);

    /**
     * Move all worms random.
     * 
     * @param[in]   width   The display width in pixel.
     * @param[in]   height  The display height in pixel.
     * 
     * @return If movement of at least one worm is successful, it will return true otherwise false.
     */
    bool moveAllWormsRandom(uint16_t width, uint16_t height);

    /**
     * Draw the worm on the display.
     * 
     * @param[in] wormId    The id of the worm which to draw.
     * @param[in] gfx       Graphic functions
     */
    void drawWorm(uint8_t wormId, YAGfx& gfx);

    /**
     * Draw all worms on the display.
     * 
     * @param[in] gfx       Graphic functions
     */
    void drawAllWorms(YAGfx& gfx);

    /**
     * Draw the meal on the display.
     * 
     * @param[in] gfx       Graphic functions
     */
    void drawMeal(YAGfx& gfx);

    /**
     * Draw the explosion on the display.
     * 
     * @param[in] gfx       Graphic functions
     */
    void drawExplosion(YAGfx& gfx);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* WORM_PLUGIN_H */

/** @} */