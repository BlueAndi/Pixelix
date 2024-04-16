# PIXELIX <!-- omit in toc -->
![PIXELIX](./images/LogoBlack.png)

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](http://choosealicense.com/licenses/mit/)

# Sensors

* [Sensors](#sensors)
* [Purpose](#purpose)
* [General](#general)
* [Ambient light sensor](#ambient-light-sensor)
  * [Photoresistor GLxx](#photoresistor-glxx)
* [Temperature and humidity sensors](#temperature-and-humidity-sensors)
  * [DHTx (Proprietary one-wire)](#dhtx-proprietary-one-wire)
  * [SHT3x (I2C)](#sht3x-i2c)
* [Audio (digital micropohone with I2S)](#audio-digital-micropohone-with-i2s)
  * [INMP441](#inmp441)
* [Calibration](#calibration)
* [Issues, Ideas And Bugs](#issues-ideas-and-bugs)
* [License](#license)
* [Contribution](#contribution)

# Purpose
Here is a list of supported sensors. Where they are connected, depends on the used [hardware board](./boards/README.md). Some of them need additional external parts e.g. the DHTx need a ext. 4.7kOhm pull-up resistor. Please check the corresponding datasheets.

# General
A "X" in the development board column means valid all development boards. Exceptions may be listed separately.

# Ambient light sensor

## Photoresistor GLxx

| Development Board | Photoresistor pin 1 | Photoresistor pin 2 |
| ----------------- | ------------------- | ------------------- |
| X | Vcc | Pin 34 |
| Lilygo&reg; ESP32 T-Display S3 | Vcc | Pin 10 |
| Lilygo&reg; TTGO ESP32 T-Display | Vcc | Pin 32 |
| WEMOS Lolin S2 Mini | Vcc | Pin 1 |

The photoresistor pin 2 needs additional a pull-down.

# Temperature and humidity sensors

## DHTx (Proprietary one-wire)
Different DHT sensors are supported. Out of the box the DHT11 is configured. It can be changed in the source code, see /lib/Sensors/Sensors.cpp:
```cpp
static SensorDhtX       gDht11(SensorDhtX::MODEL_DHT11);
```

* DHT11 (Proprietary one-wire)
* DHT12 (Proprietary one-wire)
* DHT21 (Proprietary one-wire)
* DHT22 (Proprietary one-wire)

| Development Board | DHTx pin 1 | DHTx pin 2 |
| ----------------- | ---------- | ---------- |
| X | Vcc | Pin 5 |
| Lilygo&reg; ESP32 T-Display S3 | Vcc | Pin 21 |
| Lilygo&reg; TTGO ESP32 T-Display | Vcc | Pin 27 |

## SHT3x (I2C)

| Development Board | I2C SCL | I2C SDA |
| ----------------- | ------- | ------- |
| X | Pin 22 | Pin 21 |
| Lilygo&reg; ESP32 T-Display S3 | Pin 17 | Pin 18 |
| WEMOS Lolin S2 Mini | Pin 35 | Pin 33 |

# Audio (digital micropohone with I2S)

## INMP441
Only the left channel is supported, therefore the L/R channel select signal shall be connected to GND.

| Development Board | Left/Right Clock (LRCL) | Data Output (DOUT) | Bit Clock (BCLK) | Left/Right Channel Select (L/R) | Chip Enable (CHIPEN) |
| ----------------- | ----------------------- | ------------------ | ---------------- | ------------------------------- | -------------------- |
| X | Pin 25 | Pin 33 | Pin 26 | GND | Vcc |
| Lilygo&reg; ESP32 T-Display S3 | Pin 12 | Pin 18 | Pin 11 | GND | Vcc |
| WEMOS Lolin S2 Mini | - | - | - | - | - |

# Calibration
With Pixelix v7.x.x the following sensor values can be calibrated with an offset. Open in the file browser web page the file ```/configuration/sensors.json``` to adjust them. Note, a change applies only after a reset!

The sensors.json will be automatically created after the first start. Every available sensor will be considered. If a sensor is not available, it will appear but without any channel value.

Example in case no sensor is available:
```json
{
  "GL5528": [],
  "SHT3x": [],
  "DHT11": []
}
```

The channel id order of the sensor is the order in the array:

| Sensor | Channel id | Channel description |
| - | - | - |
| GL5528 | 0 | Illuminance in lux |
| SHT3x | 0 | Temperature in °C |
| SHT3x | 1 | Humidity in % |
| DHT11 | 0 | Temperature in ° C |
| DHT11 | 1 | Humidity in % |
| DHT12 | 0 | Temperature in ° C |
| DHT12 | 1 | Humidity in % |
| DHT21 | 0 | Temperature in ° C |
| DHT21 | 1 | Humidity in % |
| DHT22 | 0 | Temperature in ° C |
| DHT22 | 1 | Humidity in % |

Example in case all sensors are available:
```json
{
  "GL5528": [
    0.0
  ],
  "SHT3x": [
    0.0, 0.0
  ],
  "DHT11": [
    0.0, 0.0
  ]
}
```

# Issues, Ideas And Bugs
If you have further ideas or you found some bugs, great! Create a [issue](https://github.com/BlueAndi/esp-rgb-led-matrix/issues) or if you are able and willing to fix it by yourself, clone the repository and create a pull request.

# License
The whole source code is published under the [MIT license](http://choosealicense.com/licenses/mit/).
Consider the different licenses of the used third party libraries too!

# Contribution
Unless you explicitly state otherwise, any contribution intentionally submitted for inclusion in the work by you, shall be licensed as above, without any
additional terms or conditions.
