# PIXELIX <!-- omit in toc -->

![PIXELIX](../images/LogoBlack.png)

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](http://choosealicense.com/licenses/mit/)

## SW Architecture  <!-- omit in toc -->

- [Purpose](#purpose)
- [Scope](#scope)
- [Context](#context)
- [Task Deployment](#task-deployment)
- [Layers](#layers)
  - [Application](#application)
  - [Services](#services)
  - [Hardware Abstraction Layer (HAL)](#hardware-abstraction-layer-hal)
- [System Startup](#system-startup)
  - [High Level](#high-level)
- [System States](#system-states)
- [Details](#details)
  - [Hardware Abstraction Layer (HAL)](#hardware-abstraction-layer-hal-1)
  - [Graphic Base Functionality](#graphic-base-functionality)
  - [Widgets](#widgets)
  - [User Button Handling](#user-button-handling)
  - [Audio Service](#audio-service)
  - [Rest Service](#rest-service)
    - [Get Request](#get-request)
    - [Service Stop](#service-stop)
    - [Plugin Stop](#plugin-stop)
  - [Topic Handler Service](#topic-handler-service)
  - [Timer Service](#timer-service)
  - [MQTT Service](#mqtt-service)
  - [Plugin Handling](#plugin-handling)
    - [Static View](#static-view)
    - [Dynamic View](#dynamic-view)
  - [Spectrum Analyzer](#spectrum-analyzer)
  - [Slot Handling](#slot-handling)
- [Issues, Ideas And Bugs](#issues-ideas-and-bugs)
- [License](#license)
- [Contribution](#contribution)

## Purpose

The SW architecture provides an overview regarding the relationships of different classes and components. It serves for understanding, maintaining, fixing and extending the software.

The architecture is service-oriented and follows a common service lifecycle (`start()`, `process()`, `stop()`) via `IService`. Services are started in `InitState` (`Services::startAll()`), processed periodically during runtime (`Services::processAll()`), and stopped in reverse order in `RestartState` (`Services::stopAll()`).

## Scope

The SW architecture document is valid for the PIXELIX project.

## Context

PIXELIX as a system is not complete alone. It interacts with other neighbor systems over different kind of interfaces.

![context-diagram](./svg/context.svg)

## Task Deployment

The diagram shows the task deployment on the esp32 dual core derivates.

![deployment-diagram](./svg/deployment.svg)

| Task name            | Core        | Description                                                                                                                                                                                                                                              |
| -------------------- | ----------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| arduinoEvents        | APP_CPU     | WiFi handling, see details in Arduino framework WiFiGeneric.cpp                                                                                                                                                                                          |
| AsyncHttpClientTask  | APP_CPU     | The AsyncHttpClient uses the task for decoupling.                                                                                                                                                                                                        |
| async_tcp            | PRO_CPU     | AsyncTCPSock library uses the task to interact with the LwIP TCP/IP stack and the application.                                                                                                                                                           |
| buttonTask           | APP_CPU     | The task is triggered by a hardware button and provides its status.                                                                                                                                                                                      |
| esp_timer            | PRO_CPU     | [High resolution timer](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/esp_timer.html#high-resolution-timer) task, which calls the timer callbacks.                                                                    |
| eventTask            | PRO_CPU     | Internal [event loop library](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/esp_event.html#event-loop-library) task of ESP-IDF. It dispatches posted system events (e.g. WiFi/TCP-IP) to registered handlers.         |
| IDLE0                | PRO_CPU     | Idle task created for CPU 0. See [FreeRTOS applications](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/freertos.html#esp-idf-freertos-applications).                                                                  |
| IDLE1                | APP_CPU     | Idle task created for CPU 1. See [FreeRTOS applications](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/freertos.html#esp-idf-freertos-applications).                                                                  |
| ipc0                 | PRO_CPU     | IPC task created for CPU 0. IPC tasks are used to implement the Inter-Processor-Call feature. See [FreeRTOS applications](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/freertos.html#esp-idf-freertos-applications). |
| ipc1                 | APP_CPU     | IPC task created for CPU 1. IPC tasks are used to implement the Inter-Processor-Call feature. See [FreeRTOS applications](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/freertos.html#esp-idf-freertos-applications). |
| loopTask             | APP_CPU     | The standard Arduino loop task. It will call setup() once and periodically loop().                                                                                                                                                                       |
| mdns                 | PRO_CPU     | mDNS (multicast UDP) service.                                                                                                                                                                                                                            |
| network_event        | APP_CPU     | The task processes system events, e.g. WiFi and TCP/IP events. See [event loop library](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/esp_event.html#event-loop-library) for more information.                        |
| processTask          | APP_CPU     | The DisplayMgr process the plugins in this task.                                                                                                                                                                                                         |
| spectrumAnalyzerTask | APP_CPU     | Spectrum analyzer task, which receives data via DMA, performs the FFT incl. further steps and provides the data to the application.                                                                                                                      |
| tiT                  | PRO_CPU     | The LwIP TCP/IP stack task.                                                                                                                                                                                                                              |
| Tmr Svc              | PRO_CPU     | FreeRTOS timer service.                                                                                                                                                                                                                                  |
| updateTask           | no-affinity | The DisplayMgr updates the physical display there.                                                                                                                                                                                                       |
| wifi                 | PRO_CPU     | WiFi handling.                                                                                                                                                                                                                                           |

## Layers

![general-layers](./svg/general_layers.svg)

### Application

![app-layer](./svg/app_layer.svg)

### Services

Most runtime features are implemented as services behind `IService` and orchestrated by the generated `Services` facade. The `TopicHandlerService` acts as integration hub between REST/MQTT topic APIs and internal feature modules. Several services provide their own settings topics (e.g. `mqttService/mqtt`, `timerService/timer`) and persist configuration in the filesystem.

![service-layer](./svg/service_layer.svg)

### Hardware Abstraction Layer (HAL)

![hal-layer](./svg/hal_layer.svg)

## System Startup

### High Level

![startup-diagram](./svg/startup.svg)

## System States

![system-states-diagram](./svg/system_state_machine.svg)

## Details

### Hardware Abstraction Layer (HAL)

![hal-diagram](./svg/hal.svg)

### Graphic Base Functionality

![gfx-diagram](./svg/gfx.svg)

### Widgets

![widgets-diagram](./svg/widgets.svg)

### User Button Handling

![button-diagram](./svg/button.svg)

### Audio Service

![audio-service-diagram](./svg/audio_service.svg)

### Rest Service

![rest-service-class-diagram](./svg/rest_service.svg)

#### Get Request

![rest-service-get-request-diagram](./svg/rest_service_get_request.svg)

#### Service Stop

![rest-service-stop-diagram](./svg/rest_service_stop.svg)

#### Plugin Stop

![rest-service-plugin-stop-diagram](./svg/rest_service_plugin_stop.svg)

### Topic Handler Service

![topic-handler-service-diagram](./svg/topic_handler_service.svg)

### Timer Service

`TimerService` implements scheduled display control with persisted timer rules. It loads `/configuration/timerService.json`, registers topic `timerService/timer` at `TopicHandlerService`, and evaluates up to 8 timer entries every 100 ms. If a rule signals, it triggers `DisplayMgr::displayOn()`, `DisplayMgr::displayOff()`, and/or `DisplayMgr::setBrightness()`.

![timer-service-diagram](./svg/timer_service.svg)

### MQTT Service

`MqttService` manages MQTT broker connectivity and topic I/O. It loads `/configuration/mqttService.json`, creates broker connections (`MqttBrokerConnection`), and delegates publish/subscribe calls to connection instances. The service registers its own settings topic (`mqttService/mqtt`) lazily in `process()` to ensure topic handlers are already started.

Each broker connection runs a small state machine (`IDLE`, `DISCONNECTED`, `CONNECTED`) with reconnect timer handling, optional TLS client setup (`WiFiClientSecure`), MQTT birth/last-will messages, and automatic resubscription after reconnect.

![mqtt-service-diagram](./svg/mqtt_service.svg)

### Plugin Handling

#### Static View

![plugin-service-static-diagram](./svg/plugin-service.svg)

#### Dynamic View

![plugin-service-dynamic-diagram](./svg/plugin-service-dynamic.svg)

![plugin-cfg-handling-dynamic-diagram](./svg/plugin_cfg_handling.svg)

### Spectrum Analyzer

![specturm-analyzer-diagram](./svg/spectrum-analyzer.svg)

### Slot Handling

![slot-handling-diagram](./svg/slot_handling.svg)

## Issues, Ideas And Bugs

If you have further ideas or you found some bugs, great! Create a [issue](https://github.com/BlueAndi/Pixelix/issues) or if you are able and willing to fix it by yourself, clone the repository and create a pull request.

## License

The whole source code is published under the [MIT license](http://choosealicense.com/licenses/mit/).
Consider the different licenses of the used third party libraries too!

## Contribution

Unless you explicitly state otherwise, any contribution intentionally submitted for inclusion in the work by you, shall be licensed as above, without any
additional terms or conditions.
