# PIXELIX
![PIXELIX](./images/LogoBlack.png)

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](http://choosealicense.com/licenses/mit/)

- [PIXELIX](#pixelix)
- [Software Build](#software-build)
  - [Build Project](#build-project)
  - [Run Tests](#run-tests)
 
# Software Build

## Build Project
Build the software for the target with:

1. Load workspace in VSCode.
2. Change to PlatformIO toolbar (click on the head of the ant in the left column).
3. _Project Tasks -> Default -> Build All_ or via hotkey ctrl-alt-b

Note, the default project is esp32 DevKit v1 board, programming via USB.

## Run Tests
The tests are running on the native system. Run them manually with:

1. Running tests with _Project Tasks -> env:test -> Advanced -> Test_

Note, the CI runs them for every git push.
