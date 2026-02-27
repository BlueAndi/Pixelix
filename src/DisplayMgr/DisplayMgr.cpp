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
 * @file   DisplayMgr.cpp
 * @brief  Display manager
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "DisplayMgr.h"
#include "BrightnessCtrl.h"
#include "PluginMgr.h"

#include <Display.h>
#include <Logging.h>
#include <ArduinoJson.h>
#include <Util.h>
#include <SettingsService.h>

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

bool DisplayMgr::begin()
{
    const uint16_t   PERCENT_100                   = 100U; /* [%] */
    bool             status                        = false;
    bool             isError                       = false;
    uint8_t          maxSlots                      = 0U;
    uint8_t          brightnessPercent             = 0U;
    uint8_t          minBrightnessHardLimitPercent = 0U;
    uint8_t          maxBrightnessHardLimitPercent = 0U;
    uint8_t          minBrightnessSoftLimitPercent = 0U;
    uint8_t          maxBrightnessSoftLimitPercent = 0U;
    uint16_t         brightness                    = 0U;
    uint16_t         minBrightnessHardLimit        = 0U;
    uint16_t         maxBrightnessHardLimit        = 0U;
    uint16_t         minBrightnessSoftLimit        = 0U;
    uint16_t         maxBrightnessSoftLimit        = 0U;
    bool             isAutoBrightnessEnabled       = false;
    uint8_t          fadeEffect                    = 0U;
    SettingsService& settings                      = SettingsService::getInstance();
    BrightnessCtrl&  brightnessCtrl                = BrightnessCtrl::getInstance();

    if (false == settings.open(true))
    {
        maxSlots                      = settings.getMaxSlots().getDefault();
        brightnessPercent             = settings.getBrightness().getDefault();
        minBrightnessSoftLimitPercent = settings.getMinBrightnessSoftLimit().getDefault();
        maxBrightnessSoftLimitPercent = settings.getMaxBrightnessSoftLimit().getDefault();
        isAutoBrightnessEnabled       = settings.getAutoBrightnessAdjustment().getDefault();
        fadeEffect                    = settings.getFadeEffect().getDefault();
    }
    else
    {
        maxSlots                      = settings.getMaxSlots().getValue();
        brightnessPercent             = settings.getBrightness().getValue();
        minBrightnessSoftLimitPercent = settings.getMinBrightnessSoftLimit().getValue();
        maxBrightnessSoftLimitPercent = settings.getMaxBrightnessSoftLimit().getValue();
        isAutoBrightnessEnabled       = settings.getAutoBrightnessAdjustment().getValue();
        fadeEffect                    = settings.getFadeEffect().getValue();

        settings.close();
    }

    /* Derive the hard limits from the min. and max. brightness values. */
    minBrightnessHardLimitPercent = settings.getBrightness().getMin();
    maxBrightnessHardLimitPercent = settings.getBrightness().getMax();
    minBrightnessHardLimit        = (static_cast<uint16_t>(minBrightnessHardLimitPercent) * UINT8_MAX) / PERCENT_100;
    maxBrightnessHardLimit        = (static_cast<uint16_t>(maxBrightnessHardLimitPercent) * UINT8_MAX) / PERCENT_100;
    minBrightnessSoftLimit        = (static_cast<uint16_t>(minBrightnessSoftLimitPercent) * UINT8_MAX) / PERCENT_100;
    maxBrightnessSoftLimit        = (static_cast<uint16_t>(maxBrightnessSoftLimitPercent) * UINT8_MAX) / PERCENT_100;

    /* Set the display brightness here just once.
     * There is no need to do this in the process() method periodically.
     */
    brightnessCtrl.init(static_cast<uint8_t>(minBrightnessHardLimit), static_cast<uint8_t>(maxBrightnessHardLimit));
    brightnessCtrl.setSoftLimits(static_cast<uint8_t>(minBrightnessSoftLimit), static_cast<uint8_t>(maxBrightnessSoftLimit));

    /* Set initial brightness (automatic brightness adjustment must be disabled). */
    brightness = (static_cast<uint16_t>(brightnessPercent) * UINT8_MAX) / PERCENT_100;
    brightnessCtrl.setBrightness(static_cast<uint8_t>(brightness));

    /* Enable/Disable automatic brightness adjustment. */
    if (false == brightnessCtrl.enable(isAutoBrightnessEnabled))
    {
        LOG_WARNING("Failed to enable autom. brightness adjustment.");
    }

    /* Set fade effect */
    m_fadeEffectController.selectFadeEffect(static_cast<FadeEffectController::FadeEffect>(fadeEffect));

    /* Allocate some stuff. */
    if (false == m_slotList.create(maxSlots))
    {
        LOG_FATAL("Not enough heap space available.");
        isError = true;
    }
    else if (false == m_doubleFrameBuffer.create(Display::getInstance().getWidth(), Display::getInstance().getHeight()))
    {
        LOG_FATAL("Couldn't create double framebuffer.");
        isError = true;
    }
    else if (false == m_mutexInterf.create())
    {
        isError = true;
    }
    else if (false == m_mutexUpdate.create())
    {
        isError = true;
    }
    else
    {
        ;
    }

    /* Process task not started yet? */
    if ((false == isError) &&
        (false == m_processTask.isRunning()))
    {
        if (false == m_processTask.start(this))
        {
            isError = true;
        }
        else
        {
            LOG_DEBUG("Process task is up.");
        }
    }

    /* Update task not started yet? */
    if ((false == isError) &&
        (false == m_updateTask.isRunning()))
    {
#if (0 != CONFIG_DISPLAY_MGR_ENABLE_STATISTICS)
        m_statisticsLogTimer.start(STATISTICS_LOG_PERIOD);
#endif /* (0 != CONFIG_DISPLAY_MGR_ENABLE_STATISTICS) */

        if (false == m_updateTask.start(this))
        {
            isError = true;
        }
        else
        {
            LOG_DEBUG("Update task is up.");
        }
    }

    /* Any error happened? */
    if (true == isError)
    {
        end();
    }
    else
    {
        LOG_INFO("DisplayMgr is up.");

        status = true;
    }

    return status;
}

