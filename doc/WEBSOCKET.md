# Pixelix
Full RGB LED matrix, based on an ESP32 and WS2812B LEDs.

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](http://choosealicense.com/licenses/mit/)

- [Pixelix](#pixelix)
- [Websocket API](#websocket-api)
  - [Get display pixel colors](#get-display-pixel-colors)
  - [Get slots information](#get-slots-information)
  - [Reset](#reset)
  - [Brightness](#brightness)
    - [Get brightness information](#get-brightness-information)
    - [Set brightness](#set-brightness)
    - [Set brightness and enable/eisable automatic brightness adjustment](#set-brightness-and-enableeisable-automatic-brightness-adjustment)
    - [Response](#response)
  - [Get plugins information](#get-plugins-information)
  - [Install a plugin](#install-a-plugin)
  - [Uninstall a plugin](#uninstall-a-plugin)
  - [Enable/Disable logging](#enabledisable-logging)
    - [Is logging enabled?](#is-logging-enabled)
    - [Enable/Disable logging to websocket](#enabledisable-logging-to-websocket)
- [Issues, Ideas And Bugs](#issues-ideas-and-bugs)
- [License](#license)

# Websocket API

## Get display pixel colors
Command: ```GETDISP```

Parameter:
* N/A

Response:
* Successful:
  * ```ACK;<slot-id>;<color>;<color>;...;<color>```
  * ```<slot-id>```: Id of current active slot.
  * ```<color>```: Colors starting with the row y = 0 and from x = 0 to N. Then the next row and etc.
* Failed:
  * ```NACK```

## Get slots information
Command: ```SLOTS```

Parameter:
* N/A

Response:
* Successful:
  * ```ACK;<max-slots>;<plugin-type>;<plugin-uid>...```
  * ```<max-slots>```: Max. number of slots.
  * ```<plugin-type>```: The name of the installed plugin in ```"..."```.
  * ```<plugin-uid>```: The plugin UID.
  * The plugin type and UID will be repeated for all slots. If a slot is empty, nothing will be in.
* Failed:
  * ```NACK```

## Reset
Command: ```RESET```

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

### Set brightness and enable/eisable automatic brightness adjustment
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

## Get plugins information
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
Command: ```INSTALL;<plugin-name>```

Parameter:
* ```<plugin-name>```: Name of the plugin to install.

Response:
* Successful:
  * ```ACK;<slot-id>```
  * ```<slot-id>```: The id of the slot, where the plugin was installed.
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
* ```EVT;<timestamp>;<level>;<filename>;<line>;<text>```
* ```<timestamp>```: Timestamp in ms
* ```<level>```: Log level
* ```<filename>```: Filename where the log message comes from, emphasized in "".
* ```<line>```: Line number if in the file where the log message comes from.
* ```<text>```: Logged text, emphasized in "".

# Issues, Ideas And Bugs
If you have further ideas or you found some bugs, great! Create a [issue](https://github.com/BlueAndi/esp-rgb-led-matrix/issues) or if you are able and willing to fix it by yourself, clone the repository and create a pull request.

# License
The whole source code is published under the [MIT license](http://choosealicense.com/licenses/mit/).
