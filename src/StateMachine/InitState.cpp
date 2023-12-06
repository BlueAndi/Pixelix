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
 * @brief  System state: Init
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "InitState.h"

#include <Arduino.h>
#include <WiFi.h>
#include <Board.h>
#include <Display.h>
#include <SensorDataProvider.h>
#include <Wire.h>
#include <IconTextPlugin.h>

#include "ButtonDrv.h"
#include "ClockDrv.h"
#include "DisplayMgr.h"
#include "SysMsg.h"
#include "Version.h"
#include "MyWebServer.h"
#include "UpdateMgr.h"
#include "PluginMgr.h"
#include "WebConfig.h"
#include "FileSystem.h"
#include "JsonFile.h"
#include "Version.h"
#include "Services.h"
#include "WiFiUtil.h"

#include "APState.h"
#include "ConnectingState.h"
#include "ErrorState.h"

#include <Logging.h>
#include <Util.h>
#include <ESPmDNS.h>
#include <SettingsService.h>

#include <lwip/init.h>

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

/**
 * The filename of the version information file.
 */
static const char* VERSION_FILE_NAME        = "/version.json";

/**
 * Plugin type of the welcome plugin. This is used to install it in the very
 * first startup. In further startups it is used in addition to the plugin
 * alias whether to show the welcome icon and message.
 */
static const char*  WELCOME_PLUGIN_TYPE     = "IconTextPlugin";

/**
 * The alias of the welcome plugin. This is used to determine in addition to
 * the plugin type whether to show the welcome icon and message after a reboot
 * again.
 */
