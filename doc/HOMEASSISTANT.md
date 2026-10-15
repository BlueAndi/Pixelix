
# PIXELIX <!-- omit in toc -->

![PIXELIX](./images/LogoBlack.png)

## Home Assistant <!-- omit in toc -->

- [Purpose](#purpose)
- [Which Way Is The Right One?](#which-way-is-the-right-one)
  - [Recommendation](#recommendation)
  - [Comparison](#comparison)
  - [Decision Guide](#decision-guide)
  - [Combining MQTT And The Pixelix Integration](#combining-mqtt-and-the-pixelix-integration)
- [MQTT API (Recommended)](#mqtt-api-recommended)
  - [Prerequisites (MQTT)](#prerequisites-mqtt)
  - [Installation (MQTT)](#installation-mqtt)
  - [MQTT Discovery](#mqtt-discovery)
  - [Automation (MQTT)](#automation-mqtt)
  - [Notification (MQTT)](#notification-mqtt)
- [Pixelix Custom Integration](#pixelix-custom-integration)
  - [What It Provides](#what-it-provides)
  - [What It Does Not Provide](#what-it-does-not-provide)
  - [Installation (Integration)](#installation-integration)
  - [Action `pixelix.notify`](#action-pixelixnotify)
- [REST API](#rest-api)
  - [Installation (REST Command)](#installation-rest-command)
  - [Automation (REST Command)](#automation-rest-command)
- [Automation Blueprint](#automation-blueprint)
- [Issues, Ideas And Bugs](#issues-ideas-and-bugs)
- [License](#license)
- [Contribution](#contribution)

## Purpose

[Home Assistant](https://www.home-assistant.io/) is a popular open-source platform for smart home automation. Pixelix can be connected to it in three different ways, so that Home Assistant can show sensor data, states and notifications on the LED matrix.

This page first helps to choose the right way and then describes each of them in detail.

## Which Way Is The Right One?

### Recommendation

**Use the MQTT API whenever possible.** It is the only way which integrates Pixelix as a complete device with entities into Home Assistant: the device appears automatically, every plugin becomes an entity, and the display, the restart and the notification can be used like any other Home Assistant entity — without a single line of YAML.

Only if MQTT is not available, choose one of the other ways:

- If just notifications are needed, install the [Pixelix custom integration](#pixelix-custom-integration). It finds the devices by mDNS and works without a broker.
- If nothing else fits, use the [REST API](#rest-api) with the RESTful Command integration.

### Comparison

| Criterion                        | MQTT API                                          | Pixelix Integration                                | REST API                                         |
| -------------------------------- | ------------------------------------------------- | -------------------------------------------------- | ------------------------------------------------ |
| Available on every device        | No, only firmware builds with MQTT support        | Yes                                                | Yes                                              |
| Additional infrastructure        | MQTT broker required                              | None                                               | None                                             |
| Setup effort                     | Low — broker URL on the device, rest is automatic | Low — copy the folder, device is discovered        | High — every command written manually in YAML    |
| Device appears in Home Assistant | Yes, automatically with all entities              | Yes, as device with the notify action              | No, only the commands which were defined by hand |
| Entities per plugin              | Yes                                               | No                                                 | No                                               |
| Display on/off, restart          | Yes                                               | No                                                 | Only with hand-written REST commands             |
| Notifications                    | Yes (`notify` entity)                             | Yes (`pixelix.notify` action)                      | Yes (hand-written REST command)                  |
| Feedback from the device         | Availability topic (online/offline)               | REST responses are confirmed, MQTT delivery is not | Yes, the HTTP response                           |
| Updates of plugin data           | Push, no polling                                  | —                                                  | Polling only                                     |
| Web login needed                 | No                                                | Yes                                                | Yes                                              |
| Needs a Home Assistant restart   | No                                                | Yes, after copying the integration                 | Yes, after changing `configuration.yaml`         |
| Maintenance by the user          | None, new plugins appear on their own             | None                                               | Every new plugin has to be added by hand         |

### Decision Guide

1. **Does the device support MQTT?** Open the *Settings* page of the Pixelix web interface. If a field *MQTT broker URL* exists, the firmware supports MQTT.
   - **Yes, and a broker is available (or can be installed):** Use the [MQTT API](#mqtt-api-recommended). This is the recommended way. The Mosquitto broker add-on of Home Assistant is sufficient and installed with a few clicks.
   - **Yes, but no broker is wanted:** Continue with 2.
   - **No:** Continue with 2. Firmware builds for boards with little flash memory may be built without MQTT support, see `config/configSmall.ini`.
2. **What shall Home Assistant do with Pixelix?**
   - **Only send notifications** (short messages on top of everything else): Install the [Pixelix custom integration](#pixelix-custom-integration). No broker, no YAML, devices are found automatically.
   - **Show sensor data permanently in a slot**, control the display or something else: Use the [REST API](#rest-api) together with the [automation blueprints](#automation-blueprint), which hide most of the YAML.

### Combining MQTT And The Pixelix Integration

Both ways may be used at the same time. The firmware reports its MAC address via MQTT discovery and the integration registers the same MAC address as a device connection. Home Assistant treats two devices which share a connection as one device, therefore a Pixelix device which is discovered via MQTT **and** added by the integration is shown as a **single** device with the entities of the MQTT discovery and the `pixelix.notify` action of the integration.

This needs a firmware which reports its MAC address in the REST `/status` response. Older firmware simply does not report it, and then both appear as two separate devices.

In practice this combination is rarely necessary, because the MQTT API already provides a notification entity.

## MQTT API (Recommended)

Using the MQTT API requires a MQTT broker which is accessible by both Pixelix and Home Assistant.

### Prerequisites (MQTT)

- A MQTT broker in the network. If none exists, the [Mosquitto broker add-on](https://github.com/home-assistant/addons/blob/master/mosquitto/DOCS.md) of Home Assistant is the easiest solution.
- A Pixelix firmware with MQTT support. To check it, open the *Settings* page and look for the *MQTT broker URL* field.

### Installation (MQTT)

If not already installed, add the MQTT integration to your Home Assistant instance.

[![MQTT Integration](https://my.home-assistant.io/badges/config_flow_start.svg)](https://my.home-assistant.io/redirect/config_flow_start?domain=mqtt)

Then configure Pixelix in the *Settings* web page like:

1. Enter the MQTT broker URL to *MQTT broker URL*.
2. If necessary, update the *Home Assistant Discovery Prefix*.
3. Activate the checkbox *Enable Home Assistant MQTT Discovery*.
4. Restart PIXELIX.

Afterwards the device appears in Home Assistant under **Settings → Devices & services → MQTT**. Nothing else has to be configured.

### MQTT Discovery

The Home Assistant MQTT discovery is supported by several plugins and features, here are some examples:

- Display on/off
- Device restart
- Notification
- Sensor information
- IconTextPlugin
- IconTextLampPlugin
- MultiIconPlugin

Pixelix will be shown as device with its entities. Every installed plugin will be shown as at least one entity. If a plugin is installed in a slot later, its entity appears automatically — no change in Home Assistant is necessary.

The device reports its firmware version, its hardware (the build target name) and its MAC address. Because Home Assistant treats two devices which share a connection such as the MAC address as one device, a Pixelix device which is discovered by MQTT and additionally added by the [Pixelix integration](#pixelix-custom-integration) is shown as a single device instead of twice.

[More technical details about MQTT](./MQTT.md)

### Automation (MQTT)

Many things can be done directly with the entities of the device, e.g. in the automation wizard. For everything which is not covered by an entity, a message can be published to the topic of a plugin.

Add an automation to `automations.yaml`, for example to show the garage door state:

```yaml
- id: garage_door_state_on_pixelix
  alias: Garage door state on Pixelix
  description: ''
  triggers:
  - trigger: state
    entity_id:
    - cover.garage_door
  conditions: []
  actions:
  - action: mqtt.publish
    metadata: {}
    data:
      evaluate_payload: false
      qos: '0'
      topic: pixelix-6F1AD6B8/display/plugin/IconText1/iconText/set
      payload: '{ "text": "{{ states(''cover.garage_door'') }}" }'
  mode: single
```

For a simpler approach without writing YAML manually, see the [Automation Blueprint](#automation-blueprint) section.

### Notification (MQTT)

With MQTT discovery enabled, Pixelix provides a *Notification* entity (`notify` domain). It shows the message on top of everything else for a limited time.

The Home Assistant `notify.send_message` action only transfers a message. To set duration, icon and melody, the message is a JSON string. A plain text message is shown with the default duration of 10 s.

| Field          | Description                                           |
| -------------- | ----------------------------------------------------- |
| `text`         | Text to show. Mandatory, max. 256 characters.         |
| `duration`     | Duration in ms. Default and for 0: 10000, max. 60000. |
| `iconFileName` | Full path of an icon on the device. Optional.         |
| `melody`       | Melody in RTTTL format. Optional, requires a buzzer.  |
| `volume`       | Volume of the melody. Optional.                       |

```yaml
- action: notify.send_message
  target:
    entity_id: notify.pixelix_notification
  data:
    message: >-
      {{ {
        "text": "Door open",
        "duration": 15000,
        "iconFileName": "/configuration/smiley.bmp",
        "melody": "Beep:d=4,o=5,b=200:c,e,g"
      } | tojson }}
```

The entity id depends on the device name. A message which starts with `{` is forwarded unchanged, everything else is handled as text.

Without writing the JSON manually, use the [notification blueprint](#automation-blueprint).

## Pixelix Custom Integration

Pixelix provides an own Home Assistant custom integration. It is the right choice if **notifications** shall be sent and no MQTT broker is available.

The complete documentation is in the [integration README](./homeassistant/integration/README.md), the sections here are only an overview.

### What It Provides

- Automatic discovery of Pixelix devices in the network by mDNS (`_pixelix._tcp`), alternatively manual setup by host name or IP address.
- Any number of devices, each as an own device in Home Assistant, selectable as target of an action.
- The action `pixelix.notify` with text, duration, icon, RTTTL melody and volume — usable in the automation wizard without YAML.
- Transport selected automatically: if Home Assistant's MQTT integration is connected and the device reports itself online, the notification is sent via MQTT, otherwise via the REST API. Devices without MQTT therefore work as well.
- Device information (firmware version, hardware, MAC address) and reauthentication if the web login changed.

### What It Does Not Provide

- **No entities.** The integration creates a device with an action, but no sensors, switches or notify entity. The display can not be switched on and off by it and no plugin can be fed with sensor data.
- **No plugin control.** Anything which shall permanently be shown in a slot needs the MQTT API or the REST API.
- **No acknowledgement for MQTT delivery.** MQTT does not provide a device-side acknowledgement, only REST responses can confirm that a notification was accepted.

### Installation (Integration)

1. Copy the folder [custom_components/pixelix](./homeassistant/integration/custom_components/pixelix) (the inner one, containing `manifest.json`) to `<HA configuration directory>/custom_components/pixelix`.
2. Restart Home Assistant.
3. Discovered devices appear under **Settings → Devices & services**. Select *Add* and enter the web login of the device, the default is `luke` / `skywalker`. If no device is discovered, e.g. because mDNS is not forwarded to Home Assistant, add it manually: **Add integration → Pixelix** and enter the IP address.

A configuration in `configuration.yaml` is not required.

### Action `pixelix.notify`

| Field            | Required | Description                                                                      |
| ---------------- | -------- | -------------------------------------------------------------------------------- |
| `message`        | yes      | Text to show. At most 256 bytes in UTF-8 (an umlaut needs 2 bytes).              |
| `duration`       | no       | Display time in ms. Default of the firmware: 10 s, limited to 60 s.              |
| `icon_file_name` | no       | Icon path on the device, e.g. `/configuration/smiley.bmp`.                       |
| `melody`         | no       | Melody in RTTTL format, at most 256 bytes. Only played by devices with a buzzer. |
| `volume`         | no       | Volume of the melody from 0 to 10. Only used together with a melody.             |

```yaml
- action: pixelix.notify
  target:
    device_id: <DEVICE-ID>
  data:
    message: "Hello world!"
    duration: 15000
    icon_file_name: "/configuration/smiley.bmp"
    melody: "Finished:d=4,o=5,b=120:8c,8e,8g,c6"
    volume: 5
```

Without a target, all configured Pixelix devices show the notification.

## REST API

Every Pixelix device provides a REST API. Use the [RESTful Command integration](https://www.home-assistant.io/integrations/rest_command) to control Pixelix via the REST API, and the [RESTful integration](https://www.home-assistant.io/integrations/rest) to create sensor entities.

This is the most flexible, but also the most laborious way: every command has to be written by hand and Pixelix does not appear as a device in Home Assistant.

### Installation (REST Command)

Setup corresponding entities in the Home Assistant `configuration.yaml` and use them via the automation wizard or manually in the `automations.yaml`.

A simple REST command example looks like:

```yaml
rest_command:
  pixelix_notify:
    url: 'http://<IP-ADDRESS>/rest/api/v1/display/plugin/{{ alias }}/iconText?text={{ text | urlencode() }}'
    method: POST
```

The REST API is described in detail on [SwaggerHub](https://app.swaggerhub.com/apis/BlueAndi/Pixelix/1.10.0).

### Automation (REST Command)

Add an automation to `automations.yaml`, for example to show the garage door state:

```yaml
- id: garage_door_state_on_pixelix
  alias: Garage door state on Pixelix
  description: ''
  triggers:
  - trigger: state
    entity_id:
    - cover.garage_door
  conditions: []
  actions:
    - service: rest_command.pixelix_notify
      data:
        uid: 42798
        text: "{{ states('cover.garage_door') }}"
  mode: single
```

For a simpler approach without writing YAML manually, see the [Automation Blueprint](#automation-blueprint) section.

## Automation Blueprint

[Automation Blueprints](https://www.home-assistant.io/docs/automation/using_blueprints/) let you set up automations through a guided interface without writing YAML from scratch. Pixelix provides blueprints for both REST API and MQTT API communication. Click a button below to import the blueprint directly into your Home Assistant instance.

| Supported Plugin                      | Blueprint                                                                                                                                                                                                                          |
| ------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| IconTextPlugin and IconTextLampPlugin | [![Import blueprint](https://my.home-assistant.io/badges/blueprint_import.svg)](https://my.home-assistant.io/redirect/blueprint_import/?blueprint_url=https%3A%2F%2Fblueandi.github.io%2FPixelix%2Fpixelix_send_sensor_data.yaml)  |
| MultiIconPlugin                       | [![Import blueprint](https://my.home-assistant.io/badges/blueprint_import.svg)](https://my.home-assistant.io/redirect/blueprint_import/?blueprint_url=https%3A%2F%2Fblueandi.github.io%2FPixelix%2Fpixelix_multi_icon_plugin.yaml) |
| Notification (MQTT and REST)          | [![Import blueprint](https://my.home-assistant.io/badges/blueprint_import.svg)](https://my.home-assistant.io/redirect/blueprint_import/?blueprint_url=https%3A%2F%2Fblueandi.github.io%2FPixelix%2Fpixelix_notification.yaml)      |

Alternatively, copy [pixelix_send_sensor_data.yaml](./homeassistant/blueprint/pixelix_send_sensor_data.yaml), [pixelix_multi_icon_plugin.yaml](./homeassistant/blueprint/pixelix_multi_icon_plugin.yaml) and [pixelix_notification.yaml](./homeassistant/blueprint/pixelix_notification.yaml) to `/config/blueprints/automation/homeassistant/`.

When using the REST API, also add the following REST command to `configuration.yaml`:

```yaml
rest_command:
  pixelix_plugin_command:
    url: "http://{{ hostname }}{{ endpoint }}?{{ url_parameter }}"
    method: POST
```

## Issues, Ideas And Bugs

If you have ideas or found a bug, create an [issue](https://github.com/BlueAndi/Pixelix/issues). If you want to fix it yourself, clone the repository and open a pull request.

## License

The whole source code is published under the [MIT license](http://choosealicense.com/licenses/mit/).
Consider the different licenses of the used third party libraries too!

## Contribution

Unless you explicitly state otherwise, any contribution intentionally submitted for inclusion in the work by you, shall be licensed as above, without any
additional terms or conditions.