void DisplayMgr::end()
{
    /* Stop the process task. */
    if (false == m_processTask.stop())
    {
        LOG_ERROR("Failed to stop process task.");
    }
    else
    {
        LOG_DEBUG("Process task is down.");
    }

    /* Stop the update task. */
    if (false == m_updateTask.stop())
    {
        LOG_ERROR("Failed to stop update task.");
    }
    else
    {
        LOG_DEBUG("Update task is down.");
    }

#if (0 != CONFIG_DISPLAY_MGR_ENABLE_STATISTICS)
    m_statisticsLogTimer.stop();
#endif /* (0 != CONFIG_DISPLAY_MGR_ENABLE_STATISTICS) */

    m_mutexUpdate.destroy();
    m_mutexInterf.destroy();

    m_doubleFrameBuffer.release();
    m_slotList.destroy();

    LOG_INFO("DisplayMgr is down.");
}

bool DisplayMgr::setAutoBrightnessAdjustment(bool enable)
{
    bool                       status = false;
    MutexGuard<MutexRecursive> guard(m_mutexInterf);

    status = BrightnessCtrl::getInstance().enable(enable);

    return status;
}

bool DisplayMgr::getAutoBrightnessAdjustment(void) const
{
    bool                       isEnabled = false;
    MutexGuard<MutexRecursive> guard(m_mutexInterf);

    isEnabled = BrightnessCtrl::getInstance().isEnabled();

    return isEnabled;
}

void DisplayMgr::setBrightness(uint8_t level)
{
    MutexGuard<MutexRecursive> guard(m_mutexInterf);

    BrightnessCtrl::getInstance().setBrightness(level);
}

uint8_t DisplayMgr::getBrightness(void) const
{
    uint8_t                    brightness = 0U;
    MutexGuard<MutexRecursive> guard(m_mutexInterf);

    brightness = BrightnessCtrl::getInstance().getBrightness();

    return brightness;
}

void DisplayMgr::setBrightnessSoftLimits(uint8_t minBrightness, uint8_t maxBrightness)
{
    MutexGuard<MutexRecursive> guard(m_mutexInterf);

    BrightnessCtrl::getInstance().setSoftLimits(minBrightness, maxBrightness);
}

void DisplayMgr::getBrightnessSoftLimits(uint8_t& minBrightness, uint8_t& maxBrightness) const
{
    MutexGuard<MutexRecursive> guard(m_mutexInterf);

    BrightnessCtrl::getInstance().getSoftLimits(minBrightness, maxBrightness);
}

uint8_t DisplayMgr::installPlugin(IPluginMaintenance* plugin, uint8_t slotId)
{
    if (nullptr == plugin)
    {
        slotId = SlotList::SLOT_ID_INVALID;
    }
    else
    {
        Slot*                      emptyAndUnlockedSlot = nullptr;
        MutexGuard<MutexRecursive> guard(m_mutexInterf);

        /* Install to any available slot? */
        if (SlotList::SLOT_ID_INVALID == slotId)
        {
            slotId               = m_slotList.getEmptyUnlockedSlot();

            emptyAndUnlockedSlot = m_slotList.getSlot(slotId);
        }
        /* Install to specific slot? */
        else if (true == m_slotList.isSlotEmptyAndUnlocked(slotId))
        {
            emptyAndUnlockedSlot = m_slotList.getSlot(slotId);
        }
        else
        /* Invalid slot id */
        {
            slotId = SlotList::SLOT_ID_INVALID;
        }

        if (nullptr != emptyAndUnlockedSlot)
        {
            if (false == emptyAndUnlockedSlot->setPlugin(plugin))
            {
                slotId = SlotList::SLOT_ID_INVALID;
            }
            else
            {
                LOG_INFO("Start plugin %s (UID %u) in slot %u.", plugin->getName(), plugin->getUID(), slotId);
                plugin->start(Display::getInstance().getWidth(), Display::getInstance().getHeight());
            }
        }
        else
        {
            slotId = SlotList::SLOT_ID_INVALID;
        }

        if (SlotList::SLOT_ID_INVALID == slotId)
        {
            LOG_ERROR("Couldn't install plugin %s (UID %u) in slot %u.", plugin->getName(), plugin->getUID(), slotId);
        }
        else
        {
            LOG_INFO("Plugin %s (UID %u) installed in slot %u.", plugin->getName(), plugin->getUID(), slotId);
        }
    }

    return slotId;
}

