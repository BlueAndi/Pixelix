# PIXELIX <!-- omit in toc -->
![PIXELIX](./images/LogoBlack.png)

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](http://choosealicense.com/licenses/mit/)

# Sensors

- [Sensors](#sensors)
- [Purpose](#purpose)
- [General](#general)
- [Ambient light sensor](#ambient-light-sensor)
  - [Photoresistor GL5528](#photoresistor-gl5528)
- [Temperature and humidity sensors](#temperature-and-humidity-sensors)
  - [DHTx (Proprietary one-wire)](#dhtx-proprietary-one-wire)
  - [SHT3x (I2C)](#sht3x-i2c)
- [Audio (digital micropohone with I2S)](#audio-digital-micropohone-with-i2s)
  - [INMP441](#inmp441)
- [Issues, Ideas And Bugs](#issues-ideas-and-bugs)
- [License](#license)
- [Contribution](#contribution)

# Purpose
Here is a list of supported sensors. Where they are connected, depends on the used [hardware board](./boards/README.md). Some of them need additional external parts e.g. the DHTx need a ext. 4.7kOhm pull-up resistor. Please check the corresponding datasheets.

# General
A "X" in the development board column means valid all development boards. Exceptions may be listed separately.

# Ambient light sensor

## Photoresistor GL5528

| Development Board | Photoresistor pin 1 | Photoresistor pin 2 |
| ----------------- | ------------------- | ------------------- |
| X | Vcc | Pin 34 |
| TTGO ESP32 T-Display | Vcc | Pin 32 |

# Temperature and humidity sensors

## DHTx (Proprietary one-wire)
* DHT11 (Proprietary one-wire)
* DHT12 (Proprietary one-wire)
* DHT21 (Proprietary one-wire)
* DHT22 (Proprietary one-wire)

| Development Board | DHTx pin 1 | DHTx pin 2 |
| ----------------- | ---------- | ---------- |
| X | Vcc | Pin 5 | 
| Adafruit ESP32 Feather V2 | Vcc | Pin 39 |
| TTGO ESP32 T-Display | Vcc | Pin 27 |

## SHT3x (I2C)

| Development Board | I2C SCL | I2C SDA |
| ----------------- | ------- | ------- |
| X | Pin 22 | Pin 21 | 

# Audio (digital micropohone with I2S)

## INMP441

| Development Board | Left/Right Clock (LRCL) | Data Output (DOUT) | Bit Clock (BCLK) | Left/Right Channel Select (L/R) | Chip Enable (CHIPEN) |
| ----------------- | ----------------------- | ------------------ | ---------------- | ------------------------------- | -------------------- |
| X | Pin 25 | Pin 33 | Pin 26 | GND | Vcc |

# Issues, Ideas And Bugs
If you have further ideas or you found some bugs, great! Create a [issue](https://github.com/BlueAndi/esp-rgb-led-matrix/issues) or if you are able and willing to fix it by yourself, clone the repository and create a pull request.

# License
The whole source code is published under the [MIT license](http://choosealicense.com/licenses/mit/).
Consider the different licenses of the used third party libraries too!

# Contribution
Unless you explicitly state otherwise, any contribution intentionally submitted for inclusion in the work by you, shall be licensed as above, without any
additional terms or conditions.
