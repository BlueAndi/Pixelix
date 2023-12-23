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

#if (0 != CONFIG_DISPLAY_MGR_ENABLE_STATISTICS)
#include <StatisticValue.hpp>
#endif /* (0 != CONFIG_DISPLAY_MGR_ENABLE_STATISTICS) */

/******************************************************************************
 * Compiler Switches
 *****************************************************************************/

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and classes
 *****************************************************************************/

#if (0 != CONFIG_DISPLAY_MGR_ENABLE_STATISTICS)

/**
 * A collection of statistics, which are interesting for debugging purposes.
 */
struct Statistics
{
    StatisticValue<uint32_t, 0U, 10U>   pluginProcessing;
    StatisticValue<uint32_t, 0U, 10U>   displayUpdate;
    StatisticValue<uint32_t, 0U, 10U>   total;
    StatisticValue<uint32_t, 0U, 10U>   refreshPeriod;
};

#endif /* (0 != CONFIG_DISPLAY_MGR_ENABLE_STATISTICS) */

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
    bool                status                  = false;
    bool                isError                 = false;
    uint8_t             maxSlots                = 0U;
    uint8_t             brightnessPercent       = 0U;
    uint8_t             minBrightnessPercent    = 0U;
    uint8_t             maxBrightnessPercent    = 0U;
    uint16_t            brightness              = 0U;
    uint16_t            minBrightness           = 0U;
    uint16_t            maxBrightness           = 0U;
    SettingsService&    settings                = SettingsService::getInstance();

    if (false == settings.open(true))
    {
        maxSlots            = settings.getMaxSlots().getDefault();
        brightnessPercent   = settings.getBrightness().getDefault();
    }
    else
    {
        maxSlots            = settings.getMaxSlots().getValue();
        brightnessPercent   = settings.getBrightness().getValue();

        settings.close();
    }

    minBrightnessPercent    = settings.getBrightness().getMin();
    maxBrightnessPercent    = settings.getBrightness().getMax();

    minBrightness   = (static_cast<uint16_t>(minBrightnessPercent) * UINT8_MAX) / 100U;
    maxBrightness   = (static_cast<uint16_t>(maxBrightnessPercent) * UINT8_MAX) / 100U;

    /* Set the display brightness here just once.
     * There is no need to do this in the process() method periodically.
     */
    BrightnessCtrl::getInstance().init(Display::getInstance(), minBrightness, maxBrightness);
    brightness = (static_cast<uint16_t>(brightnessPercent) * UINT8_MAX) / 100U;
    BrightnessCtrl::getInstance().setBrightness(static_cast<uint8_t>(brightness));

    /* No slots available? */
    if (false == m_slotList.isAvailable())
    {
        if (false == m_slotList.create(maxSlots))
        {
            LOG_FATAL("Not enough heap space available.");

            isError = true;
        }
    }

    if (false == isError)
    {
        uint8_t idx = 0U;

        /* Allocate framebuffer memory. */
        for(idx = 0U; idx < UTIL_ARRAY_NUM(m_framebuffers); ++idx)
        {
            if (false == m_framebuffers[idx].isAllocated())
            {
                if (false == m_framebuffers[idx].create(Display::getInstance().getWidth(), Display::getInstance().getHeight()))
                {
                    isError = true;

                    LOG_WARNING("Couldn't create framebuffer canvas for fade effect.");
                }
            }
        }

        if (false == isError)
        {
            m_selectedFrameBuffer = &m_framebuffers[0U];
        }
        else
        {
            /* If fade effects are not available, it just looks not so nice but
            * thats it.
            */
            isError = false;
        }

        if (false == m_mutexInterf.isAllocated())
        {
            if (false == m_mutexInterf.create())
            {
                isError = true;
            }
        }

        if (false == m_mutexUpdate.isAllocated())
        {
            if (false == m_mutexUpdate.create())
            {
                isError = true;
            }
        }

        /* Process task not started yet? */
        if ((false == isError) &&
            (nullptr == m_processTaskHandle))
        {
            if (false == createProcessTask())
            {
                isError = true;
            }
        }

        /* Update task not started yet? */
        if ((false == isError) &&
            (nullptr == m_updateTaskHandle))
        {
            if (false == createUpdateTask())
            {
                isError = true;
            }
        }
    }

    /* Any error happened? */
    if (true == isError)
    {
        destroyProcessTask();
        destroyUpdateTask();
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
    uint8_t idx = 0U;

    /* Already running? */
    if (nullptr != m_processTaskHandle)
    {
        destroyProcessTask();
    }

    if (nullptr != m_updateTaskHandle)
    {
        destroyUpdateTask();
    }

    m_mutexUpdate.destroy();
    m_mutexInterf.destroy();

    m_selectedFrameBuffer = nullptr;

    /* Release framebuffer memory. */
    for(idx = 0U; idx < UTIL_ARRAY_NUM(m_framebuffers); ++idx)
    {
        m_framebuffers[idx].release();
    }

    m_slotList.destroy();

    LOG_INFO("DisplayMgr is down.");
}

