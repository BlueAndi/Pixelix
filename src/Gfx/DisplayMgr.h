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
 * @brief  Display manager
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup gfx
 *
 * @{
 */

#ifndef DISPLAYMGR_H
#define DISPLAYMGR_H

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <stdint.h>
#include <Board.h>
#include <TextWidget.h>
#include <SimpleTimer.hpp>
#include <FadeLinear.h>
#include <FadeMoveX.h>
#include <FadeMoveY.h>
#include <Mutex.hpp>
#include <YAGfxBitmap.h>

#include "IPluginMaintenance.hpp"
#include "SlotList.h"

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

    /** Fade effects */
    enum FadeEffect
    {
        FADE_EFFECT_NO = 0, /**< No fade effect */
        FADE_EFFECT_LINEAR, /**< Linear dimming fade effect. */
        FADE_EFFECT_MOVE_X, /**< Moving fade effect into the direction of negative x-coordinates. */
        FADE_EFFECT_MOVE_Y, /**< Moving fade effect into the direction of negative y-coordinates. */
        FADE_EFFECT_COUNT   /**< Number of fade effects. */
    };

    /**
     * Get display manager instance.
     *
     * @return Display manager
     */
    static DisplayMgr& getInstance()
    {
        static DisplayMgr instance; /* singleton idiom to force initialization in the first usage. */

        return instance;
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
    bool getAutoBrightnessAdjustment(void);

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
    uint8_t getBrightness(void);

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
    uint8_t installPlugin(IPluginMaintenance* plugin, uint8_t slotId = SlotList::SLOT_ID_INVALID);

    /**
     * Remove plugin from slot.
     *
     * @param[in] plugin    Plugin which to uninstall
     *
     * @return If successful uninstalled, it will return true otherwise false.
     */
    bool uninstallPlugin(IPluginMaintenance* plugin);

    /**
     * Get the alias name of a plugin.
     * If the given plugin UID is invalid, it will return a empty alias name.
     * 
     * @param[in] uid   Plugin UID
     * @return The plugin alias name.
     */
    String getPluginAliasName(uint16_t uid);

    /**
     * Set the alias name of a plugin.
     * 
     * @param[in] uid   Plugin UID
     * @param[in] alias Plugin alias name
     * @return If successful, it will return true otherwise false.
     */
    bool setPluginAliasName(uint16_t uid, const String& alias);

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

    /**
     * Activate the slot with the given id.
     * If a different slot is marked sticky, it will fail.
     * If no enabled plugin is in the slot, it will fail.
     * 
     * @param[in] slotId    Id of the slot which to activate.
     */
    bool activateSlot(uint8_t slotId);

    /**
     * Activate next slot.
     */
    void activateNextSlot();

    /**
     * Activate previous slot.
     */
    void activatePreviousSlot();

    /**
     * Activate next fade effect.
     * 
     * @param[in] fadeEffect fadeEffect to be activated.
     */
    void activateNextFadeEffect(FadeEffect fadeEffect);

    /**
     * Get fade effect.
     * 
     * @return the currently active fadeEffect.
     */
    FadeEffect getFadeEffect();
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
     * Set slot duration in ms, how long the given plugin will be shown.
     *
     * @param[in] slotId    Slot id
     * @param[in] duration  Duration in ms
     * @param[in] store     Store duration persistent (default: true)
     *
     * @return Duration in ms
     */
    bool setSlotDuration(uint8_t slotId, uint32_t duration, bool store = true);

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
    uint8_t getMaxSlots() const;

    /**
     * Set network connection status.
     * 
     * @param[in] isConnected   Set to true for a established network connection, otherwise false.
     */
    void setNetworkStatus(bool isConnected);

    /**
     * Power display off.
     */
    void displayOff();

    /**
     * Power display on.
     */
    void displayOn();

    /**
     * Is the display powered on?
     * 
     * @return If the display is powered on, it will return true otherwise false.
     */
    bool isDisplayOn() const;

private:

    /** The process task stack size in bytes */
    static const uint32_t       PROCESS_TASK_STACK_SIZE = 4096U;

    /** The process task period in ms. */
    static const uint32_t       PROCESS_TASK_PERIOD     = 100U;

    /** The process task shall run on the APP MCU core. */
    static const BaseType_t     PROCESS_TASK_RUN_CORE   = APP_CPU_NUM;

    /** The process task priority shall be equal than the Arduino loop task priority. */
    static const UBaseType_t    PROCESS_TASK_PRIORITY   = 1U;

    /** The update task stack size in bytes */
    static const uint32_t       UPDATE_TASK_STACK_SIZE  = 4096U;

    /** The update task period in ms. */
    static const uint32_t       UPDATE_TASK_PERIOD      = 20U;

    /** The update task shall run on the MCU core with less load. */
    static const BaseType_t     UPDATE_TASK_RUN_CORE    = tskNO_AFFINITY;

    /** The update task priority shall be higher than the other application tasks. */
    static const UBaseType_t    UPDATE_TASK_PRIORITY    = 4U;

    /** Mutex to protect concurrent access through the public interface. */
    mutable MutexRecursive      m_mutexInterf;

    /** Mutex to protect the display update against concurrent access. */
    mutable MutexRecursive      m_mutexUpdate;

    /** Process task handle */
    TaskHandle_t                m_processTaskHandle;

    /** Flag to signal the process task to exit. */
    bool                        m_processTaskExit;

    /** Binary semaphore used to signal the process task exited. */
    SemaphoreHandle_t           m_processTaskSemaphore;

    /** Update task handle */
    TaskHandle_t                m_updateTaskHandle;

    /** Flag to signal the update task to exit. */
    bool                        m_updateTaskExit;

    /** Binary semaphore used to signal the update task exited. */
    SemaphoreHandle_t           m_updateTaskSemaphore;

    /** List of all slots with their connected plugins. */
    SlotList                    m_slotList;

    /** Current selected slot. */
    uint8_t                     m_selectedSlotId;

    /** Current selected plugin, which is active shown. */
    IPluginMaintenance*         m_selectedPlugin;

    /** Plugin which is requested to be activated immediately. */
    IPluginMaintenance*         m_requestedPlugin;

    /** Timer, used for changing the slot after a specific duration. */
    SimpleTimer                 m_slotTimer;

    /** Display fade state */
    enum FadeState
    {
        FADE_IDLE = 0,  /**< No fading */
        FADE_IN,        /**< Find in */
        FADE_OUT        /**< Fade out */
    };

    /** Frame buffer ids */
    enum FbId
    {
        FB_ID_0 = 0,    /**< 1. frame buffer */
        FB_ID_1,        /**< 2. frame buffer */
        FB_ID_MAX       /**< Number of frame buffers */
    };

    /**
     * A plugin change (inactive -> active) will fade the display content of
     * the old plugin out and from the new plugin in.
     */
    FadeState           m_displayFadeState;
    YAGfxBitmap*        m_selectedFrameBuffer;          /**< Points to the current framebuffer, used to update the display. */
    YAGfxDynamicBitmap  m_framebuffers[FB_ID_MAX];      /**< Two framebuffers, which will contain the old and the new plugin content. */
    FadeLinear          m_fadeLinearEffect;             /**< Linear fade effect. */
    FadeMoveX           m_fadeMoveXEffect;              /**< Moving along x-axis fade effect. */
    FadeMoveY           m_fadeMoveYEffect;              /**< Moving along y-axis fade effect. */
    IFadeEffect*        m_fadeEffect;                   /**< The fade effect itself. */
    FadeEffect          m_fadeEffectIndex;              /**< Fade effect index to determine the next fade effect. */
    bool                m_fadeEffectUpdate;             /**< Flag to indicate that the fadeEffect was updated. */
    bool                m_isNetworkConnected;           /**< Is a network connection established? */

    /**
     * Constructs the display manager.
     */
    DisplayMgr();

    /**
     * Destroys the display manager.
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
     * Schedule previous slot with a installed and enabled plugin.
     *
     * @param[in] slotId    Id of current slot
     *
     * @return Id of previous slot
     */
    uint8_t previousSlot(uint8_t slotId);

    /**
     * Start fade effect.
     */
    void startFadeOut();

    /**
     * Fade display content in/out.
     *
     * @param[in] dst   Destination display
     */
    void fadeInOut(YAGfx& dst);

    /**
     * Process the slots. This shall be called periodically in
     * a higher period than the DEFAULT_PERIOD.
     *
     * It will handle which slot to show on the display.
     */
    void process(void);

    /**
     * Process the slots. This shall be called periodically in
     * a higher period than the DEFAULT_PERIOD.
     *
     * It will handle which slot to show on the display.
     */
    void update(void);

    /**
     * Create the process task which is responsible to process all plugins.
     * 
     * @return If successful it will return true otherwise false.
     */
    bool createProcessTask();

    /**
     * Destroy the process task gracefully.
     */
    void destroyProcessTask();

    /**
     * Create the update task which is responsible to update the display content.
     * 
     * @return If successful it will return true otherwise false.
     */
    bool createUpdateTask();

    /**
     * Destroy the update task gracefully.
     */
    void destroyUpdateTask();

    /**
     * Display update task is responsible to refresh the display content.
     *
     * @param[in]   parameters  Task pParameters
     */
    static void processTask(void* parameters);

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

#endif  /* DISPLAYMGR_H */

/** @} */