bool DisplayMgr::uninstallPlugin(IPluginMaintenance* plugin)
{
    bool status = false;

    if (nullptr != plugin)
    {
        uint8_t                    slotId = SlotList::SLOT_ID_INVALID;
        MutexGuard<MutexRecursive> guard(m_mutexInterf);

        slotId = m_slotList.getSlotIdByPluginUID(plugin->getUID());

        if (true == m_slotList.isSlotIdValid(slotId))
        {
            if (false == m_slotList.isLocked(slotId))
            {
                /* Is this plugin selected at the moment? */
                if (m_selectedPlugin == plugin)
                {
                    /* Remove selection */
                    m_selectedPlugin = nullptr;
                }

                LOG_INFO("Stop plugin %s (UID %u) in slot %u.", plugin->getName(), plugin->getUID(), slotId);
                plugin->stop();
                if (false == m_slotList.setPlugin(slotId, nullptr))
                {
                    LOG_FATAL("Internal error.");
                }
                else
                {
                    status = true;
                }
            }
        }

        if (false == status)
        {
            LOG_ERROR("Couldn't remove plugin %s (UID %u) from slot %u, because slot is locked.", plugin->getName(), plugin->getUID(), slotId);
        }
        else
        {
            LOG_INFO("Plugin %s (UID %u) removed from slot %u.", plugin->getName(), plugin->getUID(), slotId);
        }
    }

    return status;
}

String DisplayMgr::getPluginAliasName(uint16_t uid) const
{
    String                     alias;
    MutexGuard<MutexRecursive> guard(m_mutexInterf);
    uint8_t                    slotId = m_slotList.getSlotIdByPluginUID(uid);
    const IPluginMaintenance*  plugin = m_slotList.getPlugin(slotId);

    if (nullptr != plugin)
    {
        alias = plugin->getAlias();
    }

    return alias;
}

bool DisplayMgr::setPluginAliasName(uint16_t uid, const String& alias)
{
    bool                       isSuccessful = false;
    MutexGuard<MutexRecursive> guard(m_mutexInterf);
    uint8_t                    slotId = m_slotList.getSlotIdByPluginUID(uid);
    IPluginMaintenance*        plugin = m_slotList.getPlugin(slotId);

    if (nullptr != plugin)
    {
        PluginMgr& pluginMgr = PluginMgr::getInstance();

        if (true == pluginMgr.setPluginAliasName(plugin, alias))
        {
            /* Save current installed plugins to persistent memory. */
            pluginMgr.save();

            isSuccessful = true;
        }
    }

    return isSuccessful;
}

Fonts::FontType DisplayMgr::getPluginFontType(uint16_t uid) const
{
    Fonts::FontType            fontType = Fonts::FontType::FONT_TYPE_DEFAULT;
    MutexGuard<MutexRecursive> guard(m_mutexInterf);
    uint8_t                    slotId = m_slotList.getSlotIdByPluginUID(uid);
    const IPluginMaintenance*  plugin = m_slotList.getPlugin(slotId);

    if (nullptr != plugin)
    {
        fontType = plugin->getFontType();
    }

    return fontType;
}

bool DisplayMgr::setPluginFontType(uint16_t uid, Fonts::FontType fontType)
{
    bool                       isSuccessful = false;
    MutexGuard<MutexRecursive> guard(m_mutexInterf);
    uint8_t                    slotId = m_slotList.getSlotIdByPluginUID(uid);
    IPluginMaintenance*        plugin = m_slotList.getPlugin(slotId);

    if (nullptr != plugin)
    {
        if (plugin->getFontType() != fontType)
        {
            PluginMgr& pluginMgr = PluginMgr::getInstance();

            plugin->setFontType(fontType);

            /* Save current installed plugins to persistent memory. */
            pluginMgr.save();
        }

        isSuccessful = true;
    }

    return isSuccessful;
}

uint8_t DisplayMgr::getSlotIdByPluginUID(uint16_t uid) const
{
    MutexGuard<MutexRecursive> guard(m_mutexInterf);
    uint8_t                    slotId = m_slotList.getSlotIdByPluginUID(uid);

    return slotId;
}

