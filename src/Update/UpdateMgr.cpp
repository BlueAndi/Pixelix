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
 * @brief  Update manager
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "UpdateMgr.h"

#include <SPIFFS.h>
#include <Logging.h>
#include <Esp.h>

#include "LedMatrix.h"
#include "MyWebServer.h"
#include "Settings.h"
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

/* Instance of the update manager. */
UpdateMgr   UpdateMgr::m_instance;

/******************************************************************************
 * Public Methods
 *****************************************************************************/

bool UpdateMgr::init()
{
    String  hostname;

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

    /* Get hostname */
    if (false == Settings::getInstance().open(true))
    {
        LOG_WARNING("Use default hostname.");
        hostname = Settings::HOSTNAME_DEFAULT;
    }
    else
    {
        hostname = Settings::getInstance().getHostname();
        Settings::getInstance().close();
    }

    ArduinoOTA.setHostname(hostname.c_str());

    /* Initialization successful */
    m_isInitialized = true;

    return m_isInitialized;
}

void UpdateMgr::begin()
{
    /* Start over-the-air server */
    ArduinoOTA.begin();

    LOG_INFO(String("OTA hostname: ") + ArduinoOTA.getHostname());
    LOG_INFO(String("Sketch size: ") + ESP.getSketchSize() + " bytes");
    LOG_INFO(String("Free size: ") + ESP.getFreeSketchSpace() + " bytes");

    return;
}

void UpdateMgr::end()
{
    /* Stop over-the-air server */
    ArduinoOTA.end();
    return;
}

void UpdateMgr::process()
{
    if (true == m_isInitialized)
    {
        ArduinoOTA.handle();
    }

    return;
}

void UpdateMgr::beginProgress()
{
    /* Stop display manager */
    DisplayMgr::getInstance().end();

    m_updateIsRunning   = true;
    m_progress          = UINT8_MAX; // Force update
    m_textWidget.setFormatStr("Update");

    /* Show user update status */
    updateProgress(0U);

    return;
}

void UpdateMgr::updateProgress(uint8_t progress)
{
    if (m_progress != progress)
    {
        m_progress = progress;

        LOG_INFO(String("[") + m_progress + "%]");
        m_progressBar.setProgress(m_progress);

        /* Update display manually. Note, that this must be done to avoid
         * artifacts on the display, caused by long flash write cycles.
         */
        LedMatrix::getInstance().clear();
        m_progressBar.update(LedMatrix::getInstance()); // Draw the progress bar in the background
        m_textWidget.update(LedMatrix::getInstance());  // Overlay with the text
        LedMatrix::getInstance().show();

        /* Wait until the LED matrix is updated to avoid artifacts on the
         * display.
         */
        while(false == LedMatrix::getInstance().isReady())
        {
            /* Just wait. */
            ;
        }
    }

    return;
}

void UpdateMgr::endProgress()
{
    /* Start display manager */
    if (false == DisplayMgr::getInstance().begin())
    {
        LOG_WARNING("Couldn't initialize display manager again.");
    }

    return;
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
    m_progressBar()
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

    /* Stop webserver, before SPIFFS may be unmounted. */
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
        SPIFFS.end();
    }

    LOG_INFO(infoStr);

    m_instance.beginProgress();

    return;
}

void UpdateMgr::onEnd()
{
    String  infoStr = "Update successful finished.";

    m_instance.m_updateIsRunning = false;

    LOG_INFO(infoStr);

    m_instance.endProgress();

    /* Note, there is no need here to start the webserver or the display
     * manager again, because we request a restart of the system now.
     */
    m_instance.reqRestart();

    return;
}

void UpdateMgr::onProgress(unsigned int progress, unsigned int total)
{
    const uint32_t  PROGRESS_PERCENT    = (progress * 100U) / total;

    m_instance.updateProgress(PROGRESS_PERCENT);

    return;
}

void UpdateMgr::onError(ota_error_t error)
{
    String infoStr;

    m_instance.m_updateIsRunning = false;
    
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

    m_instance.endProgress();

    /* Reset only if the error happened during update.
     * Security note: This avoids a reset in case the authentication failed.
     */
    if (true == m_instance.m_updateIsRunning)
    {
        /* Start display manager */
        if (false == DisplayMgr::getInstance().begin())
        {
            LOG_WARNING("Couldn't initialize display manager again.");
        }

        SysMsg::getInstance().show(infoStr);
        delay(infoStr.length() * 600U);

        /* Request a restart */
        m_instance.reqRestart();
    }

    return;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
