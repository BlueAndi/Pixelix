/* MIT License
 *
 * Copyright (c) 2019 - 2020 Andreas Merkle <web@blue-andi.de>
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
 * @brief  Display manager
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup gfx
 *
 * @{
 */

#ifndef __DISPLAYMGR_H__
#define __DISPLAYMGR_H__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <Canvas.h>
#include <TextWidget.h>
#include <SimpleTimer.hpp>

#include "Board.h"
#include "IPluginMaintenance.hpp"
#include "Slot.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/**
 * The display manager is responsible for showing stuff in the right time on the
 * display. For this several time slots are provided. Each time slot can be
 * configured with a specific layout and contains the content to show.
 */
class DisplayMgr
{
public:

    /**
     * Get LED matrix instance.
     *
     * @return LED matrix
     */
    static DisplayMgr& getInstance()
    {
        return m_instance;
    }

    /**
     * Start the display manager. Call this once during startup.
     * This will start updating the display.
     *
     * @return If initialization is successful, it will return true otherwise false.
     */
    bool begin(void);

    /**
     * Stop the display manager.
     * It will immediately stop updating the display.
     */
    void end(void);

    /**
     * Enable/Disable automatic brightness adjustment.
     *
     * @param[in] enable    Enable (true) or disable (false)
     * @return Status
     * @retval true     It was successful enabled/disabled.
     * @retval false    It failed to enable/disable it.
     */
    bool setAutoBrightnessAdjustment(bool enable);

    /**
     * Get state of automatic brightness adjustment.
     *
     * @return If enabled, it will return true otherwise false.
     */
    bool getAutoBrightnessAdjustment(void) const;

    /**
     * Set display brightness in digits [0; 255].
     *
     * @param[in] level Brightness level in digits
     */
    void setBrightness(uint8_t level);

    /**
     * Get display brightness in digits [0; 255].
     *
     * @return Display brightness in digits
     */
    uint8_t getBrightness(void) const;

    /**
     * Install plugin to slot. If the slot contains already a plugin, it will fail.
     * If a invalid slot id is given, the plugin will be installed in the next
     * available slot.
     *
     * @param[in] plugin    Plugin which to install
     * @param[in] slotId    Slot id
     *
     * @return Returns slot id. If it fails, it will return SLOT_ID_INVALID.
     */
    uint8_t installPlugin(IPluginMaintenance* plugin, uint8_t slotId = SLOT_ID_INVALID);

    /**
     * Remove plugin from slot.
     *
     * @param[in] plugin    Plugin which to uninstall
     *
     * @return If successful uninstalled, it will return true otherwise false.
     */
    bool uninstallPlugin(IPluginMaintenance* plugin);

    /**
     * Get slot id by plugin UID.
     *
     * @param[in] uid   Plugin UID
     *
     * @return Slot id
     */
    uint8_t getSlotIdByPluginUID(uint16_t uid);

    /**
     * Get plugin from slot.
     *
     * @param[in] slotId    Slot id, where to get plugin.
     *
     * @return Plugin which is installed in given slot.
     */
    IPluginMaintenance* getPluginInSlot(uint8_t slotId);

    /**
     * Activate a specific plugin immediately.
     *
     * @param[in] plugin    Plugin which to activate
     */
    void activatePlugin(IPluginMaintenance* plugin);

    /**
     * Move plugin to a different slot.
     *
     * @param[in] plugin    Plugin, which to move
     * @param[in] slotId    Slot id of destination slot
     *
     * @return If successful moved, it will return true otherwise false.
     */
    bool movePluginToSlot(IPluginMaintenance* plugin, uint8_t slotId);

    /**
     * Lock a slot.
     *
     * @param[in] slotId    Id of slot, which shall be locked.
     */
    void lockSlot(uint8_t slotId);

    /**
     * Unlock a slot.
     *
     * @param[in] slotId    Id of slot, which shall be unlocked.
     */
    void unlockSlot(uint8_t slotId);

    /**
     * Is slot locked?
     *
     * @return If slot is locked, it will return true otherwise false.
     */
    bool isSlotLocked(uint8_t slotId);