static const char*  WELCOME_PLUGIN_ALIAS    = "_welcome";

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void InitState::entry(StateMachine& sm)
{
    bool                isError     = false;
    ErrorState::ErrorId errorId     = ErrorState::ERROR_ID_UNKNOWN;
    SettingsService&    settings    = SettingsService::getInstance();
    String              uniqueId;

    /* Initialize hardware */
    Board::init();

    /* Show as soon as possible the user on the serial console that the system is booting. */
    showStartupInfoOnSerial();

    /* To avoid name clashes, add a unqiue id to some of the default values. */
    WiFiUtil::addDeviceUniqueId(uniqueId);
    settings.getWifiApSSID().setUniqueId(uniqueId);
    settings.getHostname().setUniqueId(uniqueId);

    /* Set two-wire (I2C) pins, before calling begin(). */
    if (false == Wire.setPins(Board::Pin::i2cSdaPinNo, Board::Pin::i2cSclPinNo))
    {
        LOG_FATAL("Couldn't set two-wire pins.");
        errorId = ErrorState::ERROR_ID_TWO_WIRE_ERROR;
        isError = true;
    }
    /* Initialize two-wire (I2C) */
    else if (false == Wire.begin())
    {
        LOG_FATAL("Couldn't initialize two-wire.");
        errorId = ErrorState::ERROR_ID_TWO_WIRE_ERROR;
        isError = true;
    }
    /* Initialize button driver */
    else if (false == ButtonDrv::getInstance().init())
    {
        LOG_FATAL("Couldn't initialize button driver.");
        errorId = ErrorState::ERROR_ID_NO_USER_BUTTON;
        isError = true;
    }
    /* Mounting the filesystem. */
    else if (false == FILESYSTEM.begin())
    {
        LOG_FATAL("Couldn't mount the filesystem.");
        errorId = ErrorState::ERROR_ID_BAD_FS;
        isError = true;
    }
    /* Check whether the filesystem is valid.
     * This is simply done by checking for a specific file in the root directory.
     */
    else if (false == FILESYSTEM.exists(VERSION_FILE_NAME))
    {
        LOG_FATAL("Filesystem is invalid.");
        errorId = ErrorState::ERROR_ID_BAD_FS;
        isError = true;
    }
    /* Start all services */
    else if (false == Services::startAll())
    {
        LOG_FATAL("Starting services failed.");
        errorId = ErrorState::ERROR_ID_SERVICE;
        isError = true;
    }
    else
    {
        /* Initialize clock driver */
        ClockDrv::getInstance().init(&m_rtcDrv);

        /* Initialize sensors */
        SensorDataProvider::getInstance().begin();

        /* Prepare everything for the plugins. */
        PluginMgr::getInstance().begin();
    }

    /* Continue only if there is no error yet. */
    if (true == isError)
    {
        /* Error detected. */
        ;
    }
    /* Start display */
    else if (false == Display::getInstance().begin())
    {
        LOG_FATAL("Failed to initialize display.");
        /* To set a error id here, makes no sense, because it can not be shown. */
        isError = true;
    }
    /* Initialize display manager */
    else if (false == DisplayMgr::getInstance().begin())
    {
        LOG_FATAL("Failed to initialize display manager.");
        errorId = ErrorState::ERROR_ID_DISP_MGR;
        isError = true;
    }
    /* Initialize system message handler */
    else if (false == SysMsg::getInstance().init())
    {
        LOG_FATAL("Failed to initialize system message handler.");
        errorId = ErrorState::ERROR_ID_SYS_MSG;
        isError = true;
    }
    /* Initialize over-the-air update server */
    else if (false == UpdateMgr::getInstance().init())
    {
        LOG_FATAL("Failed to initialize Arduino OTA.");
        errorId = ErrorState::ERROR_ID_UPDATE_MGR;
        isError = true;
    }
    else
    {
        /* Clean up settings first! Important step after a firmware update to
         * keep the settings up-to-date and prevent the persistency will
         * silently growing up with unused stuff.
         */
        if (true == settings.open(false))
        {
            LOG_INFO("Clean up settings.");
            settings.cleanUp();
            settings.close();
        }

        /* Load some general configuration parameters from persistent memory. */
        if (true == settings.open(true))
        {
            /* Enable or disable the automatic display brightness adjustment,
             * depended on settings. Enable it may fail in case there is no
             * LDR sensor available.
             */
            bool isEnabled = settings.getAutoBrightnessAdjustment().getValue();

            if (false == DisplayMgr::getInstance().setAutoBrightnessAdjustment(isEnabled))
            {
                LOG_WARNING("Failed to enable autom. brightness adjustment.");
            }

            /* Set text scroll pause for all text widgets. */
            uint32_t scrollPause = settings.getScrollPause().getValue();
            if (false == TextWidget::setScrollPause(scrollPause))
            {
                LOG_WARNING("Scroll pause %u ms couldn't be set.", scrollPause);
            }

            m_isQuiet = settings.getQuietMode().getValue();

            settings.close();
        }
        else
        {
            m_isQuiet = settings.getQuietMode().getDefault();
        }

        /* Don't store the wifi configuration in the NVS.
         * This seems to cause a reset after a client connected to the access point.
         * https://github.com/espressif/arduino-esp32/issues/2025#issuecomment-503415364
         */
        WiFi.persistent(false);

        /* Show some informations on the display. */
        showStartupInfoOnDisplay(m_isQuiet);

        /* Show a warning in case the filesystem may not be compatible to the firmware version. */
        if (false == isFsCompatible())
        {
            const uint32_t  DURATION_NON_SCROLLING  = 3000U; /* ms */
            const uint32_t  SCROLLING_REPEAT_NUM    = 1U;
            const uint32_t  DURATION_PAUSE          = 500U; /* ms */
            const uint32_t  SCROLLING_NO_REPEAT     = 0U;
            const char*     errMsg                  = "WARN: Filesystem may not be compatible.";

            LOG_WARNING(errMsg);

            if (false == m_isQuiet)
            {
                SysMsg::getInstance().show(errMsg, DURATION_NON_SCROLLING, SCROLLING_REPEAT_NUM);
                SysMsg::getInstance().show("", DURATION_PAUSE, SCROLLING_NO_REPEAT);
            }
        }
    }

    /* Any error happened? */
    if (true == isError)
    {
        ErrorState::getInstance().setErrorId(errorId);
        sm.setState(ErrorState::getInstance());
    }
    /* No error */
    else
    {
        /* Stay for a minium time in this state.
         * This is only to give the user the chance to enter access point mode.
         */
        m_timer.start(SHOW_LOGO_DURATION);
    }
}

