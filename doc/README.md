# PIXELIX Documentation <!-- omit in toc -->

![PIXELIX](./images/LogoBlack.png)

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](http://choosealicense.com/licenses/mit/)

This index puts user guides first. For a first installation, start with the
[installation and startup guide](../README.md#installation) in the main README.

## For users

### Get started and use PIXELIX

- [Install PIXELIX and complete the first startup](../README.md#installation)
- [Set up Wi-Fi](../README.md#very-first-startup)
- [Use the device buttons and remote button](../README.md#user-interface)
- [Update a running device](./config/SW-UPDATE.md)
- [Frequently asked questions](./FAQ.md)

### Configure the display

- [Supported boards and pin connections](./boards/README.md)
- [Available plugins and display content](./PLUGINS.md)
- [Supported sensors, wiring, and calibration](./SENSORS.md)
- [Use alternative icons](./ICONS.md)
- [GrabViaREST and GrabViaMQTT configuration examples](./grabConfigs/README.md)

### Connect PIXELIX to other systems

- [Home Assistant: choose MQTT, the custom integration, or REST](./HOMEASSISTANT.md)
- [REST API reference](https://app.swaggerhub.com/apis/BlueAndi/Pixelix/1.10.0)
- [MQTT API](./MQTT.md)
- [WebSocket API](./WEBSOCKET.md)

## For developers

- [Set up the toolchain](./config/TOOLCHAIN-INSTALLATION.md), [build firmware](./config/SW-BUILD.md), and [upload it](./config/SW-UPDATE.md)
- [Configure boards and build variants](./config/README.md)
- [Custom ESP-IDF libraries](./CUSTOM-IDF-LIBRARIES.md)
- [Software architecture](./architecture/README.md)
- [Develop a plugin](./PLUGIN-DEV.md)
- [Third-party libraries and license information](./USED-LIBRARIES.md)
- [Web frontend conventions](./WEB-CONVENTIONS.md)
- [CI and release workflows](./WORKFLOWS.md)
- [Coredump analysis and debugging](./COREDUMP-ANALYSIS.md)
