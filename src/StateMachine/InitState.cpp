/* MIT License
 *
 * Copyright (c) 2019 - 2021 Andreas Merkle <web@blue-andi.de>
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

#include "ButtonDrv.h"
#include "DisplayMgr.h"
#include "SysMsg.h"
#include "Version.h"
#include "MyWebServer.h"
#include "UpdateMgr.h"
#include "Settings.h"
#include "PluginMgr.h"
#include "WebConfig.h"
#include "FileSystem.h"
#include "JsonFile.h"
#include "Version.h"

#include "APState.h"
#include "ConnectingState.h"
#include "ErrorState.h"

#include <Logging.h>
#include <Util.h>
#include <ESPmDNS.h>

#include "BTCQuotePlugin.h"
#include "CountdownPlugin.h"
#include "DatePlugin.h"
#include "DateTimePlugin.h"
#include "FirePlugin.h"
#include "GameOfLifePlugin.h"
#include "GithubPlugin.h"
#include "GruenbeckPlugin.h"
#include "IconTextLampPlugin.h"
#include "IconTextPlugin.h"
#include "JustTextPlugin.h"
#include "OpenWeatherPlugin.h"
#include "RainbowPlugin.h"
#include "SensorPlugin.h"
#include "ShellyPlugSPlugin.h"
#include "SunrisePlugin.h"
#include "SysMsgPlugin.h"
#include "TempHumidPlugin.h"
#include "TestPlugin.h"
#include "TimePlugin.h"
#include "VolumioPlugin.h"
#include "WifiStatusPlugin.h"

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

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void InitState::entry(StateMachine& sm)
{
    bool                isError             = false;
    ErrorState::ErrorId errorId             = ErrorState::ERROR_ID_UNKNOWN;
    const char*         VERSION_FILE_NAME   = "/version.json";

    /* Initialize hardware */
    Board::init();

    /* Show as soon as possible the user on the serial console that the system is booting. */
    showStartupInfoOnSerial();

    /* Initialize two-wire (I2C) */
    if (false == Wire.begin())
    {
        LOG_FATAL("Couldn't initialize two-wire.");
        errorId = ErrorState::ERROR_ID_TWO_WIRE_ERROR;
        isError = true;
    }
    /* Initialize button driver */
    else if (ButtonDrv::RET_OK != ButtonDrv::getInstance().init())
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
    else
    {
        /* Initialize sensors */
        SensorDataProvider::getInstance().begin();

        /* Prepare everything for the plugins. */
        PluginMgr::getInstance().begin();

        /* Register plugins. This must be done before system message handler is initialized! */
        registerPlugins();
    }

    /* Continoue only if there is no error yet. */
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
        Settings*           settings = &Settings::getInstance();
        JsonFile            jsonFile(FILESYSTEM);
        const size_t        JSON_DOC_SIZE   = 512U;
        DynamicJsonDocument jsonDoc(JSON_DOC_SIZE);

        /* Load some general configuration parameters from persistent memory. */
        if (true == settings->open(true))
        {
            /* Enable or disable the automatic display brightness adjustment,
             * depended on settings. Enable it may fail in case there is no
             * LDR sensor available.
             */
            bool isEnabled = settings->getAutoBrightnessAdjustment().getValue();

            if (false == DisplayMgr::getInstance().setAutoBrightnessAdjustment(isEnabled))
            {
                LOG_WARNING("Failed to enable autom. brigthness adjustment.");
            }

            /* Set text scroll pause for all text widgets. */
            uint32_t scrollPause = settings->getScrollPause().getValue();
            if (false == TextWidget::setScrollPause(scrollPause))
            {
                LOG_WARNING("Scroll pause %u ms couldn't be set.", scrollPause);
            }

            settings->close();
        }

        /* Don't store the wifi configuration in the NVS.
         * This seems to cause a reset after a client connected to the access point.
         * https://github.com/espressif/arduino-esp32/issues/2025#issuecomment-503415364
         */
        WiFi.persistent(false);

        /* Show some informations on the display. */
        showStartupInfoOnDisplay();

        /* Show a warning in case the filesystem may not be compatible to the firmware version. */
        if (true == jsonFile.load(VERSION_FILE_NAME, jsonDoc))
        {
            JsonVariant jsonVersion             = jsonDoc["version"];
            bool        isFileSystemCompatible  = true;

            if (true == jsonVersion.isNull())
            {
                isFileSystemCompatible = false;
            }
            else
            {
                String fileSystemVersion    = jsonVersion.as<String>();
                String firmwareVersion      = Version::SOFTWARE_VER;

                /* Note that the firmware version may have a additional postfix.
                 * Example: v4.1.2:b or v4.1.2:b:lc
                 * See ./scripts/get_get_rev.py for the different postfixes.
                 */
                if (0U == firmwareVersion.startsWith(fileSystemVersion))
                {
                    isFileSystemCompatible = false;
                }
            }

            if (false == isFileSystemCompatible)
            {
                const char* errMsg  = "WARN: Filesystem may not be compatible.";

                LOG_WARNING(errMsg);

                SysMsg::getInstance().show(errMsg, 3000U, 1U, true);
                SysMsg::getInstance().show("", 500U, 0U, true);
            }
        }
    }

    /* Any error happened? */
    if (true == isError)
    {
        ErrorState::getInstance().setErrorId(errorId);
        sm.setState(ErrorState::getInstance());
    }

    return;
}

