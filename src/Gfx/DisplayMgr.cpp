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
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "DisplayMgr.h"
#include "LedMatrix.h"
#include "AmbientLightSensor.h"

#include <Logging.h>

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

/* Initialize LED matrix instance. */
DisplayMgr  DisplayMgr::m_instance;

/******************************************************************************
 * Public Methods
 *****************************************************************************/

bool DisplayMgr::begin()
{
    bool status = false;

    /* Not started yet? */
    if (nullptr == m_taskHandle)
    {
        /* Set the display brightness here just once.
        * There is no need to do this in the process() method periodically.
        */
        LedMatrix::getInstance().setBrightness(BRIGHTNESS_DEFAULT);

        /* Create mutex to lock/unlock display update */
        m_xMutex = xSemaphoreCreateMutex();

        /* Create binary semaphore to signal task exit. */
        m_xSemaphore = xSemaphoreCreateBinary();

        if ((nullptr != m_xMutex) &&
            (nullptr != m_xSemaphore))
        {
            BaseType_t  osRet   = pdFAIL;

            /* Task shall run */
            m_taskExit = false;

            osRet = xTaskCreateUniversal(   updateTask,
                                            "displayTask",
                                            TASK_STACKE_SIZE,
                                            this,
                                            4,
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

    /* Any error happened? */
    if (false == status)
    {
        if (nullptr != m_xMutex)
        {
            vSemaphoreDelete(m_xMutex);
            m_xMutex = nullptr;
        }

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

        vSemaphoreDelete(m_xMutex);
        m_xMutex = nullptr;
    }

    return;
}

bool DisplayMgr::enableAutoBrightnessAdjustment(bool enable)
{
    bool status = true;

    /* Disable automatic brightness adjustment? */
    if (false == enable)
    {
        m_autoBrightnessTimer.stop();
    }
    /* Enable automatic brightness adjustment */
    else
    {
        /* If no ambient light sensor is available, enable it makes no sense. */
        if (false == AmbientLightSensor::getInstance().isSensorAvailable())
        {
            status = false;
        }
        /* Ambient light sensor is available */
        else
        {
            /* Display brightness will be automatically adjusted in the process() method. */
            m_autoBrightnessTimer.start(ALS_AUTO_ADJUST_PERIOD);
        }
    }

    return status;
}

uint8_t DisplayMgr::installPlugin(Plugin* plugin, uint8_t slotId)
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
            lock();

            slotId = 0U;
            while((MAX_SLOTS > slotId) && (nullptr != m_slots[slotId]))
            {
                ++slotId;
            }

            if (MAX_SLOTS > slotId)
            {
                m_slots[slotId] = plugin;

                plugin->setSlotId(slotId);
                plugin->start();
            }
            else
            {
                slotId = SLOT_ID_INVALID;
            }

            unlock();
        }
        /* Install to specific slot? */
        else if ((MAX_SLOTS > slotId) &&
                 (nullptr == m_slots[slotId]))
        {
            lock();

            m_slots[slotId] = plugin;

            plugin->setSlotId(slotId);
            plugin->start();

            unlock();
        }
        else
        {
            slotId = SLOT_ID_INVALID;
        }

        if (MAX_SLOTS > slotId)
        {
            LOG_INFO("Plugin %s installed in slot %u.", plugin->getName(), slotId);
        }
    }

    return slotId;
}

void DisplayMgr::uninstallPlugin(Plugin* plugin)
{
    if (nullptr != plugin)
    {
        if (MAX_SLOTS <= plugin->getSlotId())
        {
            LOG_WARNING("Plugin %s has invalid slot id %u.", plugin->getName(), plugin->getSlotId());
        }
        else
        {
            uint8_t slotId = plugin->getSlotId();

            lock();

            /* Is this plugin selected at the moment? */
            if (m_selectedPlugin == plugin)
            {
                /* Remove selection */
                m_selectedPlugin = nullptr;
            }

            plugin->stop();
            m_slots[slotId] = nullptr;
            plugin->setSlotId(SLOT_ID_INVALID);

            unlock();

            LOG_INFO("Plugin %s removed from slot %u.", plugin->getName(), slotId);
        }
    }

    return;
}