IPluginMaintenance* DisplayMgr::getPluginInSlot(uint8_t slotId)
{
    MutexGuard<MutexRecursive> guard(m_mutexInterf);
    IPluginMaintenance*        plugin = m_slotList.getPlugin(slotId);

    return plugin;
}

uint8_t DisplayMgr::getStickySlot() const
{
    MutexGuard<MutexRecursive> guard(m_mutexInterf);
    uint8_t                    slotId = m_slotList.getStickySlot();

    return slotId;
}

bool DisplayMgr::setSlotSticky(uint8_t slotId)
{
    bool                       isSuccessful = false;
    MutexGuard<MutexRecursive> guard(m_mutexInterf);

    /* Activation will take place in process(). */
    isSuccessful = m_slotList.setSlotSticky(slotId);

    if (true == isSuccessful)
    {
        if (SlotList::SLOT_ID_INVALID == slotId)
        {
            LOG_INFO("Sticky flag cleared.");
        }
        else
        {
            LOG_INFO("Set slot %u sticky.", slotId);
        }
    }

    return isSuccessful;
}

void DisplayMgr::clearSticky()
{
    MutexGuard<MutexRecursive> guard(m_mutexInterf);

    m_slotList.clearSticky();

    if (SlotList::SLOT_ID_INVALID != m_selectedSlotId)
    {
        uint32_t duration = m_slotList.getDuration(m_selectedSlotId);

        /* If sticky flag is removed, the slot timer was original stopped and will be started again.
         * Makes only sense if the slot duration is not 0.
         */
        if ((0U != duration) &&
            (false == m_slotTimer.isTimerRunning()))
        {
            m_slotTimer.start(duration);
        }
    }

    LOG_INFO("Sticky flag cleared.");
}

bool DisplayMgr::activateSlot(uint8_t slotId)
{
    bool                       isSuccessful = false;
    MutexGuard<MutexRecursive> guard(m_mutexInterf);

    if (true == m_slotList.isSlotIdValid(slotId))
    {
        /* A disabled slot, can not be activated. */
        if (true == m_slotList.isDisabled(slotId))
        {
            /* Activation not possible */
            ;
        }
        /* Slot already active? */
        else if (slotId == m_selectedSlotId)
        {
            m_requestedPlugin = nullptr;
            isSuccessful      = true;
        }
        /* No other slot is sticky? The slot itself can't be sticky, otherwise
         * the check for being already active would be true.
         */
        else if (SlotList::SLOT_ID_INVALID == m_slotList.getStickySlot())
        {
            m_requestedPlugin = m_slotList.getPlugin(slotId);
            isSuccessful      = true;
        }
        else
        {
            /* Activation not possible. */
            ;
        }
    }

    return isSuccessful;
}

void DisplayMgr::activateNextSlot()
{
    MutexGuard<MutexRecursive> guard(m_mutexInterf);
    uint8_t                    nextSlotId = nextSlot(m_selectedSlotId);

    if (nextSlotId != m_selectedSlotId)
    {
        (void)activateSlot(nextSlotId);
    }
}

void DisplayMgr::activatePreviousSlot()
{
    MutexGuard<MutexRecursive> guard(m_mutexInterf);
    uint8_t                    previousSlotId = previousSlot(m_selectedSlotId);

    if (previousSlotId != m_selectedSlotId)
    {
        (void)activateSlot(previousSlotId);
    }
}

void DisplayMgr::activateNextFadeEffect(FadeEffectController::FadeEffect fadeEffect)
{
    MutexGuard<MutexRecursive> guard1(m_mutexInterf);
    MutexGuard<MutexRecursive> guard2(m_mutexUpdate);

    m_fadeEffectController.selectFadeEffect(fadeEffect);
}

FadeEffectController::FadeEffect DisplayMgr::getFadeEffect()
{
    FadeEffectController ::FadeEffect currentFadeEffect;
    MutexGuard<MutexRecursive>        guard(m_mutexInterf);

    currentFadeEffect = m_fadeEffectController.getFadeEffect();

    return currentFadeEffect;
}

bool DisplayMgr::movePluginToSlot(IPluginMaintenance* plugin, uint8_t slotId)
{
    bool                       status = false;
    MutexGuard<MutexRecursive> guard(m_mutexInterf);

    if ((nullptr != plugin) &&
        (true == m_slotList.isSlotEmptyAndUnlocked(slotId)))
    {
        uint8_t srcSlotId = m_slotList.getSlotIdByPluginUID(plugin->getUID());

        if ((true == m_slotList.isSlotIdValid(srcSlotId)) &&
            (srcSlotId != slotId))
        {
            Slot* srcSlot = m_slotList.getSlot(srcSlotId);
            Slot* dstSlot = m_slotList.getSlot(slotId);

            if ((nullptr != srcSlot) &&
                (nullptr != dstSlot))
            {
                if (false == dstSlot->isLocked())
                {
                    srcSlot->setPlugin(dstSlot->getPlugin());
                    dstSlot->setPlugin(plugin);

                    /* Is one of the moved plugins selected at the moment? */
                    if ((m_selectedPlugin == srcSlot->getPlugin()) ||
                        (m_selectedPlugin == dstSlot->getPlugin()))
                    {
                        /* Remove selection */
                        m_selectedPlugin = nullptr;
                    }

                    status = true;
                }
            }
        }
    }

    return status;
}

