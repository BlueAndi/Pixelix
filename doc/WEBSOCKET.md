# PIXELIX <!-- omit in toc -->

![PIXELIX](./images/LogoBlack.png)

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](http://choosealicense.com/licenses/mit/)

## Websocket API <!-- omit in toc -->

- [Get display pixel colors](#get-display-pixel-colors)
- [Get slots information](#get-slots-information)
- [Restart](#restart)
- [Brightness](#brightness)
  - [Get brightness information](#get-brightness-information)
  - [Set brightness](#set-brightness)
  - [Set brightness and enable/enable automatic brightness adjustment](#set-brightness-and-enableenable-automatic-brightness-adjustment)
  - [Response](#response)
  - [Get plugins information](#get-plugins-information)
- [Install a plugin](#install-a-plugin)
- [Uninstall a plugin](#uninstall-a-plugin)
- [Get/Set the plugin alias](#getset-the-plugin-alias)
- [Move a plugin](#move-a-plugin)
- [Enable/Disable logging](#enabledisable-logging)
  - [Is logging enabled?](#is-logging-enabled)
  - [Enable/Disable logging to websocket](#enabledisable-logging-to-websocket)
- [Trigger virtual user button](#trigger-virtual-user-button)
- [Switch to next fade effect](#switch-to-next-fade-effect)
- [Issues, Ideas And Bugs](#issues-ideas-and-bugs)
- [License](#license)
- [Contribution](#contribution)

## Get display pixel colors

Command: ```GETDISP```

Parameter:

* N/A

Response:

* Successful:
  * ```ACK;<slot-id>;<color>;<color>;...;<color>```
  * ```<slot-id>```: Id of current active slot.
  * ```<color>```: Color as 32 bit hex value, starting with the row y = 0 and from x = 0 to N. Then the next row and etc.
* Failed:
  * ```NACK```

## Get slots information

Command: ```SLOTS```

Parameter:

* N/A

Response:

* Successful:
  * ```ACK;<max-slots>;<plugin-type>;<plugin-uid>;<slot-locked>;<slot-duration>...```
  * ```<max-slots>```: Max. number of slots.
  * ```<plugin-type>```: The name of the installed plugin in ```"..."```.
  * ```<plugin-uid>```: The plugin UID.
  * ```<plugin-alias>```: The plugin alias name.
  * ```<slot-locked>```: Whether slot is locked (1) or not (0).
  * ```<slot-duration>```: Slot duration in ms.
  * The plugin type, plugin UID and whether slot is locked or not will be repeated for all slots. If a slot is empty, nothing will be in.
* Failed:
  * ```NACK```

## Restart

Command: ```RESTART```

Parameter:

* N/A

Response:

* Successful:
    * ```ACK```
* Failed:
    * ```NACK```

## Brightness

### Get brightness information

Command: ```BRIGHTNESS```

Parameter:

* N/A

### Set brightness

Command: ```BRIGHTNESS;<brightness>```

Parameter:

* ```<brightness>```: Brightness in percent [0; 100].

### Set brightness and enable/enable automatic brightness adjustment

Command: ```BRIGHTNESS;<brightness>;<automatic-brightness-control>```

Parameter:

* ```<brightness>```: Brightness in percent [0; 100].
* ```<automatic-brightness-control>```: Enable (```1```) or disable (```0```) it.

### Response

Response:

* Successful:
    * ```ACK;<brightness>;<auto-brightness-adjustment>```
* Failed:
    * ```NACK```

### Get plugins information

Command: ```PLUGINS```

Parameter:

* N/A

Response:

* Successful:
  * ```ACK;<plugin-info>; ...```
  * ```<plugin-info>```: The name of the plugin in ```"..."```. This will be repeated for all plugins.
* Failed:
  * ```NACK```

## Install a plugin

Command: ```INSTALL;<plugin-name>;<plugin-alias>```

Parameter:

* ```<plugin-name>```: Name of the plugin to install.
* ```<plugin-alias>```: Optional. The alias to assign. If omitted, invalid or already in use, a default is generated from the plugin type name.

Response:

* Successful:
  * ```ACK;<slot-id>;<plugin-uid>;<plugin-alias>```
  * ```<slot-id>```: The id of the slot, where the plugin was installed.
  * ```<plugin-uid>```: The plugin UID.
  * ```<plugin-alias>```: The alias the plugin was installed with, which may differ from the requested one.
* Failed:
  * ```NACK```

## Uninstall a plugin

Command: ```UNINSTALL;<slot-id>```

Parameter:

* ```<slot-id>```: Id of the slot, where the plugin shall be uninstalled.

Response:

* Successful:
  * ```ACK```
* Failed:
  * ```NACK```

## Get/Set the plugin alias

Command: ```ALIAS;<plugin-uid>;<plugin-alias>```

Parameter:

* ```<plugin-uid>```: UID of the plugin, which alias shall be read or changed.
* ```<plugin-alias>```: Optional. If given, the alias is changed. If omitted, the current alias is only read.

An alias is 3 to 20 characters long and may contain ```A-Z```, ```a-z```, ```0-9```, ```_``` and ```-```. It must be unique per device, because it addresses the plugin over REST and MQTT.

Response:

* Successful:
  * ```ACK;"<plugin-alias>"```
  * ```<plugin-alias>```: The current alias.
* Failed:
  * ```NACK;"Alias invalid."```: The alias violates the length or the character set.
  * ```NACK;"Alias already in use."```: Another plugin instance uses this alias.
  * ```NACK;"Parameter invalid."```: The parameters could not be parsed.

## Move a plugin

Command: ```MOVE;<plugin-uid>;<slot-id>```

Parameter:

* ```<plugin-uid>```: UID of the plugin, which to move.
* ```<slot-id>```: Id of the slot, where to move the plugin.

Response:

* Successful:
  * ```ACK```
* Failed:
  * ```NACK```

## Enable/Disable logging

### Is logging enabled?

Command: ```LOG```

Response:

* Successful:
  * ```ACK;<is-enabled>```
  * ```<is-enabled>```: 0 means disabled and 1 enabled
* Failed:
  * ```NACK```

### Enable/Disable logging to websocket

Command: ```LOG;<enable>```

Parameter:

* ```<enable>```: 0 to disable or 1 to enable

Response:

* Successful:
  * ```ACK;<is-enabled>```
  * ```<is-enabled>```: 0 means disabled and 1 enabled
* Failed:
  * ```NACK```

Event: If logging is enabled, a event will be automatically be sent for every log message.

* ```EVT;LOG;<timestamp>;<level>;<filename>;<line>;<text>```
* ```<timestamp>```: Timestamp in ms
* ```<level>```: Log level
* ```<filename>```: Filename where the log message comes from, emphasized in "".
* ```<line>```: Line number if in the file where the log message comes from.
* ```<text>```: Logged text, emphasized in "".

## Trigger virtual user button

Command: ```BUTTON```

Parameter:

* N/A

Response:

* Successful:
    * ```ACK```
* Failed:
    * ```NACK```

## Switch to next fade effect

Command: ```EFFECT```

Parameter:

* N/A

Response:

* Successful:
    * ```ACK;<fadeEffect>```
    * ```<fadeEffect>```: ID of the fade effect
* Failed:
    * ```NACK```

## Issues, Ideas And Bugs

If you have ideas or found a bug, create an [issue](https://github.com/BlueAndi/Pixelix/issues). If you want to fix it yourself, clone the repository and open a pull request.

## License

The whole source code is published under the [MIT license](http://choosealicense.com/licenses/mit/).
Consider the different licenses of the used third party libraries too!

## Contribution

Unless you explicitly state otherwise, any contribution intentionally submitted for inclusion in the work by you, shall be licensed as above, without any
additional terms or conditions.
