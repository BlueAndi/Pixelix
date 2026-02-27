/* MIT License
 *
 * Copyright (c) 2019 - 2026 Andreas Merkle <web@blue-andi.de>
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
 * @file   DisplayMgr.h
 * @brief  Display manager
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup DISPLAY_MGR
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
#include <Mutex.hpp>
#include <Task.hpp>
#include <IndicatorViewBase.hpp>
#include <Fonts.h>

#include "IPluginMaintenance.hpp"
#include "SlotList.h"
#include "FadeEffectController.h"
#include "DoubleFrameBuffer.h"

#if (0 != CONFIG_DISPLAY_MGR_ENABLE_STATISTICS)
#include <StatisticValue.hpp>
#endif /* (0 != CONFIG_DISPLAY_MGR_ENABLE_STATISTICS) */

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

    /** Slot configuration. */
    typedef struct
    {
        String          name;       /**< Plugin name. */
        uint16_t        uid;        /**< Plugin UID. */
        String          alias;      /**< Plugin alias name. */
        Fonts::FontType fontType;   /**< Font type. */
        uint32_t        duration;   /**< Duration in ms, how long the plugin will be shown. */
        bool            isLocked;   /**< Is the slot locked? */
        bool            isSticky;   /**< Is the slot sticky? */
        bool            isDisabled; /**< Is the slot disabled? */

    } SlotConfig;

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
     * Set the soft brightness limits.
     *
     * @param[in] minBrightness The minimum brightness level in digits [0; 255].
     * @param[in] maxBrightness The maximum brightness level in digits [0; 255].
     */
    void setBrightnessSoftLimits(uint8_t minBrightness, uint8_t maxBrightness);

    /**
     * Get the soft brightness limits.
     *
     * @param[out] minBrightness The minimum brightness level in digits [0; 255].
     * @param[out] maxBrightness The maximum brightness level in digits [0; 255].
     */
    void getBrightnessSoftLimits(uint8_t& minBrightness, uint8_t& maxBrightness) const;

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
    String getPluginAliasName(uint16_t uid) const;

    /**
     * Set the alias name of a plugin.
     *
     * @param[in] uid   Plugin UID
     * @param[in] alias Plugin alias name
     * @return If successful, it will return true otherwise false.
     */
    bool setPluginAliasName(uint16_t uid, const String& alias);

    /**
     * Get the font type of a plugin.
     * If the given plugin UID is invalid, it will return Fonts::FontType::FONT_TYPE_DEFAULT.
     *
     * @param[in] uid   Plugin UID
     * @return The plugin font type.
     */
    Fonts::FontType getPluginFontType(uint16_t uid) const;

    /**
     * Set the font type of a plugin.
     *
     * @param[in] uid       Plugin UID
     * @param[in] fontType  Plugin font type
     * @return If successful, it will return true otherwise false.
     */
    bool setPluginFontType(uint16_t uid, Fonts::FontType fontType);

    /**
     * Get slot id by plugin UID.
     *
     * @param[in] uid   Plugin UID
     *
     * @return Slot id
     */
    uint8_t getSlotIdByPluginUID(uint16_t uid) const;

    /**
     * Get plugin from slot.
     *
     * @param[in] slotId    Slot id, where to get plugin.
     *
     * @return Plugin which is installed in given slot.
     */
    IPluginMaintenance* getPluginInSlot(uint8_t slotId);

    /**
     * Get slot configuration.
     *
     * @param[in] slotId    Slot id, where to get configuration.
     * @param[out] config   Slot configuration
     *
     * @return If successful, it will return true otherwise false.
     */
    bool getSlotConfig(uint8_t slotId, SlotConfig& config) const;

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
     * If slot is empty or disabled, it will fail.
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
     *
     * @return If successful activated, it will return true otherwise false.
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
     * If selected fade effect is FadeEffectController::FADE_EFFECT_COUNT,
     * the next fade effect will be selected in the order as defined in the
     * FadeEffectController::FadeEffect enum.
     *
     * @param[in] fadeEffect Fade effect to be activated.
     */
    void activateNextFadeEffect(FadeEffectController::FadeEffect fadeEffect);

    /**
     * Get selected fade effect.
     *
     * @return The currently selected fade effect.
     */
    FadeEffectController::FadeEffect getFadeEffect();

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
     * @param[in] slotId    Slot id
     *
     * @return If slot is locked, it will return true otherwise false.
     */
    bool isSlotLocked(uint8_t slotId);

    /**
     * Enale slot.
     *
     * @param[in] slotId    Id of slot, which shall be enabled.
     */
    void enableSlot(uint8_t slotId);

    /**
     * Disable slot.
     *
     * A sticky slot can't be disabled.
     *
     * @param[in] slotId    Id of slot, which shall be disabled.
     *
     * @return If successful, it will return true otherwise false.
     */
    bool disableSlot(uint8_t slotId);

    /**
     * Is slot disabled?
     *
     * @param[in] slotId    Slot id
     *
     * @return If slot is disabled, it will return true otherwise false.
     */
    bool isSlotDisabled(uint8_t slotId);

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

    /**
     * Get indicator state.
     * If the indicator id is invalid, it will return STATE_OFF.
     *
     * @param[in] indicatorId  Id of indicator.
     *
     * @return State of the indicator.
     */
    IIndicatorView::State getIndicatorState(uint8_t indicatorId) const;

    /**
     * Set indicator state.
     * If the indicator id is invalid, it will do nothing.
     *
     * The indicator id 255 will be used to turn on/off all indicators at once.
     *
     * @param[in] indicatorId  Id of indicator
     * @param[in] state        State of the indicator.
     */
    void setIndicatorState(uint8_t indicatorId, IIndicatorView::State state);

    /**
     * Indicator id for all indicators.
     */
    static const uint8_t INDICATOR_ID_ALL     = IndicatorViewBase::INDICATOR_ID_ALL;

    /**
     * Indicator id for network connection.
     */
    static const uint8_t INDICATOR_ID_NETWORK = 0U;

