# PIXELIX <!-- omit in toc -->
![PIXELIX](./images/LogoBlack.png)

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](http://choosealicense.com/licenses/mit/)

# Sensors

- [Sensors](#sensors)
- [Purpose](#purpose)
- [Issues, Ideas And Bugs](#issues-ideas-and-bugs)
- [License](#license)
- [Contribution](#contribution)

# Purpose
Here is a list of supported sensors. Where they are connected, depends on the used [hardware board](BOARDS.md). Some of them need additional external parts e.g. the DHTx need a ext. 4.7kOhm pull-up resistor. Please check the corresponding datasheets.

* Ambient light sensor
  * Photoresistor GL5528
* Temperature and humidity sensors
  * DHT11 (Proprietary one-wire)
  * DHT12 (Proprietary one-wire)
  * DHT21 (Proprietary one-wire)
  * DHT22 (Proprietary one-wire)
  * SHT3x (I2C)
* Digital microphone
  * INMP441

# Issues, Ideas And Bugs
If you have further ideas or you found some bugs, great! Create a [issue](https://github.com/BlueAndi/esp-rgb-led-matrix/issues) or if you are able and willing to fix it by yourself, clone the repository and create a pull request.

# License
The whole source code is published under the [MIT license](http://choosealicense.com/licenses/mit/).
Consider the different licenses of the used third party libraries too!

# Contribution
Unless you explicitly state otherwise, any contribution intentionally submitted for inclusion in the work by you, shall be licensed as above, without any
additional terms or conditions.
