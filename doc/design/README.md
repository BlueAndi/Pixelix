# PIXELIX <!-- omit in toc -->
![PIXELIX](../images/LogoBlack.png)

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](http://choosealicense.com/licenses/mit/)

# SW Architecture  <!-- omit in toc -->

- [Purpose](#purpose)
- [Scope](#scope)
- [Context](#context)
- [Deployment](#deployment)
- [System Startup](#system-startup)
  - [High Level](#high-level)
  - [Low Level](#low-level)
- [System States](#system-states)
- [Hardware Abstraction Layer (HAL)](#hardware-abstraction-layer-hal)
- [Graphic Base Functionality](#graphic-base-functionality)
- [Widgets](#widgets)
- [User Button Handling](#user-button-handling)
- [Plugin Handling](#plugin-handling)
  - [Static View](#static-view)
  - [Dynamic View](#dynamic-view)
- [Spectrum Analyzer](#spectrum-analyzer)

# Purpose
The SW architecture provides an overview regarding the relationships of different classes and components. It serves for understanding, maintaining, fixing and extending the software.

# Scope
The SW architecture document is valid for the PIXELIX project.

# Context
PIXELIX as a system is not complete alone. It interacts with other neighbor systems over different kind of interfaces.

![context-diagram](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/esp-rgb-led-matrix/Development/doc/design/uml/context.wsd)

# Deployment

![context-diagram](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/esp-rgb-led-matrix/Development/doc/design/uml/deployment.wsd)

| Task name | Core | Description |
| --------- | ---- | ----------- |
| arduinoEvents | APP_CPU | WiFi handling, see details in Arduino framework WiFiGeneric.cpp |
| AsyncHttpClientTask | APP_CPU | The AsyncHttpClient uses the task for decoupling. |
| async_tcp | PRO_CPU | AsnycTCPSock library uses the task to interact with the LwIP TCP/IP stack and the application. |
| buttonTask | APP_CPU | The task is triggered by a hardware button and provides its status. |
| esp_timer | PRO_CPU | [High resolution timer](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/esp_timer.html#high-resolution-timer) task, which calls the timer callbacks. |
| eventTask | PRO_CPU | ? TODO ? |
| IDLE0 | PRO_CPU | Idle task created for CPU 0. See [FreeRTOS applications](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/freertos.html#esp-idf-freertos-applications). |
| IDLE1 | APP_CPU | Idle task created for CPU 1. See [FreeRTOS applications](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/freertos.html#esp-idf-freertos-applications). |
| ipc0 | PRO_CPU | IPC task created for CPU 0. IPC tasks are used to implement the Inter-Processor-Call feature. See [FreeRTOS applications](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/freertos.html#esp-idf-freertos-applications). |
| ipc1 | APP_CPU | IPC task created for CPU 1. IPC tasks are used to implement the Inter-Processor-Call feature. See [FreeRTOS applications](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/freertos.html#esp-idf-freertos-applications). |
| loopTask | APP_CPU | The standard Arduino loop task. It will call setup() once and periodically loop(). |
| mdns | PRO_CPU | mDNS (multicast UDP) service. |
| network_event | APP_CPU | The task processes system events, e.g. WiFi and TCP/IP events. See [event loop library](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/esp_event.html#event-loop-library) for more information. |
| processTask | APP_CPU | The DisplayMgr process the plugins in this task. |
| spectrumAnalyzerTask | APP_CPU | Spectrum analyzer task, which receives data via DMA, performs the FFT incl. further steps and provides the data to the application. |
| tiT | PRO_CPU | The LwIP TCP/IP stack task. |
| Tmr Svc | PRO_CPU | FreeRTOS timer service. |
| updateTask | no-affinity | The DisplayMgr updates the physical display there. |
| wifi | PRO_CPU | WiFi handling. |

# System Startup

## High Level
![context-diagram](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/esp-rgb-led-matrix/Development/doc/design/uml/startup.wsd)

## Low Level

TODO

# System States
![context-diagram](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/esp-rgb-led-matrix/Development/doc/design/uml/system_state_machine.wsd)

# Hardware Abstraction Layer (HAL)
![context-diagram](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/esp-rgb-led-matrix/Development/doc/design/uml/hal.wsd)

# Graphic Base Functionality
![context-diagram](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/esp-rgb-led-matrix/Development/doc/design/uml/gfx.wsd)

# Widgets
![context-diagram](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/esp-rgb-led-matrix/Development/doc/design/uml/widgets.wsd)

# User Button Handling
![context-diagram](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/esp-rgb-led-matrix/Development/doc/design/uml/button.wsd)

# Plugin Handling

## Static View
![context-diagram](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/esp-rgb-led-matrix/Development/doc/design/uml/plugin-service.wsd)

## Dynamic View
![context-diagram](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/esp-rgb-led-matrix/Development/doc/design/uml/plugin-service-dynamic.wsd)

# Spectrum Analyzer
![context-diagram](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/esp-rgb-led-matrix/Development/doc/design/uml/spectrum-analyzer.wsd)
