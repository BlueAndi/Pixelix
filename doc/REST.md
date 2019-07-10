# esp-rgb-led-matrix
Full RGB LED matrix, based on an ESP32 and WS2812B LEDs.

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](http://choosealicense.com/licenses/mit/)

# REST API

The base URI for the REST API v1 access is always ```http://myAddress.domain/api/v1```.

## Status Codes

Every request returns a status code whether the request was successful or not.

* 0: Request successful.
* 1: Requested object not found.

## General Response Object

### Successful Request

```json
"status": 0,
"data": {

}
```

### Failed Request

```json
"status": 1,
"error": {

}
```

### Get Status
Get status information like wifi SSID and etc.

Endpoint: ```GET /api/v1/status```

Example response:
```json
"status": 0,
"data": {
    "wifi": {
        "ssid": "DEATHSTAR"
    }
}
```

## Basic

### Clear Display
Clear the whole display and move the draw pen to origin.

Endpoint: ```POST /api/v1/display```

Operation: clear

Request:
```
POST /api/v1/display?op=clear
```

Response:
```json
"status": 0,
"data": {
}
```

### Get Fonts
Get supported fonts.

Endpoint: ```GET /api/v1/fonts```

Response:
```json
"status": 0,
"data": {
    "fonts": [{
        "id": 0,
        "name": "MyFont",
        "heigth": 6
    }]
}
```

### Get Current Font
Get current selected font.

Endpoint: ```GET /api/v1/font```

Response:
```json
"status": 0,
"data": {
    "font": {
        "id": 0,
        "name": "MyFont",
        "heigth": 6
    }
}
```

### Select Font
Select a supported font by id.

Endpoint: ```PUT /api/v1/font/{id}```

Response:
```json
"status": 0,
"data": {
}
```

### Show Message
Show text message with the current selected font. If the message width greater than the display width, it will automatically scroll.

Endpoint: ```POST /api/v1/display/message```

Parameter:
| Name | Type | Optional | Default | Description |
| --- | --- | --- | --- | --- |
| msg | string | No | - | Text message. |
| font | number | Yes | Current selected | Set font for the message. |

Response:
```json
"status": 0,
"data": {
}
```

## Advanced

# Issues, Ideas And Bugs
If you have further ideas or you found some bugs, great! Create a [issue](https://github.com/BlueAndi/esp-rgb-led-matrix/issues) or if you are able and willing to fix it by yourself, clone the repository and create a pull request.

# License
The whole source code is published under the [MIT license](http://choosealicense.com/licenses/mit/).