    /**
     * Get slot duration in ms, how long the given plugin will be shown.
     *
     * @param[in] slotId    Slot id
     *
     * @return Duration in ms
     */
    uint32_t getSlotDuration(uint8_t slotId);

    /**
     * Get slot duration in ms, how long the given plugin will be shown.
     *
     * @param[in] slotId    Slot id
     * @param[in] duration  Duration in ms
     *
     * @return Duration in ms
     */
    bool setSlotDuration(uint8_t slotId, uint32_t duration);

    /**
     * Get access to copy of framebuffer.
     *
     * @param[out] fb       Pointer to framebuffer copy
     * @param[out] length   Number of elements in the framebuffer copy
     * @param[out] slotId   Id of slot, from which the copy was taken.
     */
    void getFBCopy(uint32_t* fb, size_t length, uint8_t* slotId);

    /**
     * Get max. number of display slots, which can be used for plugins.
     *
     * @return Max. number of display slots.
     */
    uint8_t getMaxSlots() const
    {
        return m_maxSlots;
    }

    /** Invalid slot id. */
    static const uint8_t    SLOT_ID_INVALID         = UINT8_MAX;

    /** Task stack size in bytes */
    static const uint32_t   TASK_STACKE_SIZE        = 4096U;

    /** Task period in ms */
    static const uint32_t   TASK_PERIOD             = 20U;

    /** MCU core where the task shall run */
    static const BaseType_t TASK_RUN_CORE           = 1;

    /** If no ambient light sensor is available, the default brightness shall be 40%. */
    static const uint8_t    BRIGHTNESS_DEFAULT      = (UINT8_MAX * 40U) / 100U;

    /** Minimum brightness of 10% in case of a dark room. Only used during automatic brightness adjustment. */
    static const uint8_t    BRIGHTNESS_MIN          = (UINT8_MAX * 10U) / 100U;

    /** Default period for automatic brightness adjustment in ms. */
    static const uint32_t   ALS_AUTO_ADJUST_PERIOD  = 250U;

private:

    /** Display manager instance */
    static DisplayMgr   m_instance;

    /** Mutex to lock/unlock display update. */
    SemaphoreHandle_t   m_xMutex;

    /** Display update task handle */
    TaskHandle_t        m_taskHandle;

    /** Flag to signal the task to exit. */
    bool                m_taskExit;

    /** Binary semaphore used to signal the task exit. */
    SemaphoreHandle_t   m_xSemaphore;

    /** List of all slots with their connected plugins. */
    Slot*               m_slots;

    /** Max. number of slots. */
    uint8_t             m_maxSlots;

    /** Current selected slot. */
    uint8_t             m_selectedSlot;

    /** Current selected plugin, which is active shown. */
    IPluginMaintenance* m_selectedPlugin;

    /** Plugin which is requested to be activated immediately. */
    IPluginMaintenance* m_requestedPlugin;

    /** Timer, used for changing the slot after a specific duration. */
    SimpleTimer         m_slotTimer;

    /** Timer, used for automatic brightness adjustment. */
    SimpleTimer         m_autoBrightnessTimer;

    /** Display brightness in percent ([0; 100]). */
    uint8_t             m_brightness;

    /**
     * Construct LED matrix.
     */
    DisplayMgr();

    /**
     * Destroys LED matrix.
     */
    ~DisplayMgr();

    /* Prevent copying */
    DisplayMgr(const DisplayMgr& mgr);
    DisplayMgr& operator=(const DisplayMgr& mgr);

    /**
     * Schedule next slot with a installed and enabled plugin.
     *
     * @param[in] slotId    Id of current slot
     *
     * @return Id of next slot
     */
    uint8_t nextSlot(uint8_t slotId);

    /**
     * Process the slots. This shall be called periodically in
     * a higher period than the DEFAULT_PERIOD.
     *
     * It will handle which slot to show on the display.
     */
    void process(void);

    /**
     * Display update task is responsible to refresh the display content.
     *
     * @param[in]   parameters  Task pParameters
     */
    static void updateTask(void* parameters);

    /**
     * Lock display and prevent the display update, which will be done in a
     * separate task.
     */
    void lock(void);

    /**
     * Unlock display.
     */
    void unlock(void);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __DISPLAYMGR_H__ */

/** @} */