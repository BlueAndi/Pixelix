"""
MIT License

Copyright (c) 2019 - 2023 Andreas Merkle <web@blue-andi.de>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

================================================================================
Configures the plugins.

"""
import configparser
import os
import shutil
import json
from string import Template
import re

_LIB_PATH = "./lib"
_WEB_DATA_PATH = "./data/plugins"
_MENU_FULL_PATH = "./data/js/menu.json"
_PLUGIN_LIST_FULL_PATH = "./src/Generated/PluginList.hpp"
_PLUGIN_LIST_TEMPLATE_FULL_PATH = "./scripts/PluginList.hpp"

def _find_config_section(config):
    name = ""
    sections = config.sections()

    for section in sections:
        if section.startswith("config:") is True:
            name = section
            break

    return name

def _get_plugin_lib_full_path(plugin_name):
    plugin_lib_full_path = _LIB_PATH

    if not plugin_lib_full_path.endswith("/"):
        plugin_lib_full_path += "/"

    plugin_lib_full_path += plugin_name

    return plugin_lib_full_path

def _get_plugin_web_full_path(plugin_name):
    plugin_web_full_path = _WEB_DATA_PATH

    if not plugin_web_full_path.endswith("/"):
        plugin_web_full_path += "/"

    plugin_web_full_path += plugin_name

    return plugin_web_full_path

def _copy_plugin_web_to_data(plugin_name):
    plugin_web_full_path = _get_plugin_web_full_path(plugin_name)

    if os.path.isdir(plugin_web_full_path) is False:
        os.mkdir(plugin_web_full_path)

    plugin_lib_full_path = _get_plugin_lib_full_path(plugin_name)
    plugin_lib_web_full_path = plugin_lib_full_path + "/" + "web"

    src_files = os.listdir(plugin_lib_web_full_path)

    # Copy source files only if they are newer than the destination files.
    # Check this file by file.
    for file_name in src_files:
        src_file_full_path = plugin_lib_web_full_path + "/" + file_name
        dst_file_full_path = plugin_web_full_path + "/" + file_name

        # Destination file doesn't exist?
        if os.path.exists(dst_file_full_path) is False:
            shutil.copy2(src_file_full_path, dst_file_full_path)

        # Destination file exists and source file is newer?
        elif (os.stat(src_file_full_path).st_mtime - os.stat(dst_file_full_path).st_mtime) > 1:
            shutil.copy2(src_file_full_path, dst_file_full_path)

def _generate_web_menu(menu_full_path, plugin_list):
    menu = {}
    menu["menu_items"] = []

    for plugin_name in plugin_list:
        entry = {
            "title": plugin_name,
            "hyperRef": "plugins/" + plugin_name + ".html"
        }

        menu["menu_items"].append(entry)

    with open(menu_full_path, 'w', encoding="utf-8") as file_desc:
        json.dump(menu, file_desc, indent=4)

def _generate_cpp_plugin_list(plugin_list_full_path, plugin_list):
    includes = ""
    register_calls = ""

    for plugin_name in plugin_list:
        includes += f"#include <{plugin_name}.h>\n"
        register_calls += f"    pluginMgr.registerPlugin(\"{plugin_name}\", {plugin_name}::create);\n"

    data = {
        "INCLUDES": includes,
        "REGISTER": register_calls
    }

    with open(_PLUGIN_LIST_TEMPLATE_FULL_PATH, "r", encoding="utf-8") as file_desc:
        src = Template(file_desc.read())
        result = src.substitute(data)

    with open(plugin_list_full_path, "w", encoding="utf-8") as file_desc:
        file_desc.write(result)

def configure(config_path):
    """Configures the plugins according to configuration.
    """
    print(f"Configure plugins according to {config_path}")

    config = configparser.ConfigParser(empty_lines_in_values=False)

    try:
        config.read(config_path)
    except configparser.Error:
        print(f"Configuration file {config_path} is invalid.")

    # All plugins are listed in the section "config:<type>" under the key "lib_deps".
    config_section_name = _find_config_section(config)
    lib_deps = config[config_section_name]["lib_deps"].strip().splitlines()

    # Remove version information
    for idx, plugin_name in enumerate(lib_deps):
        result = re.match("([a-zA-Z0-9_\\-]*)[ ]*@.*", plugin_name)
        if result:
            lib_deps[idx] = result.group(1)

    # Prepare plugin by plugin
    # Precondition: Plugin must be available in the library folder
    for plugin_name in lib_deps:
        plugin_lib_full_path = _get_plugin_lib_full_path(plugin_name)
        plugin_lib_web_full_path = plugin_lib_full_path + "/web"

        if os.path.isdir(plugin_lib_full_path) is False:
            print(f"\tSkipping {plugin_name}, because {plugin_lib_full_path} doesn't exist.")

        elif os.path.isdir(plugin_lib_web_full_path) is False:
            print(f"\tSkipping {plugin_name}, because {plugin_lib_web_full_path} doesn't exist.")

        else:
            _copy_plugin_web_to_data(plugin_name)
            _generate_web_menu(_MENU_FULL_PATH, lib_deps)
            _generate_cpp_plugin_list(_PLUGIN_LIST_FULL_PATH, lib_deps)
