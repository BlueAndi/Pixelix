# PIXELIX <!-- omit in toc -->
![PIXELIX](./images/LogoBlack.png)

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](http://choosealicense.com/licenses/mit/)

# Requirements <!-- omit in toc -->

- [General](#general)
- [Communication](#communication)
- [User Interaction](#user-interaction)
  - [Wifi](#wifi)
  - [REST](#rest)
  - [Websocket](#websocket)
- [Firmware update](#firmware-update)
- [TCP/IP Server](#tcpip-server)
- [Brightness Control](#brightness-control)
- [Real Time Clock](#real-time-clock)
- [Issues, Ideas And Bugs](#issues-ideas-and-bugs)
- [License](#license)
- [Contribution](#contribution)

Next available REQ id: 21

# General
* REQ-01: The display shall be able to show bitmaps and text.
* REQ-02: The display shall be able to show animations.

# Communication
* REQ-03: The communication with the display shall be via wifi.
* REQ-04: The display shall provide a [REST](https://en.wikipedia.org/wiki/Representational_state_transfer) API to exchange [JSON](https://en.wikipedia.org/wiki/JSON).
* REQ-05: The display shall provide a [websocket](https://en.wikipedia.org/wiki/WebSocket) interface.

# User Interaction
* REQ-10: The display shall provide a user button for user interaction.
* REQ-17: The display shall provide a reset button.

## Wifi

* REQ-07: The display shall setting up an wifi access point on user demand (via user button) after a power-cycle.
* REQ-08: The wifi access point shall have the following default configuration:
    * SSID: pixelix
    * Security: WPA
    * Password: "Luke, I am your father."
* REQ-09: If a remote wifi network is configured, the display shall automatically connect after a power-cycle.

## REST

* REQ-19: A REST interface shall be provided to control the display or retrieve information.

## Websocket

* REQ-20: A websocket interface shall be provided to control the display or retrieve information.

# Firmware update
* REQ-06: The firmware shall be updated over the air.
* REQ-14: The firmware update shall be protected against malicious access via authorization.

# TCP/IP Server

* REQ-11: The display shall provide a [HTTP](https://en.wikipedia.org/wiki/Hypertext_Transfer_Protocol)/[HTTPS](https://en.wikipedia.org/wiki/Hypertext_Transfer_Protocol_Secure) webserver.
* REQ-12: The user shall be able to configure a remote wifi network, if a access point is spawned.
* REQ-12: The webserver shall be protected against malicious access via authorization.
* REQ-13: The webserver shall not run during an firmware update.

# Brightness Control
* REQ-15: The display brightness shall be automatically adapted to the environment.
* REQ-16: The max. display brightness shall be limited to avoid a high LED matrix current, which destroys electronic parts.

# Real Time Clock
* REQ-18: The display shall contain a real time clock, used for power modes or a clock.

# Issues, Ideas And Bugs
If you have further ideas or you found some bugs, great! Create a [issue](https://github.com/BlueAndi/esp-rgb-led-matrix/issues) or if you are able and willing to fix it by yourself, clone the repository and create a pull request.

# License
The whole source code is published under the [MIT license](http://choosealicense.com/licenses/mit/).
Consider the different licenses of the used third party libraries too!

# Contribution
Unless you explicitly state otherwise, any contribution intentionally submitted for inclusion in the work by you, shall be licensed as above, without any
additional terms or conditions.
