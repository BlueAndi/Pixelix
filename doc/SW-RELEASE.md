# PIXELIX
![PIXELIX](./images/LogoBlack.png)

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](http://choosealicense.com/licenses/mit/)

- [PIXELIX](#pixelix)
- [Configuration Management](#configuration-management)
- [Version Numbers](#version-numbers)
- [Strategy](#strategy)
- [How to release?](#how-to-release)

# Configuration Management
The following chapters explain the general configuration management.

# Version Numbers
The concept behind the version number follows the [semantic versioning](https://semver.org/).

1. MAJOR version for incompatible API changes,
2. MINOR version if functionality added in a backwards compatible manner, and
2. PATCH version for backwards compatible bug fixes.

# Strategy
* The master branch shall contain always the latest released version.
* The feature development shall take place in separate branches.
* Bugfix releases shall be prepared in separate branches.

# How to release?

1. Integrate all related feature/bugfix branches to the **dev** branch.
2. Update the [REST API on Swagger](https://app.swaggerhub.com/apis/BlueAndi/Pixelix) and publish it.
3. Update the hyperlink to the REST API version on Swagger in the [./doc/README.md](README.md)
4. Update version number in [version.json](../data/version.json) in your local **dev** branch.
5. Update version number in [Doxyfile](./doxygen/Doxyfile), see ```PROJECT_NUMBER```.
6. All must be comitted now!
7. Test local **dev** branch.
8. Push local **dev** branch to remote repository.
9. Wait till all CI actions successful finished. If CI run fails, fix it and repeat.
10. Merge remote **dev** branch to remote **master** branch.
11. Release version on github.
