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
 * @brief  System state: Init
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "InitState.h"

#include <Arduino.h>
#include <WiFi.h>
#include <SPIFFS.h>

#include "Board.h"
#include "ButtonDrv.h"
#include "LedMatrix.h"
#include "DisplayMgr.h"
#include "SysMsg.h"
#include "Version.h"
#include "AmbientLightSensor.h"
#include "MyWebServer.h"
#include "UpdateMgr.h"
#include "Settings.h"
#include "PluginMgr.h"

#include "APState.h"
#include "ConnectingState.h"
#include "ErrorState.h"

#include <Logging.h>
#include <Util.h>

#include "FirePlugin.h"
#include "GameOfLifePlugin.h"
#include "IconTextLampPlugin.h"
#include "IconTextPlugin.h"
#include "JustTextPlugin.h"
#include "SysMsgPlugin.h"
#include "TimePlugin.h"

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

/* Set system message show duration in ms */
const uint32_t  InitState::SYS_MSG_WAIT_TIME    = 2000U;

/* Set serial interface baudrate. */
const uint32_t  InitState::SERIAL_BAUDRATE      = 115200U;

/* Initialization state instance */
InitState       InitState::m_instance;

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void InitState::entry(StateMachine& sm)
{
    bool    isError = false;

    /* Initialize hardware */
    Board::init();

    /* Setup serial interface */
    Serial.begin(SERIAL_BAUDRATE);

    /* Initialize logging, which uses the serial interface as sink. */
    Logging::getInstance().init(&Serial);
    Logging::getInstance().setLogLevel(Logging::LOGLEVEL_INFO);

    /* Show as soon as possible the user on the serial console that the system is booting. */
    showStartupInfoOnSerial();

    /* Register plugins. This must be done before system message handler is initialized! */
    registerPlugins();

    /* Initialize button driver */
    if (ButtonDrv::RET_OK != ButtonDrv::getInstance().init())
    {
        LOG_FATAL("Couldn't initialize button driver.");
        isError = true;
    }
    /* Mounting the filesystem. */
    else if (false == SPIFFS.begin())
    {
        LOG_FATAL("Couldn't mount the filesystem.");
        isError = true;
    }
    /* Start LED matrix */
    else if (false == LedMatrix::getInstance().begin())
    {
        LOG_FATAL("Failed to initialize LED matrix.");
        isError = true;
    }
    /* Initialize display manager */
    else if (false == DisplayMgr::getInstance().begin())
    {
        LOG_FATAL("Failed to initialize display manager.");
        isError = true;
    }
    /* Initialize system message handler */
    else if (false == SysMsg::getInstance().init())
    {
        LOG_FATAL("Failed to initialize system message handler.");
        isError = true;
    }
    /* Initialize over-the-air update server */
    else if (false == UpdateMgr::getInstance().init())
    {
        LOG_FATAL("Failed to initialize Arduino OTA.");
        isError = true;
    }
    else
    {
        Settings* settings = &Settings::getInstance();

        /* Enable or disable the automatic display brightness adjustment,
         * depended on settings. Enable it may fail in case there is no
         * LDR sensor available.
         */
        if (true == settings->open(true))
        {
            bool isEnabled = settings->getAutoBrightnessAdjustment().getValue();

            if (false == DisplayMgr::getInstance().setAutoBrightnessAdjustment(isEnabled))
            {
                LOG_WARNING("Failed to enable autom. brigthness adjustment.");
            }
            settings->close();
        }

        /* Initialize webserver. SPIFFS must be mounted before! */
        MyWebServer::init();

        /* Don't store the wifi configuration in the NVS.
         * This seems to cause a reset after a client connected to the access point.
         * https://github.com/espressif/arduino-esp32/issues/2025#issuecomment-503415364
         */
        WiFi.persistent(false);

        /* Show some informations on the display. */
        showStartupInfoOnDisplay();

        /* Load last plugin installation. */
        PluginMgr::getInstance().load();

        /* Welcome the user on the very first time. */
        welcome();
    }

    /* Any error happened? */
    if (true == isError)
    {
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
    }
    /* Does the user request for setting up an wifi access point? */
    else if (ButtonDrv::STATE_PRESSED == buttonState)
    {
        sm.setState(APState::getInstance());
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
    UTIL_NOT_USED(sm);

    /* Nothing to do. */
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
    LOG_INFO(String("SW version: ") + Version::SOFTWARE);
    LOG_INFO(String("ESP32 chip rev.: ") + ESP.getChipRevision());
    LOG_INFO(String("ESP32 SDK version: ") + ESP.getSdkVersion());
    LOG_INFO(String("Ambient light sensor detected: ") + AmbientLightSensor::getInstance().isSensorAvailable());
    LOG_INFO(String("Wifi MAC: ") + WiFi.macAddress());

    return;
}

void InitState::showStartupInfoOnDisplay()
{
    SysMsg& sysMsg = SysMsg::getInstance();

    /* Show colored PIXELIX */
    sysMsg.show("\\#FF0000P\\#0FF000I\\#00FF00X\\#000FF0E\\#0000FFL\\#F0000FI\\#FF0000X");
    delay(SYS_MSG_WAIT_TIME);

    /* Clear and wait */
    sysMsg.show("");
    delay(SYS_MSG_WAIT_TIME / 2U);

    /* Show sw version */
    sysMsg.show(Version::SOFTWARE);
    delay(SYS_MSG_WAIT_TIME);

    /* Clear and wait */
    sysMsg.show("", SYS_MSG_WAIT_TIME / 2U);

    return;
}

void InitState::registerPlugins()
{
    PluginMgr&  pluginMgr = PluginMgr::getInstance();

    /* Register in alphabetic order. */
    pluginMgr.registerPlugin("FirePlugin", FirePlugin::create);
    pluginMgr.registerPlugin("GameOfLifePlugin", GameOfLifePlugin::create);
    pluginMgr.registerPlugin("IconTextLampPlugin", IconTextLampPlugin::create);
    pluginMgr.registerPlugin("IconTextPlugin", IconTextPlugin::create);
    pluginMgr.registerPlugin("JustTextPlugin", JustTextPlugin::create);
    pluginMgr.registerPlugin("SysMsgPlugin", SysMsgPlugin::create);
    pluginMgr.registerPlugin("TimePlugin", TimePlugin::create);

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
