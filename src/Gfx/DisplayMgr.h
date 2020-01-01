/* MIT License
 *
 * Copyright (c) 2019 Andreas Merkle <web@blue-andi.de>
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
#include "Plugin.hpp"

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
    static DisplayMgr& getInstance(void)
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
     * Lock display and prevent the display update, which will be done in a
     * separate task.
     */
    void lock(void);

    /**
     * Unlock display.
     */
    void unlock(void);

    /**
     * Enable/Disable automatic brightness adjustment.
     * 
     * @param[in] enable    Enable (true) or disable (false)
     * @return Status
     * @retval true     It was successful enabled/disabled.
     * @retval false    It failed to enable/disable it.
     */
    bool enableAutoBrightnessAdjustment(bool enable);

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
    uint8_t installPlugin(Plugin* plugin, uint8_t slotId = SLOT_ID_INVALID);

    /**
     * Remove plugin from slot.
     * 
     * @param[in] plugin    Plugin which to uninstall
     */
    void uninstallPlugin(Plugin* plugin);

    /**
     * Activate a specific plugin immediately.
     * 
     * @param[in] plugin    Plugin which to activate
     */
    void activatePlugin(Plugin* plugin);

    /**
     * Get access to copy of framebuffer.
     * 
     * @param[out] fb       Pointer to framebuffer copy
     * @param[out] length   Number of elements in the framebuffer copy
     */
    void getFBCopy(uint32_t* fb, size_t length);

    /** Invalid slot id. */
    static const uint8_t    SLOT_ID_INVALID         = UINT8_MAX;

    /** Update task stack size in bytes */
    static const uint32_t   UPDATE_TASK_STACKE_SIZE = 4096u;

    /** MCU core where the update task shall run */
    static const BaseType_t UPDATE_TASK_RUN_CORE    = 1;

    /** Maximum number of supported slots. */
    static const uint8_t    MAX_SLOTS               = 4u;

    /** If no ambient light sensor is available, the default brightness shall be 40%. */
    static const uint8_t    BRIGHTNESS_DEFAULT      = (UINT8_MAX * 40u) / 100u;

    /** Minimum brightness of 10% in case of a dark room. Only used during automatic brightness adjustment. */
    static const uint8_t    BRIGHTNESS_MIN          = (UINT8_MAX * 10u) / 100u;

    /** Default period for automatic brightness adjustment in ms. */
    static const uint32_t   ALS_AUTO_ADJUST_PERIOD  = 250u;

private:

    /** Display manager instance */
    static DisplayMgr   m_instance;

    /** Mutex to lock/unlock display update. */
    SemaphoreHandle_t   m_xMutex;

    /** Display update task handle */
    TaskHandle_t        m_taskHandle;

    /** Flag to signal the task to exit. */
    uint8_t             m_taskExit;

    /** Binary semaphore used to signal the task exit. */
    SemaphoreHandle_t   m_xSemaphore;

    /** List of all slots with their connected plugins. */
    Plugin*             m_slots[MAX_SLOTS];

    /** Current selected slot. */
    uint8_t             m_selectedSlot;

    /** Current selected plugin, which is active shown. */
    Plugin*             m_selectedPlugin;

    /** Plugin which is requested to be activated immediately. */
    Plugin*             m_requestedPlugin;

    /** Timer, used for changing the slot after a specific duration. */
    SimpleTimer         m_slotTimer;

    /** Timer, used for automatic brightness adjustment. */
    SimpleTimer         m_autoBrightnessTimer;

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

};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif  /* __DISPLAYMGR_H__ */

/** @} */