bool DisplayMgr::setAutoBrightnessAdjustment(bool enable)
{
    bool                        status = false;
    MutexGuard<MutexRecursive>  guard(m_mutexInterf);

    status = BrightnessCtrl::getInstance().enable(enable);

    return status;
}

bool DisplayMgr::getAutoBrightnessAdjustment(void)
{
    bool                        isEnabled = false;
    MutexGuard<MutexRecursive>  guard(m_mutexInterf);

    isEnabled = BrightnessCtrl::getInstance().isEnabled();

    return isEnabled;
}

void DisplayMgr::setBrightness(uint8_t level)
{
    MutexGuard<MutexRecursive>  guard(m_mutexInterf);

    BrightnessCtrl::getInstance().setBrightness(level);
}

uint8_t DisplayMgr::getBrightness(void)
{
    uint8_t                     brightness = 0U;
    MutexGuard<MutexRecursive>  guard(m_mutexInterf);

    brightness = BrightnessCtrl::getInstance().getBrightness();

    return brightness;
}

uint8_t DisplayMgr::installPlugin(IPluginMaintenance* plugin, uint8_t slotId)
{
    if (nullptr == plugin)
    {
        slotId = SlotList::SLOT_ID_INVALID;
    }
    else
    {
        Slot*                       emptyAndUnlockedSlot    = nullptr;
        MutexGuard<MutexRecursive>  guard(m_mutexInterf);

        /* Install to any available slot? */
        if (SlotList::SLOT_ID_INVALID == slotId)
        {
            slotId = m_slotList.getEmptyUnlockedSlot();

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
        uint8_t                     slotId = SlotList::SLOT_ID_INVALID;
        MutexGuard<MutexRecursive>  guard(m_mutexInterf);

        slotId = m_slotList.getSlotIdByPluginUID(plugin->getUID());

        if (true == m_slotList.isSlotIdValid(slotId))
        {
            if (false == m_slotList.isSlotLocked(slotId))
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

String DisplayMgr::getPluginAliasName(uint16_t uid)
{
    String                      alias;
    MutexGuard<MutexRecursive>  guard(m_mutexInterf);
    uint8_t                     slotId  = m_slotList.getSlotIdByPluginUID(uid);
    IPluginMaintenance*         plugin  = m_slotList.getPlugin(slotId);

    if (nullptr != plugin)
    {
        alias = plugin->getAlias();
    }

    return alias;
}

bool DisplayMgr::setPluginAliasName(uint16_t uid, const String& alias)
{
    bool                        isSuccessful    = false;
    MutexGuard<MutexRecursive>  guard(m_mutexInterf);
    uint8_t                     slotId          = m_slotList.getSlotIdByPluginUID(uid);
    IPluginMaintenance*         plugin          = m_slotList.getPlugin(slotId);

    if (nullptr != plugin)
    {
        if (true == PluginMgr::getInstance().setPluginAliasName(plugin, alias))
        {
            /* Save current installed plugins to persistent memory. */
            PluginMgr::getInstance().save();
            
            isSuccessful = true;
        }
    }

    return isSuccessful;
}

uint8_t DisplayMgr::getSlotIdByPluginUID(uint16_t uid)
{
    MutexGuard<MutexRecursive>  guard(m_mutexInterf);
    uint8_t                     slotId = m_slotList.getSlotIdByPluginUID(uid);

    return slotId;
}

IPluginMaintenance* DisplayMgr::getPluginInSlot(uint8_t slotId)
{
    MutexGuard<MutexRecursive>  guard(m_mutexInterf);
    IPluginMaintenance*         plugin = m_slotList.getPlugin(slotId);

    return plugin;
}

uint8_t DisplayMgr::getStickySlot() const
{
    MutexGuard<MutexRecursive>  guard(m_mutexInterf);
    uint8_t                     slotId = m_slotList.getStickySlot();

    return slotId;
}

bool DisplayMgr::setSlotSticky(uint8_t slotId)
{
    bool                        isSuccessful = false;
    MutexGuard<MutexRecursive>  guard(m_mutexInterf);

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
    bool                        isSuccessful = false;
    MutexGuard<MutexRecursive>  guard(m_mutexInterf);

    if (true == m_slotList.isSlotIdValid(slotId))
    {
        /* Slot already active? */
        if (slotId == m_selectedSlotId)
        {
            m_requestedPlugin   = nullptr;
            isSuccessful        = true;
        }
        /* No slot is sticky? */
        else if (SlotList::SLOT_ID_INVALID == m_slotList.getStickySlot())
        {
            m_requestedPlugin   = m_slotList.getPlugin(slotId);
            isSuccessful        = true;
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
    MutexGuard<MutexRecursive>  guard(m_mutexInterf);
    uint8_t                     nextSlotId = nextSlot(m_selectedSlotId);

    if (nextSlotId != m_selectedSlotId)
    {
        (void)activateSlot(nextSlotId);
    }
}

void DisplayMgr::activatePreviousSlot()
{
    MutexGuard<MutexRecursive>  guard(m_mutexInterf);
    uint8_t                     previousSlotId = previousSlot(m_selectedSlotId);

    if (previousSlotId != m_selectedSlotId)
    {
        (void)activateSlot(previousSlotId);
    }
}

void DisplayMgr::activateNextFadeEffect(FadeEffect fadeEffect)
{
    MutexGuard<MutexRecursive> guard(m_mutexInterf);

    if (FADE_EFFECT_COUNT <= fadeEffect)
    {
        m_fadeEffectIndex = FADE_EFFECT_LINEAR;
    }
    else
    {
        m_fadeEffectIndex = fadeEffect;
    }

    m_fadeEffectUpdate = true;
}

DisplayMgr::FadeEffect DisplayMgr::getFadeEffect()
{
    FadeEffect                  currentFadeEffect;
    MutexGuard<MutexRecursive>  guard(m_mutexInterf);

    currentFadeEffect = m_fadeEffectIndex;
    
    return currentFadeEffect;
}

bool DisplayMgr::movePluginToSlot(IPluginMaintenance* plugin, uint8_t slotId)
{
    bool                        status = false;
    MutexGuard<MutexRecursive>  guard(m_mutexInterf);

    if ((nullptr != plugin) &&
        (true == m_slotList.isSlotEmptyAndUnlocked(slotId)))
    {
        uint8_t srcSlotId = m_slotList.getSlotIdByPluginUID(plugin->getUID());

        if ((true == m_slotList.isSlotIdValid(srcSlotId)) &&
            (srcSlotId != slotId))
        {
            Slot*   srcSlot = m_slotList.getSlot(srcSlotId);
            Slot*   dstSlot = m_slotList.getSlot(slotId);

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
    MutexGuard<MutexRecursive>  guard(m_mutexInterf);
    bool                        isLocked = m_slotList.isSlotLocked(slotId);

    return isLocked;
}

uint32_t DisplayMgr::getSlotDuration(uint8_t slotId)
{
    MutexGuard<MutexRecursive>  guard(m_mutexInterf);
    uint32_t                    duration = m_slotList.getDuration(slotId);

    return duration;
}

bool DisplayMgr::setSlotDuration(uint8_t slotId, uint32_t duration, bool store)
{
    bool                        status      = false;
    MutexGuard<MutexRecursive>  guard(m_mutexInterf);
    Slot*                       slot        = m_slotList.getSlot(slotId);

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
        IDisplay&                   display = Display::getInstance();
        int16_t                     x       = 0;
        int16_t                     y       = 0;
        size_t                      index   = 0;
        MutexGuard<MutexRecursive>  guard(m_mutexInterf);

        /* Copy framebuffer after it is completely updated. */
        for(y = 0; y < display.getHeight(); ++y)
        {
            for(x = 0; x < display.getWidth(); ++x)
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
    MutexGuard<MutexRecursive>  guard(m_mutexInterf);
    uint8_t                     maxSlots = m_slotList.getMaxSlots();

    return maxSlots;
}

void DisplayMgr::setNetworkStatus(bool isConnected)
{
    MutexGuard<MutexRecursive>  guard(m_mutexInterf);

    m_isNetworkConnected = isConnected;
}

void DisplayMgr::displayOff()
{
    MutexGuard<MutexRecursive>  guard1(m_mutexInterf);
    MutexGuard<MutexRecursive>  guard2(m_mutexUpdate);
    
    Display::getInstance().off();
}

void DisplayMgr::displayOn()
{
    MutexGuard<MutexRecursive>  guard1(m_mutexInterf);
    MutexGuard<MutexRecursive>  guard2(m_mutexUpdate);

    Display::getInstance().on();
}

bool DisplayMgr::isDisplayOn() const
{
    bool                        isDisplayOn             = false;
    MutexGuard<MutexRecursive>  guard1(m_mutexInterf);
    MutexGuard<MutexRecursive>  guard2(m_mutexUpdate);

    isDisplayOn = Display::getInstance().isOn();

    return isDisplayOn;
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
    m_processTaskHandle(nullptr),
    m_processTaskExit(false),
    m_processTaskSemaphore(nullptr),
    m_updateTaskHandle(nullptr),
    m_updateTaskExit(false),
    m_updateTaskSemaphore(nullptr),
    m_slotList(),
    m_selectedSlotId(SlotList::SLOT_ID_INVALID),
    m_selectedPlugin(nullptr),
    m_requestedPlugin(nullptr),
    m_slotTimer(),
    m_displayFadeState(FADE_IN),
    m_selectedFrameBuffer(nullptr),
    m_framebuffers(),
    m_fadeLinearEffect(),
    m_fadeMoveXEffect(),
    m_fadeMoveYEffect(),
    m_fadeEffect(&m_fadeLinearEffect),
    m_fadeEffectIndex(FADE_EFFECT_LINEAR),
    m_fadeEffectUpdate(false),
    m_isNetworkConnected(false)
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
    else
    {
        ++slotId;
        slotId %= m_slotList.getMaxSlots();
    }

    /* Set next slot active, precondition is a installed plugin which is enabled.  */
    do
    {
        Slot* slot = m_slotList.getSlot(slotId);

        if (nullptr != slot)
        {
            /* Plugin installed? */
            if (false == slot->isEmpty())
            {
                /* Plugin enabled? */
                if (true == slot->getPlugin()->isEnabled())
                {
                    break;
                }
            }
        }

        ++slotId;
        slotId %= m_slotList.getMaxSlots();

        ++count;
    }
    while (m_slotList.getMaxSlots() > count);

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
    else
    {
        if (0U == slotId)
        {
            slotId = m_slotList.getMaxSlots() - 1U;
        }
        else
        {
            --slotId;
        }
    }

    /* Set previous slot active, precondition is a installed plugin which is enabled.  */
    do
    {
        Slot* slot = m_slotList.getSlot(slotId);

        if (nullptr != slot)
        {
            /* Plugin installed? */
            if (false == slot->isEmpty())
            {
                /* Plugin enabled? */
                if (true == slot->getPlugin()->isEnabled())
                {
                    break;
                }
            }
        }

        if (0U == slotId)
        {
            slotId = m_slotList.getMaxSlots() - 1U;
        }
        else
        {
            --slotId;
        }

        ++count;
    }
    while (m_slotList.getMaxSlots() > count);

    if (m_slotList.getMaxSlots() <= count)
    {
        slotId = SlotList::SLOT_ID_INVALID;
    }

    return slotId;
}

void DisplayMgr::startFadeOut()
{
    /* Select next framebuffer and keep old content, until
     * the fade effect is finished.
     */
    if (m_selectedFrameBuffer == &m_framebuffers[FB_ID_0])
    {
        m_selectedFrameBuffer = &m_framebuffers[FB_ID_1];
    }
    else
    {
        m_selectedFrameBuffer = &m_framebuffers[FB_ID_0];
    }

    m_displayFadeState = FADE_OUT;

    if (nullptr != m_fadeEffect)
    {
        m_fadeEffect->init();
    }
}

void DisplayMgr::fadeInOut(YAGfx& dst)
{
    if ((nullptr != m_selectedFrameBuffer) &&
        (nullptr != m_fadeEffect))
    {
        YAGfxBitmap* prevFb = nullptr;

        /* Determine previous frame buffer */
        if (m_selectedFrameBuffer == &m_framebuffers[FB_ID_0])
        {
            prevFb = &m_framebuffers[FB_ID_1];
        }
        else
        {
            prevFb = &m_framebuffers[FB_ID_0];
        }

        /* Continuously update the current canvas with its framebuffer. */
        if (nullptr != m_selectedPlugin)
        {
            m_selectedPlugin->update(*m_selectedFrameBuffer);
        }

        /* Handle fading */
        switch(m_displayFadeState)
        {
        /* No fading at all */
        case FADE_IDLE:
            dst.drawBitmap(0, 0, *m_selectedFrameBuffer);
            break;

        /* Fade new display content in */
        case FADE_IN:
            if (true == m_fadeEffect->fadeIn(dst, *prevFb, *m_selectedFrameBuffer))
            {
                m_displayFadeState = FADE_IDLE;
            }
            break;

        /* Fade old display content out! */
        case FADE_OUT:
            if (true == m_fadeEffect->fadeOut(dst, *prevFb, *m_selectedFrameBuffer))
            {
                m_displayFadeState = FADE_IN;
            }
            break;

        default:
            break;
        }
    }
}

void DisplayMgr::process()
{
    IDisplay&                   display     = Display::getInstance();
    uint8_t                     index       = 0U;
    uint8_t                     stickySlot  = SlotList::SLOT_ID_INVALID;
    MutexGuard<MutexRecursive>  guardInterf(m_mutexInterf);

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

    /* Plugin requested to choose? */
    if (nullptr != m_requestedPlugin)
    {
        MutexGuard<MutexRecursive>  guard(m_mutexUpdate);

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
                startFadeOut();
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
        (FADE_IDLE == m_displayFadeState))
    {
        MutexGuard<MutexRecursive>  guard(m_mutexUpdate);

        m_selectedSlotId = m_slotList.getSlotIdByPluginUID(m_selectedPlugin->getUID());

        /* Plugin disabled in the meantime? */
        if (false == m_selectedPlugin->isEnabled())
        {
            m_selectedPlugin->inactive();
            m_selectedPlugin = nullptr;
            m_slotTimer.stop();

            /* Fade old display content out */
            startFadeOut();
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
                startFadeOut();
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
        MutexGuard<MutexRecursive>  guard(m_mutexUpdate);

        /* Plugin requested to choose? */
        if (nullptr != m_requestedPlugin)
        {
            m_selectedSlotId    = m_slotList.getSlotIdByPluginUID(m_requestedPlugin->getUID());
            m_requestedPlugin   = nullptr;
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

            m_selectedPlugin = m_slotList.getPlugin(m_selectedSlotId);

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

            if (nullptr != m_selectedFrameBuffer)
            {
                m_selectedPlugin->active(*m_selectedFrameBuffer);
            }
            else
            {
                m_selectedPlugin->active(display);
            }

            LOG_INFO("Slot %u (%s) now active.", m_selectedSlotId, m_selectedPlugin->getName());
        }
        /* No plugin is active, clear the display. */
        else
        {
            if (nullptr != m_selectedFrameBuffer)
            {
                m_selectedFrameBuffer->fillScreen(ColorDef::BLACK);
            }
            display.clear();
        }
    }

    /* Avoid changing to next effect, if the there is a pending slot change. */
    if ((false != m_fadeEffectUpdate) && (FADE_IDLE == m_displayFadeState))
    {
        switch (m_fadeEffectIndex)
        {
        case FADE_EFFECT_LINEAR:
            m_fadeEffect = &m_fadeLinearEffect;
            break;

        case FADE_EFFECT_MOVE_X:
            m_fadeEffect = &m_fadeMoveXEffect;
            break;

        case FADE_EFFECT_MOVE_Y:
            m_fadeEffect = &m_fadeMoveYEffect;
            break;

        default:
            m_fadeEffect = nullptr;
            m_fadeEffectIndex = FADE_EFFECT_NO;
            break;
        }

        m_fadeEffectUpdate = false;
    }

    /* Process all installed plugins. */
    for(index = 0U; index < m_slotList.getMaxSlots(); ++index)
    {
        MutexGuard<MutexRecursive>  guard(m_mutexUpdate);
        IPluginMaintenance*         plugin = m_slotList.getPlugin(index);

        if (nullptr != plugin)
        {
            plugin->process(m_isNetworkConnected);
        }
    }
}

void DisplayMgr::update()
{
    IDisplay&                   display = Display::getInstance();
    MutexGuard<MutexRecursive>  guard(m_mutexUpdate);

    /* Update display (main canvas available) */
    if (nullptr != m_selectedFrameBuffer)
    {
        fadeInOut(display);
    }
    /* Update display (main canvas not available) */
    else if (nullptr != m_selectedPlugin)
    {
        m_selectedPlugin->update(display);
    }
    /* No plugin selected. */
    else
    {
        /* Nothing to do. */
        ;
    }

    display.show();
}

bool DisplayMgr::createProcessTask()
{
    bool isSuccessful = false;

    if (nullptr == m_processTaskSemaphore)
    {
        /* Create binary semaphore to signal task exit. */
        m_processTaskSemaphore = xSemaphoreCreateBinary();

        if (nullptr != m_processTaskSemaphore)
        {
            BaseType_t  osRet   = pdFAIL;

            /* Task shall run */
            m_processTaskExit = false;

            osRet = xTaskCreateUniversal(   processTask,
                                            "processTask",
                                            PROCESS_TASK_STACK_SIZE,
                                            this,
                                            PROCESS_TASK_PRIORITY,
                                            &m_processTaskHandle,
                                            PROCESS_TASK_RUN_CORE);

            /* Couldn't task be created? */
            if (pdPASS != osRet)
            {
                vSemaphoreDelete(m_processTaskSemaphore);
                m_processTaskSemaphore = nullptr;
            }
            else
            {
                (void)xSemaphoreGive(m_processTaskSemaphore);

                LOG_DEBUG("ProcessTask is up.");

                isSuccessful = true;
            }
        }
    }
    
    return isSuccessful;
}

void DisplayMgr::destroyProcessTask()
{
    /* Is the task running? */
    if (nullptr != m_processTaskSemaphore)
    {
        /* Request task to exit and wait until its done. */
        m_processTaskExit = true;
        (void)xSemaphoreTake(m_processTaskSemaphore, portMAX_DELAY);
        m_processTaskHandle = nullptr;

        /* After task is destroyed, the signal semaphore can safely be destroyed. */
        vSemaphoreDelete(m_processTaskSemaphore);
        m_processTaskSemaphore = nullptr;

        LOG_DEBUG("ProcessTask is down.");
    }
}

bool DisplayMgr::createUpdateTask()
{
    bool isSuccessful = false;

    if (nullptr == m_updateTaskSemaphore)
    {
        /* Create binary semaphore to signal task exit. */
        m_updateTaskSemaphore = xSemaphoreCreateBinary();

        if (nullptr != m_updateTaskSemaphore)
        {
            BaseType_t  osRet   = pdFAIL;

            /* Task shall run */
            m_updateTaskExit = false;

            osRet = xTaskCreateUniversal(   updateTask,
                                            "updateTask",
                                            UPDATE_TASK_STACK_SIZE,
                                            this,
                                            UPDATE_TASK_PRIORITY,
                                            &m_updateTaskHandle,
                                            UPDATE_TASK_RUN_CORE);

            /* Couldn't task be created? */
            if (pdPASS != osRet)
            {
                vSemaphoreDelete(m_updateTaskSemaphore);
                m_updateTaskSemaphore = nullptr;
            }
            else
            {
                (void)xSemaphoreGive(m_updateTaskSemaphore);

                LOG_DEBUG("UpdateTask is up.");

                isSuccessful = true;
            }
        }
    }
    
    return isSuccessful;
}

void DisplayMgr::destroyUpdateTask()
{
    /* Is the task running? */
    if (nullptr != m_updateTaskSemaphore)
    {
        /* Request task to exit and wait until its done. */
        m_updateTaskExit = true;
        (void)xSemaphoreTake(m_updateTaskSemaphore, portMAX_DELAY);
        m_updateTaskHandle = nullptr;

        /* After task is destroyed, the signal semaphore can safely be destroyed. */
        vSemaphoreDelete(m_updateTaskSemaphore);
        m_updateTaskSemaphore = nullptr;

        LOG_DEBUG("UpdateTask is down.");
    }
}

void DisplayMgr::processTask(void* parameters)
{
    DisplayMgr* tthis = static_cast<DisplayMgr*>(parameters);

    if ((nullptr != tthis) &&
        (nullptr != tthis->m_processTaskSemaphore))
    {
        (void)xSemaphoreTake(tthis->m_processTaskSemaphore, portMAX_DELAY);

        while(false == tthis->m_processTaskExit)
        {
            uint32_t    timestamp   = millis();
            uint32_t    duration    = 0U;

            /* Process all slot and plugin related stuff. */
            tthis->process();

            /* Calculate overall duration */
            duration = millis() - timestamp;

            /* Give other tasks a chance. */
            if (PROCESS_TASK_PERIOD <= duration)
            {
                delay(1U);
            }
            else
            {
                delay(PROCESS_TASK_PERIOD - duration);
            }
        }

        (void)xSemaphoreGive(tthis->m_processTaskSemaphore);
    }

    vTaskDelete(nullptr);
}

void DisplayMgr::updateTask(void* parameters)
{
    DisplayMgr* tthis = static_cast<DisplayMgr*>(parameters);

    if ((nullptr != tthis) &&
        (nullptr != tthis->m_updateTaskSemaphore))
    {
#if (0 != CONFIG_DISPLAY_MGR_ENABLE_STATISTICS)
        Statistics      statistics;
        SimpleTimer     statisticsLogTimer;
        const uint32_t  STATISTICS_LOG_PERIOD   = 4000U;    /* [ms] */
        uint32_t        timestampLastUpdate     = millis();

        statisticsLogTimer.start(STATISTICS_LOG_PERIOD);

#endif /* (0 != CONFIG_DISPLAY_MGR_ENABLE_STATISTICS) */

        (void)xSemaphoreTake(tthis->m_updateTaskSemaphore, portMAX_DELAY);

        while(false == tthis->m_updateTaskExit)
        {
            uint32_t    timestamp           = millis();
            uint32_t    duration            = 0U;
            uint32_t    timestampPhyUpdate  = 0U;
            uint32_t    durationPhyUpdate   = 0U;
            bool        abort               = false;

            /* Observe the physical display refresh and limit the duration to 70% of refresh period. */
            const uint32_t  MAX_LOOP_TIME   = (UPDATE_TASK_PERIOD * 7U) / (10U);

            /* Refresh display content periodically */
            tthis->update();

#if (0 != CONFIG_DISPLAY_MGR_ENABLE_STATISTICS)
            statistics.pluginProcessing.update(millis() - timestamp);
#endif /* (0 != CONFIG_DISPLAY_MGR_ENABLE_STATISTICS) */

            /* Wait until the physical update is ready to avoid flickering
             * and artifacts on the display, because of e.g. webserver flash
             * access.
             */
            timestampPhyUpdate = millis();
            while((false == Display::getInstance().isReady()) && (false == abort))
            {
                durationPhyUpdate = millis() - timestampPhyUpdate;

                if (MAX_LOOP_TIME <= durationPhyUpdate)
                {
                    abort = true;
                }
            }

#if (0 != CONFIG_DISPLAY_MGR_ENABLE_STATISTICS)
            statistics.displayUpdate.update(durationPhyUpdate);
            statistics.total.update(statistics.pluginProcessing.getCurrent() + statistics.displayUpdate.getCurrent());

            if (true == statisticsLogTimer.isTimeout())
            {
                LOG_DEBUG("[ %2u, %2u, %2u ]", 
                    statistics.refreshPeriod.getMin(),
                    statistics.refreshPeriod.getAvg(),
                    statistics.refreshPeriod.getMax()
                );
                
                LOG_DEBUG("[ %2u, %2u, %2u ] [ %2u, %2u, %2u ] [ %2u, %2u, %2u ]",
                    statistics.pluginProcessing.getMin(),
                    statistics.pluginProcessing.getAvg(),
                    statistics.pluginProcessing.getMax(),
                    statistics.displayUpdate.getMin(),
                    statistics.displayUpdate.getAvg(),
                    statistics.displayUpdate.getMax(),
                    statistics.total.getMin(),
                    statistics.total.getAvg(),
                    statistics.total.getMax()
                );

                /* Reset the statistics to get a new min./max. determination. */
                statistics.pluginProcessing.reset();
                statistics.displayUpdate.reset();
                statistics.total.reset();
                statistics.refreshPeriod.reset();

                statisticsLogTimer.restart();
            }
#endif /* (0 != CONFIG_DISPLAY_MGR_ENABLE_STATISTICS) */

            /* Calculate overall duration */
            duration = millis() - timestamp;

            /* Give other tasks a chance. */
            if (UPDATE_TASK_PERIOD <= duration)
            {
                delay(1U);
            }
            else
            {
                delay(UPDATE_TASK_PERIOD - duration);
            }

#if (0 != CONFIG_DISPLAY_MGR_ENABLE_STATISTICS)
            statistics.refreshPeriod.update(millis() - timestampLastUpdate);
            timestampLastUpdate = millis();
#endif /* (0 != CONFIG_DISPLAY_MGR_ENABLE_STATISTICS) */
        }

        (void)xSemaphoreGive(tthis->m_updateTaskSemaphore);
    }

    vTaskDelete(nullptr);
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
