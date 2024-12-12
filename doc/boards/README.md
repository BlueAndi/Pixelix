# PIXELIX <!-- omit in toc -->
![PIXELIX](../images/LogoBlack.png)

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](http://choosealicense.com/licenses/mit/)

# Boards <!-- omit in toc -->
Pixelix supports different ESP32 development boards. Own PCBs were developed too for easier assembling of e.g. the LED matrix, sensors, etc.

To find out which part shall be connected to which pin of your development board you can see in the following chapters.

* [Development Boards](#development-boards)
  * [Adafruit ESP32 Feather V2](#adafruit-esp32-feather-v2)
  * [Adafruit MatrixPortal ESP32-S3](#adafruit-matrixportal-esp32-s3)
  * [AZ-Delivery ESP-32 Dev Kit C V4](#az-delivery-esp-32-dev-kit-c-v4)
  * [DOIT ESP32 DEVKIT V1](#doit-esp32-devkit-v1)
  * [DOIT ESP32 DEVKIT V1 - HUB75 panel](#doit-esp32-devkit-v1---hub75-panel)
  * [ESP32-S3 DEVKITC-1-N16R8V](#esp32-s3-devkitc-1-n16r8v)
  * [ESP32 NodeMCU](#esp32-nodemcu)
  * [LILYGO® TTGO ESP32 T-Display](#lilygo-ttgo-esp32-t-display)
  * [LILYGO® ESP32 T-Display S3](#lilygo-esp32-t-display-s3)
  * [M5Stack Core ESP32 WiFi and Bluetooth Module Development Board](#m5stack-core-esp32-wifi-and-bluetooth-module-development-board)
  * [WEMOS Lolin S2 Mini](#wemos-lolin-s2-mini)
* [Pixelix Board v1.5](#pixelix-board-v15)
* [Pixelix Board v2.0](#pixelix-board-v20)
* [Pixelix Board v2.1](#pixelix-board-v21)
* [Ulanzi TC001 smart pixel clock](#ulanzi-tc001-smart-pixel-clock)

# Development Boards

## Adafruit ESP32 Feather V2
* Manufacturer: [Adafruit](https://learn.adafruit.com/adafruit-esp32-feather-v2)
* Pinning:
  * Board: [board:adafruit_feather_esp32_v2-LED-32x8](../../config/board.ini)
  * Display: [display:led_matrix_column_major_alternating](../../config/display.ini)
* Compatible with Pixelix boards.

## Adafruit MatrixPortal ESP32-S3
* Manufacturer: [Adafruit](https://learn.adafruit.com/adafruit-matrixportal-s3)
* Pinning:
  * Board: [board:adafruit_matrixportal_s3-HUB75-64x64](../../config/board.ini)
  * Display: [display:hub75-adafruit-matrixportal-esp32s3](../../config/display.ini)
* Not compatible with Pixelix boards.

## AZ-Delivery ESP-32 Dev Kit C V4
* Manufacturer: [AZ-Delivery](https://www.az-delivery.de/products/esp-32-dev-kit-c-v4)
* Pinning:
  * Board: [board:az-delivery-devkit-v4-LED-32x8](../../config/board.ini)
  * Display: [display:led_matrix_column_major_alternating](../../config/display.ini)
* Compatible with Pixelix boards.

## DOIT ESP32 DEVKIT V1
* Pinning:
  * Board: [board:esp32doit-devkit-v1-LED-32x8](../../config/board.ini)
  * Display: [display:led_matrix_column_major_alternating](../../config/display.ini)
* Compatible with Pixelix boards.

## DOIT ESP32 DEVKIT V1 - HUB75 panel
* Pinning:
  * Board: [board:esp32doit-devkit-v1-HUB75-64x64](../../config/board.ini)
  * Display: [display:hub75-esp32](../../config/display.ini)
* Pixelix boards doesn't support HUB75 panels.

## ESP32-S3 DEVKITC-1-N16R8V
* Reference design: [ESP32-S3-DevKitC-1](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/hw-reference/esp32s3/user-guide-devkitc-1-v1.0.html)
* Pinning:
  * Board: [board:esp32-s3-devkitc-1-n16r8v-LED-32x8](../../config/board.ini)
  * Display: [display:led_matrix_column_major_alternating](../../config/display.ini)
* Not compatible with Pixelix boards.

## ESP32 NodeMCU
* Pinning:
  * Board: [board:esp32-nodemcu-LED-32x8](../../config/board.ini)
  * Display: [display:led_matrix_column_major_alternating](../../config/display.ini)
* Compatible with Pixelix boards.

## LILYGO&reg; TTGO ESP32 T-Display
The LILYGO&reg; TTGO ESP32 T-Display development board is handled separately, because the onboard display is used instead of a external LED matrix.

* Manufacturer: [Lilygo](http://www.lilygo.cn/prod_view.aspx?TypeId=50033&Id=1126&FId=t3:50033:3)
* Pinning:
  * Board: [board:lilygo-ttgo-t-display-TFT-32x8](../../config/board.ini)
  * Display: [display:lilygo_ttgo_tdisplay](../../config/display.ini)
* Not compatible with Pixelix boards.

## LILYGO&reg; ESP32 T-Display S3
The LILYGO&reg; ESP32 T-Display S3 development board is handled separately, because the onboard display is used instead of a external LED matrix.

* Manufacturer: [Lilygo](https://www.lilygo.cc/products/t-display-s3)
* Pinning:
  * Board: [board:lilygo-t-display-s3-TFT-32x8](../../config/board.ini)
  * Display: [display:lilygo_tdisplay-s3](../../config/display.ini)
* Not compatible with Pixelix boards.

## M5Stack Core ESP32 WiFi and Bluetooth Module Development Board
* Manufacturer: [M5Stack](https://m5stack.com/)
* Pinning:
  * Board: [board:m5stack_core-TFT-32x8](../../config/board.ini)
  * Display: [display:m5stack_core](../../config/display.ini)
* Not compatible with Pixelix boards.

## WEMOS Lolin S2 Mini
* Manufacturer: [WEMOS](https://www.wemos.cc/en/latest/s2/s2_mini.html)
* Pinning:
  * Board: [board:wemos_lolin_s2_mini-LED-32x8](../../config/board.ini)
  * Display: [display:led_matrix_column_major_alternating](../../config/display.ini)
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
* [Housing](./pixelix/housing/)

# Pixelix Board v2.1
The Pixelix board v2.1 is the successor of Pixelix board v2.0. It was designed for ESP32 DevKit V4 (4 MB flash) and the Adafruit ESP32 Feather V2 (8 MB flash) development board as piggy bag.
* [Hardware description](./pixelix/v2.1/ELECTRONIC.md)
* [Bill of material](./pixelix/v2.1/BOM.md)
* [Pinning](../../lib/HalLedMatrix/Board.h)
* [Housing](./pixelix/housing/)

# Ulanzi TC001 smart pixel clock
* Manufacturer: [Ulanzi](https://www.ulanzi.com/products/ulanzi-pixel-smart-clock-2882)
* Pinning:
  * Board: [board:ulanzi-tc001-LED-32x8](../../config/board.ini)
  * Display: [display:led_matrix_row_major_alternating](../../config/display.ini)

# Issues, Ideas And Bugs <!-- omit in toc -->
If you have further ideas or you found some bugs, great! Create a [issue](https://github.com/BlueAndi/Pixelix/issues) or if you are able and willing to fix it by yourself, clone the repository and create a pull request.

# License <!-- omit in toc -->
The whole source code is published under the [MIT license](http://choosealicense.com/licenses/mit/).
Consider the different licenses of the used third party libraries too!

# Contribution <!-- omit in toc -->
Unless you explicitly state otherwise, any contribution intentionally submitted for inclusion in the work by you, shall be licensed as above, without any
additional terms or conditions.
