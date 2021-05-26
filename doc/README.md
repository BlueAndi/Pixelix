# PIXELIX
![PIXELIX](./images/LogoBlack.png)

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](http://choosealicense.com/licenses/mit/)

- [PIXELIX](#pixelix)
- [Documentation](#documentation)
  - [Requirements](#requirements)
  - [Electronic](#electronic)
  - [Software](#software)
  - [Structure](#structure)

# Documentation

## Requirements

* [Requirements](REQUIREMENTS.md)

## Electronic

* [Detail](ELECTRONIC.md)
* [Bill of material](BOM.md)

## Software

* [Toolchain installation](TOOLCHAIN-INSTALLATION.md)
* [Software build](SW-BUILD.md)
* [Software update](SW-UPDATE.md)
* [Plugins](PLUGINS.md)
* [REST API description](REST.md)
* [Websocket API description](WEBSOCKET.md)

## Structure

<pre>
+---data            (All filesystem files (SPIFFS))
+---doc             (Documentation)
    +---datasheets  (Datasheets of electronic parts)
    +---design      (Design related documents)
    +---doxygen     (Sourcecode documentation)
    +---eagle-libs  (Eagle libraries)
    +---images      (Images used for documentation)
    +---pcb         (Electronic PCB images)
    +---rfc         (Requests for Comments specifications)
    \---schematics  (Schematics)
+---include         (Include files)
+---lib             (Project specific (private) libraries)
+---scripts         (PlatformIO specific Python scripts)
+---src             (Sourcecode)
+---test            (Unit tests)
\---platform.ini    (Project configuration file)
</pre>
