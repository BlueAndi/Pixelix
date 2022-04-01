/* MIT License
 *
 * Copyright (c) 2019 - 2022 Andreas Merkle <web@blue-andi.de>
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
#include "Settings.h"
#include "BrightnessCtrl.h"
#include "PluginMgr.h"

#include <Display.h>
#include <Logging.h>
#include <ArduinoJson.h>
#include <Util.h>

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
    bool        status              = false;
    uint8_t     idx                 = 0U;
    bool        isError             = false;
    uint8_t     maxSlots            = 0U;
    uint8_t     brightnessPercent   = 0U;
    uint16_t    brightness          = 0U;
    Settings&   settings            = Settings::getInstance();

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

    /* Set the display brightness here just once.
     * There is no need to do this in the process() method periodically.
     */
    BrightnessCtrl::getInstance().init(Display::getInstance());
    brightness = (static_cast<uint16_t>(brightnessPercent) * UINT8_MAX) / 100U; /* Calculate brightness in digits */
    BrightnessCtrl::getInstance().setBrightness(static_cast<uint8_t>(brightness));

    /* No slots available? */
    if (nullptr == m_slots)
    {
        m_maxSlots = maxSlots;

        if (0U < m_maxSlots)
        {
            m_slots = new(std::nothrow) Slot[m_maxSlots];

            /* Load slot configuration */
            load();
        }
    }

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

    /* Not started yet? */
    if ((nullptr == m_taskHandle) &&
        (nullptr != m_slots))
    {
        if (true == m_mutex.create())
        {
            /* Create binary semaphore to signal task exit. */
            m_xSemaphore = xSemaphoreCreateBinary();

            if (nullptr != m_xSemaphore)
            {
                BaseType_t  osRet   = pdFAIL;

                /* Task shall run */
                m_taskExit = false;

                osRet = xTaskCreateUniversal(   updateTask,
                                                "displayTask",
                                                TASK_STACK_SIZE,
                                                this,
                                                TASK_PRIORITY,
                                                &m_taskHandle,
                                                TASK_RUN_CORE);

                /* Task successful created? */
                if (pdPASS == osRet)
                {
                    (void)xSemaphoreGive(m_xSemaphore);
                    status = true;
                }
            }
        }
    }

    /* Any error happened? */
    if (false == status)
    {
        if (nullptr != m_xSemaphore)
        {
            vSemaphoreDelete(m_xSemaphore);
            m_xSemaphore = nullptr;
        }
    }
    else
    {
        LOG_INFO("DisplayMgr is up.");
    }

    return status;
}

void DisplayMgr::end()
{
    /* Note, don't destroy the slots and the canvas framebuffers here. They shall live until the system restarts. */

    /* Already running? */
    if (nullptr != m_taskHandle)
    {
        m_taskExit = true;

        /* Join */
        (void)xSemaphoreTake(m_xSemaphore, portMAX_DELAY);
        m_taskHandle = nullptr;

        LOG_INFO("DisplayMgr is down.");

        vSemaphoreDelete(m_xSemaphore);
        m_xSemaphore = nullptr;

        m_mutex.destroy();
    }

    return;
}

bool DisplayMgr::setAutoBrightnessAdjustment(bool enable)
{
    bool                        status = false;
    MutexGuard<MutexRecursive>  guard(m_mutex);

    status = BrightnessCtrl::getInstance().enable(enable);

    return status;
}

bool DisplayMgr::getAutoBrightnessAdjustment(void)
{
    bool                        isEnabled = false;
    MutexGuard<MutexRecursive>  guard(m_mutex);

    isEnabled = BrightnessCtrl::getInstance().isEnabled();

    return isEnabled;
}

void DisplayMgr::setBrightness(uint8_t level)
{
    MutexGuard<MutexRecursive>  guard(m_mutex);

    BrightnessCtrl::getInstance().setBrightness(level);

    return;
}

uint8_t DisplayMgr::getBrightness(void)
{
    uint8_t                     brightness = 0U;
    MutexGuard<MutexRecursive>  guard(m_mutex);

    brightness = BrightnessCtrl::getInstance().getBrightness();

    return brightness;
}

