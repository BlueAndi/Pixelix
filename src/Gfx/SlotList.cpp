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
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "SlotList.h"

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

SlotList& SlotList::operator=(const SlotList& list)
{
    if (this != (&list))
    {
        if (m_maxSlots != list.m_maxSlots)
        {
            destroy();
            (void)create(list.m_maxSlots);
        }

        if (nullptr != m_slots)
        {
            uint8_t idx = 0U;

            for(idx = 0U; idx < m_maxSlots; ++idx)
            {
                m_slots[idx] = list.m_slots[idx];
            }

            m_stickySlot = list.m_stickySlot;
        }
    }

    return *this;
}

bool SlotList::isSlotIdValid(uint8_t slotId) const
{
    bool isValid = false;

    /* As long as no slots are allocated, every slot id is invalid. */
    if (nullptr != m_slots)
    {
        /* The slot id corresponds to the slot array index. */
        if (m_maxSlots > slotId)
        {
            isValid = true;
        }
    }

    return isValid;
}

bool SlotList::create(uint8_t maxSlots)
{
    bool isSuccessful = false;

    if ((nullptr == m_slots) &&
        (0 < maxSlots) &&
        (SLOT_ID_INVALID > maxSlots))
    {
        m_slots = new(std::nothrow) Slot[maxSlots];

        if (nullptr != m_slots)
        {
            m_maxSlots = maxSlots;

            isSuccessful = true;
        }
    }

    return isSuccessful;
}

void SlotList::destroy()
{
    if (nullptr != m_slots)
    {
        delete[] m_slots;
        
        m_slots         = nullptr;
        m_maxSlots      = 0U;
        m_stickySlot    = SLOT_ID_INVALID;
    }
}

IPluginMaintenance* SlotList::getPlugin(uint8_t slotId)
{
    IPluginMaintenance* plugin = nullptr;

    if (true == isSlotIdValid(slotId))
    {
        plugin = m_slots[slotId].getPlugin();
    }

    return plugin;
}

bool SlotList::setPlugin(uint8_t slotId, IPluginMaintenance* plugin)
{
    bool isSuccessful = false;

    if (true == isSlotIdValid(slotId))
    {
        isSuccessful = m_slots[slotId].setPlugin(plugin);

        /* If plugin is removed from sticky slot, the sticky flag will be removed. */
        if ((true == isSuccessful) &&
            (nullptr == plugin) &&
            (m_stickySlot == slotId))
        {
            m_stickySlot = SLOT_ID_INVALID;
        }

        isSuccessful = true;
    }

    return isSuccessful;
}

Slot* SlotList::getSlot(uint8_t slotId)
{
    Slot* slot = nullptr;

    if (true == isSlotIdValid(slotId))
    {
        slot = &m_slots[slotId];
    }

    return slot;
}

uint8_t SlotList::getEmptyUnlockedSlot()
{
    uint8_t slotId = SLOT_ID_INVALID;

    if (nullptr != m_slots)
    {
        slotId = 0U;

        while((m_maxSlots > slotId) && (false == isSlotEmptyAndUnlocked(slotId)))
        {
            ++slotId;
        }

        if (m_maxSlots <= slotId)
        {
            slotId = SLOT_ID_INVALID;
        }
    }

    return slotId;
}

bool SlotList::isSlotEmpty(uint8_t slotId) const
{
    bool isEmpty = false;

    if (true == isSlotIdValid(slotId))
    {
        isEmpty = m_slots[slotId].isEmpty();
    }

    return isEmpty;
}

uint32_t SlotList::getDuration(uint8_t slotId) const
{
    uint32_t duration = Slot::DURATION_DEFAULT;

    if (true == isSlotIdValid(slotId))
    {
        duration = m_slots[slotId].getDuration();
    }

    return duration;
}

void SlotList::setDuration(uint8_t slotId, uint32_t duration)
{
    if (true == isSlotIdValid(slotId))
    {
        m_slots[slotId].setDuration(duration);
    }
}

void SlotList::lock(uint8_t slotId)
{
    if (true == isSlotIdValid(slotId))
    {
        m_slots[slotId].lock();
    }
}

void SlotList::unlock(uint8_t slotId)
{
    if (true == isSlotIdValid(slotId))
    {
        m_slots[slotId].unlock();
    }
}

bool SlotList::isSlotLocked(uint8_t slotId) const
{
    bool isLocked = false;

    if (true == isSlotIdValid(slotId))
    {
        isLocked = m_slots[slotId].isLocked();
    }

    return isLocked;
}

bool SlotList::isSlotEmptyAndUnlocked(uint8_t slotId)
{
    bool isEmptyAndUnlocked = false;

    if (true == isSlotIdValid(slotId))
    {
        if (true == m_slots[slotId].isEmpty())
        {
            if (false == m_slots[slotId].isLocked())
            {
                isEmptyAndUnlocked = true;
            }
        }
    }

    return isEmptyAndUnlocked;
}

uint8_t SlotList::getSlotIdByPluginUID(uint16_t pluginUid)
{
    uint8_t slotId = 0U;

    if (nullptr != m_slots)
    {
        while(m_maxSlots > slotId)
        {
            if (false == m_slots[slotId].isEmpty())
            {
                if (pluginUid == m_slots[slotId].getPlugin()->getUID())
                {
                    break;
                }
            }

            ++slotId;
        }

        if (m_maxSlots <= slotId)
        {
            slotId = SLOT_ID_INVALID;
        }
    }

    return slotId;
}

uint8_t SlotList::getStickySlot() const
{
    return m_stickySlot;
}

/**
 * Set slot sticky. Only one slot can be sticky!
 * If a different slot is already sticky, the sticky flag will be moved.
 * 
 * If slot is empty or the plugin is disabled, it will fail.
 * 
 * @param[in] slotId    The id of the slot which to set sticky.
 * 
 * @return If successful it will return true otherwise false.
 */
bool SlotList::setSlotSticky(uint8_t slotId)
{
    bool isSuccessful = false;

    if (true == isSlotIdValid(slotId))
    {
        if (false == m_slots[slotId].isEmpty())
        {
            if (true == m_slots[slotId].getPlugin()->isEnabled())
            {
                m_stickySlot    = slotId;
                isSuccessful    = true;
            }
        }
    }

    return isSuccessful;
}

/**
 * Removes the sticky flag.
 */
void SlotList::clearSticky()
{
    m_stickySlot = SLOT_ID_INVALID;
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
