/* MIT License
 *
 * Copyright (c) 2019 - 2025 Andreas Merkle <web@blue-andi.de>
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
#include <Display.h>
#include <SettingsService.h>

#include "Services.h"
#include "FileSystem.h"
#include "MyWebServer.h"
#include "DisplayMgr.h"
#include "SysMsg.h"
#include "PluginMgr.h"
#include "SensorDataProvider.h"
#include "TextWidget.h"
#include "ProgressBar.h"
#include "Topics.h"

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
    String           hostname;
    SettingsService& settings = SettingsService::getInstance();

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
        String hostname = ArduinoOTA.getHostname();

        /* Start over-the-air server */
        ArduinoOTA.begin();

        LOG_INFO("Arduino-OTA ready (hostname: %s).", hostname.c_str());
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

void UpdateMgr::prepareUpdate(bool isFilesystemUpdate)
{
    if (true == m_isInitialized)
    {
        /* Avoid any external request. */
        Topics::end();

        /* Stop display manager first, because this will stop the plugin
         * processing at all.
         */
        DisplayMgr::getInstance().end();

        /* Unregister sensor topics (no purge). */
        SensorDataProvider::getInstance().end();

        /* Unregister all plugin topics (no purge). */
        PluginMgr::getInstance().unregisterAllPluginTopics();

        /* Disable HomeAssistant MQTT automatic discovery to avoid that the welcome plugin
         * will be discovered, after a filesystem update.
         */
        if (true == isFilesystemUpdate)
        {
            SettingsService& settings                = SettingsService::getInstance();

            /* Key see HomeAssistantMqtt::KEY_HA_DISCOVERY_ENABLE
             * Include the header is not possible, because MQTT might not be compiled in.
             */
            KeyValue* kvHomeAssistantEnableDiscovery = settings.getSettingByKey("ha_ena");

            if ((nullptr != kvHomeAssistantEnableDiscovery) &&
                (KeyValue::TYPE_BOOL == kvHomeAssistantEnableDiscovery->getValueType()))
            {
                if (true == settings.open(false))
                {
                    KeyValueBool* homeAssistantEnableDiscovery = static_cast<KeyValueBool*>(kvHomeAssistantEnableDiscovery);

                    homeAssistantEnableDiscovery->setValue(false);
                    settings.close();

                    LOG_INFO("HA discovery disabled for filesystem update.");
                }
            }
        }

        /* Stop services, but keep webserver running! */
        Services::stopAll();

        if (true == isFilesystemUpdate)
        {
            /* Close filesystem before continue.
             * Note, this needs a restart after update is finished.
             */
            FILESYSTEM.end();
        }

        m_updateIsRunning = true;
        m_progress        = UINT8_MAX; /* Force progress update of inital value. */
        m_textWidget.setFormatStr("Update");
    }
}

void UpdateMgr::prepareForRestart()
{
    getInstance().m_updateIsRunning = false;

    /* Mount filesystem, because it may be unmounted because of an filesystem
     * update.
     */
    if (false == FILESYSTEM.begin())
    {
        LOG_FATAL("Couldn't mount filesystem.");
    }
}

void UpdateMgr::beginProgress()
{
    if (true == m_updateIsRunning)
    {
        /* Show user update status */
        updateProgress(0U);
    }
}

void UpdateMgr::updateProgress(uint8_t progress)
{
    if ((true == m_updateIsRunning) &&
        (m_progress != progress))
    {
        m_progress = progress;
        m_progressBar.setProgress(m_progress);
        updateDisplay(true);

        /* Show update status on console. */
        LOG_INFO(String("[") + m_progress + "%]");
    }
}

