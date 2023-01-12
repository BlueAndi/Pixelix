"""Configures the plugins."""

# MIT License
#
# Copyright (c) 2019 - 2023 Andreas Merkle (web@blue-andi.de)
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
import configparser
import os
import shutil
import json
from string import Template
import re

################################################################################
# Variables
################################################################################
_LIB_PATH = "./lib"
_WEB_DATA_PATH = "./data/plugins"
_MENU_FULL_PATH = "./data/js/menu.json"
_PLUGIN_LIST_FULL_PATH = "./src/Generated/PluginList.hpp"
_PLUGIN_LIST_TEMPLATE_FULL_PATH = "./scripts/PluginList.hpp"

################################################################################
# Classes
################################################################################

################################################################################
# Functions
################################################################################

def _find_config_section(config):
    """Find the config:<type> section in the configuration.

    Args:
        config (obj): Configuration

    Returns:
        str: Section name
    """
    name = ""
    sections = config.sections()

    for section in sections:
        if section.startswith("config:") is True:
            name = section
            break

    return name

def _copy_plugin_web_to_data(plugin_name):
    """Copy plugin web related files from /lib/<plugin-name> to /data/plugins/<plugin-name>.
        If no folder on the destination exists, it will be created.
        Only updated files are copied with their meta information.

    Args:
        plugin_name (str): Plugin name

    Returns:
        bool: If something is copied, it will return True otherwise False.
    """
    is_copied = False

    plugin_web_data_path = _WEB_DATA_PATH + "/" + plugin_name

    if os.path.isdir(plugin_web_data_path) is False:
        os.mkdir(plugin_web_data_path)

    plugin_lib_web_path = _LIB_PATH + "/" + plugin_name + "/" + "web"

    src_files = []
    for src_file in os.listdir(plugin_lib_web_path):
        if os.path.isdir(plugin_lib_web_path + "/" + src_file) is True:
            src_files.append(src_file)

    # Copy source files only if they are newer than the destination files.
    # Check this file by file.
    for file_name in src_files:
        src_file_full_path = plugin_lib_web_path + "/" + file_name
        dst_file_full_path = plugin_web_data_path + "/" + file_name

        # Destination file doesn't exist?
        if os.path.exists(dst_file_full_path) is False:
            shutil.copy2(src_file_full_path, dst_file_full_path)
            is_copied = True

        # Destination file exists and source file is newer?
        elif (os.stat(src_file_full_path).st_mtime - os.stat(dst_file_full_path).st_mtime) > 1:
            shutil.copy2(src_file_full_path, dst_file_full_path)
            is_copied = True

    return is_copied

def _generate_web_menu(menu_full_path, plugin_list):
    """Generate the menu.json file.

    Args:
        menu_full_path (str): Full path to menu.json where it shall be created.
        plugin_list (list): List of all plugin names
    """
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
    """Generate the PluginList.hpp source file.

    Args:
        plugin_list_full_path (str): Full path to PluginList.hpp where it shall be created.
        plugin_list (list): List of all plugin names
    """
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

def _clean_up_files_and_folders(src_path, dst_path):
    """Remove files and folders in destination path, which are not present in the source path.

    Args:
        src_path (str): Source path
        dst_path (str): Destination path

    Return:
        bool: If cleaned something, it will return True otherwise False.
    """
    is_cleaned_up = False

    if os.path.isdir(src_path) is False:
        pass
    elif os.path.isdir(dst_path) is False:
        pass
    else:
        src_files = os.listdir(src_path)
        dst_files = os.listdir(dst_path)

        # Remove files which exists only in the destination
        for file in dst_files:
            if file not in src_files:
                dst_full_path = dst_path + "/" + file

                print(f"\t\"{dst_full_path}\" removed.")

                if os.path.isdir(dst_full_path) is True:
                    os.rmdir(dst_full_path)
                else:
                    os.remove(dst_full_path)

                is_cleaned_up = True

    return is_cleaned_up

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
                print(f"\t\"{dst_full_path}\" removed.")
                os.rmdir(dst_full_path)
                is_cleaned_up = True

    return is_cleaned_up

def configure(config_full_path):
    """Configures the plugins according to configuration.

    Args:
        config_full_path (str): Full path to configuration file
    """
    print(f"Configure plugins: {config_full_path}")

    # Load configuration
    config = configparser.ConfigParser(empty_lines_in_values=False)

    try:
        config.read(config_full_path)
    except configparser.Error:
        print(f"Configuration file {config_full_path} is invalid.")

    # All plugins are listed in the section "config:<type>" under the key "lib_deps".
    config_section_name = _find_config_section(config)
    lib_deps = config[config_section_name]["lib_deps"].strip().splitlines()

    # Consider only plugins and remove their version information
    plugin_list = []
    for lib_name in lib_deps:
        result = re.match("([a-zA-Z0-9_\\-]*Plugin)[ ]*@.*", lib_name)
        if result:
            plugin_list.append(result.group(1))

    # Avoid generation if possible, because it will cause a compilation step.
    is_generation_required = False

    # Remove all obsolete plugins in the web data.
    if _clean_up_folders(plugin_list, _WEB_DATA_PATH) is True:
        is_generation_required = True

    # Prepare plugin by plugin
    for plugin_name in plugin_list:
        plugin_lib_path = _LIB_PATH + "/" + plugin_name
        plugin_lib_web_path = plugin_lib_path + "/web"

        if os.path.isdir(plugin_lib_path) is False:
            print(f"\tSkipping {plugin_name}, because {plugin_lib_path} doesn't exist.")

        elif os.path.isdir(plugin_lib_web_path) is False:
            print(f"\tSkipping {plugin_name}, because {plugin_lib_web_path} doesn't exist.")

        else:
            plugin_web_data_path = _WEB_DATA_PATH + "/" + plugin_name

            if _clean_up_files_and_folders(plugin_lib_web_path, plugin_web_data_path) is True:
                is_generation_required = True

            if _copy_plugin_web_to_data(plugin_name) is True:
                is_generation_required = True

            if is_generation_required is True:
                print("\tGenerating web menu.")
                _generate_web_menu(_MENU_FULL_PATH, plugin_list)
                print("\tGenerating plugin list.")
                _generate_cpp_plugin_list(_PLUGIN_LIST_FULL_PATH, plugin_list)

################################################################################
# Main
################################################################################
