# esp-rgb-led-matrix
Full RGB LED matrix, based on an ESP and WS2812B LEDs.

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](http://choosealicense.com/licenses/mit/)

## Motivation
Having a remote display to show any kind of information, running 24/7 reliable.

## Requirements
See [requirements](https://github.com/BlueAndi/esp-rgb-led-matrix/blob/master/REQUIREMENTS.md).

## Electronic
The main parts are:
* [Wemos D1 Mini](https://wiki.wemos.cc/products:d1:d1_mini) - A mini wifi board with 4 MB flash, 80 KB RAM and 80 MHz clock based on ESP-8266EX.
* WS2812B 5050 8x32 RGB Flexible LED Matrix Panel
* Power supply 5 V / 4 A

![system-diagram](https://github.com/BlueAndi/esp-rgb-led-matrix/blob/master/design/system.png)

## Software

### IDE
The [PlatformIO IDE](https://platformio.org/platformio-ide) is used for the development. Its build on top of Microsoft Visual Studio Code.

### Installation
1. Install [VSCode](https://code.visualstudio.com/).
2. Install PlatformIO IDE according to this [HowTo](https://platformio.org/install/ide?install=vscode).
3. Recommended is to take a look to the [quick-start guide](https://docs.platformio.org/en/latest/ide/vscode.html#quick-start).

### Used libraries
* [ESP8266 Non-OS SDK](https://docs.platformio.org/en/latest/frameworks/esp8266-nonos-sdk.html#framework-esp8266-nonos-sdk) - ESP framework
* [FastLED](https://github.com/FastLED/FastLED) - Controlling the LED matrix.

## Issues, Ideas and bugs
If you have further ideas or you found some bugs, great! Create a [issue](https://github.com/BlueAndi/esp-rgb-led-matrix/issues) or if you are able and willing to fix it by yourself, clone the repository and create a pull request.

## License
The whole source code is published under the [MIT license](http://choosealicense.com/licenses/mit/).
