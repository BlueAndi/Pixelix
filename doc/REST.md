# esp-rgb-led-matrix
Full RGB LED matrix, based on an ESP32 and WS2812B LEDs.

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](http://choosealicense.com/licenses/mit/)

# REST API

The base URI (`<base-uri>`) for the REST API access is always ```http://<host>/rest/api/<version>```.

## Version

The current API version is "v1".

## In general

Every response will provide three items:
* result: The result as string.
  * "ok": The request was successful executed.
  * "error": The request failed.
* resultCode: The result as integer number.
  * 0: The request was successful executed.
  * 1: The request failed.
* data: If request was successful, the data object will contain the requested data otherwise null.
* error: If request failed, the error object contain some more information about the error itself.

```json
{
    "result": "ok",
    "resultCode": 0,
    "data": {
        "software": {
            "version": "v0.1.0"
        }
    }
}
```

## Endpoint `<base-uri>`/status
Get status information:
* Software version.
* Wifi SSID of the network, the display will connect to.

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
    "result": "ok",
    "resultCode": 0,
    "data": {
        "software": {
            "version": "v0.1.0"
        },
        "wifi": {
            "ssid": "Number5Lives"
        }
    }
}
```

## Endpoint `<base-uri>`/display/slots
Get the number of supported slots.

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
    "result": "ok",
    "resultCode": 0,
    "data": {
        "slots": 4
    }
}
```

## Endpoint `<base-uri>`/display/slot/`<slot-id>`/text
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
    "result": "ok",
    "resultCode": 0,
    "data": null
}
```

## Endpoint `<base-uri>`/display/slot/`<slot-id>`/bitmap
Show bitmap in the specified slot. The bitmap must be BASE64 encoded.
At the momet only bitmaps in the size of 8 x 8 pixel are supported.

Detail:
* Method: POST
* Arguments:
  * width=`<bitmap-width>`
  * height=`<bitmap-height>`
  * data=`<base64-encoded-bitmap>`

Example:
```
POST <base-uri>/rest/v1/display/slot/0/bitmap?width=8&height=8&data=XXXXXX
```

Result:
```json
{
    "result": "ok",
    "resultCode": 0,
    "data": null
}
```

## Endpoint `<base-uri>`/display/slot/`<slot-id>`/lamp/`<lamp-id>`/state
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
POST <base-uri>/rest/v1/display/slot/0/lamp/0/state?set=on
```

Result:
```json
{
    "result": "ok",
    "resultCode": 0,
    "data": null
}
```

# Issues, Ideas And Bugs
If you have further ideas or you found some bugs, great! Create a [issue](https://github.com/BlueAndi/esp-rgb-led-matrix/issues) or if you are able and willing to fix it by yourself, clone the repository and create a pull request.

# License
The whole source code is published under the [MIT license](http://choosealicense.com/licenses/mit/).
