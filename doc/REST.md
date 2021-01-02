# PIXELIX
![PIXELIX](./images/LogoBlack.png)

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](http://choosealicense.com/licenses/mit/)

- [PIXELIX](#pixelix)
- [REST API](#rest-api)
  - [Version](#version)
  - [In general](#in-general)
  - [Common](#common)
    - [Endpoint `<base-uri>`/status](#endpoint-base-uristatus)
    - [Endpoint `<base-uri>`/display/slots](#endpoint-base-uridisplayslots)
    - [Endpoint `<base-uri>`/plugin](#endpoint-base-uriplugin)
    - [Endpoint `<base-uri>`/button](#endpoint-base-uributton)
  - [Plugin depended](#plugin-depended)
    - [Endpoint `<base-uri>`/display/uid/`<plugin-uid>`/text](#endpoint-base-uridisplayuidplugin-uidtext)
    - [Endpoint `<base-uri>`/display/uid/`<plugin-uid>`/bitmap](#endpoint-base-uridisplayuidplugin-uidbitmap)
    - [Endpoint `<base-uri>`/display/uid/`<plugin-uid>`/lamps](#endpoint-base-uridisplayuidplugin-uidlamps)
    - [Endpoint `<base-uri>`/display/uid/`<plugin-uid>`/lamp/`<lamp-id>`](#endpoint-base-uridisplayuidplugin-uidlamplamp-id)
    - [Endpoint `<base-uri>`/display/uid/`<plugin-uid>`/countdown](#endpoint-base-uridisplayuidplugin-uidcountdown)
    - [Endpoint `<base-uri>`/display/uid/`<plugin-uid>`/location](#endpoint-base-uridisplayuidplugin-uidlocation)
    - [Endpoint `<base-uri>`/display/uid/`<plugin-uid>`/ipAddress](#endpoint-base-uridisplayuidplugin-uidipaddress)
    - [Endpoint `<base-uri>`/display/uid/`<plugin-uid>`/host](#endpoint-base-uridisplayuidplugin-uidhost)
- [Issues, Ideas And Bugs](#issues-ideas-and-bugs)
- [License](#license)

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
GET <base-uri>/rest/api/v1/status
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
            "revision": "ebb5a05faba9af13ae614dab682866c0cbf12fc9",
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
```bash
$ curl -u luke:skywalker -X GET http://192.168.2.166/rest/api/v1/status
```

### Endpoint `<base-uri>`/display/slots
Get the following informations:
* Max. number of slots.
* Per slot:
  * The current installed plugin.
  * Whether the slot is locked or not.
  * Slot duration in ms.
The plugins are listed in the ascending order of the slots.

Detail:
* Method: GET
  * Arguments: N/A

Example:
```
GET <base-uri>/rest/api/v1/display/slots
```

Result:
```json
{
  "data": {
    "slots": [
      {
        "name": "SysMsgPlugin",
        "uid": 372,
        "isLocked": true,
        "duration": 21000
      },
      {
        "name": "IconTextLampPlugin",
        "uid": 22076,
        "isLocked": false,
        "duration": 30000
      },
      {
        "name": "FirePlugin",
        "uid": 28133,
        "isLocked": false,
        "duration": 30000
      },
      {
        "name": "DateTimePlugin",
        "uid": 58620,
        "isLocked": false,
        "duration": 30000
      },
      {
        "name": "",
        "uid": 0,
        "isLocked": false,
        "duration": 30000
      }
    ],
    "maxSlots": 5
  },
  "status": 0
}
```

Example with curl:
```bash
$ curl -u luke:skywalker -X GET http://192.168.2.166/rest/api/v1/display/slots
```

### Endpoint `<base-uri>`/plugin
Install/Uninstall plugins to display slots.

Detail:
* Method: GET
  * List all plugins:
    * Arguments:
      * list
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
POST <base-uri>/rest/api/v1/display?install=JustTextPlugin
```

Result:
```json
{
    "status": 0,
    "data": {
        "slotId": 1,
        "uid": 27065
    }
}
```

Example with curl:
```bash
$ curl -u luke:skywalker -d "list=" -X GET http://192.168.2.166/rest/api/v1/plugin
$ curl -u luke:skywalker -d "install=JustTextPlugin" -X POST http://192.168.2.166/rest/api/v1/plugin
$ curl -u luke:skywalker -d "uninstall=JustTextPlugin&slotId=1" -X POST http://192.168.2.166/rest/api/v1/plugin
```

### Endpoint `<base-uri>`/button
Trigger the virtual user button.

Detail:
* Method: GET
  * Arguments: N/A

Example:
```
GET <base-uri>/rest/api/v1/button
```

Result:
```json
{
    "status": 0,
    "data": {
        }
    }
}
```

Example with curl:
```bash
$ curl -u luke:skywalker -X GET http://192.168.2.166/rest/api/v1/button
```

## Plugin depended
The plugin depended API.

### Endpoint `<base-uri>`/display/uid/`<plugin-uid>`/text
Get/Set text in the specified slot.

Detail:
* Method: GET
  * Get text.
    * Arguments:
      * N/A
* Method: POST
  * Set text.
    * Arguments:
      * show=`<text>`

Example:
```
GET <base-uri>/rest/api/v1/display/uid/0/text
```

Result:
```json
{
    "status": 0,
    "data": {
        "text": "Hello World!"
    }
}
```

```
POST <base-uri>/rest/api/v1/display/uid/0/text?show=Test
```

Result:
```json
{
    "status": 0,
    "data": null
}
```

Example with curl:
```bash
$ curl -u luke:skywalker -X GET http://192.168.2.166/rest/api/v1/display/uid/0/text
$ curl -u luke:skywalker -d "show=Hi" -X POST http://192.168.2.166/rest/api/v1/display/uid/0/text
```

### Endpoint `<base-uri>`/display/uid/`<plugin-uid>`/bitmap
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
POST <base-uri>/rest/api/v1/display/uid/0/bitmap
```

Result:
```json
{
    "status": 0,
    "data": null
}
```

```bash
$ curl -H "Content-Type: multipart/form-data" -F "data=@test.bmp" http://192.168.2.166/rest/api/v1/display/uid/0/bitmap
```

### Endpoint `<base-uri>`/display/uid/`<plugin-uid>`/lamps
Get the state of all lamps in the specified slot.

Detail:
* Method: GET
* Arguments:
  * N/A

Example:
```
GET <base-uri>/rest/api/v1/display/uid/0/lamps
```

Result:
```json
{
    "status": 0,
    "data": {
        "lamps": [{
            "id": 0,
            "state": "off"
        }, {
            "id": 1,
            "state": "on"
        }]
    }
}
```

Example with curl:
```bash
$ curl -u luke:skywalker -X GET http://192.168.2.166/rest/api/v1/display/uid/0/lamps
```

### Endpoint `<base-uri>`/display/uid/`<plugin-uid>`/lamp/`<lamp-id>`
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
POST <base-uri>/rest/api/v1/display/uid/0/lamp/0?set=on
```

Result:
```json
{
    "status": 0,
    "data": null
}
```

Example with curl:
```bash
$ curl -u luke:skywalker -d "set=on" -X POST http://192.168.2.166/rest/api/v1/display/uid/0/lamp/0
```

### Endpoint `<base-uri>`/display/uid/`<plugin-uid>`/countdown
Get/Set the target date and target day description of the CountdownPlugin plugin.

Detail:
* Method: GET
  * Get target date and target day description.
    * Arguments:
      * N/A
* Method: POST
  * Set target date and target day description. Note, every parameter is optional.
  * Arguments:
    * day=`<day>`
    * month=`<month>`
    * year=`<year>`
    * plural=`<unit-plural>`
    * singular=`<unit-singular>`

Examples:

```
GET <base-uri>/rest/api/v1/display/uid/0/countdown
```

Result:
```json
{
    "status": 0,
    "data": {
        "day": 2,
        "month": 9,
        "year": 2020,
        "plural": "DAYS",
        "singular": "DAY"
    }
}
```

```
POST <base-uri>/rest/api/v1/display/uid/0/countdown?day=29;month=8;year=2019;plural=DAYS;singular=DAY
```

Result:
```json
{
    "status": 0,
    "data": null
}
```

Example with curl:
```bash
$ curl -u luke:skywalker -X GET http://192.168.2.166/rest/api/v1/display/uid/0/countdown
$ curl -u luke:skywalker -d "day=29" -d "month=8" -d "year=2019" -d "plural=DAYS" -d "singular=DAY" -X POST http://192.168.2.166/rest/api/v1/display/uid/0/countdown
```

### Endpoint `<base-uri>`/display/uid/`<plugin-uid>`/location
Get/Set the location of the SunrisePlugin plugin.

Detail:
* Method: GET
  * Get the location of the SunrisePlugin plugin.
    * Arguments:
      * N/A
* Method: POST
  * Set the location of the SunrisePlugin plugin.
    * Arguments:
      * longitude=`<longitude>`
      * latitude=`<latitude>`

Example:
```
GET <base-uri>/rest/api/v1/display/uid/0/location
```

Result:
```json
{
    "status": 0,
    "data": {
        "longitude": "2.295",
        "latitude": "48.858"
    }
}
```

```
POST <base-uri>/rest/api/v1/display/uid/0/location?longitude=2.295;latitude=48.858
```

Result:
```json
{
    "status": 0,
    "data": null
}
```

Example with curl:
```bash
$ curl -u luke:skywalker -X GET http://192.168.2.166/rest/api/v1/display/uid/0/location
$ curl -u luke:skywalker -d "longitude=2.295" -d "latitude=48.858" -X POST http://192.168.2.166/rest/api/v1/display/uid/0/location
```

### Endpoint `<base-uri>`/display/uid/`<plugin-uid>`/ipAddress
Get/Set the ip-address of the GruenbeckPlugin plugin.

Detail:
* Method: GET
  * Get the ip-address.
    * Arguments:
      * N/A
* Method: POST
  * Set the ip-address.
    * Arguments:
      * set=`<ip-address>`

Example:
```
GET <base-uri>/rest/api/v1/display/uid/0/ipAddress
```

Result:
```json
{
    "status": 0,
    "data": {
        "ipAddress": "192.168.178.12"
    }
}
```

```
POST <base-uri>/rest/api/v1/display/uid/0/ipAddress?set=192.168.178.12
```

Result:
```json
{
    "status": 0,
    "data": null
}
```

Example with curl:
```bash
$ curl -u luke:skywalker -X GET http://192.168.2.166/rest/api/v1/display/uid/0/ipAddress
$ curl -u luke:skywalker -d "set=192.168.178" -X POST http://192.168.2.166/rest/api/v1/display/uid/0/ipAddress
```

### Endpoint `<base-uri>`/display/uid/`<plugin-uid>`/host
Get/Set the host address of the VolumioPlugin plugin.

Detail:
* Method: GET
  * Get the host address.
    * Arguments:
      * N/A
* Method: POST
  * Set the host address.
    * Arguments:
      * set=`<host-address>`

Example:
```
GET <base-uri>/rest/api/v1/display/uid/0/host
```

Result:
```json
{
    "status": 0,
    "data": {
        "host": "volumio.fritz.box"
    }
}
```

```
POST <base-uri>/rest/api/v1/display/uid/0/host?set=volumio.fritz.box
```

Result:
```json
{
    "status": 0,
    "data": null
}
```

Example with curl:
```bash
$ curl -u luke:skywalker -X GET http://192.168.2.166/rest/api/v1/display/uid/0/host
$ curl -u luke:skywalker -d "set=volumio.fritz.box" -X POST http://192.168.2.166/rest/api/v1/display/uid/0/host
```

# Issues, Ideas And Bugs
If you have further ideas or you found some bugs, great! Create a [issue](https://github.com/BlueAndi/esp-rgb-led-matrix/issues) or if you are able and willing to fix it by yourself, clone the repository and create a pull request.

# License
The whole source code is published under the [MIT license](http://choosealicense.com/licenses/mit/).
