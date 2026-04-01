
# PIXELIX <!-- omit in toc -->

![PIXELIX](./images/LogoBlack.png)

## Home Assistant <!-- omit in toc -->

- [Purpose](#purpose)
- [Communication](#communication)
  - [REST API](#rest-api)
    - [Installation (REST Command)](#installation-rest-command)
    - [Automation (REST Command)](#automation-rest-command)
  - [MQTT API](#mqtt-api)
    - [Installation (MQTT)](#installation-mqtt)
    - [MQTT Discovery](#mqtt-discovery)
    - [Automation (MQTT)](#automation-mqtt)
- [Automation Blueprint](#automation-blueprint)
- [Issues, Ideas And Bugs](#issues-ideas-and-bugs)
- [License](#license)
- [Contribution](#contribution)

## Purpose

[Home Assistant](https://www.home-assistant.io/) is a popular open-source platform for smart home automation. Pixelix supports displaying sensor data from Home Assistant using both the REST API and the MQTT API. Especially the MQTT automatic discovery support is fast and seamless.

## Communication

Every Pixelix device provides a REST API, and some also support the MQTT API. To check whether MQTT is available, open the *Settings* page and look for the *MQTT broker URL* field.

### REST API

Use the [RESTful Command integration](https://www.home-assistant.io/integrations/rest_command) to control Pixelix via the REST API, and the [RESTful integration](https://www.home-assistant.io/integrations/rest) to create sensor entities.

#### Installation (REST Command)

Setup corresponding entities in the Home Assistant `configuration.yaml` and use them via the automation wizard or manually in the `automations.yaml`.

A simple REST command example looks like:

```yaml
rest_command:
  pixelix_notify:
    url: 'http://<IP-ADDRESS>/rest/api/v1/display/uid/{{ uid }}/iconText?text={{ text | urlencode() }}'
    method: POST
```

The REST API is described in detail on [SwaggerHub](https://app.swaggerhub.com/apis/BlueAndi/Pixelix/1.8.0).

#### Automation (REST Command)

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

### MQTT API

Using the MQTT API requires a MQTT broker that is accessible by both Pixelix and Home Assistant.

#### Installation (MQTT)

If not already installed, add the MQTT integration to your Home Assistant instance.

[![MQTT Integration](https://my.home-assistant.io/badges/config_flow_start.svg)](https://my.home-assistant.io/redirect/config_flow_start?domain=mqtt)

Then configure Pixelix in the *Settings* web page like:

1. Enter the MQTT broker URL to *MQTT broker URL*.
2. If necessary, update the *Home Assistant Discovery Prefix*.
3. Activate the checkbox *Enable Home Assistant MQTT Discovery*.
4. Restart PIXELIX.

#### MQTT Discovery

The Home Assistant MQTT discovery is supported by several plugins and features, here are some examples:

- Display on/off
- Device restart
- Sensor information
- IconTextPlugin
- IconTextLampPlugin
- MultiIconPlugin

Pixelix will be shown as device with its entities. Every installed plugin will be shown as at least one entity.

[More technical details about MQTT](./MQTT.md)

#### Automation (MQTT)

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
      topic: pixelix-6F1AD6B8/display/uid/42798/iconText/set
      payload: '{ "text": "{{ states(''cover.garage_door'') }}" }'
  mode: single
```

For a simpler approach without writing YAML manually, see the [Automation Blueprint](#automation-blueprint) section.

## Automation Blueprint

[Automation Blueprints](https://www.home-assistant.io/docs/automation/using_blueprints/) let you set up automations through a guided interface without writing YAML from scratch. Pixelix provides blueprints for both REST API and MQTT API communication. Click a button below to import the blueprint directly into your Home Assistant instance.

| Supported Plugin                      | Blueprint                                                                                                                                                                                                                                                                               |
| ------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| IconTextPlugin and IconTextLampPlugin | [![Import blueprint](https://my.home-assistant.io/badges/blueprint_import.svg)](https://my.home-assistant.io/redirect/blueprint_import/?blueprint_url=https%3A%2F%2Fgithub.com%2FBlueAndi%2FPixelix%2Fblob%2Fmaster%2Fdoc%2Fhomeassistant%2Fblueprint%2Fpixelix_send_sensor_data.yaml)  |
| MultiIconPlugin                       | [![Import blueprint](https://my.home-assistant.io/badges/blueprint_import.svg)](https://my.home-assistant.io/redirect/blueprint_import/?blueprint_url=https%3A%2F%2Fgithub.com%2FBlueAndi%2FPixelix%2Fblob%2Fmaster%2Fdoc%2Fhomeassistant%2Fblueprint%2Fpixelix_multi_icon_plugin.yaml) |

Alternatively, copy [pixelix_send_sensor_data.yaml](./homeassistant/blueprint/pixelix_send_sensor_data.yaml) and [pixelix_multi_icon_plugin.yaml](./homeassistant/blueprint/pixelix_multi_icon_plugin.yaml) to `/config/blueprints/automation/homeassistant/`.

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
