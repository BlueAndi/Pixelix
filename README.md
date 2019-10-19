# esp-rgb-led-matrix
Full RGB LED matrix, based on an ESP32 and WS2812B LEDs.

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](http://choosealicense.com/licenses/mit/)
[![Release](https://img.shields.io/github/release/BlueAndi/esp-rgb-led-matrix.svg)](https://github.com/BlueAndi/esp-rgb-led-matrix/releases)
[![Build Status](https://travis-ci.org/BlueAndi/esp-rgb-led-matrix.svg?branch=master)](https://travis-ci.org/BlueAndi/esp-rgb-led-matrix)

## Motivation
Having a remote display to show any kind of information, running 24/7 reliable.

## Requirements
See [requirements](https://github.com/BlueAndi/esp-rgb-led-matrix/blob/master/doc/REQUIREMENTS.md).

## Electronic
The main parts are:
* [ESP32 DevKitV1](https://github.com/playelek/pinout-doit-32devkitv1)
* WS2812B 5050 8x32 RGB Flexible LED Matrix Panel
* Power supply 5 V / 4 A

See [electronic detail](https://github.com/BlueAndi/esp-rgb-led-matrix/blob/master/doc/ELECTRONIC.md).

## Software

### IDE
The [PlatformIO IDE](https://platformio.org/platformio-ide) is used for the development. Its build on top of Microsoft Visual Studio Code.

### Installation
1. Install [VSCode](https://code.visualstudio.com/).
2. Install PlatformIO IDE according to this [HowTo](https://platformio.org/install/ide?install=vscode).
3. Close and start VSCode again.
4. Recommended is to take a look to the [quick-start guide](https://docs.platformio.org/en/latest/ide/vscode.html#quick-start).

### Installation of test environment
1. For the test environment on windows platform, install [MinGW](http://www.mingw.org/).
    * Install as basic setup:
        * mingw-developer-toolkit-bin
        * mingw32-base-bin
        * mingw32-gcc-g++-bin
        * msys-base-bin
    * Install additional packages:
        * mingw32-libmingwex-dev
        * mingw32-libmingwex-dll
2. Place ```c:\mingw\bin``` on path.

### Build Project
1. Load workspace in VSCode.
2. Change to PlatformIO toolbar.
3. _Project Tasks -> Build_ or via hotkey ctrl-alt-b
4. Running tests with _Project Tasks -> env:native -> Test_

### Update of the device

#### Update via usb
Set the following in the _platformio.ini_ configuration file:
* Set _upload_protocol_ to _esptool_.
* Clear _upload_port_.
* Clear _upload_flags_.

Build and upload the software via _Project Tasks -> Upload_ and the filesystem via _Project Tasks -> Upload File System image_.

#### Update via over-the-air
Set the following in the _platformio.ini_ configuration file:
* Set _upload_protocol_ to _espota_.
* Set _upload_port_ to the device ip-address.
* Set _upload_flags_ to _--port=3232_

Build and upload the software via _Project Tasks -> Upload_ and the filesystem via _Project Tasks -> Upload File System image_.

### Used Libraries
* [Arduino](https://docs.platformio.org/en/latest/frameworks/arduino.html#framework-arduino) - ESP framework.
* [FastLED](https://github.com/FastLED/FastLED) - Controlling the LED matrix with hardware support (RMT).
* [FastLED_NeoMatrix](https://github.com/marcmerlin/FastLED_NeoMatrix) - Matrix support.
* [Framebuffer_GFX](https://github.com/marcmerlin/Framebuffer_GFX) - Adapter for using Adafruit_GFX.
* [Adafruit_GFX](https://github.com/adafruit/Adafruit-GFX-Library) - GFX.
* [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer) - Webserver

### Structure

<pre>
+---doc             (Documentation)
    +---datasheets  (Datasheets of electronic parts)
    +---design      (Design related documents)
    +---eagle-libs  (Eagle libraries)
    \---schematics  (Schematics)
+---include         (Include files)
+---lib             (Project specific (private) libraries)
+---src             (Sourcecode)
+---test            (Unit tests)
\---platform.ini    (Project configuration file)
</pre>

# Issues, Ideas And Bugs
If you have further ideas or you found some bugs, great! Create a [issue](https://github.com/BlueAndi/esp-rgb-led-matrix/issues) or if you are able and willing to fix it by yourself, clone the repository and create a pull request.

# License
The whole source code is published under the [MIT license](http://choosealicense.com/licenses/mit/).
