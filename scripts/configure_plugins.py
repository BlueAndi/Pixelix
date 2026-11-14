"""Plugin handler."""

# MIT License
#
# Copyright (c) 2019 - 2026 Andreas Merkle (web@blue-andi.de)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

################################################################################
# Imports
################################################################################
from __future__ import annotations

import os
from string import Template

from configure_common import (
    clean_up_folders as _clean_up_folders,
    sync_web_files,
)

################################################################################
# Variables
################################################################################
_LIB_PATH = "./lib"

_WEB_DATA_PATH = "./data/plugins"

_MENU_FULL_PATH = "./scripts/pluginsSubMenu.js"

_PLUGIN_LIST_FULL_PATH = "./src/Generated/PluginList.cpp"
_PLUGIN_LIST_TEMPLATE_FULL_PATH = "./scripts/PluginList.cpp"

################################################################################
# Classes
################################################################################

################################################################################
# Functions
################################################################################


def _generate_web_menu(menu_full_path: str, plugin_list: list[str]) -> None:
    """Generate the plugin web menu.

    Args:
        menu_full_path: Destination for the JavaScript menu.
        plugin_list: Plugin names to include.

    Returns:
        None.
    """
    with open(menu_full_path, 'w', encoding="utf-8") as file_desc:
        file_desc.write("var pluginSubMenu = [\n")

        for idx, plugin_name in enumerate(plugin_list):

            file_desc.write("    {\n")
            file_desc.write(f"        title: \"{plugin_name}\",\n")
            file_desc.write(
                f"        hyperRef: "
                f"\"/plugins/{plugin_name}/{plugin_name}.html\"\n"
            )
            file_desc.write("    }")

            if idx == (len(plugin_list) - 1):
                file_desc.write("\n")
            else:
                file_desc.write(",\n")

        file_desc.write("];\n")


def _generate_cpp_plugin_list(
    plugin_list_full_path: str, plugin_list: list[str]
) -> None:
    """Generate the plugin list C++ source file.

    Args:
        plugin_list_full_path: Destination for the generated source file.
        plugin_list: Plugin names to include.

    Returns:
        None.
    """
    includes = ""
    list_entries = ""

    for idx, plugin_name in enumerate(plugin_list):
        if 0 < idx:
            includes += "\n"
            list_entries += ",\n"

        includes += f"#include <{plugin_name}.h>"
        list_entries += f"    {{ \"{plugin_name}\", {plugin_name}::create }}"

    data = {
        "INCLUDES": includes,
        "LIST_ENTRIES": list_entries
    }

    with open(_PLUGIN_LIST_TEMPLATE_FULL_PATH, "r", encoding="utf-8") as file_desc:
        src = Template(file_desc.read())
        result = src.substitute(data)

    with open(plugin_list_full_path, "w", encoding="utf-8") as file_desc:
        file_desc.write(result)


def configure_plugins(plugin_list: list[str], layout: str) -> None:
    """Generate plugin artifacts, removing skipped names from the input list.

    Args:
        plugin_list: Mutable list of plugin names.
        layout: Display layout.

    Returns:
        None.
    """
    # Avoid generation if possible, because it will cause a compilation step.
    is_generation_required = False

    if os.path.isdir(_WEB_DATA_PATH) is False:
        os.mkdir(_WEB_DATA_PATH)
        is_generation_required = True

    else:
        # Remove all obsolete plugins in the web data if there are any.
        if _clean_up_folders(plugin_list, _WEB_DATA_PATH) is True:
            is_generation_required = True

    skip_list = []
    for plugin_name in plugin_list:
        plugin_lib_path = _LIB_PATH + "/" + plugin_name
        plugin_lib_web_path = plugin_lib_path + "/web"

        if os.path.isdir(plugin_lib_path) is False:
            print(f"\tSkipping {plugin_name}, because "
                  f"{plugin_lib_path} doesn't exist.")
            skip_list.append(plugin_name)

        elif os.path.isdir(plugin_lib_web_path) is False:
            print(f"\tSkipping {plugin_name}, because "
                  f"{plugin_lib_web_path} doesn't exist.")
            skip_list.append(plugin_name)

        else:
            invalid_type, changed = sync_web_files(
                plugin_name, "plugin", plugin_lib_path,
                _WEB_DATA_PATH + "/" + plugin_name, layout
            )
            if invalid_type:
                skip_list.append(plugin_name)
            if changed:
                is_generation_required = True

    # Remove skipped plugins from list
    for plugin_name in skip_list:
        plugin_list.remove(plugin_name)

    if (is_generation_required is True) or \
        (os.path.exists(_MENU_FULL_PATH) is False) or \
            (os.path.exists(_PLUGIN_LIST_FULL_PATH) is False):

        print("\tGenerating plugins web menu.")
        _generate_web_menu(_MENU_FULL_PATH, plugin_list)
        print("\tGenerating plugin list.")
        _generate_cpp_plugin_list(_PLUGIN_LIST_FULL_PATH, plugin_list)

################################################################################
# Main
################################################################################
