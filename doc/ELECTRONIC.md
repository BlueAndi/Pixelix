# esp-rgb-led-matrix
Full RGB LED matrix, based on an ESP and WS2812B LEDs.

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](http://choosealicense.com/licenses/mit/)

## Electronic

## System diagram

![system-diagram](https://github.com/BlueAndi/esp-rgb-led-matrix/blob/master/doc/design/system.png)

### Wemos D1 mini

| Wemos D1 mini Pin | Function | ESP-8266 Pin | Connected to |
| --- | --- | --- | --- |
| TX | TXD | TXD | - |
| RX | RXD | RXD | - |
| A0 | Analog input, max 3.3V input | A0 | - |
| D0 | IO | GPIO16 | - |
| D1 | IO, SCL | GPIO5 | - |
| D2 | IO, SDA | GPIO4 | - |
| D3 | IO, 10k Pull-up | GPIO0 | - |
| D4 | IO, 10k Pull-up, BUILTIN_LED | GPIO2 | - |
| D5 | IO, SCK | GPIO14 | - |
| D6 | IO, MISO | GPIO12 | - |
| D7 | IO, MOSI | GPIO13 | - |
| D8 | IO, 10k Pull-down, SS | GPIO15 | - |
| G | Ground | GND | GND |
| 5V | 5V | - | Vcc_protected |
| 3V3 | 3.3V | 3.3V | - |
| RST | Reset | RST | - |
