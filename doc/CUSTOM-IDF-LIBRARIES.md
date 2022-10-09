# PIXELIX <!-- omit in toc -->
![PIXELIX](./images/LogoBlack.png)

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](http://choosealicense.com/licenses/mit/)

# Building ESP-IDF Libraries for Arduino using custom sdkconfig options

- [Building ESP-IDF Libraries](#Building-ESP-ID-Libraries)
- [Purpose](#purpose)
- [Instructions](#instructions)
- [Issues, Ideas And Bugs](#issues-ideas-and-bugs)
- [License](#license)
- [Contribution](#contribution)

# Purpose

This tutorial explains how to re-compile the ESP-IDF Libraries using custom sdkconfig settings that are no usually available for the Arduino Core.
This has been done in a Linux (Ubuntu) environment to simplify any installation process.

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

3. Run the build script.

``` bash
./build.sh -t esp32 -b idf_libs qio 80m
```

The used flags specify:
- the target chip (`-t esp32`): Required when not building the complete system but just the libraries. Accepts other Espressif Chips such as the `esp32s2` or `esp32s3`, depending on your needs.

- the build mode (`-b idf_libs`): Builds just the IDF libraries and not the complete ESP32 core.

- the SPI Mode (`-b qio`): As set in the Arduino Core sdkconfig.

- the Frequency (`-b 80m`): As set in the Arduino Core sdkconfig.

4. Once finished, compare the generated sdkconfig (./out/tools/sdk/esp32/sdkconfig) to the one used by the [Arduino Core](https://github.com/espressif/arduino-esp32/blob/master/tools/sdk/esp32/sdkconfig). The best way to do this using VSCode is opening both files and then use the command `File: Compare Active File With...` from the Command Palette. This process will highlight the differences between both files. This is important so we generate the same sdkconfig as the Arduino Core.

5. Create the file `./configs/defconfig.compat`. (compat stands for Compatibility). In here you can set the sdkconfig options that were different in the generated file. For example, these options were set in the generated sdkconfig but unset in the Arduino Core, so these lines force the parameters to remain unset:

```
#CONFIG_ESP_SLEEP_PSRAM_LEAKAGE_WORKAROUND is not set
#CONFIG_ESP_SLEEP_FLASH_LEAKAGE_WORKAROUND is not set
```

6. Create a configuration file for the options you want to change. For this example, we will call it "trace" as it activates the Trace Facility of FreeRTOS. Therefore, the file would be `./configs/defconfig.compat`, and contain the following lines:
```
CONFIG_FREERTOS_USE_TRACE_FACILITY=y
CONFIG_FREERTOS_USE_STATS_FORMATTING_FUNCTIONS=y
CONFIG_FREERTOS_VTASKLIST_INCLUDE_COREID=y
```

7. Run the build script again, but include the names of the two configuration files that you created.

``` bash
./build.sh -t esp32 -b idf_libs qio 80m compat trace
```

8. When the build process is finished, the libraries will be found in the `./out/tools/sdk/esp32/lib/` folder.

# Issues, Ideas And Bugs
If you have further ideas or you found some bugs, great! Create a [issue](https://github.com/BlueAndi/esp-rgb-led-matrix/issues) or if you are able and willing to fix it by yourself, clone the repository and create a pull request.

# License
The whole source code is published under the [MIT license](http://choosealicense.com/licenses/mit/).
Consider the different licenses of the used third party libraries too!

# Contribution
Unless you explicitly state otherwise, any contribution intentionally submitted for inclusion in the work by you, shall be licensed as above, without any
additional terms or conditions.