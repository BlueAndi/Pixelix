# PIXELIX <!-- omit in toc -->
![PIXELIX](../../../images/LogoBlack.png)

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](http://choosealicense.com/licenses/mit/)

# Electronic <!-- omit in toc -->

- [Version](#version)
- [System Diagram](#system-diagram)
- [ESP32 DevKit V1](#esp32-devkit-v1)
  - [Strapping Pins](#strapping-pins)
  - [Input Only Pins](#input-only-pins)
  - [Analog Pins](#analog-pins)
  - [JTAG](#jtag)
- [LED Matrix](#led-matrix)
- [Ambient Light Sensor](#ambient-light-sensor)
- [Issues, Ideas And Bugs](#issues-ideas-and-bugs)
- [License](#license)
- [Contribution](#contribution)

# Version

PCB board v1.5

# System Diagram

![system-diagram](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/esp-rgb-led-matrix/master/doc/architecture/uml/system.wsd)

# ESP32 DevKit V1

![ESP32 DevKitV1 Pinout](../datasheets/pinoutDOIT32devkitv1.png)

| ESP32 DevKit V1 Pin | ESP32 DevKit V1 Pin Header | WROOM32 Pin | Notes | Usage |
| --- | --- | --- | --- | --- |
| VIN | J1-1 | VIN | 5V power supply | Vcc (protected) |
| GND | J1-2 | GND | Ground | Ground |
| D13 | J1-3 | GPIO 13 | - | JTAG MTCK |
| D12 | J1-4 | GPIO 12 | Strapping pin; Fails if high at power-up, therefore not used. | JTAG MTDI |
| D14 | J1-5 | GPIO 14 | Output PWM at power-up! | JTAG MTMS |
| D27 | J1-6 | GPIO 27 | - | Data Out to LED matrix |
| D26 | J1-7 | GPIO 26 (A9) | - | - |
| D25 | J1-8 | GPIO 25 (A8) | - | - |
| D33 | J1-9 | GPIO 33 (A5) | - | 32.768 kHz crystal (32K_XN) |
| D32 | J1-10 | GPIO 32 (A4) | - | 32.768 kHz crystal (32K_XP) |
| D35 | J1-11 | GPIO 35 (A7) | Input only! See ESP32 tech. reference manual, chapter 4.1 (no internal pull-up/down circuitry) | - |
| D34 | J1-12 | GPIO 34 (A6) | Input only! See ESP32 tech. reference manual, chapter 4.1 (no internal pull-up/down circuitry) | LDR analog in |
| VN | J1-13 | GPIO 39 (VN) | Input only! See ESP32 tech. reference manual, chapter 4.1 (no internal pull-up/down circuitry) | - |
| VP | J1-14 | GPIO 36 (VP) | Input only! See ESP32 tech. reference manual, chapter 4.1 (no internal pull-up/down circuitry) | - |
| EN | J1-15 | EN | 10k Pull-Up; Button on ESP32 DevKit V1, closing to GND | USB RTS |
| VDD3V3 | J2-1 | VDD3V3 | 3.3 V output | 3.3 V output |
| GND | J2-2 | GND | Ground | Ground |
| D15 | J2-3 | GPIO 15 | Strapping pin | JTAG MTDO |
| D2 | J2-4 | GPIO 2 | Strapping pin; Onboard LED on ESP32 DevKit V1 | Onboard LED |
| D4 | J2-5 | GPIO 4 | - | User Button |
| RX2 | J2-6 | GPIO 16 (RX2) | - | - |
| TX2 | J2-7 | GPIO 17 (TX)2 | - | - |
| D5 | J2-8 | GPIO 5 | Strapping pin; 10k Pull-Up on ESP32 DevKit V1 | (DHT In) |
| D18 | J2-9 | GPIO 18 | - | - |
| D19 | J2-10 | GPIO 19 | - | - |
| D21 | J2-11 | GPIO 21 | - | I2C SDA (Arduino Standard) |
| RX0 | J2-12 | GPIO 3 (RX0) | - | USB RX |
| TX0 | J2-13 | GPIO 1 (TX0) | - | USB TX |
| D22 | J2-14 | GPIO 22 | - | I2C SCL (Arduino Standard) |
| D23 | J2-15 | GPIO 23 | - | Test Pin |
| - | - | GPIO 0 | Strapping pin, 10k Pull-Up; Button on ESP32 DevKit V1, closing to GND | USB DTR |

## Strapping Pins

The following table shows the strapping bit status on the ESP32 DevKitV1 board. See also ESP32 series datasheet chapter 2.4

| Pin | Default | Function 1 | Function 2 |
| --- | --- | --- | --- |
| Voltage of internal LDO (VDD_SDIO) |
| Pin | Default | 3.3 V | 1.8 V |
| GPIO 12 (MTDI) | Low | Low | High |
Result: 3.3 V

| Pin | Default | Function 1 | Function 2 |
| --- | --- | --- | --- |
| Booting mode |
| Pin | Default | SPI Boot | Download Boot |
| GPIO 0 | High | High | Low |
| GPIO 2 | Low | Don't care | 0 |
Result: SPI Boot

| Pin | Default | Function 1 | Function 2 |
| --- | --- | --- | --- |
| Enabling/Disabling debugging log print over U0TXD during booting |
| Pin | Default | U0TXD Toggling | U0TXD Silent |
| GPIO 15 (MTDO) | High | High | Low |
Result: U0TXD Toggling

| Pin | Default | Function 1 | Function 2 | Function 3 | Function 4 |
| --- | --- | --- | --- | --- | --- |
| Timing of SDIO slave |
| Pin | Default | Falling-edge Sampling | Falling-edge Sampling | Rising-edge Sampling | Rising-edge Sampling |
| GPIO 15 (MTDO) | High | Low | Low | High | High |
| GPIO 5 | High | Low | High | Low | High |
Result: Rising-edge Sampling

## Input Only Pins
GPIOs 34 to 39 are input only pins. These pins don’t have internal pull-ups or pull-down resistors. They can’t be used as outputs, so use these pins only as inputs.

## Analog Pins
ADC2 is used by the Wi-Fi driver. Therefore the application can only use ADC2 when the Wi-Fi driver has not started.
See [Analog to Digital Converter](https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/peripherals/adc.html).

## JTAG

| Pin | Function |
| --- | --- |
| MTDI (GPIO 12) | Test Data Input |
| MTCK (GPIO 13) | Test Clock |
| MTMS (GPIO 14) | Test Mode Select |
| MTDO (GPIO 15) | Test Data Output |

See [ESP-PROG](https://docs.platformio.org/en/latest/plus/debug-tools/esp-prog.html#debugging-tool-esp-prog).

# LED Matrix

The LED matrix consists of 8 x 32 WS2812B LEDs. Each LED has a max. current of 60 mA, which would result in a max. power consumption of 5 V / 15.36 A. The flexible LED panel matrix would be damaged with this high nominal power consumption, therefore it shall be limited to 5 V / 4 A.

Input voltage level:
* High - Min. 0.7 * VDD
  * => 0.7 * 5 V = 3.5 V
* Low - Max. 0.3 * VDD
  * => 0.3 * 5 V = 1.5 V

The i/o pins of the ESP32 are running with a 3.3 V level, which could cause a problem in detection a high!
Therefore the level must be shifted from 3.3 V to 5V.

# Ambient Light Sensor

The following table shows the output voltage, generated by a LDR like the GL5528 and a 1 k resistor. Both assembled as voltage divider.

| Ambient light | Ambient light | Photocell resistance | LDR + R | Voltage across R|
| --- | --- | --- | --- | --- |
| Moonlit night | 1 lux | 70 kΩ | 71 kΩ | 0.046 V |
| Dark room | 10 lux | 10 kΩ | 11 kΩ | 0.3 V |
| Dark overcast day / Bright room | 100 lux | 1.5 kΩ | 2.5 kΩ | 1.32 V |
| Overcast day | 1000 lux | 300 Ω | 1.3 kΩ | 2.538 V |
| Full daylight | 10,000 lux | 100 Ω | 1.1 kΩ | 3.0 V |

# Issues, Ideas And Bugs
If you have further ideas or you found some bugs, great! Create a [issue](https://github.com/BlueAndi/esp-rgb-led-matrix/issues) or if you are able and willing to fix it by yourself, clone the repository and create a pull request.

# License
The whole source code is published under the [MIT license](http://choosealicense.com/licenses/mit/).
Consider the different licenses of the used third party libraries too!

# Contribution
Unless you explicitly state otherwise, any contribution intentionally submitted for inclusion in the work by you, shall be licensed as above, without any
additional terms or conditions.
