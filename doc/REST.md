# Pixelix
Full RGB LED matrix, based on an ESP32 and WS2812B LEDs.

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](http://choosealicense.com/licenses/mit/)

# REST API

The base URI (`<base-uri>`) for the REST API access is always ```http://<host>/rest/api/<version>```.

## Version

The current API version is "v1".

## In general

Every response will provide two items:
* status: The result as integer number.
  * 0: The request was successful executed.
  * 1: The request failed.
* data: If request was successful, the data object will contain the requested data otherwise null.
* error: If request failed, the error object contain some more information about the error itself.

```json
{
    "status": 0,
    "data": {
        "software": {
            "version": "v0.1.0"
        }
    }
}
```

## Common
The common API, which is always there.

### Endpoint `<base-uri>`/status
Get status information:
* Software version.
* ESP SDK version.
* ESP chip revision.
* ESP cpu frequency in MHz.
* Internal RAM heap size and available heap.
* Wifi SSID of the network, the display will connect to.
* Wifi receive signal strength indicator in dBm.
* Wifi signal quality in percent.

Detail:
* Method: GET
* Arguments: N/A

Example:
```
GET <base-uri>/rest/v1/status
```

Result:
```json
{
    "status": 0,
    "data": {
        "hardware": {
            "chipRev": 1,
            "cpuFreqMhz": 120
        },
        "software": {
            "version": "v0.1.0",
            "espSdkVersion": "xx",
            "internalRam": {
                "heapSize": 496,
                "availableHeap": 412
            }
        },
        "wifi": {
            "ssid": "Number5Lives",
            "rssi": -50,
            "quality": 100
        }
    }
}
```

Example with curl:
```
$ curl -u luke:skywalker -X GET http://192.168.2.166/rest/api/v1/status
```

### Endpoint `<base-uri>`/display/slots
Get the max. number of slots and the current installed plugins.
The plugins are listed in the ascending order of the slots.
If a slot is empty, "empty" will be shown at the corresponding place.

Detail:
* Method: GET
* Arguments: N/A

Example:
```
GET <base-uri>/rest/v1/display/slots
```

Result:
```json
{
    "data": {
        "slot": [
            "SysMsgPlugin",
            "IconTextLampPlugin",
            "empty",
            "empty",
            "empty"
        ],
        "slots": 5
    },
    "status": 0
}
```

Example with curl:
```
$ curl -u luke:skywalker -X GET http://192.168.2.166/rest/api/v1/display/slots
```

### Endpoint `<base-uri>`/plugin
Install/Uninstall plugins to display slots.

Detail:
* Method: POST
* Install a plugin:
  * Arguments:
    * install=`<plugin-name>`
* Uninstall a plugin:
  * Arguments:
    * uninstall=`<plugin-name>`
    * slotId=`<slot-id>`

Example:
```
POST <base-uri>/rest/v1/display?install=JustTextPlugin
```

Result:
```json
{
    "status": 0,
    "data": {
        "slotId": 1
    }
}
```

Example with curl:
```
$ curl -u luke:skywalker -d "install=JustTextPlugin" -X POST http://192.168.2.166/rest/api/v1/display
$ curl -u luke:skywalker -d "uninstall=JustTextPlugin&slotId=1" -X POST http://192.168.2.166/rest/api/v1/display
```

## Plugin depended
The plugin depended API.

### Endpoint `<base-uri>`/display/slot/`<slot-id>`/text
Show text in the specified slot.

Detail:
* Method: POST
* Arguments:
  * show=`<text>`

Example:
```
POST <base-uri>/rest/v1/display/slot/0/text?show=Test
```

Result:
```json
{
    "status": 0,
    "data": null
}
```

Example with curl:
```
$ curl -u luke:skywalker -d "show=Hi" -X POST http://192.168.2.166/rest/api/v1/display/slot/0/text
```

### Endpoint `<base-uri>`/display/slot/`<slot-id>`/bitmap
Show bitmap in the specified slot. Supported are bitmap files (.bmp) with:
* 24 or 32 bits per pixel.
* 1 plane.
* No compression.

Note, if you are using _gimp_ to create bitmap files, please configure like:
* Compatibility options:
  * Don't write color informations.
* Extended options:
  * Select 24 bit per pixel.

Detail:
* Method: POST
* Arguments:
    * -

Example:
```
POST <base-uri>/rest/v1/display/slot/0/bitmap
```

Result:
```json
{
    "status": 0,
    "data": null
}
```

```
$ curl -H "Content-Type: multipart/form-data" -F "data=@test.bmp" http://192.168.2.166/rest/api/v1/display/slot/0/bitmap
```

### Endpoint `<base-uri>`/display/slot/`<slot-id>`/lamp/`<lamp-id>`
Set the state of a lamp in the specified slot.

Detail:
* Method: POST
* Arguments:
  * set=`<lamp-state>`

Supported lamp states:
* "off"
* "on"

Example:
```
POST <base-uri>/rest/v1/display/slot/0/lamp/0?set=on
```

Result:
```json
{
    "status": 0,
    "data": null
}
```

Example with curl:
```
$ curl -u luke:skywalker -d "set=on" -X POST http://192.168.2.166/rest/api/v1/display/slot/0/lamp/0
```

# Issues, Ideas And Bugs
If you have further ideas or you found some bugs, great! Create a [issue](https://github.com/BlueAndi/esp-rgb-led-matrix/issues) or if you are able and willing to fix it by yourself, clone the repository and create a pull request.

# License
The whole source code is published under the [MIT license](http://choosealicense.com/licenses/mit/).
