# PIXELIX <!-- omit in toc -->
![PIXELIX](./images/LogoBlack.png)

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](http://choosealicense.com/licenses/mit/)

# Plugins <!-- omit in toc -->
The content of the display can be configured by installing an individual set of plugins.
Each plugin is identified by its unique UID.

* [Generic plugins](#generic-plugins)
  * [GrabViaMqttPlugin](#grabviamqttplugin)
  * [GrabViaRestPlugin](#grabviarestplugin)
  * [IconTextPlugin](#icontextplugin)
  * [IconTextLampPlugin](#icontextlampplugin)
  * [JustTextPlugin](#justtextplugin)
  * [MultiIconPlugin](#multiiconplugin)
* [Dedicated plugins](#dedicated-plugins)
  * [BatteryPlugin](#batteryplugin)
  * [BTCQuotePlugin](#btcquoteplugin)
  * [CountdownPlugin](#countdownplugin)
  * [DateTimePlugin](#datetimeplugin)
  * [DDPPlugin](#ddpplugin)
    * [xlights Configuration](#xlights-configuration)
  * [FirePlugin](#fireplugin)
  * [GameOfLifePlugin](#gameoflifeplugin)
  * [GruenbeckPlugin](#gruenbeckplugin)
  * [MatrixPlugin](#matrixplugin)
  * [OpenWeatherPlugin](#openweatherplugin)
  * [RainbowPlugin](#rainbowplugin)
  * [SensorPlugin](#sensorplugin)
  * [SignalDetectorPlugin](#signaldetectorplugin)
  * [SoundReactivePlugin](#soundreactiveplugin)
  * [SunrisePlugin](#sunriseplugin)
  * [SysMsgPlugin](#sysmsgplugin)
  * [TempHumidPlugin](#temphumidplugin)
  * [TestPlugin](#testplugin)
  * [VolumioPlugin](#volumioplugin)
  * [WifiStatusPlugin](#wifistatusplugin)
  * [WormPlugin](#wormplugin)
* [Issues, Ideas And Bugs](#issues-ideas-and-bugs)
* [License](#license)
* [Contribution](#contribution)

# Generic plugins
The generic plugins allow the user to control the different UI elements described in the plugin name via the [REST API](https://app.swaggerhub.com/apis/BlueAndi/Pixelix/1.5.0).

## GrabViaMqttPlugin
The plugin can grab information in JSON format via MQTT and shows it on the display.
[Configuration examples](./grabConfigs/mqtt/) may help to configure.

## GrabViaRestPlugin
The plugin can grab information in JSON format via REST API and shows it on the display.
Each part can be set separately via the [REST API](https://app.swaggerhub.com/apis/BlueAndi/Pixelix/1.5.0#/GrabViaRestPlugin).
[Configuration examples](./grabConfigs/rest/) may help to configure.

## IconTextPlugin
The IconTextPlugin shows an icon on left side, text on right side. If no text is set, the plugin will be skipped in the slot.\
Each part can be set separately via the [REST API](https://app.swaggerhub.com/apis/BlueAndi/Pixelix/1.5.0#/IconTextPlugin).

If MQTT is built in and enabled, it will support Home Assistant MQTT discovery.

## IconTextLampPlugin
The IconTextLampPlugin shows an icon on left side, text on right side and lamps at the bottom.\
Each part can be set separately via the [REST API](https://app.swaggerhub.com/apis/BlueAndi/Pixelix/1.5.0#/IconTextLampPlugin).

If MQTT is built in and enabled, it will support Home Assistant MQTT discovery.

## JustTextPlugin
The JustTextPlugin shows only text on the whole display. If no text is set, the plugin will be skipped in the slot.\
The text to be displayed can be set via the [REST API](https://app.swaggerhub.com/apis/BlueAndi/Pixelix/1.5.0#/JustTextPlugin).

If MQTT is built in and enabled, it will support Home Assistant MQTT discovery.

## MultiIconPlugin
The MultiIconPlugin shows multiple icons on the display.\
Each icon can be set separately via the [REST API](https://app.swaggerhub.com/apis/BlueAndi/Pixelix/1.5.0#/MultiIconPlugin).

# Dedicated plugins
Dedicated plugins are plugins which only serves one single purpose thy are only internally configurable.

## BatteryPlugin
This plugin displays the battery symbol with state of charge bar.

## BTCQuotePlugin
The BTCQuotePlugin shows the current exchange rate from Bitcoin to USD.\
Powered by [CoinDesk](https://www.coindesk.com/price/bitcoin).

## CountdownPlugin
The CountdownPlugin shows the remaining days until a configured target date.\
Target date and the description of the target day (plural/singular form) can be set via the [REST API](https://app.swaggerhub.com/apis/BlueAndi/Pixelix/1.5.0#/CountdownPlugin).

## DateTimePlugin
The plugin shows the current time and/or date.

If date and time are configured, first the time will be shown for half of the slot duration and after it the date. It can be configured to show only the date or only the time as well.

Below the day of the week is shown, starting from the left with Monday.

Configure the date and time format in the plugin configuration JSON file. The format itself is according to strftime(). For colorization text properties can be added.

By default the local time (see timezone in the settings) is used. It can be overwritten by the plugin configuration.

It can be set what shall be shown via the [REST API](https://app.swaggerhub.com/apis/BlueAndi/Pixelix/1.5.0#/DateTimePlugin).

## DDPPlugin
The plugin setup a server supporting the Distributed Display Protocol (DDP), which is used e.g. by [xlights](https://www.xlights.org) or [LedFx](https://www.ledfx.app).

Supported formats:
* RGB with 24-bit per pixel

### xlights Configuration
* Add Ethernet controller
    * Name: Pixelix
    * IP Address: &lt;IP-ADDRESS&gt;
    * Protocol: DDP
* Add layout
    * Create new matrix
        * Name: Matrix8x32
    * Matrix
        * Strings: 8
        * Nodes/String: 32
        * Strands/String: 1
        * Starting Location: Top Left
        * Controller: Pixelix
    * Controller Connection
        * Port: 1
        * Protocol: LED Panel Matrix
    * String Properties
        * String Type: RGB Nodes
    * Appearance
        * Pixel Size: 10
        * Pixel Style: Square

## FirePlugin
The FirePlugin shows a animated fire on the display.

## GameOfLifePlugin
The GameOfLifePlugin shows the game of life game on the display.

## GruenbeckPlugin
The GruenbeckPlugin shows the remaining system capacity (parameter = D_Y_10_1 ) of the Gruenbeck softliQ SC18 via the system's RESTful webservice.\
The IP address of the Gruenbeck webserver can be set via the [REST API](https://app.swaggerhub.com/apis/BlueAndi/Pixelix/1.5.0#/GruenbeckPlugin).

## MatrixPlugin
The plugin shows the effect from the film "Matrix" over the whole display.

## OpenWeatherPlugin
The OpenWeatherPlugin shows the current weather condition (icon and temperature) and one additional information (uvIndex, humidity or windspeed) .\
Information provided by [OpenWeather](https://openweathermap.org/).\
In order to use the plugin an API key is necessary, see https://openweathermap.org/appid for further information.\
The coordinates (latitude & longitude) of your location, your API key and the desired additional information to be displayed can be set via the [REST API](https://app.swaggerhub.com/apis/BlueAndi/Pixelix/1.5.0#/OpenWeatherPlugin).

## RainbowPlugin
The RainbowPlugin shows an animated rainbow on the display.

## SensorPlugin
The plugin shows sensor values of the selected sensor channel.

## SignalDetectorPlugin
The plugin is able to detect a signal, which can be combined with up to 2 frequencies.\
Each frequency must be detected for a specific configureable time.\
As long as nothing is detected, the plugin will disable itself.\
If a signal is detected, it will be shown on the display for the configured slot duration. After slot duration timeout or user changed the slot, the plugin will be disabled until next signal detection. \
Additional a push notification can be configured. By default a GET is triggered. Using "GET" or "POST" as prefix its configureable. Example: "POST http://..."
Each part can be set separately via the [REST API](https://app.swaggerhub.com/apis/BlueAndi/Pixelix/1.5.0#/SignalDetectorPlugin).

## SoundReactivePlugin
The plugin shows octave frequency bands, depended on the environment sound.
Required: A digital microphone (INMP441) is required, connected to the I2S port.
The number of shown frequency bands can be set via the [REST API](https://app.swaggerhub.com/apis/BlueAndi/Pixelix/1.5.0#/SoundReactivePlugin).

## SunrisePlugin
The SunrisePlugin shows the current sunrise / sunset times for a configured location.\
The coordinates (latitude & longitude) of your location can be set via the [REST API]([REST.md#endpoint-base-uridisplayuidplugin-uidlocation](https://app.swaggerhub.com/apis/BlueAndi/Pixelix/1.5.0#/SunrisePlugin)).\
Powered by sunrise-sunset.org

Configure the time format in the plugin configuration JSON file. The format itself is according to strftime(). For colorization text properties can be added.

## SysMsgPlugin
The SysMsgPlugin is a system plugin, which is used to splash important information's to the user. Note, it can not be uninstalled.

## TempHumidPlugin
Th TempHumidPlugin displays the temperature (in °C) and Humidity (in %) based on the readings of sensor connected to Pixelix.

## TestPlugin
The TestPlugin can be used to check whether the LED matrix topology (layout) is correct or not.

## VolumioPlugin
The VolumioPlugin shows the current VOLUMIO state as icon and the played artist/title.\
If the VOLUMIO server is offline, the plugin gets automatically disabled, otherwise enabled.\
The host address of the Volumio webserver can be set via the [REST API](https://app.swaggerhub.com/apis/BlueAndi/Pixelix/1.5.0#/VolumioPlugin).

## WifiStatusPlugin
The WifiStatusPlugin shows the current wireless signal strength.

## WormPlugin
The plugin shows animated worms on the display. If they find some meal, they will grow up. If they eat too much, they will explode.

# Issues, Ideas And Bugs
If you have further ideas or you found some bugs, great! Create a [issue](https://github.com/BlueAndi/esp-rgb-led-matrix/issues) or if you are able and willing to fix it by yourself, clone the repository and create a pull request.

# License
The whole source code is published under the [MIT license](http://choosealicense.com/licenses/mit/).
Consider the different licenses of the used third party libraries too!

# Contribution
Unless you explicitly state otherwise, any contribution intentionally submitted for inclusion in the work by you, shall be licensed as above, without any
additional terms or conditions.
