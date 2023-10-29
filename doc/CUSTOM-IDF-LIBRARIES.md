# PIXELIX <!-- omit in toc -->
![PIXELIX](./images/LogoBlack.png)

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](http://choosealicense.com/licenses/mit/)

# Building ESP-IDF Libraries for Arduino using custom sdkconfig options

* [Building ESP-IDF Libraries for Arduino using custom sdkconfig options](#building-esp-idf-libraries-for-arduino-using-custom-sdkconfig-options)
* [Purpose](#purpose)
* [Instructions](#instructions)
* [Issues, Ideas And Bugs](#issues-ideas-and-bugs)
* [License](#license)
* [Contribution](#contribution)

# Purpose

This tutorial explains how to re-compile the ESP-IDF Libraries using custom sdkconfig settings that are no usually available for the Arduino Core.
This has been done in a WSL (Ubuntu) environment to simplify any installation process.

# Instructions

1. Install all dependencies:

``` bash
sudo apt-get install git wget curl libssl-dev libncurses-dev flex bison gperf python python-pip python-setuptools python-serial python-click python-cryptography python-future python-pyparsing python-pyelftools cmake ninja-build ccache jq
```

2. Clone the [Espressif Library Builder](https://github.com/espressif/esp32-arduino-lib-builder) Github project.

``` bash
# Clone Repository
git clone https://github.com/espressif/esp32-arduino-lib-builder

# Change Directory
cd esp32-arduino-lib-builder
```

3. Add the desired configuration lines to the different configuration possibilities inside the `./configs` folder.

In order to activate the tracing facility of FreeRTOS, add the following lines at the end of `./configs/defconfig.common`:

```
CONFIG_FREERTOS_USE_TRACE_FACILITY=y
CONFIG_FREERTOS_USE_STATS_FORMATTING_FUNCTIONS=y
CONFIG_FREERTOS_VTASKLIST_INCLUDE_COREID=y
```

Note:
As this projects includes its own partition tables, it is necessary to **remove** those options that do not use the default partition tables from `./configs/defconfig.common`:

```
CONFIG_ESP_COREDUMP_ENABLE=y
CONFIG_ESP_COREDUMP_ENABLE_TO_FLASH=y
CONFIG_ESP_COREDUMP_DATA_FORMAT_ELF=y
CONFIG_ESP_COREDUMP_CHECKSUM_CRC32=y
CONFIG_ESP_COREDUMP_MAX_TASKS_NUM=64
CONFIG_ESP_COREDUMP_STACK_SIZE=1024
CONFIG_MBEDTLS_DYNAMIC_BUFFER=y
CONFIG_MBEDTLS_DYNAMIC_FREE_PEER_CERT=y
CONFIG_MBEDTLS_DYNAMIC_FREE_CONFIG_DATA=y
```

4. Run the build script.

``` bash
./build.sh
```

If only the `esp32` libraries are required, the compilation can be reduced by using:

``` bash
./build.sh -t esp32
```

5. Replace the libraries in the PlatformIO package folder `.../.platformio/packages/framework-arduinoespressif32/tools/sdk/esp32` with the `./out/tools/sdk/esp32` output folder.

# Issues, Ideas And Bugs
If you have further ideas or you found some bugs, great! Create a [issue](https://github.com/BlueAndi/esp-rgb-led-matrix/issues) or if you are able and willing to fix it by yourself, clone the repository and create a pull request.

# License
The whole source code is published under the [MIT license](http://choosealicense.com/licenses/mit/).
Consider the different licenses of the used third party libraries too!

# Contribution
Unless you explicitly state otherwise, any contribution intentionally submitted for inclusion in the work by you, shall be licensed as above, without any
additional terms or conditions.