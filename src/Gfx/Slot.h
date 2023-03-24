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
 * @brief  Slot
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup gfx
 *
 * @{
 */

#ifndef SLOT_H
#define SLOT_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include "IPluginMaintenance.hpp"
#include "ISlotPlugin.hpp"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * A slot, where a plugin can be plugged in or removed.
 */
class Slot : public ISlotPlugin
{
public:

    /**
     * Constructs a slot.
     */
    Slot();

    /**
     * Destroys slot.
     */
    ~Slot();

    /**
     * Constructs a slot by copy.
     * 
     * @param[in] slot  The slot which to copy.
     */
    Slot(const Slot& slot);

    /**
     * Assign a slot.
     * 
     * @param[in] slot  The slot which to assign.
     */
    Slot& operator=(const Slot& slot);

    /**
     * Get plugin which is plugged in.
     *
     * @return Plugin
     */
    IPluginMaintenance* getPlugin();

    /**
     * Set plugin to slot.
     * If slot is locked, unlock it first!
     * Remove plugin from slot, use nullptr as argument.
     *
     * @param[in] plugin Plugin
     *
     * @return If successful it will return true otherwise false.
     */
    bool setPlugin(IPluginMaintenance* plugin);

    /**
     * Is slot empty?
     *
     * @return If slot is empty, it will return true otherwise false.
     */
    bool isEmpty() const;

    /**
     * Get duration in ms, how long the plugin shall be active.
     *
     * @return Duration in ms
     */
    uint32_t getDuration() const final;

    /**
     * Set duration in ms, how long the plugin shall be active.
     *
     * @param[in] duration Duration in ms
     */
    void setDuration(uint32_t duration);

    /**
     * Lock slot to protect the plugin against removing it.
     */
    void lock();

    /**
     * Unlock slot to be able to remove a plugin or plug a new one in.
     */
    void unlock();

    /**
     * Is slot locked?
     *
     * @return If slot is locked, it will return true otherwise false.
     */
    bool isLocked() const;

    /** Default duration in ms */
    static const uint32_t DURATION_DEFAULT  = 30000U;

private:

    IPluginMaintenance* m_plugin;   /**< Plugged in slot */
    uint32_t            m_duration; /**< Duration in ms, how long the plugin shall be active. */
    bool                m_isLocked; /**< Is slot locked or not. */
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* SLOT_H */

/** @} */