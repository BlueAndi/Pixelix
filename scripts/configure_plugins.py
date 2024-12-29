"""Plugin handler."""

# MIT License
#
# Copyright (c) 2019 - 2024 Andreas Merkle (web@blue-andi.de)
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
import os
import shutil
import json
import hashlib
from string import Template

################################################################################
# Variables
################################################################################
_LIB_PATH = "./lib"

_WEB_DATA_PATH = "./data/plugins"

_MENU_FULL_PATH = "./data/js/pluginsSubMenu.js"

_PLUGIN_LIST_FULL_PATH = "./src/Generated/PluginList.cpp"
_PLUGIN_LIST_TEMPLATE_FULL_PATH = "./scripts/PluginList.cpp"

################################################################################
# Classes
################################################################################

################################################################################
# Functions
################################################################################

def _sort_key(file_path):
    return os.path.basename(file_path)

def calculate_path_checksum(paths):
    """Recursively calculates a checksum representing the contents of all files
        found with a sequence of file and/or directory paths.

    Args:
        paths (list[str]): List of paths

    Returns:
        int: MD5 checksum
    """
    hasher = hashlib.md5()
    file_list = []

    for path in paths:
        if os.path.isdir(path):
            filenames = os.listdir(path)
            for filename in filenames:
                file_path = os.path.normpath(path + "/" + filename)
                file_list.append(file_path)
        elif os.path.isfile(path):
            file_list.append(path)

    file_list_sorted = sorted(file_list, key=_sort_key)

    for file_path in file_list_sorted:
        with open(file_path, "rb") as file:
            while chunk := file.read(8192):
                hasher.update(chunk)

    return hasher.hexdigest()

def _load_json_file(full_path):
    """Load JSON file.

    Args:
        full_path (str): Full path to JSON file.

    Returns:
        dict|None: Data dictionary or None
    """
    data = None

    try:
        with open(full_path, encoding="utf-8") as json_data:
            data = json.load(json_data)

    except FileNotFoundError:
        pass

    return data

def _clean_up_folders(plugin_list, dst_path):
    """Remove folders in destination path, which are not present in the source path.

    Args:
        plugin_list (list): List with all plugin names
        dst_path (str): Destination path

    Return:
        bool: If cleaned something, it will return True otherwise False.
    """
    is_cleaned_up = False

    if os.path.isdir(dst_path) is False:
        pass
    else:

        dst_subfolders = []
        for dst_file in os.listdir(dst_path):
            if os.path.isdir(dst_path + "/" + dst_file) is True:
                dst_subfolders.append(dst_file)

        # Remove folders which exists only in the destination
        for folder in dst_subfolders:
            if folder not in plugin_list:
                dst_full_path = dst_path + "/" + folder
                try:
                    shutil.rmtree(dst_full_path)
                    print(f"\t\"{dst_full_path}\" removed.")
                except FileNotFoundError:
                    pass
                is_cleaned_up = True

    return is_cleaned_up

def _copy_files(src_files, dst_path):
    """Copy plugin web related files from /lib/<plugin-name> to /data/plugins/<plugin-name>.
        If no destination folder exists, it will be created.

    Args:
        plugin_name (str): Plugin name
    """
    if os.path.exists(dst_path) is False:
        os.mkdir(dst_path)

    # Copy source files to destination.
    for src_file in src_files:
        print(f"\t\t{src_file} -> {dst_path}", end="")
        try:
            shutil.copy2(src_file, dst_path)
            print("")
        except FileNotFoundError:
            print(" -> file not found!")

def _generate_web_menu(menu_full_path, plugin_list):
    """Generate the menu.json file.

    Args:
        menu_full_path (str): Full path to menu.json where it shall be created.
        plugin_list (list): List of all plugin names
    """
    with open(menu_full_path, 'w', encoding="utf-8") as file_desc:
        file_desc.write("var pluginSubMenu = [\n")

        for idx, plugin_name in enumerate(plugin_list):

            file_desc.write("    {\n")
            file_desc.write(f"        title: \"{plugin_name}\",\n")
            file_desc.write(f"        hyperRef: \"/plugins/{plugin_name}/{plugin_name}.html\"\n")
            file_desc.write("    }")

            if idx == (len(plugin_list) - 1):
                file_desc.write("\n")
            else:
                file_desc.write(",\n")

        file_desc.write("]\n")

def _generate_cpp_plugin_list(plugin_list_full_path, plugin_list):
    """Generate the PluginList.cpp source file.

    Args:
        plugin_list_full_path (str): Full path to PluginList.cpp where it shall be created.
        plugin_list (list): List of all plugin names.
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

def configure_plugins(plugin_list, layout):
    """Handle all plugin related artifacts.

    Args:
        plugin_list (list): List of plugin names
        layout (str): The display layout.
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
            print(f"\tSkipping {plugin_name}, because {plugin_lib_path} doesn't exist.")
            skip_list.append(plugin_name)

        elif os.path.isdir(plugin_lib_web_path) is False:
            print(f"\tSkipping {plugin_name}, because {plugin_lib_web_path} doesn't exist.")
            skip_list.append(plugin_name)

        else:
            data_web_plugin_path = _WEB_DATA_PATH + "/" + plugin_name

            data_web_plugin_path_checksum = calculate_path_checksum([data_web_plugin_path])

            src_files = []

            plugin_lib_data = _load_json_file(plugin_lib_path + "/pixelix.json")

            if plugin_lib_data is None:
                src_files = os.listdir(plugin_lib_web_path)
                src_files = [plugin_lib_web_path + "/" + src_file for src_file in src_files]
            else:

                if plugin_lib_data["pixelix"]["type"] != "plugin":
                    print(f"\tSkipping {plugin_name}, because its type isn't plugin in pixelix.json.")
                    skip_list.append(plugin_name)

                elif plugin_lib_data["pixelix"]["name"] != plugin_name:
                    print(f"\tSkipping {plugin_name}, because plugin name doesn't match with pixelix.json.")

                else:
                    web_files = plugin_lib_data["pixelix"]["web"]["files"]
                    layout_specific_files = []
                    src_files = [plugin_lib_path + "/" + file_rel_path for file_rel_path in web_files]

                    # Search for layout specific files or use generic file list.
                    for lib_layout in plugin_lib_data["pixelix"]["web"]["layouts"]:

                        if lib_layout["name"] == layout:
                            layout_specific_files = lib_layout["files"]

                        elif lib_layout["name"] == "LAYOUT_GENERIC":
                            if layout_specific_files is None:
                                layout_specific_files = lib_layout["files"]

                    layout_specific_files_full_path = [plugin_lib_path + "/" + file_rel_path for file_rel_path in layout_specific_files]
                    src_files = src_files + layout_specific_files_full_path

            if src_files:
                plugin_lib_web_path_checksum = calculate_path_checksum(src_files)

                if data_web_plugin_path_checksum != plugin_lib_web_path_checksum:
                    print("\tCopy web data:")
                    try:
                        shutil.rmtree(data_web_plugin_path)
                    except FileNotFoundError:
                        pass
                    _copy_files(src_files, data_web_plugin_path)
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