private:

    /** The process task stack size in bytes */
    static const uint32_t PROCESS_TASK_STACK_SIZE  = 5120U;

    /** The process task period in ms. */
    static const uint32_t PROCESS_TASK_PERIOD      = 100U;

    /** The process task shall run on the APP MCU core. */
    static const BaseType_t PROCESS_TASK_RUN_CORE  = APP_CPU_NUM;

    /** The process task priority shall be equal than the Arduino loop task priority. */
    static const UBaseType_t PROCESS_TASK_PRIORITY = 1U;

    /** The update task stack size in bytes */
    static const uint32_t UPDATE_TASK_STACK_SIZE   = 4096U;

    /** The update task period in ms. */
    static const uint32_t UPDATE_TASK_PERIOD       = 20U;

    /** The update task shall run on the MCU core with less load. */
    static const BaseType_t UPDATE_TASK_RUN_CORE   = tskNO_AFFINITY;

    /** The update task priority shall be higher than the other application tasks. */
    static const UBaseType_t UPDATE_TASK_PRIORITY  = 4U;

    /** Mutex to protect concurrent access through the public interface. */
    mutable MutexRecursive m_mutexInterf;

    /** Mutex to protect the display update against concurrent access. */
    mutable MutexRecursive m_mutexUpdate;

    /** Process task */
    Task<DisplayMgr> m_processTask;

    /** Update task */
    Task<DisplayMgr> m_updateTask;

    /** List of all slots with their connected plugins. */
    SlotList m_slotList;

    /** Current selected slot. */
    uint8_t m_selectedSlotId;

    /** Current selected plugin, which is active shown. */
    IPluginMaintenance* m_selectedPlugin;

    /** Plugin which is requested to be activated immediately. */
    IPluginMaintenance* m_requestedPlugin;

    /** Timer, used for changing the slot after a specific duration. */
    SimpleTimer          m_slotTimer;

    DoubleFrameBuffer    m_doubleFrameBuffer;    /**< Double framebuffer. */
    FadeEffectController m_fadeEffectController; /**< Fade effect controller. */
    bool                 m_isNetworkConnected;   /**< Is a network connection established? */
    IndicatorViewBase    m_indicatorView;        /**< Indicator view shown as overlay to indicate user defined states. */


#if (0 != CONFIG_DISPLAY_MGR_ENABLE_STATISTICS)

    /**
     * A collection of statistics, which are interesting for debugging purposes.
     */
    struct Statistics
    {
        StatisticValue<uint32_t, 0U, 10U> pluginProcessing;
        StatisticValue<uint32_t, 0U, 10U> displayUpdate;
        StatisticValue<uint32_t, 0U, 10U> total;
        StatisticValue<uint32_t, 0U, 10U> refreshPeriod;
    };

    /** Statistics log period in ms. */
    static const uint32_t STATISTICS_LOG_PERIOD = 4000U; /* [ms] */
    Statistics            m_statistics;                  /**< Statistics data. */
    SimpleTimer           m_statisticsLogTimer;          /**< Statistics log timer. */
    uint32_t              m_timestampLastUpdate;         /**< Timestamp of last display update. */

#endif /* (0 != CONFIG_DISPLAY_MGR_ENABLE_STATISTICS) */

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
     * Schedule next enbeld slot with a installed and enabled plugin.
     *
     * @param[in] slotId    Id of current slot
     *
     * @return Id of next slot
     */
    uint8_t nextSlot(uint8_t slotId);

    /**
     * Schedule previous enabled slot with a installed and enabled plugin.
     *
     * @param[in] slotId    Id of current slot
     *
     * @return Id of previous slot
     */
    uint8_t previousSlot(uint8_t slotId);

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
     * Display update task is responsible to refresh the display content.
     *
     * @param[in] self Display manager instance.
     */
    static void processTask(DisplayMgr* self);

    /**
     * Display update task is responsible to refresh the display content.
     *
     * @param[in] self Display manager instance.
     */
    static void updateTask(DisplayMgr* self);
};

/******************************************************************************
 * Functions
 *****************************************************************************/

#endif /* DISPLAYMGR_H */

/** @} */