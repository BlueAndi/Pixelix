# PIXELIX
![PIXELIX](./images/LogoBlack.png)

# Toolchain Installation

## IDE
The [PlatformIO IDE](https://platformio.org/platformio-ide) is used for the development. Its build on top of Microsoft Visual Studio Code.

## Installation
1. Install [VSCode](https://code.visualstudio.com/).
2. Install PlatformIO IDE according to this [HowTo](https://platformio.org/install/ide?install=vscode).
3. Close and start VSCode again.
4. Recommended is to take a look to the [quick-start guide](https://docs.platformio.org/en/latest/ide/vscode.html#quick-start).

## Installation of test environment
1. For the test environment on windows platform, install [MinGW](http://www.mingw.org/).
    * Install as basic setup:
        * mingw-developer-toolkit-bin
        * mingw32-base-bin
        * mingw32-gcc-g++-bin
        * msys-base-bin
    * Install additional packages:
        * mingw32-libmingwex-dev
        * mingw32-libmingwex-dll
2. Place ```c:\mingw\bin``` on path.