Plugin* DisplayMgr::getPluginInSlot(uint8_t slotId)
{
    Plugin* plugin = nullptr;

    if (MAX_SLOTS > slotId)
    {
        lock();

        plugin = m_slots[slotId];

        unlock();
    }

    return plugin;
}

void DisplayMgr::activatePlugin(Plugin* plugin)
{
    if (nullptr != plugin)
    {
        lock();

        if (MAX_SLOTS > plugin->getSlotId())
        {
            if (plugin != m_slots[plugin->getSlotId()])
            {
                LOG_WARNING("Plugin %s should be in slot %u, but isn't!", plugin->getName(), plugin->getSlotId());
            }
            else
            {
                m_requestedPlugin = plugin;
            }
        }

        unlock();
    }

    return;
}

void DisplayMgr::getFBCopy(uint32_t* fb, size_t length, uint8_t* slotId)
{
    if ((nullptr != fb) &&
        (0 < length))
    {
        LedMatrix&  matrix  = LedMatrix::getInstance();
        int16_t     x       = 0;
        int16_t     y       = 0;
        size_t      index   = 0;

        lock();

        /* Copy framebuffer after it is completely updated. */
        for(y = 0; y < matrix.height(); ++y)
        {
            for(x = 0; x < matrix.width(); ++x)
            {
                fb[index] = ColorDef::convert565To888(matrix.getColor(x, y));
                ++index;

                if (length <= index)
                {
                    break;
                }
            }
        }

        if (NULL != slotId)
        {
            *slotId = m_selectedSlot;
        }

        unlock();
    }

    return;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

DisplayMgr::DisplayMgr() :
    m_xMutex(nullptr),
    m_taskHandle(nullptr),
    m_taskExit(false),
    m_xSemaphore(nullptr),
    m_slots(),
    m_selectedSlot(SLOT_ID_INVALID),
    m_selectedPlugin(nullptr),
    m_requestedPlugin(nullptr),
    m_slotTimer(),
    m_autoBrightnessTimer()
{
    uint8_t index = 0U;

    /* Initialize all slots */
    for(index = 0U; index < MAX_SLOTS; ++index)
    {
        m_slots[index] = nullptr;
    }
}

DisplayMgr::~DisplayMgr()
{
    /* Will never be called. */
}

uint8_t DisplayMgr::nextSlot(uint8_t slotId)
{
    uint8_t count = 0U;

    if (MAX_SLOTS <= slotId)
    {
        slotId = 0U;
    }
    else
    {
        ++slotId;
        slotId %= MAX_SLOTS;
    }

    /* Set next slot active, precondition is a installed plugin which is enabled.  */
    do
    {
        /* Plugin installed? */
        if (nullptr != m_slots[slotId])
        {
            /* Plugin enabled? */
            if (true == m_slots[slotId]->isEnabled())
            {
                break;
            }
        }

        ++slotId;
        slotId %= MAX_SLOTS;

        ++count;
    }
    while (MAX_SLOTS > count);

    if (MAX_SLOTS <= count)
    {
        slotId = SLOT_ID_INVALID;
    }

    return slotId;
}

void DisplayMgr::process()
{
    LedMatrix&  matrix  = LedMatrix::getInstance();
    uint8_t     index   = 0U;

    /* Ambient light sensor available for automatic brightness adjustment? */
    if (true == m_autoBrightnessTimer.isTimerRunning())
    {
        float   lightNormalized         = AmbientLightSensor::getInstance().getNormalizedLight();
        uint8_t BRIGHTNESS_DYN_RANGE    = UINT8_MAX - BRIGHTNESS_MIN;
        float   fBrightness             = static_cast<float>(BRIGHTNESS_MIN) + ( static_cast<float>(BRIGHTNESS_DYN_RANGE) * lightNormalized );
        uint8_t brightness              = static_cast<uint8_t>(fBrightness);

        matrix.setBrightness(brightness);

        m_autoBrightnessTimer.restart();
    }

    lock();

    /* Plugin requested to choose? */
    if (nullptr != m_requestedPlugin)
    {
        /* Plugin must be enabled. */
        if (false == m_requestedPlugin->isEnabled())
        {
            LOG_WARNING("Requested plugin %s in slot %u is disabled.", m_requestedPlugin->getName(), m_requestedPlugin->getSlotId());
            m_requestedPlugin = nullptr;
        }
        /* Plugin selected? */
        else if (nullptr != m_selectedPlugin)
        {
            /* Remove selected plugin, which forces to select the requested one. */
            m_selectedPlugin->inactive();
            m_selectedPlugin = nullptr;
        }
        else
        {
            /* Nothing to do. */
            ;
        }
    }

    /* Plugin selected? */
    if (nullptr != m_selectedPlugin)
    {
        m_selectedSlot = m_selectedPlugin->getSlotId();

        /* Plugin disabled in the meantime? */
        if (false == m_selectedPlugin->isEnabled())
        {
            m_selectedPlugin->inactive();
            m_selectedPlugin = nullptr;
            m_slotTimer.stop();
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
            }
        }
        else
        {
            /* Nothing to do. */
            ;
        }
    }

    /* No plugin selected? */
    if (nullptr == m_selectedPlugin)
    {
        /* Plugin requested to choose? */
        if (nullptr != m_requestedPlugin)
        {
            m_selectedSlot      = m_requestedPlugin->getSlotId();
            m_requestedPlugin   = nullptr;
        }
        /* Select next slot, which contains a enabled plugin. */
        else
        {
            m_selectedSlot = nextSlot(m_selectedSlot);
        }

        /* Next enabled plugin found? */
        if (MAX_SLOTS > m_selectedSlot)
        {
            uint32_t duration = 0U;

            m_selectedPlugin    = m_slots[m_selectedSlot];
            duration            = m_selectedPlugin->getDuration();

            if (Plugin::DURATION_INFINITE != duration)
            {
                m_slotTimer.start(duration);
            }

            m_selectedPlugin->active(matrix);
            LOG_INFO("Slot %u (%s) now active.", m_selectedSlot, m_selectedPlugin->getName());
        }
    }

    /* Process all installed plugins. */
    for(index = 0U; index < MAX_SLOTS; ++index)
    {
        Plugin* plugin = m_slots[index];

        if (nullptr != plugin)
        {
            plugin->process();
        }
    }

    /* Update display */
    if (nullptr != m_selectedPlugin)
    {
        m_selectedPlugin->update(matrix);
    }

    matrix.show();

    unlock();

    return;
}

void DisplayMgr::updateTask(void* parameters)
{
    DisplayMgr* displayMgr = reinterpret_cast<DisplayMgr*>(parameters);

    if ((nullptr != displayMgr) &&
        (nullptr != displayMgr->m_xSemaphore))
    {
        (void)xSemaphoreTake(displayMgr->m_xSemaphore, portMAX_DELAY);

        while(false == displayMgr->m_taskExit)
        {
            /* Refresh display content periodically */
            displayMgr->process();

            /* Wait until the physical update is ready to avoid flickering
             * and artifacts on the display, because of e.g. webserver flash
             * access.
             */
            while(false == LedMatrix::getInstance().isReady())
            {
                ;
            }

            delay(TASK_PERIOD);
        }

        (void)xSemaphoreGive(displayMgr->m_xSemaphore);
    }

    vTaskDelete(nullptr);

    return;
}

void DisplayMgr::lock()
{
    if (nullptr != m_xMutex)
    {
        (void)xSemaphoreTake(m_xMutex, portMAX_DELAY);
    }

    return;
}

void DisplayMgr::unlock()
{
    if (nullptr != m_xMutex)
    {
        (void)xSemaphoreGive(m_xMutex);
    }

    return;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
