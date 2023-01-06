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
 * @brief  Plugin list
 * @author Andreas Merkle <web@blue-andi.de>
 *
 * @addtogroup plugin
 *
 * @{
 */

#ifndef __PLUGIN_LIST_HPP__
#define __PLUGIN_LIST_HPP__

/******************************************************************************
 * Compile Switches
 *****************************************************************************/

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "PluginMgr.h"

#include "BTCQuotePlugin.h"
#include "CountdownPlugin.h"
#include "DateTimePlugin.h"
#include "FirePlugin.h"
#include "GameOfLifePlugin.h"
#include "GithubPlugin.h"
#include "GruenbeckPlugin.h"
#include "IconTextLampPlugin.h"
#include "IconTextPlugin.h"
#include "JustTextPlugin.h"
#include "MatrixPlugin.h"
#include "OpenWeatherPlugin.h"
#include "RainbowPlugin.h"
#include "SensorPlugin.h"
#include "ShellyPlugSPlugin.h"
#include "SignalDetectorPlugin.h"
#include "SoundReactivePlugin.h"
#include "SunrisePlugin.h"
#include "SysMsgPlugin.h"
#include "TempHumidPlugin.h"
#include "TestPlugin.h"
#include "ThreeIconPlugin.h"
#include "VolumioPlugin.h"
#include "WifiStatusPlugin.h"
#include "WormPlugin.h"

/******************************************************************************
 * Macros
 *****************************************************************************/

/******************************************************************************
 * Types and Classes
 *****************************************************************************/

/******************************************************************************
 * Functions
 *****************************************************************************/

/** List with all compiled-in plugins. */
namespace PluginList
{

/**
 * Registers all plugins by the plugin manager.
 * 
 */
static void registerAll()
{
    PluginMgr&  pluginMgr = PluginMgr::getInstance();

    /* Register in alphabetic order. */

    pluginMgr.registerPlugin("BTCQuotePlugin", BTCQuotePlugin::create);
    pluginMgr.registerPlugin("CountdownPlugin", CountdownPlugin::create);
    pluginMgr.registerPlugin("DateTimePlugin", DateTimePlugin::create);
    pluginMgr.registerPlugin("FirePlugin", FirePlugin::create);
    pluginMgr.registerPlugin("GameOfLifePlugin", GameOfLifePlugin::create);
    pluginMgr.registerPlugin("GithubPlugin", GithubPlugin::create);
    pluginMgr.registerPlugin("GruenbeckPlugin", GruenbeckPlugin::create);
    pluginMgr.registerPlugin("IconTextLampPlugin", IconTextLampPlugin::create);
    pluginMgr.registerPlugin("IconTextPlugin", IconTextPlugin::create);
    pluginMgr.registerPlugin("JustTextPlugin", JustTextPlugin::create);
    pluginMgr.registerPlugin("MatrixPlugin", MatrixPlugin::create);
    pluginMgr.registerPlugin("OpenWeatherPlugin", OpenWeatherPlugin::create);
    pluginMgr.registerPlugin("RainbowPlugin", RainbowPlugin::create);
    pluginMgr.registerPlugin("SensorPlugin", SensorPlugin::create);
    pluginMgr.registerPlugin("ShellyPlugSPlugin", ShellyPlugSPlugin::create);
    pluginMgr.registerPlugin("SignalDetectorPlugin", SignalDetectorPlugin::create);
    pluginMgr.registerPlugin("SoundReactivePlugin", SoundReactivePlugin::create);
    pluginMgr.registerPlugin("SunrisePlugin", SunrisePlugin::create);
    pluginMgr.registerPlugin("SysMsgPlugin", SysMsgPlugin::create);
    pluginMgr.registerPlugin("TempHumidPlugin", TempHumidPlugin::create);
    pluginMgr.registerPlugin("TestPlugin", TestPlugin::create);
    pluginMgr.registerPlugin("ThreeIconPlugin", ThreeIconPlugin::create);
    pluginMgr.registerPlugin("VolumioPlugin", VolumioPlugin::create);
    pluginMgr.registerPlugin("WifiStatusPlugin", WifiStatusPlugin::create);
    pluginMgr.registerPlugin("WormPlugin", WormPlugin::create);

}

};

#endif  /* __PLUGIN_LIST_HPP__ */

/** @} */