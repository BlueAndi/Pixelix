# PIXELIX
![PIXELIX](./images/LogoBlack.png)

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](http://choosealicense.com/licenses/mit/)

- [PIXELIX](#pixelix)
- [Software Build](#software-build)
  - [Download](#download)
    - [Pixelix v3.x and lower](#pixelix-v3x-and-lower)
    - [Pixelix v4.x and higher](#pixelix-v4x-and-higher)
  - [Build Project](#build-project)
  - [Run Tests](#run-tests)
 
# Software Build

## Download

### Pixelix v3.x and lower
Pixelix v3.x and lower need git in the toolchain for the compilation process. Therefore download the sourcecode via git on the console (win: command shell):
```
$ git clone https://github.com/BlueAndi/esp-rgb-led-matrix
```

### Pixelix v4.x and higher
Since Pixelix v4.x and higher it is not necessary anymore to download the sourcecode with git, but of course recommended (see above). You can just download the sources as well via your favorite browser.

## Build Project
Build the software for the target with:

1. Load workspace in VSCode.
2. Change to PlatformIO toolbar (click on the head of the ant in the left column).
3. _Project Tasks -> Default -> Build All_ or via hotkey ctrl-alt-b

Note, the default project is esp32 DevKit v1 board, programming via USB.

## Run Tests
The tests are running on the native system. Note, the test environment must be installed!

Run them manually with:

1. Running tests with _Project Tasks -> env:test -> Advanced -> Test_

Note, the CI runs them for every git push.
