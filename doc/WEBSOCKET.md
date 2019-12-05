# Pixelix
Full RGB LED matrix, based on an ESP32 and WS2812B LEDs.

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](http://choosealicense.com/licenses/mit/)

# Websocket API

## Get display pixel colors
Command: ```GETDISP```
Parameter:
* -

Response:
* Successful: ```ACK;<color>;<color>;...;<color>```
    Colors starting with the row y = 0 and from x = 0 to N. Then the next row and etc.
* Failed: ```NACK```

# Issues, Ideas And Bugs
If you have further ideas or you found some bugs, great! Create a [issue](https://github.com/BlueAndi/esp-rgb-led-matrix/issues) or if you are able and willing to fix it by yourself, clone the repository and create a pull request.

# License
The whole source code is published under the [MIT license](http://choosealicense.com/licenses/mit/).
