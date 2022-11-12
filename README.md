# PIXELIX <!-- omit in toc -->
![PIXELIX](./doc/images/LogoBlack.png)

Full RGB LED matrix, based on an ESP32 and WS2812B LEDs.

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](http://choosealicense.com/licenses/mit/)
[![Repo Status](https://www.repostatus.org/badges/latest/wip.svg)](https://www.repostatus.org/#wip)
[![Release](https://img.shields.io/github/release/BlueAndi/esp-rgb-led-matrix.svg)](https://github.com/BlueAndi/esp-rgb-led-matrix/releases)
[![Build Status](https://github.com/BlueAndi/esp-rgb-led-matrix/workflows/PlatformIO%20CI/badge.svg?branch=master)](https://github.com/BlueAndi/esp-rgb-led-matrix/actions?query=workflow%3A%22PlatformIO+CI%22)

[![pixelix](https://img.youtube.com/vi/dik8Rm6f3o0/0.jpg)](https://www.youtube.com/watch?v=dik8Rm6f3o0 "Pixelix")
[![pixelix](https://img.youtube.com/vi/UCjJCI5JShY/0.jpg)](https://www.youtube.com/watch?v=UCjJCI5JShY "Pixelix - Remote Button")

- [Motivation](#motivation)
- [Overview](#overview)
  - [Original setup](#original-setup)
  - [Others](#others)
- [Installation](#installation)
- [Very First Startup](#very-first-startup)
- [User Interface](#user-interface)
- [Documentation](#documentation)
- [Used Libraries](#used-libraries)
- [FAQ](#faq)
  - [Where to change panel topology of the LED matrix?](#where-to-change-panel-topology-of-the-led-matrix)
  - [How to change text properties?](#how-to-change-text-properties)
  - [The display only shows a error code, like "E4". What does that mean?](#the-display-only-shows-a-error-code-like-e4-what-does-that-mean)
  - [How can I use animated icons?](#how-can-i-use-animated-icons)
  - [How do I know that my sensor is recognized?](#how-do-i-know-that-my-sensor-is-recognized)
  - [Why do I see sometimes values from the LDR in the SensorPlugin, although no LDR is installed?](#why-do-i-see-sometimes-values-from-the-ldr-in-the-sensorplugin-although-no-ldr-is-installed)
  - [How can I use alternative icons?](#how-can-i-use-alternative-icons)
  - [Is it possible to use a font with 8px height?](#is-it-possible-to-use-a-font-with-8px-height)
  - [How to configure the date/time format?](#how-to-configure-the-datetime-format)
- [Issues, Ideas And Bugs](#issues-ideas-and-bugs)
- [License](#license)
- [Contribution](#contribution)

# Motivation
I want to have a remote display to show multiple kind of information, running 24/7 reliable. Connected over wifi to the local network, it can be controlled via REST API or websocket.

| Some impressions |   |
| - | - |
| ![TimePlugin](./data/images/TimePlugin.jpg) | ![DatePlugin](./data/images/DatePlugin.jpg) |
| ![IconTextLampPlugin](./data/images/IconTextLampPlugin.jpg) | ![IconTextPlugin](./data/images/IconTextPlugin.jpg) |
| ![WifiStatusPlugin](./data/images/WifiStatusPlugin.jpg) | ... |

# Overview

The firmware contains a plugin concept (at compile time) to provide different functionalities. Each plugin can create its own layout and place the information as required. If you are only interested in showing just text provided via REST API, choose the JustTextPlugin. It uses the whole display size and will scroll text automatically. Sometimes in front of the text a nice icon is required, in this case choose the IconTextPlugin. These are only examples and you will find more in the [plugin list](./doc/PLUGINS.md).

A little bit more detail about the generic plugins and the first idea can be found in this [rough overview](./doc/Overview.pdf).

## Original setup
The original setup during development and the first release was:
* [ESP32 DevKitV1](https://github.com/playelek/pinout-doit-32devkitv1)
* WS2812B 5050 8x32 RGB Flexible LED Matrix Panel
* Power supply 5 V / 4 A

The following shows the absolute minimal wiring setup:

![PixelixMinimalSetup](./doc/images/PixelixMinimalSetup.png)

> :warning: **If you power all via USB**: Be very careful, because it may destroy your esp32 board if the LED current gets too high. Avoid increasing the LED display brightness or filling it complete with white pixels. Please use a external power supply with at least 5V / 4A.

## Others

In the meantime several other boards are supported as well. You can see them in the platformio configuration (platformio.ini) or the [list of boards](./doc/boards/README.md).

Additional supported variants, which were original not in focus:
* [TTGO T-Display ESP32 WiFi and Bluetooth Module Development Board For Arduino 1.14 Inch LCD](http://www.lilygo.cn/prod_view.aspx?TypeId=50033&Id=1126&FId=t3:50033:3)

Although Pixelix was designed to show information, that is pushed or pulled via REST API, the following sensors can be directly connected and evaluated:
* Temperature and humidity sensors DHTx
* Temperature and humidity sensors SHT3x
* Digitial microphone INMP441 for some sound reactive stuff.

# Installation
The following steps are necessary for the first time and to get PIXELIX initial running on the target. Once it runs, later on the firmware and filesystem can be updated via the PIXELIX webinterface.

1. Setup the [toolchain](./doc/TOOLCHAIN-INSTALLATION.md).
2. [Build the software](./doc/SW-BUILD.md) and check whether the toolchain works.
3. [Upload/Update the software and firmware](./doc/SW-UPDATE.md) to the target.

# Very First Startup
If the device starts the very first time, the wifi station SSID and passphrase settings are empty. To be able to configure them, start the device and keep the button pressed until it shows the SSID. The device will start up as wifi access point with the default SSID "pixelix" and the default password "Luke, I am your father.".

Use the browser to connect to the captive portal and configure the wifi station SSID and passphrase. Depended on the type of device you are using for connecting to Pixelix, you may get a notification that further information is necessary and automatically routed to the captive portal. In any other case enter the URL http://192.168.4.1 in the browser address field.

After configuration restart and voila, Pixelix will be available in your wifi network.

# User Interface
* The user button activates always the next slot.
* If the display's location is hard to reach, the virtual user button can be used. It is controllable via REST API and perfect for remote buttons like the [Shelly Button 1](https://shelly.cloud/products/shelly-button-1-smart-home-automation-device/).
* If a ambilight sensor (LDR) is connected, the display brightness is automatically adapted.
* The web interface provides the possibility to install plugins, control their duration in the slots and etc.
* Some plugin's spawn a dedicated REST API, see the web page of the plugin or have a look to the REST API documentation.

Note, the websocket interface is currently only used as a service in the web interface.

# Documentation
For more information, see the [documentation](./doc/README.md).

# Used Libraries
* [Arduino](https://github.com/platformio/platform-espressif32) - ESP32 Arduino framework - Apache-2.0 License
* [NeoPixelBus](https://github.com/Makuna/NeoPixelBus) - Controlling the LED matrix with hardware support (RMT) - LGPL-3.0 License
* [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer) - Webserver - LGPL-2.1 License
* [AsyncTCPSock](https://github.com/yubox-node-org/AsyncTCPSock) - TCP library, Reimplementation of the API of me-no-dev/AsyncTCP using high-level BSD sockets - LGPL-3.0 License
* [ArduinoJson](https://arduinojson.org/) - JSON handling - MIT License
* [StreamUtils](https://github.com/bblanchon/ArduinoStreamUtils) - Stream utilities - MIT License
* [Bootstrap](https://getbootstrap.com/) - CSS Framework - MIT License
* [POPPER JS](https://popper.js.org/) - POPPER JS - MIT License
* [jQuery](https://jquery.com/) - Javascript librariy for DOM handling - MIT License
* [Adafruit Unified Sensor Driver](https://github.com/adafruit/Adafruit_Sensor) - A unified sensor abstraction layer. - Apache License 2.0 License
* [Adafruit DHT sensor library](https://github.com/adafruit/DHT-sensor-library) - An Arduino library for the DHT series of low-cost temperature/humidity sensors. - MIT License
* [arduino-sht](https://github.com/Sensirion/arduino-sht) - An Arduino library for reading the SHT3x family of temperature and humidity sensors. - BSD-3-Clause License
* [TFT_eSPI](https://github.com/Bodmer/TFT_eSPI) - Arduino and PlatformIO IDE compatible TFT library optimised for the Raspberry Pi Pico (RP2040), STM32, ESP8266 and ESP32 that supports different driver chips - Mixed licenses: MIT, BSD, FreeBSD
* [arduinoFFT](https://github.com/kosme/arduinoFFT) - Fast Fourier Transform for Arduino. - GPL 3.0 License
* [mufonts](https://github.com/muwerk/mufonts) - A collection of fonts compatible with Adafruit GFX library. These fonts were developed when creating various samples for mupplet display code. - MIT License
* [JSZip](https://github.com/Stuk/jszip) - A library for creating, reading and editing .zip files with JavaScript, with a lovely and simple API. - MIT License
* [JSZipUtils](https://github.com/Stuk/jszip-utils) - A collection of cross-browser utilities to go along with JSZip. - MIT License
* [FileSaver.js](https://github.com/eligrey/FileSaver.js) - FileSaver.js is the solution to saving files on the client-side. - MIT License

# FAQ

## Where to change panel topology of the LED matrix?
First adapt in ```./lib/HalLedMatrix/Board.h``` the _width_ and _height_ according your LED matrix.

In the ```./lib/HalLedMatrix/LedMatrix.h``` file you have to change the member variable _m\_topo_ according to your physical panel topology. Take a look how your pixels are wired on the pcb and use the following page to choose the right one: https://github.com/Makuna/NeoPixelBus/wiki/Layout-objects

## How to change text properties?
Text properties can be changed using different keywords added to the string to be displayed.  
In order to be able to use these keywords, they must be prefixed by a backslash, otherwise they will only be treated as text.

The following keywords are available:
Keyword   | Description
----------|---------------------------------
\\#RRGGBB | Change text color (RRGGBB in hex)
\lalign   | Alignment left
\ralign   | Alignment right
\calign   | Alignment center

**Note**
- If theses keywords are used within the sourcecode they have to be prefixed with two backslashes (one additional for escaping).
- If these keywords are used via the [REST API](https://app.swaggerhub.com/apis/BlueAndi/Pixelix/1.2.0) all unsafe ASCII characters must be replaced by the respective percent encoding (see also [ASCII Encoding Reference](https://www.w3schools.com/tags/ref_urlencode.ASP)).
- The keywords can be combined.  

**Examples**

Sourcecode   | URL   | Result
----------|--------------------|-------------
\\\lalign\\\\#ff0000Hi! | %5Clalign%23ff0000Hi! | I<span style="color:red">Hi!</span>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;I
\\\calign\\#ff0000Hi! | %5Ccalign%23ff0000Hi! | I&nbsp;&nbsp;&nbsp;&nbsp;<span style="color:red">Hi!</span>&nbsp;&nbsp;&nbsp;&nbsp;I
\\\ralign\\#ff0000Hi! | %5Cralign%23ff0000Hi!| I&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<span style="color:red">Hi!</span>I

## The display only shows a error code, like "E4". What does that mean?

This is a low level error code. Please have a look into the following table.

| Error code | Description |
| ---------- | ----------- |
| E1 | Something happened, which can not be further explained, but was fatal. |
| E2 | There is a problem with the two-wire (i2c) interface. |
| E3 | There is no user button available. |
| E4 | Bad filesystem, did you explicit program the filesystem too? If not, please upload it. |
| E5 | The display manager didn't start up. |
| E6 | The system message handler didn't start up. |
| E7 | The update manager didn't start up. |

## How can I use animated icons?
Upload first the bitmap texture image (.bmp) and afterwards the sprite sheet file (.sprite). See the details [here](./doc/SPRITESHEET.md).

## How do I know that my sensor is recognized?

1. Check the log output from USB (not via web interface) right after startup (at this point WiFi is not connected yet).
2. Install the SensorPlugin and open the SensorPlugin website. There the sensor should be shown in the drop-down.

## Why do I see sometimes values from the LDR in the SensorPlugin, although no LDR is installed?

The LDR pin is configured as input (ADC) and it seems that the pin is floating, because there is the ext. pull-down missing.

## How can I use alternative icons?

Find details [here](./doc/ICONS.md).

## Is it possible to use a font with 8px height?
There are three font types defined:
* default (6px height)
* normal (6px height)
* large (8px height)

Note, the default font type is "normal".

The font type can be selected per plugin instance in the settings web page.

Example:
```json
{
    "name": "JustTextPlugin",
    "uid": 32690,
    "alias": "",
    "fontType": "large"
}
```

Not all plugin may support this in case they get conflicts with their layout.
If a plugin don't support it, it will use the default font type.

## How to configure the date/time format?
The date/time format can be configured in the settings of Pixelix. The format specifiers following [strftime()](https://cplusplus.com/reference/ctime/strftime/).

Examples:
* ```%I:%M %p```: 02:30 PM
* ```%H:%M```: 14:30
* ```%m/%d```: 11/12
* ```%d.%m.```: 11.12.
* ```%d - %b```: 11 - Nov

# Issues, Ideas And Bugs
If you have further ideas or you found some bugs, great! Create a [issue](https://github.com/BlueAndi/esp-rgb-led-matrix/issues) or if you are able and willing to fix it by yourself, clone the repository and create a pull request.

# License
The whole source code is published under the [MIT license](http://choosealicense.com/licenses/mit/).
Consider the different licenses of the used third party libraries too!

# Contribution
Unless you explicitly state otherwise, any contribution intentionally submitted for inclusion in the work by you, shall be licensed as above, without any
additional terms or conditions.