void DisplayMgr::lockSlot(uint8_t slotId)
{
    MutexGuard<MutexRecursive> guard(m_mutexInterf);

    m_slotList.lock(slotId);
}

void DisplayMgr::unlockSlot(uint8_t slotId)
{
    MutexGuard<MutexRecursive> guard(m_mutexInterf);

    m_slotList.unlock(slotId);
}

bool DisplayMgr::isSlotLocked(uint8_t slotId)
{
    MutexGuard<MutexRecursive> guard(m_mutexInterf);
    bool                       isLocked = m_slotList.isLocked(slotId);

    return isLocked;
}

void DisplayMgr::enableSlot(uint8_t slotId)
{
    MutexGuard<MutexRecursive> guard(m_mutexInterf);

    m_slotList.enable(slotId);
}

bool DisplayMgr::disableSlot(uint8_t slotId)
{
    MutexGuard<MutexRecursive> guard(m_mutexInterf);
    bool                       isSuccessful = m_slotList.disable(slotId);

    return isSuccessful;
}

bool DisplayMgr::isSlotDisabled(uint8_t slotId)
{
    MutexGuard<MutexRecursive> guard(m_mutexInterf);
    bool                       isDisabled = m_slotList.isDisabled(slotId);

    return isDisabled;
}

uint32_t DisplayMgr::getSlotDuration(uint8_t slotId)
{
    MutexGuard<MutexRecursive> guard(m_mutexInterf);
    uint32_t                   duration = m_slotList.getDuration(slotId);

    return duration;
}

bool DisplayMgr::setSlotDuration(uint8_t slotId, uint32_t duration, bool store)
{
    bool                       status = false;
    MutexGuard<MutexRecursive> guard(m_mutexInterf);
    Slot*                      slot = m_slotList.getSlot(slotId);

    if (nullptr != slot)
    {
        if (slot->getDuration() != duration)
        {
            slot->setDuration(duration);
        }

        status = true;
    }

    return status;
}

void DisplayMgr::getFBCopy(uint32_t* fb, size_t length, uint8_t* slotId)
{
    if ((nullptr != fb) &&
        (0 < length))
    {
        IDisplay&                  display = Display::getInstance();
        int16_t                    x;
        int16_t                    y;
        size_t                     index = 0;
        MutexGuard<MutexRecursive> guard(m_mutexInterf);

        /* Copy framebuffer after it is completely updated. */
        for (y = 0; y < display.getHeight(); ++y)
        {
            for (x = 0; x < display.getWidth(); ++x)
            {
                fb[index] = display.getColor(x, y);
                ++index;

                if (length <= index)
                {
                    break;
                }
            }
        }

        if (nullptr != slotId)
        {
            *slotId = m_selectedSlotId;
        }
    }
}

uint8_t DisplayMgr::getMaxSlots() const
{
    MutexGuard<MutexRecursive> guard(m_mutexInterf);
    uint8_t                    maxSlots = m_slotList.getMaxSlots();

    return maxSlots;
}

void DisplayMgr::setNetworkStatus(bool isConnected)
{
    MutexGuard<MutexRecursive> guard(m_mutexInterf);

    m_isNetworkConnected = isConnected;
}

void DisplayMgr::displayOff()
{
    MutexGuard<MutexRecursive> guard1(m_mutexInterf);
    MutexGuard<MutexRecursive> guard2(m_mutexUpdate);

    Display::getInstance().off();
}

void DisplayMgr::displayOn()
{
    MutexGuard<MutexRecursive> guard1(m_mutexInterf);
    MutexGuard<MutexRecursive> guard2(m_mutexUpdate);

    Display::getInstance().on();
}

bool DisplayMgr::isDisplayOn() const
{
    bool                       isDisplayOn = false;
    MutexGuard<MutexRecursive> guard1(m_mutexInterf);
    MutexGuard<MutexRecursive> guard2(m_mutexUpdate);

    isDisplayOn = Display::getInstance().isOn();

    return isDisplayOn;
}

IIndicatorView::State DisplayMgr::getIndicatorState(uint8_t indicatorId) const
{
    MutexGuard<MutexRecursive> guard(m_mutexInterf);
    IIndicatorView::State      state = m_indicatorView.getIndicatorState(indicatorId);

    return state;
}

