/* MIT License
 *
 * Copyright (c) 2019 - 2024 Andreas Merkle <web@blue-andi.de>
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
 * @brief  System state: Restart
 * @author Andreas Merkle <web@blue-andi.de>
 */

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "RestartState.h"
#include "DisplayMgr.h"
#include "MyWebServer.h"
#include "UpdateMgr.h"
#include "FileSystem.h"
#include "Services.h"
#include "SensorDataProvider.h"
#include "PluginMgr.h"

#include <Board.h>
#include <Display.h>
#include <Logging.h>
#include <Util.h>
#include <ESPmDNS.h>

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

void RestartState::entry(StateMachine& sm)
{
    UTIL_NOT_USED(sm);

    LOG_INFO("Going in restart state.");

    m_timer.start(WAIT_TILL_STOP_SVC);
}

void RestartState::process(StateMachine& sm)
{
    UTIL_NOT_USED(sm);

    MyWebServer::process();
    UpdateMgr::getInstance().process();

    /* Wait a certain amount of time, because there may be still some pending tasks, which
     * need to be finished before the system is restarted.
     */
    if ((true == m_timer.isTimerRunning()) &&
        (true == m_timer.isTimeout()))
    {
        /* Notes:
         * - The wifi connection is required for a successful topic purge (MQTT).
         * - The order of the shutdown is important and their dependencies shall be considered.
         */

        /* Purge sensor topics (MQTT) and remove REST API endpoints. */
        SensorDataProvider::getInstance().end();

        /* Unregister all plugins, which will purge all of their topics (MQTT) and remove REST API endpoints. */
        PluginMgr::getInstance().unregisterAllPluginTopics();

        /* Stop display manager first, because this will stop the plugin
         * processing at all.
         */
        DisplayMgr::getInstance().end();

        /* Clear display */
        Display::getInstance().clear();
        Display::getInstance().show();

        /* Wait till all physical pixels are cleared. */
        while(false == Display::getInstance().isReady())
        {
            /* Just wait ... */
            ;
        }

        /* Avoid any external update request. */
        UpdateMgr::getInstance().end();

        /* Stop services.
         *
         * Important order (reverse order of start, see config files.):
         * 1. Audio service, because it will stop the audio processing.
         * 2. FileMgrService, because it will remove all REST API endpoints.
         * 3. TopicHandlerService, because it will purge all published MQTT topics and remove all REST API endpoints.
         * 4. MQTT service, because it will publish a offline status.
         * 5. SettingsService, because it will save all settings.
         */
        Services::stopAll();

        /* Disconnect wifi connection to avoid any further external request. */
        (void)WiFi.disconnect();

        /* Stop webserver. */
        MyWebServer::end();

        /* Stop DNS. */
        MDNS.end();

        /* Unmount filesystem at the end. */
        FILESYSTEM.end();

        /* Reset */
        Board::reset();
    }
}

void RestartState::exit(StateMachine& sm)
{
    UTIL_NOT_USED(sm);

    /* Nothing to do. */
}

/******************************************************************************
 * Protected Methods
 *****************************************************************************/

/******************************************************************************
 * Private Methods
 *****************************************************************************/

/******************************************************************************
 * External Functions
 *****************************************************************************/

/******************************************************************************
 * Local Functions
 *****************************************************************************/
