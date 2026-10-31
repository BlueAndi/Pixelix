# Pixelix – Home Assistant custom integration

A Home Assistant integration which finds Pixelix devices in the network by mDNS and registers the action `pixelix.notify` to show notifications on them. When Home Assistant's MQTT integration is connected and the Pixelix device reports itself online, notifications are sent via MQTT. Otherwise, the integration uses the REST API, so devices without MQTT continue to work.

- Automatic discovery of Pixelix devices by mDNS
- Manual setup by host name or IP address
- Any number of Pixelix devices
- Notifications with text, duration, icon, RTTTL melody and volume
- Update of the web login without removing the device (reauthentication)

For MQTT delivery, enable MQTT on the Pixelix device and connect it to the same broker used by Home Assistant. The integration monitors the device availability topic and publishes notifications to its MQTT command topic. If the device is offline or MQTT is unavailable, notifications use REST instead. MQTT does not provide a device-side acknowledgement, so only REST responses can confirm that a notification was accepted.

## Installation

The integration needs a Pixelix firmware **v11.0.0 or newer**, because it identifies a device by the REST `GET /rest/api/v1/device` response, which older firmware does not provide (see [Device identity](#device-identity)).

1. Copy the folder `custom_components/pixelix` (the inner one, containing `manifest.json`) to `<HA configuration directory>/custom_components/pixelix` (create the directory if necessary). The `<HA configuration directory>` is the one which contains `configuration.yaml`.
2. Restart Home Assistant.
3. Add the devices, see below.

A configuration in `configuration.yaml` is not required. A `pixelix:` section of an earlier version has to be removed.

### Automatic discovery

Pixelix devices announce themselves by mDNS (`_pixelix._tcp`). Home Assistant shows them as discovered devices under **Settings → Devices & services**. Select *Add* and enter the web login of the device, which is set in the settings of the device. The default is `luke` / `skywalker`.

### Manual configuration

If a device is not found, e.g. because mDNS is not forwarded to Home Assistant, add it manually: **Settings → Devices & services → Add integration → Pixelix**. Enter the host name or IP address, the port (default 80) and the web login. An IP address is more reliable than a host name.

### Device identity

A device is identified by its unique device id (the ESP chip id) which it reports in the REST `/device` response (`device.id`). Manual setup and mDNS discovery therefore lead to the same device, and it is added only once. The IP address and host name are never used as the unique id.

The `/device` endpoint exists since firmware v11.0.0. An older firmware (e.g. v10.0.0) answers `404`, the integration reports *The device answered unexpectedly* and the device can not be added.

If the IP address of a device changes, it is not updated on its own: the mDNS discovery knows a device only by its host name, while the entry is identified by the chip id. Therefore a device which is discovered again at a new address appears as a **new discovery card**. Confirm it with the web login: the integration then reads the chip id, recognises the existing entry, updates its address and keeps it, no second device is created. A fixed IP address (DHCP reservation) avoids this.

### Device information

The device page under **Settings → Devices & services → Pixelix** shows what the device reports in its REST `/device` response:

| Shown as    | Source (`device.*`) | Example               |
| ----------- | ------------------- | --------------------- |
| Firmware    | `swVersion`         | `7.4.0`               |
| Hardware    | `hwVersion`         | `ulanzi-TC001`        |
| MAC address | `mac`               | `a1:b2:c3:d4:e5:f6`   |

The hardware is the PlatformIO target name of the firmware, which identifies the board.

The MAC address is registered as a device connection. Home Assistant treats two devices which share a connection as one, so the device of this integration and the device created by the MQTT discovery of the firmware are merged into a single device instead of appearing twice.

All of them are part of the `/device` response since firmware v11.0.0, the minimum firmware version of the integration. A detail is only updated while the device is reachable — if it is offline during a restart of Home Assistant, the last known values are kept.

### Several devices

Every device is an own entry with an own device in Home Assistant. Select one or more of them as target of the action. Without a target, all configured devices show the notification. The devices are contacted in parallel. An unreachable device only delays itself, its request ends after 10 s at the latest.

## Action `pixelix.notify`

| Field            | Required | Description                                                                                                                                                       |
| ---------------- | -------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `message`        | yes      | Text to show. At most 256 bytes in UTF-8 (an umlaut needs 2 bytes). A longer text is rejected.                                                                    |
| `duration`       | no       | Display time in ms. Without it or with `0`, the firmware uses its default of 10 s. The firmware limits it to 60 s.                                                |
| `icon_file_name` | no       | Icon path on the device, e.g. `/configuration/smiley.bmp`. A leading slash is added automatically if omitted. An unknown file does not suppress the notification. |
| `melody`         | no       | Melody in RTTTL format, at most 256 bytes. A longer melody is rejected, it is never cut. Only played by devices with a buzzer.                                    |
| `volume`         | no       | Volume of the melody from 0 to 10 (the firmware's `MAX_VOLUME` is 10). Only used together with a melody.                                                          |

Only fields which are given are sent. Everything else stays at the defaults of the firmware.

### Example

```yaml
action: pixelix.notify
data:
  message: "Hello world!"
  duration: 10000
  icon_file_name: "/configuration/smiley.bmp"
  melody: "Finished:d=4,o=5,b=120:8c,8e,8g,c6"
  volume: 5
```

The action sends a JSON body to every selected device (or to all devices if no target is specified):

```text
POST http://<HOST>/rest/api/v1/notify
{"text": "Hello world!", "duration": 10000, "iconFileName": "/washing_machine.gif", "melody": "Finished:d=4,o=5,b=120:8c,8e,8g,c6", "volume": 5}
```

The values are sent as JSON on purpose. The REST API turns URL parameters into strings, and the notification handler of the firmware does not accept a string for `duration` and `volume`.

### Example automation

```yaml
automation:
  - alias: Greet the world
    triggers:
      - trigger: state
        entity_id: sensor.washing_machine
        to: "finished"
    actions:
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

A target which is not a loaded Pixelix device is an error, nothing is sent then. The action never falls back to all devices.

If a device is not reachable or rejects the notification, the action fails with a message which names the device. The other devices still show the notification.

## Troubleshooting

| Problem                                  | Cause and solution                                                                                                                           |
| ---------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------- |
| `Integration 'pixelix' not found`        | The folder is not in `<HA configuration directory>/custom_components/pixelix`, or Home Assistant was not restarted.                          |
| The device is not discovered             | mDNS is not forwarded to Home Assistant (e.g. other network, container). Add the device manually by its IP address.                          |
| *The device is not reachable*            | Wrong host, port or the device is off. Check `http://<HOST>/` in a browser. A host name needs working name resolution, use the IP address.   |
| *The web login was rejected*             | The user or password differ from the settings of the device. Enter them again. For a configured device Home Assistant offers to update them. |
| *The device answered unexpectedly*       | The address belongs to another device, or the firmware is older than v11.0.0 and has no `/rest/api/v1/device` endpoint. Update it.           |
| Reauthentication says *another device*   | The device now reports another host name. Remove the device and add it again.                                                                |
| A notification is rejected by the device | Text and melody together exceed the memory which the firmware reserves for a request (about 500 bytes). Shorten them.                        |
| The same device exists twice             | It was added with an earlier version and was not reachable during the update. Remove the entry which uses the IP address.                    |

Set `custom_components.pixelix: debug` in the `logger` configuration for details. The web login and the content of notifications are never logged.

## Tests

The tests mock all HTTP requests. No hardware is required. Home Assistant needs a POSIX system and the Python version of its current release (Linux, macOS or WSL).

```bash
cd doc/homeassistant/integration
pip install -r requirements_test.txt
pytest
```

## Issues, Ideas And Bugs

If you have ideas or found a bug, create an [issue](https://github.com/BlueAndi/Pixelix/issues). If you want to fix it yourself, clone the repository and open a pull request.

## License

The whole source code is published under the [MIT license](http://choosealicense.com/licenses/mit/).
Consider the different licenses of the used third party libraries too!

## Contribution

Unless you explicitly state otherwise, any contribution intentionally submitted for inclusion in the work by you, shall be licensed as above, without any
additional terms or conditions.
