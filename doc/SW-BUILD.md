# PIXELIX
![PIXELIX](./images/LogoBlack.png)

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](http://choosealicense.com/licenses/mit/)

- [PIXELIX](#pixelix)
- [Build The Software](#build-the-software)
  - [Retrieve The Project](#retrieve-the-project)
  - [Build Project](#build-project)
- [Next Step](#next-step)
 
# Build The Software

## Retrieve The Project

Open a console (windows: git bash) and get the project via
```
$ git clone https://github.com/BlueAndi/esp-rgb-led-matrix
```

Example:

![GitCloneOnConsole](./images/GitCloneOnConsole.png)

:warning: If you download the project from github, because you skipped the git installation, the build may fail because it can happen that other necessary libraries can not be automatically cloned by PlatformIO.

## Build Project
Now its time to build the software for the appropriate target.

1. Load workspace in VSCode.
2. Change to PlatformIO toolbar (click on the head of the ant in the left column).

![VSCodePIOIcon](./images/VSCodePIOIcon.png)

3. Lets now build the software independed of your specific board, just to see whether it works by _Project Tasks -> Default -> Build All_ or via hotkey ctrl-alt-b

![VSCodePIODefault](./images/VSCodePIODefault.png)

4. If no error happened, you are ready.

![VSCodePIODefaultBuild](./images/VSCodePIODefaultBuild.png)

Note, the default project is esp32 DevKit v1 board, programming via USB.

# Next Step
The toolchain is working and the firmware gets built. [Next step](SW-UPDATE.md) is now to build it specific for your target and upload it.
