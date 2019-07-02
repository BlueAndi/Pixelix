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

### Compiling project
1. Load workspace in VSCode.
2. Change to PlatformIO toolbar.
3. _Project Tasks -> Build_ or via hotkey ctrl-alt-b

### Used libraries
* [Arduino](https://docs.platformio.org/en/latest/frameworks/arduino.html#framework-arduino) - ESP framework.
* [FastLED](https://github.com/FastLED/FastLED) - Controlling the LED matrix with hardware support (RMT).
* [NeoPixelBus](https://github.com/Makuna/NeoPixelBus) - Alternative to FastLED with hardware support (I2S).
* [FastLED_NeoMatrix](https://github.com/marcmerlin/FastLED_NeoMatrix) - Matrix support.
* [Framebuffer_GFX](https://github.com/marcmerlin/Framebuffer_GFX) - Adapter for using Adafruit_GFX.
* [Adafruit_GFX](https://github.com/adafruit/Adafruit-GFX-Library) - GFX.

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

## Issues, Ideas and bugs
If you have further ideas or you found some bugs, great! Create a [issue](https://github.com/BlueAndi/esp-rgb-led-matrix/issues) or if you are able and willing to fix it by yourself, clone the repository and create a pull request.

## License
The whole source code is published under the [MIT license](http://choosealicense.com/licenses/mit/).