void InitState::process(StateMachine& sm)
{
    ButtonState buttonState = ButtonDrv::getInstance().getState(BUTTON_ID_OK);

    /* Connect to a remote wifi network? */
    if (BUTTON_STATE_RELEASED == buttonState)
    {
        m_isApModeRequested = false;
        SysMsg::getInstance().disableSignal();
    }
    /* Does the user request for setting up an wifi access point? */
    else if (BUTTON_STATE_PRESSED == buttonState)
    {
        m_isApModeRequested = true;
        SysMsg::getInstance().enableSignal();
    }
    else
    {
        /* Don't care. */
        ;
    }

    Services::processAll();

    /* Leave this state? */
    if (true == m_timer.isTimeout())
    {
        m_timer.stop();

        if (false == m_isApModeRequested)
        {
            sm.setState(ConnectingState::getInstance());
        }
        else
        {
            sm.setState(APState::getInstance());
        }
    }
}

void InitState::exit(StateMachine& sm)
{
    /* Continue initialization steps only, if there was no low level error before. */
    if (ErrorState::ERROR_ID_NO_ERROR == ErrorState::getInstance().getErrorId())
    {
        SettingsService&    settings    = SettingsService::getInstance();
        wifi_mode_t         wifiMode    = WIFI_MODE_NULL;
        String              hostname;
        bool                isQuiet     = false;

        /* Get hostname. */
        if (false == settings.open(true))
        {
            LOG_WARNING("Use default hostname.");
            
            hostname    = settings.getHostname().getDefault();
            isQuiet     = settings.getQuietMode().getDefault();
        }
        else
        {
            hostname    = settings.getHostname().getValue();
            isQuiet     = settings.getQuietMode().getValue();

            settings.close();
        }

        /* Start wifi and initialize the LwIP stack here. */
        if (false == m_isApModeRequested)
        {
            wifiMode = WIFI_MODE_STA;
        }
        else
        {
            wifiMode = WIFI_MODE_AP;
        }

        if (false == WiFi.mode(wifiMode))
        {
            String errorStr = "Set wifi mode failed.";

            /* Fatal error */
            LOG_FATAL(errorStr);
            SysMsg::getInstance().show(errorStr);

            sm.setState(ErrorState::getInstance());
        }
        /* Enable mDNS */
        else if (false == MDNS.begin(hostname.c_str()))
        {
            String errorStr = "Failed to setup mDNS.";

            /* Fatal error */
            LOG_FATAL(errorStr);
            SysMsg::getInstance().show(errorStr);

            sm.setState(ErrorState::getInstance());
        }
        else
        {
            /* Initialize webserver. The filesystem must be mounted before! */
            MyWebServer::init(m_isApModeRequested);
            MDNS.addService("http", "tcp", WebConfig::WEBSERVER_PORT);

            /* Do some stuff only in wifi station mode. */
            if (false == m_isApModeRequested)
            {
                if (false == isQuiet)
                {
                    const uint32_t MIN_WAIT_TIME = 500U; /* Min. wait time in ms to avoid splash screen. */

                    /* In the next step the plugins are loaded and would automatically be shown.
                     * To avoid this until the connection establishment takes place, show the following
                     * message infinite.
                     */
                    SysMsg::getInstance().show("...");
                    delay(MIN_WAIT_TIME); /* Just to avoid a short splash */
                }

                /* Loading plugin installation failed? */
                if (false == PluginMgr::getInstance().load())
                {
                    /* Welcome the user on the very first time (plugin installation is empty).
                     * Of course a error may happened during loading the plugin installation,
                     * in this case show the welcome screen too.
                     */
                    welcome(nullptr);

                    /* Save the plugin installation, so the user can configure it by its own in the web page settings. */
                    PluginMgr::getInstance().save();
                }
                else
                {
                    /* Loading of the plugin installation was successful.
                     *
                     * If the plugin in slot 1 is still the welcome plugin (determined by plugin type and alias),
                     * show welcome message.
                     */
                    IPluginMaintenance* pluginInSlot1 = DisplayMgr::getInstance().getPluginInSlot(1U);

                    /* Is a plugin in slot1? */
                    if (nullptr != pluginInSlot1)
                    {
                        if (0 == strcmp(WELCOME_PLUGIN_TYPE,  pluginInSlot1->getName()))
                        {
                            if (0U != pluginInSlot1->getAlias().equals(WELCOME_PLUGIN_ALIAS))
                            {
                                welcome(pluginInSlot1);
                            }
                        }
                    }
                }

                /* Start over-the-air update server. */
                UpdateMgr::getInstance().begin();
                MDNS.enableArduino(WebConfig::ARDUINO_OTA_PORT, true); /* This typically set by ArduinoOTA, but is disabled there. */
            }

            /* Start webserver after the wifi access point is running.
             * If its done earlier, it will cause an exception because the LwIP stack
             * is not initialized.
             * The LwIP stack is initialized with wifiLowLevelInit()!
             */
            MyWebServer::begin();
        }
    }
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

void InitState::showStartupInfoOnSerial()
{
    LOG_INFO("PIXELIX starts up ...");
    LOG_INFO("Target: %s", Version::TARGET);
    LOG_INFO("SW version: %s", Version::SOFTWARE_VER);
    delay(20U); /* To avoid missing log messages on the console */
    LOG_INFO("SW revision: %s", Version::SOFTWARE_REV);
    LOG_INFO("ESP32 chip rev.: %u", ESP.getChipRevision());
    LOG_INFO("ESP32 SDK version: %s", ESP.getSdkVersion());
    delay(20U); /* To avoid missing log messages on the console */
    LOG_INFO("Wifi MAC: %s", WiFi.macAddress().c_str());
    LOG_INFO("LwIP version: %s", LWIP_VERSION_STRING);
    delay(20U); /* To avoid missing log messages on the console */
}

void InitState::showStartupInfoOnDisplay(bool isQuietEnabled)
{
    const uint32_t  DURATION_NON_SCROLLING  = 2000U; /* ms */
    const uint32_t  SCROLLING_REPEAT_NUM    = 1U;
    const uint32_t  DURATION_PAUSE          = 500U; /* ms */
    const uint32_t  SCROLLING_NO_REPEAT     = 0U;
    SysMsg&         sysMsg                  = SysMsg::getInstance();

    /* Show colored PIXELIX */
    sysMsg.show("\\calign\\#FFFFFF.:\\#FF0000P\\#FFFF00I\\#00FF00X\\#00FFFFE\\#0000FFL\\#FF00FFI\\#FF0000X\\#FFFFFF:.", SHOW_LOGO_DURATION, SCROLLING_REPEAT_NUM);

    if (false == isQuietEnabled)
    {
        /* Clear and wait */
        sysMsg.show("", DURATION_PAUSE, SCROLLING_NO_REPEAT);

        /* Show sw version (short) */
        sysMsg.show(String("\\calign") + Version::SOFTWARE_VER, DURATION_NON_SCROLLING, SCROLLING_REPEAT_NUM);

        /* Clear and wait */
        sysMsg.show("", DURATION_PAUSE, SCROLLING_NO_REPEAT);
    }
}

void InitState::welcome(IPluginMaintenance* plugin)
{
    IconTextPlugin* welcomePlugin = nullptr;

    if (nullptr == plugin)
    {
        /* Install default plugin. */
        welcomePlugin = static_cast<IconTextPlugin*>(PluginMgr::getInstance().install(WELCOME_PLUGIN_TYPE));

        PluginMgr::getInstance().setPluginAliasName(welcomePlugin, WELCOME_PLUGIN_ALIAS);
        welcomePlugin->enable();
    }
    else
    {
        welcomePlugin = static_cast<IconTextPlugin*>(plugin);
    }

    if (nullptr != welcomePlugin)
    {
        (void)welcomePlugin->loadBitmap("/images/smiley.bmp");
        welcomePlugin->setText("Hello World!");
    }
}

bool InitState::isFsCompatible()
{
    bool                isCompatible            = false;
    JsonFile            jsonFile(FILESYSTEM);
    const size_t        JSON_DOC_SIZE           = 512U;
    DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);

    if (true == jsonFile.load(VERSION_FILE_NAME, jsonDoc))
    {
        JsonVariantConst jsonVersion = jsonDoc["version"];

        if (false == jsonVersion.isNull())
        {
            String fileSystemVersion    = jsonVersion.as<String>();
            String firmwareVersion      = Version::SOFTWARE_VER;

            /* Note that the firmware version may have a additional postfix.
             * Example: v4.1.2:b or v4.1.2:b:lc
             * See ./scripts/get_get_rev.py for the different postfixes.
             */
            if (true == firmwareVersion.startsWith(fileSystemVersion))
            {
                isCompatible = true;
            }
        }
    }

    return isCompatible;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
