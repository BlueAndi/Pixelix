# PIXELIX <!-- omit in toc -->
![PIXELIX](../images/LogoBlack.png)

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](http://choosealicense.com/licenses/mit/)

# SW Architecture  <!-- omit in toc -->

* [Purpose](#purpose)
* [Scope](#scope)
* [Context](#context)
* [Deployment](#deployment)
* [Layers](#layers)
  * [Application](#application)
  * [Services](#services)
  * [Hardware Abstraction Layer (HAL)](#hardware-abstraction-layer-hal)
* [System Startup](#system-startup)
  * [High Level](#high-level)
* [System States](#system-states)
* [Details](#details)
  * [Hardware Abstraction Layer (HAL)](#hardware-abstraction-layer-hal-1)
  * [Graphic Base Functionality](#graphic-base-functionality)
  * [Widgets](#widgets)
  * [User Button Handling](#user-button-handling)
  * [Audio Service](#audio-service)
  * [Topic Handler Service](#topic-handler-service)
  * [Plugin Handling](#plugin-handling)
    * [Static View](#static-view)
    * [Dynamic View](#dynamic-view)
  * [Spectrum Analyzer](#spectrum-analyzer)
  * [Slot Handling](#slot-handling)
* [Issues, Ideas And Bugs](#issues-ideas-and-bugs)
* [License](#license)
* [Contribution](#contribution)

# Purpose
The SW architecture provides an overview regarding the relationships of different classes and components. It serves for understanding, maintaining, fixing and extending the software.

# Scope
The SW architecture document is valid for the PIXELIX project.

# Context
PIXELIX as a system is not complete alone. It interacts with other neighbor systems over different kind of interfaces.

![context-diagram](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/esp-rgb-led-matrix/master/doc/architecture/uml/context.wsd)

# Deployment

![deployment-diagram](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/esp-rgb-led-matrix/master/doc/architecture/uml/deployment.wsd)

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

# Layers
![general-layers](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/esp-rgb-led-matrix/master/doc/architecture/uml/general_layers.wsd)

## Application
![app-layer](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/esp-rgb-led-matrix/master/doc/architecture/uml/app_layer.wsd)

## Services
![hal-layer](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/esp-rgb-led-matrix/master/doc/architecture/uml/service_layer.wsd)

## Hardware Abstraction Layer (HAL)
![hal-layer](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/esp-rgb-led-matrix/master/doc/architecture/uml/hal_layer.wsd)

# System Startup

## High Level
![startup-diagram](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/esp-rgb-led-matrix/master/doc/architecture/uml/startup.wsd)

# System States
![system-states-diagram](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/esp-rgb-led-matrix/master/doc/architecture/uml/system_state_machine.wsd)

# Details

## Hardware Abstraction Layer (HAL)
![hal-diagram](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/esp-rgb-led-matrix/master/doc/architecture/uml/hal.wsd)

## Graphic Base Functionality
![gfx-diagram](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/esp-rgb-led-matrix/master/doc/architecture/uml/gfx.wsd)

## Widgets
![widgets-diagram](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/esp-rgb-led-matrix/master/doc/architecture/uml/widgets.wsd)

## User Button Handling
![button-diagram](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/esp-rgb-led-matrix/master/doc/architecture/uml/button.wsd)

## Audio Service
![audio-service-diagram](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/esp-rgb-led-matrix/master/doc/architecture/uml/audio_service.wsd)

## Topic Handler Service
![topic-handler-service-diagram](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/esp-rgb-led-matrix/master/doc/architecture/uml/topic_handler_service.wsd)

## Plugin Handling

### Static View
![plugin-service-static-diagram](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/esp-rgb-led-matrix/master/doc/architecture/uml/plugin-service.wsd)

### Dynamic View
![plugin-service-dynamic-diagram](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/esp-rgb-led-matrix/master/doc/architecture/uml/plugin-service-dynamic.wsd)

![plugin-cfg-handling-dynamic-diagram](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/esp-rgb-led-matrix/master/doc/architecture/uml/plugin_cfg_handling.wsd)

## Spectrum Analyzer
![specturm-analyzer-diagram](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/esp-rgb-led-matrix/master/doc/architecture/uml/spectrum-analyzer.wsd)

## Slot Handling
![slot-handling-diagram](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/esp-rgb-led-matrix/master/doc/architecture/uml/slot_handling.wsd)

# Issues, Ideas And Bugs
If you have further ideas or you found some bugs, great! Create a [issue](https://github.com/BlueAndi/esp-rgb-led-matrix/issues) or if you are able and willing to fix it by yourself, clone the repository and create a pull request.

# License
The whole source code is published under the [MIT license](http://choosealicense.com/licenses/mit/).
Consider the different licenses of the used third party libraries too!

# Contribution
Unless you explicitly state otherwise, any contribution intentionally submitted for inclusion in the work by you, shall be licensed as above, without any
additional terms or conditions.
