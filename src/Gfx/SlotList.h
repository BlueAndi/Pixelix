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
 * @brief  Slot list
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup gfx
 *
 * @{
 */

#ifndef SLOT_LIST_H
#define SLOT_LIST_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include "Slot.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * This class handles a list of slots and provides access functionality by
 * the slot id.
 * 
 */
class SlotList
{
public:

    /**
     * Constructs a empty slot list.
     */
    SlotList() :
        m_maxSlots(0U),
        m_slots(nullptr),
        m_stickySlot(SLOT_ID_INVALID)
    {
    }

    /**
     * Destroys the slot list.
     * 
     */
    ~SlotList()
    {
        destroy();
    }

    /**
     * Constructs the slot list by copy of the given list.
     * 
     * @param[in] list  Slot list which shall be copied.
     */
    SlotList(const SlotList& list) :
        m_maxSlots(0U),
        m_slots(nullptr),
        m_stickySlot(SLOT_ID_INVALID)
    {
        *this = list;
    }

    /**
     * Assigns a slot list, which will recreate the slot list and copy the
     * given one.
     * 
     * @param[in] list  Slot list which shall be assigned.
     * 
     * @return Current slot list
     */
    SlotList& operator=(const SlotList& list);

    /**
     * Are slots available or not?
     * If slots are not available, create them first @see create().
     * 
     * @return If slots are available, it will return true otherwise false.
     */
    bool isAvailable() const
    {
        return (nullptr != m_slots);
    }

    /**
     * Checks whether the given slot id is valid or not.
     * If no slots are available, every slot id will be invalid.
     * 
     * @param[in] slotId    The slot id to check.
     * 
     * @return If slot id is valid, it will return true otherwise false.
     */
    bool isSlotIdValid(uint8_t slotId) const;

    /**
     * Create slots in the list.
     * This will allocate the slots depended on the given max number of slots.
     * 
     * @param[in] maxSlots  The maximum number of slots to create.
     * 
     * @return  If the slots were successful created, it will return true otherwise false.
     *          If slots already available, it will return false.
     */
    bool create(uint8_t maxSlots);

    /**
     * Destroys all slots in the list.
     */
    void destroy();

    /**
     * Get the maximum number of slots in the list.
     * 
     * @return  The maximum number of slots.
     *          If no slots are available, it will return 0.
     */
    uint8_t getMaxSlots() const
    {
        return m_maxSlots;
    }

    /**
     * Get plugin which is plugged into the slot.
     * 
     * @param[in] slotId    The id of the slot.
     *
     * @return Plugin
     */
    IPluginMaintenance* getPlugin(uint8_t slotId);

    /**
     * Set plugin to slot.
     * If slot is locked, unlock it first!
     * Remove plugin from slot, use nullptr as argument.
     *
     * @param[in] slotId    The id of the slot.
     * @param[in] plugin    Plugin to set.
     *
     * @return If successful it will return true otherwise false.
     */
    bool setPlugin(uint8_t slotId, IPluginMaintenance* plugin);

    /**
     * Get slot by the slot id.
     * 
     * @param[in] slotId    The id of the slot.
     
     * @return If slot is available, it will return it otherwise nullptr.
     */
    Slot* getSlot(uint8_t slotId);

    /**
     * Get id of a empty and unlocked slot.
     * 
     * @return If slot is found, it will return its id otherwise SLOT_ID_INVALID.
     */
    uint8_t getEmptyUnlockedSlot();

    /**
     * Is slot empty (no plugin plugged in)?
     * 
     * @param[in] slotId    The id of the slot to check.
     *
     * @return The empty status of the slot. If slot id is invalid, it will return false.
     */
    bool isSlotEmpty(uint8_t slotId) const;

    /**
     * Get duration in ms, how long the plugin shall be active.
     *
     * @param[in] slotId    The id of the slot.
     * 
     * @return Duration in ms
     */
    uint32_t getDuration(uint8_t slotId) const;

    /**
     * Set duration in ms, how long the plugin shall be active.
     *
     * @param[in] slotId    The id of the slot.
     * @param[in] duration  Duration in ms
     */
    void setDuration(uint8_t slotId, uint32_t duration);

    /**
     * Lock slot to protect the plugin against removing it.
     * 
     * @param[in] slotId    The id of the slot.
     */
    void lock(uint8_t slotId);

    /**
     * Unlock slot to be able to remove a plugin or plug a new one in.
     * 
     * @param[in] slotId    The id of the slot.
     */
    void unlock(uint8_t slotId);

    /**
     * Is slot locked?
     * 
     * @param[in] slotId    The id of the slot to check.
     *
     * @return The lock status of the slot. If slot id is invalid, it will return false.
     */
    bool isSlotLocked(uint8_t slotId) const;

    /**
     * Is slot empty (no plugin plugged in) and unlocked?
     * 
     * @param[in] slotId    The id of the slot to check.
     *
     * @return If slot is empty and unlocked, it will return true otherwise false. If slot id is invalid, it will return false.
     */
    bool isSlotEmptyAndUnlocked(uint8_t slotId);

    /**
     * Get the slot id by plugin UID.
     * 
     * @param[in] pluginUid The plugin UID which to search for.
     * 
     * @return If plugin slot is found, it will return its slot id otherwise SLOT_ID_INVALID.
     */
    uint8_t getSlotIdByPluginUID(uint16_t pluginUid);

    /**
     * Get slot which is marked sticky.
     * 
     * @return Id of sticky slot. If no slot is sticky, it will return SLOT_ID_INVALID.
     */
    uint8_t getStickySlot() const;

    /**
     * Set slot sticky. Only one slot can be sticky!
     * If a different slot is already sticky, the sticky flag will be moved.
     * 
     * If slot is empty or the plugin is disabled, it will fail.
     * 
     * Use SLOT_ID_INVALID to clear the sticky flag. Recommended: clearSticky()
     * 
     * @param[in]   slotId  The id of the slot which to set sticky.
     * 
     * @return If successful it will return true otherwise false.
     */
    bool setSlotSticky(uint8_t slotId);

    /**
     * Removes the sticky flag.
     */
    void clearSticky();

    /** Invalid slot id. */
    static const uint8_t    SLOT_ID_INVALID = UINT8_MAX;

private:

    uint8_t m_maxSlots;     /**< The maximum number of slots. */
    Slot*   m_slots;        /**< The slots itself. */
    uint8_t m_stickySlot;   /**< Only one slot can be sticky. If no slot is sticky, it contains a invalid id. */

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* SLOT_LIST_H */

/** @} */