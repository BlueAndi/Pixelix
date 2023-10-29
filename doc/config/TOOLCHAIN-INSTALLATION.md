# PIXELIX <!-- omit in toc -->
![PIXELIX](../images/LogoBlack.png)

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](http://choosealicense.com/licenses/mit/)

# Toolchain Installation <!-- omit in toc -->

* [Purpose](#purpose)
* [Installation Of VSCode](#installation-of-vscode)
* [Installation Of git](#installation-of-git)
  * [Linux (Debian, Ubuntu)](#linux-debian-ubuntu)
  * [Windows](#windows)
* [Installation Of Test Environment](#installation-of-test-environment)
  * [Linux (Debian, Ubuntu)](#linux-debian-ubuntu-1)
  * [Windows](#windows-1)
* [Next Step](#next-step)
* [Issues, Ideas And Bugs](#issues-ideas-and-bugs)
* [License](#license)
* [Contribution](#contribution)

# Purpose

The [PlatformIO IDE](https://platformio.org/platformio-ide) is used for the development. Its a new generation toolset for embedded C/C++ development, integrated as extension in Microsoft Visual Studio Code Editor.

# Installation Of VSCode
1. Install [VSCode](https://code.visualstudio.com/) which depends on your operating system.
2. Install PlatformIO IDE according to this [HowTo](https://platformio.org/install/ide?install=vscode).
3. Close and start VSCode again.
4. Recommended is to take a look to the [quick-start guide](https://docs.platformio.org/en/latest/ide/vscode.html#quick-start).

# Installation Of git

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

If you like to hack on the console/command shell, just [git for windows](https://gitforwindows.org) is necessary to install. Alternative with a graphical user interface install [TortoiseGIT](https://tortoisegit.org/), which includes [git for windows](https://gitforwindows.org).

# Installation Of Test Environment
The test environment is only necessary if you want to modify the sourcecode and test it afterwards.
:raising_hand: If your only interesting in getting PIXELIX running on your target, skip this installation.

## Linux (Debian, Ubuntu)

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

## Windows

1. For the test environment on windows platform, install [Mingw-w64](http://mingw-w64.org/) by using the [online installer](https://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win32/Personal%20Builds/mingw-builds/installer) and following installation settings:
    * Version: Choose the newest
    * Architecture: x86_64
    * Threads: posix
    * [Exception](https://wiki.qt.io/MinGW-64-bit#Exception_handling:_SJLJ.2C_DWARF.2C_and_SEH): seh
    * Build revision: 0
2. Add Mingw binary folder (```C:\Program Files\mingw-w64\<depends-on-your-installation-settings>\mingw64\bin```)to path, see environment variables.

# Next Step
The necessary tools are now installed and ready to use. The [next step](SW-BUILD.md) is to get the sourcecode and all related files and build the firmware and filesystem image.

# Issues, Ideas And Bugs
If you have further ideas or you found some bugs, great! Create a [issue](https://github.com/BlueAndi/esp-rgb-led-matrix/issues) or if you are able and willing to fix it by yourself, clone the repository and create a pull request.

# License
The whole source code is published under the [MIT license](http://choosealicense.com/licenses/mit/).
Consider the different licenses of the used third party libraries too!

# Contribution
Unless you explicitly state otherwise, any contribution intentionally submitted for inclusion in the work by you, shall be licensed as above, without any
additional terms or conditions.