void InitState::process(StateMachine& sm)
{
    ButtonDrv::State    buttonState = ButtonDrv::getInstance().getState();

    /* Connect to a remote wifi network? */
    if (ButtonDrv::STATE_RELEASED == buttonState)
    {
        sm.setState(ConnectingState::getInstance());
        m_isApModeRequested = false;
    }
    /* Does the user request for setting up an wifi access point? */
    else if (ButtonDrv::STATE_PRESSED == buttonState)
    {
        sm.setState(APState::getInstance());
        m_isApModeRequested = true;
    }
    else
    {
        /* Don't care. */
        ;
    }

    return;
}

void InitState::exit(StateMachine& sm)
{
    /* Continue initialization steps only, if there was no low level error before. */
    if (ErrorState::ERROR_ID_NO_ERROR == ErrorState::getInstance().getErrorId())
    {
        wifi_mode_t wifiMode = WIFI_MODE_NULL;
        String      hostname;

        /* Get hostname. */
        if (false == Settings::getInstance().open(true))
        {
            LOG_WARNING("Use default hostname.");
            hostname = Settings::getInstance().getHostname().getDefault();
        }
        else
        {
            hostname = Settings::getInstance().getHostname().getValue();
            Settings::getInstance().close();
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
                /* In the next step the plugins are loaded and would be automatically be shown.
                * To avoid this until the connection establishment takes place, show the following
                * message infinite.
                */
                SysMsg::getInstance().show("...");
                delay(500U); /* Just to avoid a short splash */

                /* Load last plugin installation. */
                PluginMgr::getInstance().load();

                /* Welcome the user on the very first time. */
                welcome();

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

    return;
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
    LOG_INFO(String("SW version: ") + Version::SOFTWARE_VER);
    LOG_INFO(String("SW revision: ") + Version::SOFTWARE_REV);
    LOG_INFO(String("ESP32 chip rev.: ") + ESP.getChipRevision());
    LOG_INFO(String("ESP32 SDK version: ") + ESP.getSdkVersion());
    LOG_INFO(String("Wifi MAC: ") + WiFi.macAddress());
    LOG_INFO(String("LwIP version: ") + LWIP_VERSION_STRING);

    return;
}

void InitState::showStartupInfoOnDisplay()
{
    SysMsg& sysMsg = SysMsg::getInstance();

    /* Show colored PIXELIX */
    sysMsg.show("\\calign\\#FF0000P\\#0FF000I\\#00FF00X\\#000FF0E\\#0000FFL\\#F0000FI\\#FF0000X", 3000U, 2U, true);

    /* Clear and wait */
    sysMsg.show("", 500U, 0U, true);

    /* Show sw version (short) */
    sysMsg.show(String("\\calign") + Version::SOFTWARE_VER, 3000U, 2U, true);

    /* Clear and wait */
    sysMsg.show("", 500U, 0U, true);

    return;
}

void InitState::registerPlugins()
{
    PluginMgr&  pluginMgr = PluginMgr::getInstance();

    /* Register in alphabetic order. */

    pluginMgr.registerPlugin("BTCQuotePlugin", BTCQuotePlugin::create);
    pluginMgr.registerPlugin("CountdownPlugin", CountdownPlugin::create);
    pluginMgr.registerPlugin("DatePlugin", DatePlugin::create);
    pluginMgr.registerPlugin("DateTimePlugin", DateTimePlugin::create);
    pluginMgr.registerPlugin("FirePlugin", FirePlugin::create);
    pluginMgr.registerPlugin("GameOfLifePlugin", GameOfLifePlugin::create);
    pluginMgr.registerPlugin("GithubPlugin", GithubPlugin::create);
    pluginMgr.registerPlugin("GruenbeckPlugin", GruenbeckPlugin::create);
    pluginMgr.registerPlugin("IconTextLampPlugin", IconTextLampPlugin::create);
    pluginMgr.registerPlugin("IconTextPlugin", IconTextPlugin::create);
    pluginMgr.registerPlugin("JustTextPlugin", JustTextPlugin::create);
    pluginMgr.registerPlugin("OpenWeatherPlugin", OpenWeatherPlugin::create);
    pluginMgr.registerPlugin("RainbowPlugin", RainbowPlugin::create);
    pluginMgr.registerPlugin("SensorPlugin", SensorPlugin::create);
    pluginMgr.registerPlugin("ShellyPlugSPlugin", ShellyPlugSPlugin::create);
    pluginMgr.registerPlugin("SunrisePlugin", SunrisePlugin::create);
    pluginMgr.registerPlugin("SysMsgPlugin", SysMsgPlugin::create);
    pluginMgr.registerPlugin("TempHumidPlugin", TempHumidPlugin::create);
    pluginMgr.registerPlugin("TestPlugin", TestPlugin::create);
    pluginMgr.registerPlugin("TimePlugin", TimePlugin::create);
    pluginMgr.registerPlugin("VolumioPlugin", VolumioPlugin::create);
    pluginMgr.registerPlugin("WifiStatusPlugin", WifiStatusPlugin::create);

    return;
}

void InitState::welcome()
{
    Settings& settings = Settings::getInstance();

    /* On the very first start, the plugin installation is empty.
     * In this case we welcome the user.
     */
    if (true == settings.open(true))
    {
        String pluginInstallation = settings.getPluginInstallation().getValue();

        if (true == pluginInstallation.isEmpty())
        {
            IconTextLampPlugin* plugin = nullptr;

            /* Install default plugin. */
            plugin = static_cast<IconTextLampPlugin*>(PluginMgr::getInstance().install("IconTextLampPlugin"));

            if (nullptr != plugin)
            {
                (void)plugin->loadBitmap("/images/smiley.bmp");
                plugin->setText("Hello World!");
                plugin->enable();
            }
        }

        settings.close();
    }

    return;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