void DisplayMgr::setIndicatorState(uint8_t indicatorId, IIndicatorView::State state)
{
    MutexGuard<MutexRecursive> guard1(m_mutexInterf);
    MutexGuard<MutexRecursive> guard2(m_mutexUpdate);

    m_indicatorView.setIndicator(indicatorId, state);
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

DisplayMgr::DisplayMgr() :
    m_mutexInterf(),
    m_mutexUpdate(),
    m_processTask("processTask", processTask, PROCESS_TASK_STACK_SIZE, PROCESS_TASK_PRIORITY, PROCESS_TASK_RUN_CORE),
    m_updateTask("updateTask", updateTask, UPDATE_TASK_STACK_SIZE, UPDATE_TASK_PRIORITY, UPDATE_TASK_RUN_CORE),
    m_slotList(),
    m_selectedSlotId(SlotList::SLOT_ID_INVALID),
    m_selectedPlugin(nullptr),
    m_requestedPlugin(nullptr),
    m_slotTimer(),
    m_doubleFrameBuffer(),
    m_fadeEffectController(m_doubleFrameBuffer),
    m_isNetworkConnected(false),
    m_indicatorView()

#if (0 != CONFIG_DISPLAY_MGR_ENABLE_STATISTICS)
    ,
    m_statistics(),
    m_statisticsLogTimer(),
    m_timestampLastUpdate(0U)

#endif /* (0 != CONFIG_DISPLAY_MGR_ENABLE_STATISTICS) */

{
}

DisplayMgr::~DisplayMgr()
{
    end();
}

uint8_t DisplayMgr::nextSlot(uint8_t slotId)
{
    uint8_t count = 0U;

    if (m_slotList.getMaxSlots() <= slotId)
    {
        slotId = 0U;
    }

    /* Set next slot active, precondition is
     * - the slot is not disabled and
     * - the slot is not empty and
     * - the installed plugin is enabled.
     */
    do
    {
        Slot* slot;

        ++slotId;
        slotId %= m_slotList.getMaxSlots();

        slot    = m_slotList.getSlot(slotId);

        if (nullptr != slot)
        {
            /* Slot must be enabled with a installed plugin, which is enabled too. */
            if ((false == slot->isDisabled()) &&
                (false == slot->isEmpty()) &&
                (true == slot->getPlugin()->isEnabled()))
            {
                break;
            }
        }

        ++count;
    }
    while (m_slotList.getMaxSlots() > count);

    /* Not slot found, which can be activated? */
    if (m_slotList.getMaxSlots() <= count)
    {
        slotId = SlotList::SLOT_ID_INVALID;
    }

    return slotId;
}

uint8_t DisplayMgr::previousSlot(uint8_t slotId)
{
    uint8_t count = 0U;

    if (m_slotList.getMaxSlots() <= slotId)
    {
        slotId = 0U;
    }

    /* Set previous slot active, precondition is a installed plugin which is enabled.  */
    do
    {
        Slot* slot;

        if (0U == slotId)
        {
            slotId = m_slotList.getMaxSlots() - 1U;
        }
        else
        {
            --slotId;
        }

        slot = m_slotList.getSlot(slotId);

        if (nullptr != slot)
        {
            /* Slot must be enabled with a installed plugin, which is enabled too. */
            if ((false == slot->isDisabled()) &&
                (false == slot->isEmpty()) &&
                (true == slot->getPlugin()->isEnabled()))
            {
                break;
            }
        }

        ++count;
    }
    while (m_slotList.getMaxSlots() > count);

    /* Not slot found, which can be activated? */
    if (m_slotList.getMaxSlots() <= count)
    {
        slotId = SlotList::SLOT_ID_INVALID;
    }

    return slotId;
}

void DisplayMgr::process()
{
    IDisplay&                  display    = Display::getInstance();
    uint8_t                    index      = 0U;
    uint8_t                    stickySlot = SlotList::SLOT_ID_INVALID;
    MutexGuard<MutexRecursive> guardInterf(m_mutexInterf);

    /* Handle display brightness */
    BrightnessCtrl::getInstance().process();

    /* Check whether a different slot got sticky and it shall be activated. */
    stickySlot = m_slotList.getStickySlot();
    if (SlotList::SLOT_ID_INVALID != stickySlot)
    {
        /* If slot is set sticky which is active, the slot timer will be stopped to prevent scheduling of other slots. */
        if (m_selectedSlotId == stickySlot)
        {
            m_slotTimer.stop();
        }
        else
        {
            m_requestedPlugin = m_slotList.getPlugin(stickySlot);
        }
    }

    /* Check whether active slot is disabled. */
    if (SlotList::SLOT_ID_INVALID != m_selectedSlotId)
    {
        if (true == m_slotList.isDisabled(m_selectedSlotId))
        {
            /* Remove selected plugin, which forces to select the requested one in the next step. */
            m_selectedPlugin->inactive();
            m_selectedPlugin = nullptr;
        }
    }

    /* Plugin requested to choose? */
    if (nullptr != m_requestedPlugin)
    {
        MutexGuard<MutexRecursive> guard(m_mutexUpdate);

        /* Requested plugin must be enabled, otherwise it won't be scheduled. */
        if (false == m_requestedPlugin->isEnabled())
        {
            LOG_WARNING("Requested plugin %s (UID %u) in slot %u is disabled.",
                m_requestedPlugin->getName(),
                m_requestedPlugin->getUID(),
                m_slotList.getSlotIdByPluginUID(m_requestedPlugin->getUID()));
            m_requestedPlugin = nullptr;
        }
        /* Requested plugin is enabled. Is currently a plugin selected? */
        else if (nullptr != m_selectedPlugin)
        {
            /* If the selected plugin is the same as the requested plugin,
             * keep it to avoid a fade in/out sequence. But restart the
             * slot duration timer, because the duration may have changed.
             */
            if (m_requestedPlugin == m_selectedPlugin)
            {
                uint32_t duration = m_slotList.getDuration(m_selectedSlotId);

                m_requestedPlugin = nullptr;

                /* If plugin shall not be infinite active, start the slot timer. */
                if (0U == duration)
                {
                    m_slotTimer.stop();
                }
                else
                {
                    m_slotTimer.start(duration);
                }
            }
            else
            {
                /* Remove selected plugin, which forces to select the requested one in the next step. */
                m_selectedPlugin->inactive();
                m_selectedPlugin = nullptr;

                /* Fade old display content out */
                m_fadeEffectController.start();
            }
        }
        else
        {
            /* Nothing to do. */
            ;
        }
    }

    /* Any plugin selected? */
    if ((nullptr != m_selectedPlugin) &&
        (false == m_fadeEffectController.isRunning()))
    {
        MutexGuard<MutexRecursive> guard(m_mutexUpdate);

        m_selectedSlotId = m_slotList.getSlotIdByPluginUID(m_selectedPlugin->getUID());

        /* Plugin disabled in the meantime? */
        if (false == m_selectedPlugin->isEnabled())
        {
            m_selectedPlugin->inactive();
            m_selectedPlugin = nullptr;
            m_slotTimer.stop();

            /* Fade old display content out */
            m_fadeEffectController.start();
        }
        /* Plugin run duration timeout? */
        else if ((true == m_slotTimer.isTimerRunning()) &&
                 (true == m_slotTimer.isTimeout()))
        {
            uint8_t slotId = nextSlot(m_selectedSlotId);

            /* If the next slot is the same as the current slot,
             * just restart the plugin duration timer.
             */
            if (m_selectedSlotId == slotId)
            {
                if (true == m_slotTimer.isTimerRunning())
                {
                    m_slotTimer.restart();
                }
            }
            else
            {
                m_selectedPlugin->inactive();
                m_selectedPlugin = nullptr;
                m_slotTimer.stop();

                /* Fade old display content out */
                m_fadeEffectController.start();
            }
        }
        else
        {
            /* Nothing to do. */
            ;
        }
    }

    /* If no plugin is selected, choose the next one. */
    if (nullptr == m_selectedPlugin)
    {
        MutexGuard<MutexRecursive> guard(m_mutexUpdate);
        YAGfxDynamicBitmap&        selectedFrameBuffer = m_doubleFrameBuffer.getSelectedFramebuffer();

        /* Plugin requested to choose? */
        if (nullptr != m_requestedPlugin)
        {
            m_selectedSlotId  = m_slotList.getSlotIdByPluginUID(m_requestedPlugin->getUID());
            m_requestedPlugin = nullptr;
        }
        /* Select next slot, which contains a enabled plugin. */
        else
        {
            m_selectedSlotId = nextSlot(m_selectedSlotId);
        }

        /* Next enabled plugin found? */
        if (true == m_slotList.isSlotIdValid(m_selectedSlotId))
        {
            uint32_t duration = m_slotList.getDuration(m_selectedSlotId);

            m_selectedPlugin  = m_slotList.getPlugin(m_selectedSlotId);

            /* If plugin shall be infinite active or is in a sticky slot, the slot timer will be stopped otherwise started. */
            if ((0U == duration) ||
                (m_selectedSlotId == m_slotList.getStickySlot()))
            {
                m_slotTimer.stop();
            }
            else
            {
                m_slotTimer.start(duration);
            }

            m_selectedPlugin->active(selectedFrameBuffer);

            LOG_INFO("Slot %u (%s) now active.", m_selectedSlotId, m_selectedPlugin->getName());
        }
        /* No plugin is active, clear the display. */
        else
        {
            selectedFrameBuffer.fillScreen(ColorDef::BLACK);
        }
    }

    /* Process all installed plugins. */
    for (index = 0U; index < m_slotList.getMaxSlots(); ++index)
    {
        MutexGuard<MutexRecursive> guard(m_mutexUpdate);
        IPluginMaintenance*        plugin = m_slotList.getPlugin(index);

        if (nullptr != plugin)
        {
            plugin->process(m_isNetworkConnected);
        }
    }
}

void DisplayMgr::update()
{
    IDisplay&                  display = Display::getInstance();
    MutexGuard<MutexRecursive> guard(m_mutexUpdate);
    YAGfxDynamicBitmap&        selectedFrameBuffer = m_doubleFrameBuffer.getSelectedFramebuffer();

    /* Update frame buffer with plugin content. */
    if (nullptr != m_selectedPlugin)
    {
        m_selectedPlugin->update(selectedFrameBuffer);
    }

    /* Update frame buffer with indicators (foreground). */
    m_indicatorView.update(selectedFrameBuffer);

    /* Update the display buffer. */
    m_fadeEffectController.update(display);

    /* Apply brightness changes safely before LED output to avoid race conditions. */
    BrightnessCtrl::getInstance().applyBrightness(display);

    /* Latch display buffer. */
    display.show();
}

void DisplayMgr::processTask(DisplayMgr* self)
{
    uint32_t timestamp = millis();
    uint32_t duration  = 0U;

    /* Process all slot and plugin related stuff. */
    self->process();

    /* Calculate overall duration */
    duration = millis() - timestamp;

    /* Processing shall take place in aquidistant intervals. */
    delay(PROCESS_TASK_PERIOD - (duration % PROCESS_TASK_PERIOD));
}

void DisplayMgr::updateTask(DisplayMgr* self)
{
    uint32_t timestamp           = millis(); /* ms */
    uint32_t duration            = 0U;       /* ms */
    uint32_t timestampPhyUpdate  = 0U;       /* ms */
    uint32_t durationPhyUpdate   = 0U;       /* ms */
    bool     abort               = false;

    /* Observe the physical display refresh and limit the duration to 70% of refresh period. */
    const uint32_t MAX_LOOP_TIME = (UPDATE_TASK_PERIOD * 7U) / (10U);

    /* Refresh display content periodically */
    self->update();

#if (0 != CONFIG_DISPLAY_MGR_ENABLE_STATISTICS)
    /* Update statistics for active plugin processing time. */
    self->m_statistics.pluginProcessing.update(millis() - timestamp);
#endif /* (0 != CONFIG_DISPLAY_MGR_ENABLE_STATISTICS) */

    /* Wait until the physical update is ready to avoid flickering
     * and artifacts on the display, because of e.g. webserver flash
     * access.
     */
    timestampPhyUpdate = millis();
    while ((false == Display::getInstance().isReady()) && (false == abort))
    {
        durationPhyUpdate = millis() - timestampPhyUpdate;

        if (MAX_LOOP_TIME <= durationPhyUpdate)
        {
            abort = true;
        }
    }

#if (0 != CONFIG_DISPLAY_MGR_ENABLE_STATISTICS)
    /* Update statistics for physical display update time. */
    self->m_statistics.displayUpdate.update(durationPhyUpdate);

    /* Update statistics for total processing time. */
    self->m_statistics.total.update(self->m_statistics.pluginProcessing.getCurrent() + self->m_statistics.displayUpdate.getCurrent());

    if (true == self->m_statisticsLogTimer.isTimeout())
    {
        LOG_DEBUG("Refresh period   : min %2u avg %2u max %2u",
            self->m_statistics.refreshPeriod.getMin(),
            self->m_statistics.refreshPeriod.getAvg(),
            self->m_statistics.refreshPeriod.getMax());

        LOG_DEBUG("Plugin processing: min %2u avg %2u max %2u",
            self->m_statistics.pluginProcessing.getMin(),
            self->m_statistics.pluginProcessing.getAvg(),
            self->m_statistics.pluginProcessing.getMax());

        LOG_DEBUG("Display update   : min %2u avg %2u max %2u",
            self->m_statistics.displayUpdate.getMin(),
            self->m_statistics.displayUpdate.getAvg(),
            self->m_statistics.displayUpdate.getMax());

        LOG_DEBUG("Total            : min %2u avg %2u max %2u",
            self->m_statistics.total.getMin(),
            self->m_statistics.total.getAvg(),
            self->m_statistics.total.getMax());

        /* Reset the statistics to get a new min./max. determination. */
        self->m_statistics.pluginProcessing.reset();
        self->m_statistics.displayUpdate.reset();
        self->m_statistics.total.reset();
        self->m_statistics.refreshPeriod.reset();

        self->m_statisticsLogTimer.restart();
    }
#endif /* (0 != CONFIG_DISPLAY_MGR_ENABLE_STATISTICS) */

    /* Calculate overall duration */
    duration = millis() - timestamp;

    /* Updating the display shall take place in aquidistant intervals. */
    delay(UPDATE_TASK_PERIOD - (duration % UPDATE_TASK_PERIOD));

#if (0 != CONFIG_DISPLAY_MGR_ENABLE_STATISTICS)
    self->m_statistics.refreshPeriod.update(millis() - self->m_timestampLastUpdate);
    self->m_timestampLastUpdate = millis();
#endif /* (0 != CONFIG_DISPLAY_MGR_ENABLE_STATISTICS) */
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
