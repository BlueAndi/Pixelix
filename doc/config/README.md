# PIXELIX <!-- omit in toc -->
![PIXELIX](../images/LogoBlack.png)

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](http://choosealicense.com/licenses/mit/)

# SW Configuration Management  <!-- omit in toc -->

* [Purpose](#purpose)
* [Scope](#scope)
* [PlatformIO](#platformio)
  * [Logical Configuration Hierarchy And Dependencies](#logical-configuration-hierarchy-and-dependencies)
* [Version Numbers](#version-numbers)
* [Development Strategy](#development-strategy)
* [Work Instructions](#work-instructions)
  * [How to release?](#how-to-release)
* [Issues, Ideas And Bugs](#issues-ideas-and-bugs)
* [License](#license)
* [Contribution](#contribution)

# Purpose
The SW configuration management document provides a general overview about what is configured and how it is done.

# Scope
The SW configuration management document is valid for the [PIXELIX](https://github.com/BlueAndi/esp-rgb-led-matrix) project.

# PlatformIO
The SW build environment is based on [PlatformIO](https://platformio.org/). The ```platformio.ini``` is the central configuration file in the root folder and contains all related build environments (targets).

## Logical Configuration Hierarchy And Dependencies

![pio-env-dependencies](https://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/esp-rgb-led-matrix/master/doc/config/uml/pio-env-dependencies.wsd)

# Version Numbers
The concept behind the version number follows the [semantic versioning](https://semver.org/).

1. MAJOR version for incompatible API changes,
2. MINOR version if functionality added in a backwards compatible manner, and
2. PATCH version for backwards compatible bug fixes.

# Development Strategy
* The master branch shall contain always the latest released version.
* The feature development shall take place in separate branches.
* Bugfix releases shall be prepared in separate branches.

# Work Instructions

## How to release?

1. Integrate all related feature/bugfix branches to the **dev** branch.
2. Update the [REST API on Swagger](https://app.swaggerhub.com/apis/BlueAndi/Pixelix) and publish it.
3. Update the hyperlink to the REST API version on Swagger at all references. To find them search for ```https://app.swaggerhub.com/apis/BlueAndi/Pixelix```.
4. Update version number in [version.json](../data/version.json) in your local **dev** branch.
5. Update version number in [Doxyfile](./doxygen/Doxyfile), see ```PROJECT_NUMBER```.
6. All must be committed now!
7. Test local **dev** branch.
8. Push local **dev** branch to remote repository.
9. Wait till all CI actions successful finished. If CI run fails, fix it and repeat.
10. Merge remote **dev** branch to remote **master** branch.
11. Release version on github.

# Issues, Ideas And Bugs
If you have further ideas or you found some bugs, great! Create a [issue](https://github.com/BlueAndi/esp-rgb-led-matrix/issues) or if you are able and willing to fix it by yourself, clone the repository and create a pull request.

# License
The whole source code is published under the [MIT license](http://choosealicense.com/licenses/mit/).
Consider the different licenses of the used third party libraries too!

# Contribution
Unless you explicitly state otherwise, any contribution intentionally submitted for inclusion in the work by you, shall be licensed as above, without any
additional terms or conditions.
