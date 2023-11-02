# PIXELIX <!-- omit in toc -->
![PIXELIX](./images/LogoBlack.png)

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](http://choosealicense.com/licenses/mit/)

# Plugin development <!-- omit in toc -->

* [MQTT](#mqtt)
* [Overview Mindmap](#overview-mindmap)
* [MQTT Topics](#mqtt-topics)
  * [Birth and last will](#birth-and-last-will)
  * [Plugin topic path](#plugin-topic-path)
  * [Topic name](#topic-name)
  * [Sending a bitmap](#sending-a-bitmap)
  * [Sensors](#sensors)
* [Issues, Ideas And Bugs](#issues-ideas-and-bugs)
* [License](#license)
* [Contribution](#contribution)

# MQTT
Pixelix is a MQTT client which can be connected to a MQTT broker. The MQTT broker URL is configued in the settings via webinterface.

Format: \[mqtt://\]\[&lt;USER&gt;:&lt;PASSWORD&gt;@\]&lt;URI&gt;

Examples:
* without authentication:
    * mosquitto.at.home
    * mqtt://mosquitto.at.home
* with authentication:
    * myuser:mypassword@mosquitto.at.home
    * mqtt://myuser:mypassword@mosquitto.at.home

# Overview Mindmap

![topic-handling-minmap](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/esp-rgb-led-matrix/master/doc/architecture/uml/topic_handling_mindmap.wsd)

# MQTT Topics

## Birth and last will
Pixelix supports birth and last will messages.

After the successful connection establishment to the MQTT broker, Pixelix will send "online" to the &lt;HOSTNAME&gt;/status topic. In any disconnect case, "offline" will be sent to the &lt;HOSTNAME&gt;/status topic.

## Plugin topic path
The base topic path to access plugin related topics can be setup with the plugin UID or the plugin alias:
* &lt;HOSTNAME&gt;/display/uid/&lt;PLUGIN-UID&gt;/...
* &lt;HOSTNAME&gt;/display/alias/&lt;PLUGIN-ALIAS&gt;/...

## Topic name
The complete topic name can be derived from the REST API documentation.

Example: JustTextPlugin

The REST API URL looks like the following: http://&lt;HOSTNAME&gt;/rest/api/v1/display/uid/&lt;PLUGIN-UID&gt;/text?text=&lt;TEXT&gt;
1. Replace the http://&lt;HOSTNAME&gt;/rest/api/v1/ part with &lt;HOSTNAME&gt; --> &lt;HOSTNAME&gt;/display/uid/&lt;PLUGIN-UID&gt;/text?text=&lt;TEXT&gt;
2. Every URL parameter, which is in this case show=&lt;TEXT&gt; must be sent in JSON format.

```json
{
    "text": "<text>"
}
```

## Sending a bitmap

The JSON data must have two keys:
* fileName: The name of the file which to upload.
* file: The file itself in BASE64 encoded.

Example: IconTextPlugin

```json
{
    "fileName": "hacker.bmp",
    "file": "Qk32AAAAAAAAADYAAAAoAAAACAAAAAgAAAABABgAAAAAAMAAAAAAAAAAAAAAAAAAAAAAAAAAzEg/AAAAAAAAAAAAzEg/zEg/AAAAAAAAAAAAzEg/zEg/zEg/zEg/AAAAAAAAAAAAAAAAzEg/AAAAAAAAAAAAAAAAJBztJBztAAAAzEg/AAAAzEg/zEg/zEg/AAAAJBztAAAAAAAAzEg/AAAAAAAAAAAAAAAAJBztAAAAAAAAAAAAzEg/zEg/AAAAAAAAJBztAAAAAAAAAAAAzEg/zEg/AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
}
```

## Sensors
The sensor topic path is valid if the sensor is available!

* Temperature in °C: &lt;HOSTNAME&gt;/sensors/0/temperature/state
* Humidity in %: &lt;HOSTNAME&gt;/sensors/1/humidity/state
* Illuminance in lx: &lt;HOSTNAME&gt;/sensors/2/illuminance/state
* Battery SOC in %: &lt;HOSTNAME&gt;/sensors/3/soc/state

# Issues, Ideas And Bugs
If you have further ideas or you found some bugs, great! Create a [issue](https://github.com/BlueAndi/esp-rgb-led-matrix/issues) or if you are able and willing to fix it by yourself, clone the repository and create a pull request.

# License
The whole source code is published under the [MIT license](http://choosealicense.com/licenses/mit/).
Consider the different licenses of the used third party libraries too!

# Contribution
Unless you explicitly state otherwise, any contribution intentionally submitted for inclusion in the work by you, shall be licensed as above, without any
additional terms or conditions.
