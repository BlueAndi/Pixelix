# PIXELIX <!-- omit in toc -->
![PIXELIX](./images/LogoBlack.png)

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](http://choosealicense.com/licenses/mit/)

# Plugin development <!-- omit in toc -->

- [MQTT](#mqtt)
- [MQTT Channels](#mqtt-channels)
  - [Notifications: \<HOSTNAME\>](#notifications-hostname)
  - [Plugin base URI](#plugin-base-uri)
  - [Channel name](#channel-name)
  - [Sending a bitmap](#sending-a-bitmap)
- [Issues, Ideas And Bugs](#issues-ideas-and-bugs)
- [License](#license)
- [Contribution](#contribution)

# MQTT
Pixelix is a MQTT client which can be connected to a MQTT broker. The MQTT broker URL is configued in the settings via webinterface.\

# MQTT Channels

## Notifications: &lt;HOSTNAME&gt;
After the connection to the MQTT broker is established, Pixelix will send "Obi Wan Kenobi" over the &lt;HOSTNAME&gt; channel.

## Plugin base URI
The base URI to access plugin related channels can be setup with the plugin UID or the plugin alias:
* &lt;HOSTNAME&gt;/uid/&lt;PLUGIN-UID&gt;/...
* &lt;HOSTNAME&gt;/alias/&lt;PLUGIN-ALIAS&gt;/...

## Channel name
The complete channel name can be derived from the REST API documentation.

Example: JustTextPlugin

The REST API URL looks like the following: http://&lt;HOSTNAME&gt;/rest/api/v1/display/uid/&lt;PLUGIN-UID&gt;/text?show=&lt;TEXT&gt;
1. Replace the http://&lt;HOSTNAME&gt;/rest/api/v1 part with &lt;HOSTNAME&gt; --> &lt;HOSTNAME&gt;/display/uid/&lt;PLUGIN-UID&gt;/text?show=&lt;TEXT&gt;
2. Every URL parameter, which is in this case show=&lt;TEXT&gt; must be sent in JSON format.

```json
{
    "show": "<text>"
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

# Issues, Ideas And Bugs
If you have further ideas or you found some bugs, great! Create a [issue](https://github.com/BlueAndi/esp-rgb-led-matrix/issues) or if you are able and willing to fix it by yourself, clone the repository and create a pull request.

# License
The whole source code is published under the [MIT license](http://choosealicense.com/licenses/mit/).
Consider the different licenses of the used third party libraries too!

# Contribution
Unless you explicitly state otherwise, any contribution intentionally submitted for inclusion in the work by you, shall be licensed as above, without any
additional terms or conditions.