void UpdateMgr::endProgress()
{
    if (true == m_updateIsRunning)
    {
        m_textWidget.setFormatStr("...");
        updateDisplay(false);
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
    m_textWidget(CONFIG_LED_MATRIX_WIDTH, CONFIG_LED_MATRIX_HEIGHT, 1, 1),
    m_progressBar(CONFIG_LED_MATRIX_WIDTH, CONFIG_LED_MATRIX_HEIGHT),
    m_timer()
{
    /* Disable fade effect, because the update() method will not be
     * called periodially but only on update progress change.
     * Therefore the text widget fade effect won't look good.
     */
    m_textWidget.disableFadeEffect();
}

UpdateMgr::~UpdateMgr()
{
}

void UpdateMgr::updateDisplay(bool showProgress)
{
    Display& display = Display::getInstance();

    /* Update display manually. Note, that this must be done to avoid
     * artifacts on the display, caused by long flash write cycles.
     */
    display.fillScreen(ColorDef::BLACK);
    if (true == showProgress)
    {
        m_progressBar.update(display); /* Draw the progress bar in the background. */
    }
    m_textWidget.update(display); /* Overlay with the text. */
    display.show();

    /* Wait until the LED matrix is updated to avoid artifacts on the
     * display.
     */
    while (false == display.isReady())
    {
        /* Just wait and give other tasks a chance. */
        delay(1U);
    }
}

void UpdateMgr::onStart()
{
    String infoStr            = "Start OTA update of ";
    bool   isFilesystemUpdate = false;

    /* Shall the firmware be updated? */
    if (U_FLASH == ArduinoOTA.getCommand())
    {
        infoStr += "firmware.";
    }
    /* The filesystem will be updated. */
    else if (U_SPIFFS == ArduinoOTA.getCommand())
    {
        infoStr            += "filesystem.";

        isFilesystemUpdate  = true;
    }
    else
    {
        infoStr += "unknown type.";
    }

    LOG_INFO(infoStr);

    /* Stop webserver, before filesystem may be unmounted.
     * This can not be moved to prepareUpdate(), because the update may come
     * via the webserver. Therefore it can only be stopped in case of
     * ArudinoOTA.
     */
    MyWebServer::end();

    getInstance().prepareUpdate(isFilesystemUpdate);
    getInstance().beginProgress();
}

void UpdateMgr::onEnd()
{
    LOG_INFO("Update successful finished.");

    getInstance().m_updateIsRunning = false;
    getInstance().endProgress();
    getInstance().prepareForRestart();

    /* Note, there is no need here to start the webserver or the display
     * manager again, because we request a restart of the system now.
     */
    getInstance().reqRestart(0U);
}

void UpdateMgr::onProgress(unsigned int progress, unsigned int total)
{
    const uint32_t PROGRESS_PERCENT = (progress * 100U) / total;

    getInstance().updateProgress(PROGRESS_PERCENT);
}

void UpdateMgr::onError(ota_error_t error)
{
    const uint32_t RESTART_DELAY = 4000U; /* ms */
    String         errorStr;

    /* Keep error information short to avoid that text scrolling is needed.
     * Because the display manager is stopped during the update.
     */
    switch (error)
    {
    case OTA_AUTH_ERROR:
        errorStr = "EAuth";
        break;

    case OTA_BEGIN_ERROR:
        errorStr = "EBegin";
        break;

    case OTA_CONNECT_ERROR:
        errorStr = "EErr";
        break;

    case OTA_RECEIVE_ERROR:
        errorStr = "ERcv";
        break;

    case OTA_END_ERROR:
        errorStr = "EEnd";
        break;

    default:
        errorStr = "EUndef";
        break;
    }

    LOG_ERROR(errorStr);

    /* If the authentication fails, the onError() is called and there is no
     * running update. Therefore no restart is necessary, just notify
     * the user.
     */
    if (false == getInstance().m_updateIsRunning)
    {
        const uint32_t DURATION_NON_SCROLLING = 4000U; /* ms */
        const uint32_t SCROLLING_REPEAT_NUM   = 1U;

        SysMsg::getInstance().show(errorStr, DURATION_NON_SCROLLING, SCROLLING_REPEAT_NUM);
    }
    else
    {
        getInstance().endProgress();
        getInstance().prepareForRestart();

        getInstance().m_textWidget.setFormatStr(errorStr);
        getInstance().updateDisplay(false);

        getInstance().reqRestart(RESTART_DELAY);
    }
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
