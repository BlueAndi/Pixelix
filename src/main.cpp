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
 * @file   main.cpp
 * @brief  Main entry point
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include <Arduino.h>
#include <Logging.h>
#include <LogSinkPrinter.h>
#include "LogSinkWebsocket.h"
#include <StateMachine.hpp>
#include <Board.h>
#include <esp_task_wdt.h>
#include "InitState.h"
#include "RestartState.h"
#include "ErrorState.h"
#include "MemMon.h"
#include "MiniTerminal.h"
#include "RestartMgr.h"

#include "ButtonDrv.h"
#include "ButtonHandler.hpp"
#include "OneButtonCtrl.hpp"
#include "TwoButtonCtrl.hpp"
#include "ThreeButtonCtrl.hpp"

#if (configCHECK_FOR_STACK_OVERFLOW > 0)
#include "freertos/task.h"
#endif /* (configCHECK_FOR_STACK_OVERFLOW > 0) */

/******************************************************************************
 * Macros
 *****************************************************************************/

#ifndef CONFIG_ESP_LOG_SEVERITY

/** The ESP log severity level (default). */
#define CONFIG_ESP_LOG_SEVERITY (ESP_LOG_INFO)

#endif /* CONFIG_ESP_LOG_SEVERITY */

#ifndef CONFIG_LOG_SEVERITY

/** The Pixelix log severity level (default). */
#define CONFIG_LOG_SEVERITY (Logging::LOG_LEVEL_INFO)

#endif /* CONFIG_LOG_SEVERITY */

#if CONFIG_BUTTON_CTRL == 2

/**
 * Button control policy defines the number and kind of buttons, which are used to
 * control Pixelix.
 */
#define BUTTON_CTRL_POLICY TwoButtonCtrl<BUTTON_ID_LEFT, BUTTON_ID_RIGHT>

#elif CONFIG_BUTTON_CTRL == 3

/**
 * Button control policy defines the number and kind of buttons, which are used to
 * control Pixelix.
 */
#define BUTTON_CTRL_POLICY ThreeButtonCtrl<BUTTON_ID_LEFT, BUTTON_ID_OK, BUTTON_ID_RIGHT>

#else

/**
 * Button control policy defines the number and kind of buttons, which are used to
 * control Pixelix.
 */
#define BUTTON_CTRL_POLICY OneButtonCtrl<BUTTON_ID_OK>

#endif

/**
 * Task watchdog timeout in seconds. See CONFIG_ESP_TASK_WDT_TIMEOUT_S for original
 * configuration.
 */
#define TASK_WDT_TIMEOUT_S (10U)

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/******************************************************************************
 * Prototypes
 *****************************************************************************/

#ifdef BOARD_HAS_PSRAM
static void* main_mbedtls_calloc(size_t count, size_t size);
#endif /* BOARD_HAS_PSRAM */

/******************************************************************************
 * Variables
 *****************************************************************************/

/** Serial terminal */
static MiniTerminal gTerminal(Serial);

/** System state machine */
static StateMachine gSysStateMachine(InitState::getInstance());

/** Serial log sink */
static LogSinkPrinter gLogSinkSerial("Serial", &Serial);

/** Websocket log sink */
static LogSinkWebsocket gLogSinkWebsocket("Websocket", &WebSocketSrv::getInstance());

/** Button handler */
static ButtonHandler<BUTTON_CTRL_POLICY> gButtonHandler;

/** Serial interface baudrate. */
static const uint32_t SERIAL_BAUDRATE  = 115200U;

/** Task period in ms of the loop() task. */
static const uint32_t LOOP_TASK_PERIOD = 40U;

#if ARDUINO_USB_MODE
#if ARDUINO_USB_CDC_ON_BOOT /* Serial used for USB CDC */

/**
 * Minimize the USB tx timeout (ms) to avoid too long blocking behaviour during
 * writing e.g. log messages to it. If the value is too high, it will influence
 * the display refresh bad.
 */
static const uint32_t HWCDC_TX_TIMEOUT = 4U;

#endif /* ARDUINO_USB_CDC_ON_BOOT */
#endif /* ARDUINO_USB_MODE */

/******************************************************************************
 * External functions
 *****************************************************************************/

/**
 * Setup the system.
 */
