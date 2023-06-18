# PIXELIX <!-- omit in toc -->
![PIXELIX](../images/LogoBlack.png)

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](http://choosealicense.com/licenses/mit/)

# Boards <!-- omit in toc -->
Pixelix supports different ESP32 development boards. Own PCBs were developed too for easier assembling of e.g. the LED matrix, sensors, etc.

To find out which part shall be connected to which pin of your development board you can see in the following chapters.

* [Development Boards](#development-boards)
  * [AZ-Delivery ESP-32 Dev Kit C V4](#az-delivery-esp-32-dev-kit-c-v4)
  * [DOIT ESP32 DEVKIT V1](#doit-esp32-devkit-v1)
  * [DOIT ESP32 DEVKIT V4](#doit-esp32-devkit-v4)
  * [ESP32 NodeMCU](#esp32-nodemcu)
  * [LILYGO® ESP32 T-Display S3](#lilygo-esp32-t-display-s3)
  * [LILYGO® TTGO ESP32 T-Display](#lilygo-ttgo-esp32-t-display)
  * [WEMOS Lolin S2 Mini](#wemos-lolin-s2-mini)
* [Pixelix Board v1.5](#pixelix-board-v15)
* [Pixelix Board v2.0](#pixelix-board-v20)
* [Pixelix Board v2.1](#pixelix-board-v21)
* [Ulanzi TC001 smart pixel clock](#ulanzi-tc001-smart-pixel-clock)

# Development Boards

## AZ-Delivery ESP-32 Dev Kit C V4
* Manufacturer: [AZ-Delivery](https://www.az-delivery.de/products/esp-32-dev-kit-c-v4)
* [Pinning](../../config/board.ini)
* Compatible with Pixelix boards.

## DOIT ESP32 DEVKIT V1
* [Pinning](../../config/board.ini)
* Compatible with Pixelix boards.

## DOIT ESP32 DEVKIT V4
* [Pinning](../../config/board.ini)
* Compatible with Pixelix boards.

## ESP32 NodeMCU
* [Pinning](../../config/board.ini)
* Compatible with Pixelix boards.

## LILYGO&reg; ESP32 T-Display S3
The LILYGO&reg; ESP32 T-Display S3 development board is handled separately, because the onboard display is used instead of a external LED matrix.

* Manufacturer: [Lilygo](https://www.lilygo.cc/products/t-display-s3)
* [Pinning](../../config/board.ini)
* Not compatible with Pixelix boards.

## LILYGO&reg; TTGO ESP32 T-Display
The LILYGO&reg; TTGO ESP32 T-Display development board is handled separately, because the onboard display is used instead of a external LED matrix.

* Manufacturer: [Lilygo](http://www.lilygo.cn/prod_view.aspx?TypeId=50033&Id=1126&FId=t3:50033:3)
* [Pinning](../../config/board.ini)
* Not compatible with Pixelix boards.

## WEMOS Lolin S2 Mini
* Manufacturer: [WEMOS](https://www.wemos.cc/en/latest/s2/s2_mini.html)
* [Pinning](../../config/board.ini)
* Not compatible with Pixelix boards.

# Pixelix Board v1.5
This board was original developed with a ESP32 DevKit V1 development board as piggy bag.
* [Hardware description](./pixelix/v1.5/ELECTRONIC.md)
* [Bill of material](./pixelix/v1.5/BOM.md)
* [Pinning](../../lib/HalLedMatrix/Board.h)
* [Schematic](./pixelix/v1.5/schematics/)
* [PCB](./pixelix/v1.5/pcb/)
* [Housing Part 1](https://www.thingiverse.com/thing:2791276)
* [Housing Part 2](https://www.thingiverse.com/thing:3884621)

# Pixelix Board v2.0
The Pixelix board v2.0 is the successor of Pixelix board v1.5. It was designed for ESP32 DevKit V4 (4 MB flash) and the Adafruit ESP32 Feather V2 (8 MB flash) development board as piggy bag.
* [Hardware description](./pixelix/v2.0/ELECTRONIC.md)
* [Bill of material](./pixelix/v2.0/BOM.md)
* [Pinning](../../lib/HalLedMatrix/Board.h)
* [Housing](./pixelix/v2.0/3D-files/)

# Pixelix Board v2.1
The Pixelix board v2.1 is the successor of Pixelix board v2.0. It was designed for ESP32 DevKit V4 (4 MB flash) and the Adafruit ESP32 Feather V2 (8 MB flash) development board as piggy bag.
* [Hardware description](./pixelix/v2.1/ELECTRONIC.md)
* [Bill of material](./pixelix/v2.1/BOM.md)
* [Pinning](../../lib/HalLedMatrix/Board.h)
* [Housing](./pixelix/v2.1/3D-files/)

# Ulanzi TC001 smart pixel clock
* Manufacturer: [Ulanzi](https://www.ulanzi.com/products/ulanzi-pixel-smart-clock-2882)
* [Pinning](../../config/board.ini)

# Issues, Ideas And Bugs <!-- omit in toc -->
If you have further ideas or you found some bugs, great! Create a [issue](https://github.com/BlueAndi/esp-rgb-led-matrix/issues) or if you are able and willing to fix it by yourself, clone the repository and create a pull request.

# License <!-- omit in toc -->
The whole source code is published under the [MIT license](http://choosealicense.com/licenses/mit/).
Consider the different licenses of the used third party libraries too!

# Contribution <!-- omit in toc -->
Unless you explicitly state otherwise, any contribution intentionally submitted for inclusion in the work by you, shall be licensed as above, without any
additional terms or conditions.
