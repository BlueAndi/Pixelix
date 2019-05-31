# esp-rgb-led-matrix
Full RGB LED matrix, based on an ESP and WS2812B LEDs.

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](http://choosealicense.com/licenses/mit/)

## Electronic

### System diagram

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

Important note: All of the I/O pins run at 3.3 V.

### LED Matrix

The LED matrix consists of 8 x 12 WS2812B LEDs. Each LED has a max. current of 60 mA, which would result in a max. power consumption of 5 V / 15.36 A. The flexible LED panel matrix would be damaged with this high nominal power consumption, therefore it shall be limited to 5 V / 4 A.

Input voltage level:
* High - Min. 0.7 * VDD
  * => 0.7 * 5 V = 3.5 V
* Low - Max. 0.3 * VDD
  * => 0.3 * 5 V = 1.5 V

The I/O pins of the Wemos D1 mini are running with a 3.3 V level, which could cause a problem in detection a high!

Therefore the level must be shifted from 3.3 V to 5V.