void setup()
{
    /* Setup serial interface */
    Serial.begin(SERIAL_BAUDRATE);

#if ARDUINO_USB_MODE
#if ARDUINO_USB_CDC_ON_BOOT
    Serial.setTxTimeoutMs(HWCDC_TX_TIMEOUT);
#endif /* ARDUINO_USB_CDC_ON_BOOT */
#endif /* ARDUINO_USB_MODE */

    /* Ensure a distance between the boot mode message and the first log message.
     * Otherwise the first log message appears in the same line than the last
     * boot mode message.
     */
    Serial.println("\n");

    /* Set severity for esp logging system. */
    esp_log_level_set("*", CONFIG_ESP_LOG_SEVERITY);

    /* Register serial log sink and select it per default. */
    if (true == Logging::getInstance().registerSink(&gLogSinkSerial))
    {
        (void)Logging::getInstance().selectSink("Serial");
    }

    /* Register websocket log sink. */
    (void)Logging::getInstance().registerSink(&gLogSinkWebsocket);

    /* Set severity for Pixelix logging system. */
    Logging::getInstance().setLogLevel(CONFIG_LOG_SEVERITY);

#ifdef BOARD_HAS_PSRAM
    /* Set mbedTLS memory allocation functions to use PSRAM if available. */
    mbedtls_platform_set_calloc_free(main_mbedtls_calloc, free);
#endif /* BOARD_HAS_PSRAM */

    /* Setup memory monitoring. */
    MemMon::getInstance().start();

    /* The setup routine shall handle only the initialization state.
     * All other states are handled in the loop routine.
     */
    do
    {
        gSysStateMachine.process();

        /* Memory monitor */
        MemMon::getInstance().process();
    }
    while (static_cast<AbstractState*>(&InitState::getInstance()) == gSysStateMachine.getState());

    /* Observe button state changes and derrive actions.
     * Do this after init state!
     */
    ButtonDrv::getInstance().registerObserver(gButtonHandler);

    /* Initialize task watchdog. */
    (void)esp_task_wdt_init(TASK_WDT_TIMEOUT_S, true);

    /* Enable task watchdog for the loop task.
     *
     * The task watchdog is used to detect a deadlock of the main loop.
     * If the main loop does not reset the watchdog, it will trigger a reset.
     * The task watchdog is not used to detect a deadlock of the init state,
     * because it is expected that the init state will finish in a short time.
     */
    (void)esp_task_wdt_add(nullptr);
}

/**
 * Main loop, which is called periodically.
 */
void loop()
{
    RestartMgr& restartMgr = RestartMgr::getInstance();

    /* Reset task watchdog to avoid a timeout.
     * The task watchdog is used to detect a deadlock of the main loop.
     * If the main loop does not reset the watchdog, it will trigger a reset.
     */
    esp_task_wdt_reset();

    /* Process system state machine */
    gSysStateMachine.process();

    /* Memory monitor */
    MemMon::getInstance().process();

    /* Process terminal */
    gTerminal.process();

    /* Handle delayed restart request. */
    restartMgr.process();

    /* Restart requested by restart manager? */
    if (true == restartMgr.isRestartRequested())
    {
        gSysStateMachine.setState(RestartState::getInstance());
    }

    /* Handle button actions only if not in RestartState and not in ErrorState. */
    if (&RestartState::getInstance() != gSysStateMachine.getState() &&
        &ErrorState::getInstance() != gSysStateMachine.getState())
    {
        gButtonHandler.process();
    }

    /* Schedule other tasks with same or lower priority. */
    delay(LOOP_TASK_PERIOD);
}

/******************************************************************************
 * Local functions
 *****************************************************************************/

#if (configCHECK_FOR_STACK_OVERFLOW > 0)

void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName)
{
    /* Inform via serial and avoid the logging feature, because it may never show up. */
    Serial.write("Task stack overflow detected: ");
    Serial.write(pcTaskName);
    Serial.write("\n");
    Serial.flush();

    /* Trigger watchdog reset. */
    while (1)
    {
        /* Waiting for watchdog reset. */
        ;
    }
}

#endif /* (configCHECK_FOR_STACK_OVERFLOW > 0) */

#ifdef BOARD_HAS_PSRAM

/**
 * mbedTLS memory allocation function using PSRAM if available.
 *
 * @param count Number of elements to allocate.
 * @param size  Size of each element.
 *
 * @return Pointer to allocated memory or nullptr if allocation failed
 */
static void* main_mbedtls_calloc(size_t count, size_t size)
{
    void* ptr = ps_calloc(count, size);

    /* If it failed, try regular calloc. */
    if (nullptr == ptr)
    {
        ptr = calloc(count, size);
    }

    return ptr;
}

#endif /* BOARD_HAS_PSRAM */