uint8_t DisplayMgr::installPlugin(IPluginMaintenance* plugin, uint8_t slotId)
{
    if (nullptr == plugin)
    {
        slotId = SLOT_ID_INVALID;
    }
    else
    {
        /* Install to any available slot? */
        if (SLOT_ID_INVALID == slotId)
        {
            MutexGuard<MutexRecursive> guard(m_mutex);

            /* Find a empty unlocked slot. */
            slotId = 0U;
            while((m_maxSlots > slotId) && ((false == m_slots[slotId].isEmpty()) || (true == m_slots[slotId].isLocked())))
            {
                ++slotId;
            }

            if (m_maxSlots > slotId)
            {
                if (false == m_slots[slotId].setPlugin(plugin))
                {
                    slotId = SLOT_ID_INVALID;
                }
                else
                {
                    LOG_INFO("Start plugin %s (UID %u) in slot %u.", plugin->getName(), plugin->getUID(), slotId);
                    plugin->start(Display::getInstance().getWidth(), Display::getInstance().getHeight());
                }
            }
            else
            {
                slotId = SLOT_ID_INVALID;
            }
        }
        /* Install to specific slot? */
        else if ((m_maxSlots > slotId) &&
                 (true == m_slots[slotId].isEmpty()) &&
                 (false == m_slots[slotId].isLocked()))
        {
            MutexGuard<MutexRecursive> guard(m_mutex);

            if (false == m_slots[slotId].setPlugin(plugin))
            {
                slotId = SLOT_ID_INVALID;
            }
            else
            {
                LOG_INFO("Start plugin %s (UID %u) in slot %u.", plugin->getName(), plugin->getUID(), slotId);
                plugin->start(Display::getInstance().getWidth(), Display::getInstance().getHeight());
            }
        }
        else
        {
            slotId = SLOT_ID_INVALID;
        }

        if (m_maxSlots > slotId)
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
        uint8_t                     slotId = SLOT_ID_INVALID;
        MutexGuard<MutexRecursive>  guard(m_mutex);

        slotId = getSlotIdByPluginUID(plugin->getUID());

        if (m_maxSlots > slotId)
        {
            if (false == m_slots[slotId].isLocked())
            {
                /* Is this plugin selected at the moment? */
                if (m_selectedPlugin == plugin)
                {
                    /* Remove selection */
                    m_selectedPlugin = nullptr;
                }

                LOG_INFO("Stop plugin %s (UID %u) in slot %u.", plugin->getName(), plugin->getUID(), slotId);
                plugin->stop();
                if (false == m_slots[slotId].setPlugin(nullptr))
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
            LOG_INFO("Couldn't remove plugin %s (UID %u) from slot %u, because slot is locked.", plugin->getName(), plugin->getUID(), slotId);
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
    uint8_t                     slotId          = getSlotIdByPluginUID(uid);
    MutexGuard<MutexRecursive>  guard(m_mutex);

    if (SLOT_ID_INVALID != slotId)
    {
        IPluginMaintenance* plugin = m_slots[slotId].getPlugin();

        if (nullptr != plugin)
        {
            alias = plugin->getAlias();
        }
    }

    return alias;
}

bool DisplayMgr::setPluginAliasName(uint16_t uid, const String& alias)
{
    bool                        isSuccessful    = false;
    uint8_t                     slotId          = getSlotIdByPluginUID(uid);
    MutexGuard<MutexRecursive>  guard(m_mutex);

    if (SLOT_ID_INVALID != slotId)
    {
        IPluginMaintenance* plugin = m_slots[slotId].getPlugin();

        if (nullptr != plugin)
        {
            if (true == PluginMgr::getInstance().setPluginAliasName(plugin, alias))
            {
                /* Save current installed plugins to persistent memory. */
                PluginMgr::getInstance().save();
                
                isSuccessful = true;
            }
        }
    }

    return isSuccessful;
}

uint8_t DisplayMgr::getSlotIdByPluginUID(uint16_t uid)
{
    uint8_t                     index   = 0U;
    uint8_t                     slotId  = SLOT_ID_INVALID;
    MutexGuard<MutexRecursive>  guard(m_mutex);

    while((m_maxSlots > index) && (m_maxSlots <= slotId))
    {
        if (false == m_slots[index].isEmpty())
        {
            if (uid == m_slots[index].getPlugin()->getUID())
            {
                slotId = index;
            }
        }

        ++index;
    }

    return slotId;
}

IPluginMaintenance* DisplayMgr::getPluginInSlot(uint8_t slotId)
{
    IPluginMaintenance* plugin = nullptr;

    if (m_maxSlots > slotId)
    {
        MutexGuard<MutexRecursive> guard(m_mutex);

        plugin = m_slots[slotId].getPlugin();
    }

    return plugin;
}

void DisplayMgr::activatePlugin(IPluginMaintenance* plugin)
{
    if (nullptr != plugin)
    {
        uint8_t                     slotId = SLOT_ID_INVALID;
        MutexGuard<MutexRecursive>  guard(m_mutex);

        slotId = getSlotIdByPluginUID(plugin->getUID());

        if (m_maxSlots > slotId)
        {
            m_requestedPlugin = plugin;
        }
    }

    return;
}

void DisplayMgr::activateNextSlot()
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    /* Avoid changing to next slot, if the there is a pending slot change. */
    if (FADE_IDLE == m_displayFadeState)
    {
        /* If slot timer is running, force a slot change by setting the duration to 0. */
        if (true == m_slotTimer.isTimerRunning())
        {
            m_slotTimer.start(0U);
        }
    }

    return;
}

void DisplayMgr::activateNextFadeEffect(FadeEffect fadeEffect)
{
    MutexGuard<MutexRecursive> guard(m_mutex);

    if (FADE_EFFECT_COUNT <= fadeEffect)
    {
        m_fadeEffectIndex = FADE_EFFECT_LINEAR;
    }
    else
    {
        m_fadeEffectIndex = fadeEffect;
    }

    m_fadeEffectUpdate = true;

    return;
}

DisplayMgr::FadeEffect DisplayMgr::getFadeEffect()
{
    FadeEffect                  currentFadeEffect;
    MutexGuard<MutexRecursive>  guard(m_mutex);

    currentFadeEffect = m_fadeEffectIndex;
    
    return currentFadeEffect;
}

bool DisplayMgr::movePluginToSlot(IPluginMaintenance* plugin, uint8_t slotId)
{
    bool status = false;

    if ((nullptr != plugin) &&
        (m_maxSlots > slotId))
    {
        uint8_t srcSlotId = getSlotIdByPluginUID(plugin->getUID());

        if ((m_maxSlots > srcSlotId) &&
            (srcSlotId != slotId))
        {
            Slot*                       srcSlot = &m_slots[srcSlotId];
            Slot*                       dstSlot = &m_slots[slotId];
            MutexGuard<MutexRecursive>  guard(m_mutex);

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

    return status;
}

void DisplayMgr::lockSlot(uint8_t slotId)
{
    if (m_maxSlots > slotId)
    {
        MutexGuard<MutexRecursive> guard(m_mutex);

        m_slots[slotId].lock();
    }

    return;
}

void DisplayMgr::unlockSlot(uint8_t slotId)
{
    if (m_maxSlots > slotId)
    {
        MutexGuard<MutexRecursive> guard(m_mutex);

        m_slots[slotId].unlock();
    }

    return;
}

bool DisplayMgr::isSlotLocked(uint8_t slotId)
{
    bool isLocked = true;

    if (m_maxSlots > slotId)
    {
        MutexGuard<MutexRecursive> guard(m_mutex);

        isLocked = m_slots[slotId].isLocked();
    }

    return isLocked;
}

uint32_t DisplayMgr::getSlotDuration(uint8_t slotId)
{
    uint32_t duration = 0U;

    if (m_maxSlots > slotId)
    {
        MutexGuard<MutexRecursive> guard(m_mutex);

        duration = m_slots[slotId].getDuration();
    }

    return duration;
}

bool DisplayMgr::setSlotDuration(uint8_t slotId, uint32_t duration, bool store)
{
    bool status = false;

    if (m_maxSlots > slotId)
    {
        MutexGuard<MutexRecursive> guard(m_mutex);

        if (m_slots[slotId].getDuration() != duration)
        {
            m_slots[slotId].setDuration(duration);

            /* Save slot configuration */
            if (true == store)
            {
                save();
            }
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
        MutexGuard<MutexRecursive>  guard(m_mutex);

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
            *slotId = m_selectedSlot;
        }
    }

    return;
}

void DisplayMgr::setNetworkStatus(bool isConnected)
{
    MutexGuard<MutexRecursive>  guard(m_mutex);

    m_isNetworkConnected = isConnected;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

DisplayMgr::DisplayMgr() :
    m_mutex(),
    m_taskHandle(nullptr),
    m_taskExit(false),
    m_xSemaphore(nullptr),
    m_slots(nullptr),
    m_maxSlots(0U),
    m_selectedSlot(SLOT_ID_INVALID),
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

    if (m_maxSlots <= slotId)
    {
        slotId = 0U;
    }
    else
    {
        ++slotId;
        slotId %= m_maxSlots;
    }

    /* Set next slot active, precondition is a installed plugin which is enabled.  */
    do
    {
        /* Plugin installed? */
        if (false == m_slots[slotId].isEmpty())
        {
            /* Plugin enabled? */
            if (true == m_slots[slotId].getPlugin()->isEnabled())
            {
                break;
            }
        }

        ++slotId;
        slotId %= m_maxSlots;

        ++count;
    }
    while (m_maxSlots > count);

    if (m_maxSlots <= count)
    {
        slotId = SLOT_ID_INVALID;
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

        /* Continously update the current canvas with its framebuffer. */
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

    return;
}

void DisplayMgr::process()
{
    IDisplay&                   display = Display::getInstance();
    uint8_t                     index   = 0U;
    MutexGuard<MutexRecursive>  guard(m_mutex);

    /* Handle display brightness */
    BrightnessCtrl::getInstance().process();

    /* Plugin requested to choose? */
    if (nullptr != m_requestedPlugin)
    {
        /* Requested plugin must be enabled, otherwise it won't be scheduled. */
        if (false == m_requestedPlugin->isEnabled())
        {
            LOG_WARNING("Requested plugin %s (UID %u) in slot %u is disabled.",
                m_requestedPlugin->getName(),
                m_requestedPlugin->getUID(),
                getSlotIdByPluginUID(m_requestedPlugin->getUID()));
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
                uint32_t duration = m_slots[m_selectedSlot].getDuration();

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
        m_selectedSlot = getSlotIdByPluginUID(m_selectedPlugin->getUID());

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
            uint8_t slotId = nextSlot(m_selectedSlot);

            /* If the next slot is the same as the current slot,
             * just restart the plugin duration timer.
             */
            if (m_selectedSlot == slotId)
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

    /* If no plugin is selected, choose the next on. */
    if (nullptr == m_selectedPlugin)
    {
        /* Plugin requested to choose? */
        if (nullptr != m_requestedPlugin)
        {
            m_selectedSlot      = getSlotIdByPluginUID(m_requestedPlugin->getUID());
            m_requestedPlugin   = nullptr;
        }
        /* Select next slot, which contains a enabled plugin. */
        else
        {
            m_selectedSlot = nextSlot(m_selectedSlot);
        }

        /* Next enabled plugin found? */
        if (m_maxSlots > m_selectedSlot)
        {
            uint32_t duration = 0U;

            m_selectedPlugin    = m_slots[m_selectedSlot].getPlugin();
            duration            = m_slots[m_selectedSlot].getDuration();

            /* If plugin shall not be infinite active, start the slot timer. */
            if (0U != duration)
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

            LOG_INFO("Slot %u (%s) now active.", m_selectedSlot, m_selectedPlugin->getName());
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
    for(index = 0U; index < m_maxSlots; ++index)
    {
        IPluginMaintenance* plugin = m_slots[index].getPlugin();

        if (nullptr != plugin)
        {
            plugin->process(m_isNetworkConnected);
        }
    }

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

    delay(1U);
    display.show();

    return;
}

void DisplayMgr::updateTask(void* parameters)
{
    DisplayMgr* tthis = reinterpret_cast<DisplayMgr*>(parameters);

    if ((nullptr != tthis) &&
        (nullptr != tthis->m_xSemaphore))
    {
#if (0 != CONFIG_DISPLAY_MGR_ENABLE_STATISTICS)
        Statistics      statistics;
        SimpleTimer     statisticsLogTimer;
        const uint32_t  STATISTICS_LOG_PERIOD   = 4000U;    /* [ms] */
        uint32_t        timestampLastUpdate     = millis();

        statisticsLogTimer.start(STATISTICS_LOG_PERIOD);

#endif /* (0 != CONFIG_DISPLAY_MGR_ENABLE_STATISTICS) */

        (void)xSemaphoreTake(tthis->m_xSemaphore, portMAX_DELAY);

        while(false == tthis->m_taskExit)
        {
            uint32_t    timestamp           = millis();
            uint32_t    duration            = 0U;
            uint32_t    timestampPhyUpdate  = millis();
            uint32_t    durationPhyUpdate   = 0U;
            bool        abort               = false;

            /* Observe the physical display refresh and limit the duration to 70% of refresh period. */
            const uint32_t  MAX_LOOP_TIME   = (TASK_PERIOD * 7U) / (10U);

            /* Refresh display content periodically */
            tthis->process();

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
            if (TASK_PERIOD <= duration)
            {
                delay(1U);
            }
            else
            {
                delay(TASK_PERIOD - duration);
            }

#if (0 != CONFIG_DISPLAY_MGR_ENABLE_STATISTICS)
            statistics.refreshPeriod.update(millis() - timestampLastUpdate);
            timestampLastUpdate = millis();
#endif /* (0 != CONFIG_DISPLAY_MGR_ENABLE_STATISTICS) */
        }

        (void)xSemaphoreGive(tthis->m_xSemaphore);
    }

    vTaskDelete(nullptr);

    return;
}

void DisplayMgr::load()
{
    Settings& settings = Settings::getInstance();

    if (nullptr == m_slots)
    {
        LOG_WARNING("No slot exists.");
    }
    else if (false == settings.open(true))
    {
        LOG_WARNING("Couldn't open filesystem.");
    }
    else
    {
        String config = settings.getDisplaySlotConfig().getValue();

        if (true == config.isEmpty())
        {
            LOG_WARNING("Display slot configuration is empty.");
        }
        else
        {
            const size_t            JSON_DOC_SIZE   = 512U;
            DynamicJsonDocument     jsonDoc(JSON_DOC_SIZE);
            DeserializationError    error           = deserializeJson(jsonDoc, config);

            if (true == jsonDoc.overflowed())
            {
                LOG_ERROR("JSON document has less memory available.");
            }
            else
            {
                LOG_INFO("JSON document size: %u", jsonDoc.memoryUsage());
            }

            if (DeserializationError::Ok != error.code())
            {
                LOG_WARNING("JSON deserialization failed: %s", error.c_str());
            }
            else if (false == jsonDoc["slots"].is<JsonArray>())
            {
                LOG_WARNING("Invalid JSON format.");
            }
            else
            {
                JsonArray   jsonSlots   = jsonDoc["slots"].as<JsonArray>();
                uint8_t     slotId      = 0;

                for(JsonObject jsonSlot: jsonSlots)
                {
                    if (true == jsonSlot["duration"].is<uint32_t>())
                    {
                        uint32_t duration = jsonSlot["duration"].as<uint32_t>();

                        m_slots[slotId].setDuration(duration);

                        ++slotId;
                        if (DisplayMgr::getInstance().getMaxSlots() <= slotId)
                        {
                            break;
                        }
                    }
                }
            }
        }

        settings.close();
    }
}

void DisplayMgr::save()
{
    if (nullptr != m_slots)
    {
        String              config;
        uint8_t             slotId      = 0;
        Settings&           settings    = Settings::getInstance();
        const size_t        JSON_DOC_SIZE   = 512U;
        DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);
        JsonArray           jsonSlots   = jsonDoc.createNestedArray("slots");

        for(slotId = 0; slotId < m_maxSlots; ++slotId)
        {
            JsonObject jsonSlot = jsonSlots.createNestedObject();

            jsonSlot["duration"] = m_slots[slotId].getDuration();
        }

        if (true == jsonDoc.overflowed())
        {
            LOG_ERROR("JSON document has less memory available.");
        }
        else
        {
            LOG_INFO("JSON document size: %u", jsonDoc.memoryUsage());
        }

        if (false == settings.open(false))
        {
            LOG_WARNING("Couldn't open filesystem.");
        }
        else
        {
            (void)serializeJson(jsonDoc, config);

            settings.getDisplaySlotConfig().setValue(config);
            settings.close();
        }
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
