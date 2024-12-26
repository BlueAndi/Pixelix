# PIXELIX <!-- omit in toc -->
![PIXELIX](./images/LogoBlack.png)

[![License](https://img.shields.io/badge/license-MIT-blue.svg)](http://choosealicense.com/licenses/mit/)

# Plugin development <!-- omit in toc -->

* [What must be done?](#what-must-be-done)
* [Optional](#optional)
* [Rules](#rules)
* [Recommendations](#recommendations)
* [Typical use cases](#typical-use-cases)
  * [Initial configuration in filesystem](#initial-configuration-in-filesystem)
  * [Reload configuration from filesystem periodically](#reload-configuration-from-filesystem-periodically)
  * [Request information from URL periodically](#request-information-from-url-periodically)
* [Traps and pitfalls](#traps-and-pitfalls)
  * [active/inactive](#activeinactive)
* [SW Architecture](#sw-architecture)
  * [Static View](#static-view)
  * [Dynamic View](#dynamic-view)
* [Issues, Ideas And Bugs](#issues-ideas-and-bugs)
* [License](#license)
* [Contribution](#contribution)

# What must be done?
1. Create the plugin as library:
   1. Create a folder in ```/lib/<plugin-name>``` with the plugin name. Very important: The plugin name shall end with "Plugin", e.g. "FirePlugin".
   2. Create two additional folders underneath:
      * ```src```: Contains the plugin sourcecode.
      * ```web```: Contains all web relevant files, including icons the plugin may need.
   3. Create a ```library.json``` with the library meta information.
2. The plugin sources (.h/.cpp) shall be in ```/lib/<plugin-name>/src``` folder.
3. The plugin shall be added in all configurations in ```/config/config-<type>.ini``` like PlatformIO libraries. Please add it in alphabetic order. To skip it in a config file, because e.g. there is not enough space in the flash, comment it out there via ";".
4. A short plugin description shall be in ```/doc/PLUGINS.md```.
5. A plugin specific HTML page shall be in ```/lib/<plugin-name>/web```.
6. Place a plugin screenshot as image in ```/lib/<plugin-name>/web```, using the plugin name and the file extension. It shall be shown in the HTML page.
7. If the plugin spawns a REST API:
   1. The HTML page must be able to get/set the corresponding information via REST API.
   2. The [REST API description](https://app.swaggerhub.com/apis/BlueAndi/Pixelix) shall be extended. Ask the owner to do this via issue or pull-request.

# Optional
If the plugin supports different display sizes, you might want to copy display size depended icons to the filesystem. This can be achieved by adding a ```pixelix.json``` to the folder ```/lib/<plugin-name>```.

Example:
```json
{
    "pixelix": {
        "type": "plugin",
        "name": "OpenWeatherPlugin",
        "web": {
            "files": [
                "./web/OpenWeatherPlugin.html"
            ],
            "layouts": [{
                "name": "LAYOUT_GENERIC",
                "files": [
                    "./web/generic/01.gif"
                ]
            }, {
                "name": "LAYOUT_32X8",
                "files": [
                    "./web/32x8/01.gif"
                ]
            }, {
                "name": "LAYOUT_64X64",
                "files": [
                    "./web/64x64/01.gif"
                ]
            }]
        }
    }
}
```
Note, the file paths are relative from ```/lib/<plugin-name>```.

# Rules
* Follow the [boy scout rule](https://biratkirat.medium.com/step-8-the-boy-scout-rule-robert-c-martin-uncle-bob-9ac839778385#:~:text=The%20Boy%20Scouts%20have%20a,not%20add%20more%20smelling%20code.), especially for coding style. Check other plugins or sourcecode in the repository, to know how to do it right.
* All one time jobs shall be done in ```start()``` method. E.g. the initial layout creation.
* The ```update()``` method shall be efficient, therefore
  * never read/write something from filesystem,
  * never perform any web related stuff,
  * never do one time jobs inside,
  * just update the display!
* Remove the metadata from jpeg files to avoid wasting filesystem space, which can be done e.g. with [Exiv2](https://exiv2.org/).
  * ```$ exiv2 rm image.jpg```

# Recommendations
* Update the display always to support animated GIFs and scrolling texts.

# Typical use cases

## Initial configuration in filesystem
The first time a plugin instance starts up, it will try to load a configuration from the filesystem (if applicable) in ```start()``` method. If this fails, it creates a default one.

## Reload configuration from filesystem periodically
Because a plugin instance configuration in the filesystem can be edited via file browser too, it makes sense to periodically reload it. It is recommended to do this in the ```process()``` method.

## Request information from URL periodically
Any http request can be started in the ```process()``` method. The response will be evaluated in the context of the corresponding web task. Only the take over of the relevant data shall be protected against concurrent access.

# Traps and pitfalls

## active/inactive
The ```active()``` and ```inactive()``` methods are called once before a plugin instance is activated or deactivated. But consider the case, that only one plugin instance is installed at all (except SysMsgPlugin). ```active()``` will be called just once and stays active.

Therefore if you need periodically stuff, but you can't do it in the ```update()``` method, use the ```process()``` method.

# SW Architecture

## Static View

![plugin-service-dynamic](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/Pixelix/master/doc/architecture/uml/plugin-service.wsd)

## Dynamic View

![plugin-service-dynamic](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/BlueAndi/Pixelix/master/doc/architecture/uml/plugin-service-dynamic.wsd)

# Issues, Ideas And Bugs
If you have further ideas or you found some bugs, great! Create a [issue](https://github.com/BlueAndi/Pixelix/issues) or if you are able and willing to fix it by yourself, clone the repository and create a pull request.

# License
The whole source code is published under the [MIT license](http://choosealicense.com/licenses/mit/).
Consider the different licenses of the used third party libraries too!

# Contribution
Unless you explicitly state otherwise, any contribution intentionally submitted for inclusion in the work by you, shall be licensed as above, without any
additional terms or conditions.
