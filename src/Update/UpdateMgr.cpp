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
 * @brief  Update manager
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "UpdateMgr.h"

#include <Logging.h>
#include <Esp.h>
#include <Display.h>
#include <SettingsService.h>

#include "FileSystem.h"
#include "MyWebServer.h"
#include "DisplayMgr.h"
#include "SysMsg.h"
#include "PluginMgr.h"

#include "TextWidget.h"
#include "ProgressBar.h"

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

/* Set over-the-air update password */
const char* UpdateMgr::OTA_PASSWORD = "maytheforcebewithyou";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

bool UpdateMgr::init()
{
    String              hostname;
    SettingsService&    settings    = SettingsService::getInstance();

    /* Prepare over the air update. Note, the configuration must be done
     * before the update server is running.
     */
    ArduinoOTA.setPassword(OTA_PASSWORD);
    ArduinoOTA.onStart(onStart);
    ArduinoOTA.onEnd(onEnd);
    ArduinoOTA.onProgress(onProgress);
    ArduinoOTA.onError(onError);

    /* Don't reboot on success, this will be done in the RESTART state later.
     * Note, currently the ArduinoOTA only uses ESP.restart(), which doesn't
     * reset the peripherals.
     */
    ArduinoOTA.setRebootOnSuccess(false);

    /* Do not additional enable mDNS. */
    ArduinoOTA.setMdnsEnabled(false);

    /* Get hostname */
    if (false == settings.open(true))
    {
        LOG_WARNING("Use default hostname.");
        hostname = settings.getHostname().getDefault();
    }
    else
    {
        hostname = settings.getHostname().getValue();
        settings.close();
    }

    ArduinoOTA.setHostname(hostname.c_str());

    /* Initialization successful */
    m_isInitialized = true;

    return m_isInitialized;
}

void UpdateMgr::begin()
{
    if (true == m_isInitialized)
    {
        /* Start over-the-air server */
        ArduinoOTA.begin();

        LOG_INFO(String("OTA hostname: ") + ArduinoOTA.getHostname());
        LOG_INFO(String("Sketch size: ") + ESP.getSketchSize() + " bytes");
        LOG_INFO(String("Free size: ") + ESP.getFreeSketchSpace() + " bytes");
    }
}

void UpdateMgr::end()
{
    if (true == m_isInitialized)
    {
        /* Stop over-the-air server */
        ArduinoOTA.end();
    }
}

void UpdateMgr::process()
{
    if (true == m_isInitialized)
    {
        ArduinoOTA.handle();

        /* Delayed restart request? */
        if ((true == m_timer.isTimerRunning()) &&
            (true == m_timer.isTimeout()))
        {
            m_isRestartReq = true;
            m_timer.stop();
        }
    }
}

void UpdateMgr::beginProgress()
{
    if (true == m_isInitialized)
    {
        /* Stop display manager */
        DisplayMgr::getInstance().end();

        m_updateIsRunning   = true;
        m_progress          = UINT8_MAX; // Force update
        m_textWidget.setFormatStr("Update");

        /* Show user update status */
        updateProgress(0U);
    }
}

void UpdateMgr::updateProgress(uint8_t progress)
{
    if ((true == m_isInitialized) &&
        (m_progress != progress))
    {
        m_progress = progress;

        m_progressBar.setProgress(m_progress);

        /* Update display manually. Note, that this must be done to avoid
         * artifacts on the display, caused by long flash write cycles.
         */
        Display::getInstance().clear();
        m_progressBar.update(Display::getInstance()); // Draw the progress bar in the background
        m_textWidget.update(Display::getInstance());  // Overlay with the text
        Display::getInstance().show();

        /* Wait until the LED matrix is updated to avoid artifacts on the
         * display.
         */
        while(false == Display::getInstance().isReady())
        {
            /* Just wait and give other tasks a chance. */
            delay(1U);
        }

        /* Show update status on console. */
        LOG_INFO(String("[") + m_progress + "%]");
    }
}

void UpdateMgr::endProgress()
{
    if (true == m_isInitialized)
    {
        /* Start display manager */
        if (false == DisplayMgr::getInstance().begin())
        {
            LOG_WARNING("Couldn't initialize display manager again.");
        }
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

UpdateMgr::UpdateMgr() :
    m_isInitialized(false),
    m_updateIsRunning(false),
    m_progress(0U),
    m_isRestartReq(false),
    m_textWidget(),
    m_progressBar(),
    m_timer()
{
    /* Move text for a better look. */
    m_textWidget.move(1, 1);
}

UpdateMgr::~UpdateMgr()
{
}

void UpdateMgr::onStart()
{
    String infoStr = "Update ";

    /* Stop webserver, before filesystem may be unmounted. */
    MyWebServer::end();

    /* Shall the firmware be updated? */
    if (U_FLASH == ArduinoOTA.getCommand())
    {
        infoStr += "sketch.";
    }
    /* The filesystem will be updated. */
    else
    {
        infoStr += "filesystem.";

        /* Close filesystem before continue.
         * Note, this needs a restart after update is finished.
         */
        FILESYSTEM.end();
    }

    LOG_INFO(infoStr);

    getInstance().beginProgress();
}

void UpdateMgr::onEnd()
{
    String  infoStr = "Update successful finished.";

    getInstance().m_updateIsRunning = false;

    LOG_INFO(infoStr);

    getInstance().endProgress();

    /* Note, there is no need here to start the webserver or the display
     * manager again, because we request a restart of the system now.
     */
    getInstance().reqRestart(0U);
}

void UpdateMgr::onProgress(unsigned int progress, unsigned int total)
{
    const uint32_t  PROGRESS_PERCENT    = (progress * 100U) / total;

    getInstance().updateProgress(PROGRESS_PERCENT);
}

void UpdateMgr::onError(ota_error_t error)
{
    String infoStr;

    switch(error)
    {
    case OTA_AUTH_ERROR:
        infoStr = "OTA - Authentication error.";
        break;

    case OTA_BEGIN_ERROR:
        infoStr = "OTA - Begin error.";
        break;

    case OTA_CONNECT_ERROR:
        infoStr = "OTA - Connect error.";
        break;

    case OTA_RECEIVE_ERROR:
        infoStr = "OTA - Receive error.";
        break;

    case OTA_END_ERROR:
        infoStr = "OTA - End error.";
        break;

    default:
        infoStr = "OTA - Unknown error.";
        break;
    }

    LOG_INFO(infoStr);

    /* Mount filesystem, because it may be unmounted in case of failed filesystem update. */
    if (false == FILESYSTEM.begin())
    {
        /* To ensure the log information will be shown. */
        const uint32_t RESTART_DELAY = 100U; /* ms */

        LOG_FATAL("Couldn't mount filesystem.");

        getInstance().reqRestart(RESTART_DELAY);
    }
    else
    {
        getInstance().endProgress();

        /* Reset only if the error happened during update.
         * Security note: This avoids a reset in case the authentication failed.
         */
        if (true == getInstance().m_updateIsRunning)
        {
            const uint32_t  DURATION_NON_SCROLLING  = 4000U; /* ms */
            const uint32_t  SCROLLING_REPEAT_NUM    = 2U;

            SysMsg::getInstance().show(infoStr, DURATION_NON_SCROLLING, SCROLLING_REPEAT_NUM);

            /* Request a restart */
            getInstance().reqRestart(DURATION_NON_SCROLLING);
        }
    }

    getInstance().m_updateIsRunning = false;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
