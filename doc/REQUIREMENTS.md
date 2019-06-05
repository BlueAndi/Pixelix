# esp-rgb-led-matrix
Full RGB LED matrix, based on an ESP32 and WS2812B LEDs.

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](http://choosealicense.com/licenses/mit/)

## Requirements
Next available REQ id: 17

### General
* REQ-01: The display shall be able to show bitmaps and text.
* REQ-02: The display shall be able to show animations.

### Communication
* REQ-03: The communication with the display shall be via wifi.
* REQ-04: The display shall provide a REST API.
* REQ-05: The display shall provide a websocket interface.
* REQ-10: The display shall provide a button for user interaction.

#### Wifi

* REQ-07: The display shall spawn an wifi access point on user demand (via button) after a power-cycle.
* REQ-08: The wifi access point shall have the following configuration:
    * SSID: ESP-RGB-LED-MATRIX
    * Security: TBD
    * Password: TBD
* REQ-09: If a remote wifi network is configured, the display shall automatically connect after a power-cycle.

#### REST

#### Websocket

### Firmware update
* REQ-06: The firmware shall be updated over the air.
* REQ-14: The firmware update shall be protected against malicious access via authorization.

### Webserver

* REQ-11: The display shall provide a HTTPS webserver.
* REQ-12: The user shall be able to configure a remote wifi network, if a access point is spawned.
* REQ-12: The webserver shall be protected against malicious access via authorization.
* REQ-13: The webserver shall not run during an firmware update.

### Brightness Control
* REQ-15: The display brightness shall be automatically adapted to the environment.
* REQ-16: The max. display brightness shall be limited to avoid a high LED matrix current, which destroys electronic parts.

## Issues, Ideas and bugs
If you have further ideas or you found some bugs, great! Create a [issue](https://github.com/BlueAndi/esp-rgb-led-matrix/issues) or if you are able and willing to fix it by yourself, clone the repository and create a pull request.

## License
The whole source code is published under the [MIT license](http://choosealicense.com/licenses/mit/).
