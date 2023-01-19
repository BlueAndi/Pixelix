# PIXELIX <!-- omit in toc -->
![PIXELIX](./images/LogoBlack.png)

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](http://choosealicense.com/licenses/mit/)

# Plugin development <!-- omit in toc -->

- [MQTT](#mqtt)
- [MQTT Channels](#mqtt-channels)
  - [Notifications: /\<hostname\>](#notifications-hostname)
  - [Plugin base URI](#plugin-base-uri)
- [Issues, Ideas And Bugs](#issues-ideas-and-bugs)
- [License](#license)
- [Contribution](#contribution)

# MQTT
Pixelix is a MQTT client which can be connected to a MQTT broker. The MQTT broker URL is configued in the settings via webinterface.\

# MQTT Channels

## Notifications: /&lt;hostname&gt;
After the connection to the MQTT broker is established, Pixelix will send "Obi Wan Kenobi" over the /&lt;hostname&gt; channel.

## Plugin base URI
The base URI to access plugin related channels can be setup with the plugin UID or the plugin alias:
* "/&lt;hostname&gt;/uid/&lt;plugin-uid&gt;/..."
* "/&lt;hostname&gt;/alias/&lt;plugin-alias&gt;/..."

# Issues, Ideas And Bugs
If you have further ideas or you found some bugs, great! Create a [issue](https://github.com/BlueAndi/esp-rgb-led-matrix/issues) or if you are able and willing to fix it by yourself, clone the repository and create a pull request.

# License
The whole source code is published under the [MIT license](http://choosealicense.com/licenses/mit/).
Consider the different licenses of the used third party libraries too!

# Contribution
Unless you explicitly state otherwise, any contribution intentionally submitted for inclusion in the work by you, shall be licensed as above, without any
additional terms or conditions.
