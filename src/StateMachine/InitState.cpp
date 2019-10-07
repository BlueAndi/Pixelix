/* MIT License
 *
 * Copyright (c) 2019 Andreas Merkle <web@blue-andi.de>
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
#include "Version.h"
#include "AmbientLightSensor.h"
#include "MyWebServer.h"

#include "APState.h"
#include "ConnectingState.h"

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

/* Set short wait time for showing a system message in ms. */
const uint32_t  InitState::SYS_MSG_WAIT_TIME_SHORT  = 250u;

/* Set serial interface baudrate. */
const uint32_t  InitState::SERIAL_BAUDRATE          = 115200u;

/* Initialization state instance */
InitState       InitState::m_instance;

/******************************************************************************
 * Public Methods
 *****************************************************************************/

void InitState::entry(StateMachine& sm)
{
    uint8_t index = 0u;

    /* Initialize hardware */
    Board::init();

    /* Setup serial interface */
    Serial.begin(SERIAL_BAUDRATE);

    /* Initialize logging, which uses the serial interface as sink. */
    Logging::getInstance().init(&Serial);
    Logging::getInstance().setLogLevel(Logging::LOGLEVEL_INFO);

    /* Show as soon as possible the user that the system is booting. */
    LOG_INFO("Booting ...");

    /* Initialize drivers */
    if (ButtonDrv::RET_OK != ButtonDrv::getInstance().init())
    {
        LOG_WARNING("Couldn't initialize button driver.");
    }

    /* Start LED matrix */
    LedMatrix::getInstance().begin();

    /* Initialize display manager */
    DisplayMgr::getInstance().init();

    /* Initialize display layouts */
    for(index = 0u; index < DisplayMgr::MAX_SLOTS; ++index)
    {
        DisplayMgr::getInstance().setLayout(index, DisplayMgr::LAYOUT_ID_2);
    }

    /* Mounting the filesystem. */
    if (false == SPIFFS.begin())
    {
        LOG_WARNING("Couldn't mount the filesystem.");
    }

    /* Show some interesting boot information */    
    showBootInfo();

    /* Enable the automatic display brightness adjustment according to the
     * ambient light.
     */
    if (false == DisplayMgr::getInstance().enableAutoBrightnessAdjustment(true))
    {
        LOG_WARNING("Failed to enable autom. brigthness adjustment.");
    }

    /* Initialize webserver */
    MyWebServer::init();

    /* Don't store the wifi configuration in the NVS.
     * This seems to cause a reset after a client connected to the access point.
     * https://github.com/espressif/arduino-esp32/issues/2025#issuecomment-503415364
     */
    WiFi.persistent(false);

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
    /* Nothing to do. */
    return;
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/**
 * Show boot information on the serial interface.
 */
void InitState::showBootInfo(void)
{
    LOG_INFO(String("SW version: ") + Version::SOFTWARE);
    DisplayMgr::getInstance().showSysMsg(Version::SOFTWARE);

    LOG_INFO(String("ESP32 chip rev.: ") + ESP.getChipRevision());
    LOG_INFO(String("ESP32 SDK version: ") + ESP.getSdkVersion());

    LOG_INFO(String("Ambient light sensor detected: ") + AmbientLightSensor::getInstance().isSensorAvailable());

    LOG_INFO(String("Wifi MAC: ") + WiFi.macAddress());

    /* User shall be able to read it on the display. But it shall be really a short delay. */
    DisplayMgr::getInstance().delay(SYS_MSG_WAIT_TIME_SHORT);

    /* Debug information */
    LOG_INFO(String("AMPDU RX feature: ") + CONFIG_ESP32_WIFI_AMPDU_RX_ENABLED);
    LOG_INFO(String("AMPDU TX feature: ") + CONFIG_ESP32_WIFI_AMPDU_TX_ENABLED);

    return;
}

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
