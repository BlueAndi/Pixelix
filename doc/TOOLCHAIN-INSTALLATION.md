# PIXELIX
![PIXELIX](./images/LogoBlack.png)

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](http://choosealicense.com/licenses/mit/)

# Toolchain Installation

## IDE
The [PlatformIO IDE](https://platformio.org/platformio-ide) is used for the development. Its build on top of Microsoft Visual Studio Code.

## Installation
1. Install [VSCode](https://code.visualstudio.com/).
2. Install PlatformIO IDE according to this [HowTo](https://platformio.org/install/ide?install=vscode).
3. Close and start VSCode again.
4. Recommended is to take a look to the [quick-start guide](https://docs.platformio.org/en/latest/ide/vscode.html#quick-start).

## Version control system
The sourcecode is hosted on github, so of course git is used. :-)

Attention:
* Pixelix version <= 3.x: git installation is necessary for the compilation process.
* Pixelix version >= 4.x: git is not necessary, but recommended.

## Linux (Debian, Ubuntu)

1.
```
$ sudo apt update
```
2.
```
$ sudo apt install git
```

## Windows

If you like to hack on the console (win: command shell), just [git for windows](https://gitforwindows.org) is necessary to install.

If you are new on this, [TortoiseGIT](https://tortoisegit.org/) is recommended.

## Installation of test environment
Note, the test environment is only necessary if you want to make changes in the sourcecode and test it afterwards.

### Linux (Debian, Ubuntu)

1.
```
$ sudo apt update
```
2.
```
$ sudo apt install build-essential
```
3.
```
$ sudo apt-get install manpages-dev
```

### Windows

1. For the test environment on windows platform, install [Mingw-w64](http://mingw-w64.org/) by using the [online installer](https://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win32/Personal%20Builds/mingw-builds/installer) and following installation settings:
    * Version: Choose the newest
    * Architecture: x86_64
    * Threads: posix
    * [Exception](https://wiki.qt.io/MinGW-64-bit#Exception_handling:_SJLJ.2C_DWARF.2C_and_SEH): seh
    * Build revision: 0
2. Add Mingw binary folder (```C:\Program Files\mingw-w64\<depends-on-your-installation-settings>\mingw64\bin```)to path, see environment variables.

