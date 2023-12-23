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
from string import Template
import re

################################################################################
# Variables
################################################################################
_LIB_PATH = "./lib"

_WEB_DATA_PATH = "./data/plugins"

_MENU_FULL_PATH = "./data/js/pluginsSubMenu.js"

_PLUGIN_LIST_FULL_PATH = "./src/Generated/PluginList.cpp"
_PLUGIN_LIST_TEMPLATE_FULL_PATH = "./scripts/PluginList.cpp"

_SERVICE_LIST_FULL_PATH = "./src/Generated/Services.cpp"
_SERVICE_LIST_TEMPLATE_FULL_PATH = "./scripts/Services.cpp"

_TOPIC_HANDLER_LIST_FULL_PATH = "./src/Generated/TopicHandlers.cpp"
_TOPIC_HANDLER_LIST_TEMPLATE_FULL_PATH = "./scripts/TopicHandlers.cpp"

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

    if os.path.exists(plugin_web_data_path) is False:
        os.mkdir(plugin_web_data_path)

    plugin_lib_web_path = _LIB_PATH + "/" + plugin_name + "/" + "web"

    src_files = []
    for src_file in os.listdir(plugin_lib_web_path):
        if os.path.isdir(plugin_lib_web_path + "/" + src_file) is False:
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
                shutil.rmtree(dst_full_path)
                is_cleaned_up = True

    return is_cleaned_up

def _get_lib_list(lib_deps, postfix):
    """Get list of libraries from library dependencies, which name has the
        given postfix.

    Args:
        lib_deps (list): Library dependencies

    Returns:
        list: List of libraries
    """
    lib_list = []
    for lib_name in lib_deps:
        result = re.match(f"([a-zA-Z0-9_\\-]*{postfix})[ ]*@.*", lib_name)
        if result:
            lib_list.append(result.group(1))

    return lib_list

def _get_plugin_list(lib_deps):
    """Get list of plugins from library dependencies.

    Args:
        lib_deps (list): Library dependencies

    Returns:
        list: List of plugins
    """
    return _get_lib_list(lib_deps, "Plugin")

def _get_service_list(lib_deps):
    """Get list of services from library dependencies.

    Args:
        lib_deps (list): Library dependencies

    Returns:
        list: List of services
    """
    return _get_lib_list(lib_deps, "Service")

def _get_topic_handler_list(lib_deps):
    """Get list of topic handlers from library dependencies.

    Args:
        lib_deps (list): Library dependencies

    Returns:
        list: List of topic handlers
    """
    return _get_lib_list(lib_deps, "TopicHandler")

def _generate_plugins(plugin_list):
    """Generate all plugin related artifacts.

    Args:
        plugin_list (list): List of plugin names
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
            plugin_web_data_path = _WEB_DATA_PATH + "/" + plugin_name

            if _clean_up_files_and_folders(plugin_lib_web_path, plugin_web_data_path) is True:
                is_generation_required = True

            if _copy_plugin_web_to_data(plugin_name) is True:
                is_generation_required = True

    # Remove skipped plugins from list
    for plugin_name in skip_list:
        plugin_list.remove(plugin_name)

    if (is_generation_required is True) or \
        (os.path.exists(_MENU_FULL_PATH) is False) or \
        (os.path.exists(_PLUGIN_LIST_FULL_PATH) is False):

        print("\tGenerating web menu.")
        _generate_web_menu(_MENU_FULL_PATH, plugin_list)
        print("\tGenerating plugin list.")
        _generate_cpp_plugin_list(_PLUGIN_LIST_FULL_PATH, plugin_list)

def _generate_cpp_service(service_list_full_path, service_list):
    """Generate the Service.cpp source file.

    Args:
        service_list_full_path (str): Full path to where the source code shall be created.
        service_list (list): List of all service names
    """
    includes = ""
    start_calls = ""
    stop_calls = ""
    process_calls = ""

    for idx, service_name in enumerate(service_list):

        if idx > 0:
            includes += "\n"
            start_calls += "\n"
            stop_calls += "\n"
            process_calls += "\n"

        includes += f"#include <{service_name}.h>"

        start_calls += f"    if (false == {service_name}::getInstance().start())\n"
        start_calls +=  "    {\n"
        start_calls +=  "        isSuccessful = false;\n"
        start_calls +=  "    }\n"

        stop_calls += f"    {service_name}::getInstance().stop();"

        process_calls += f"    {service_name}::getInstance().process();"

    data = {
        "INCLUDES": includes,
        "START_SERVICES": start_calls,
        "STOP_SERVICES": stop_calls,
        "PROCESS_SERVICES": process_calls
    }

    with open(_SERVICE_LIST_TEMPLATE_FULL_PATH, "r", encoding="utf-8") as file_desc:
        src = Template(file_desc.read())
        result = src.substitute(data)

    with open(service_list_full_path, "w", encoding="utf-8") as file_desc:
        file_desc.write(result)

def _generate_services(service_list):
    """Generate all service related artifacts.

    Args:
        service_list (list): List of service names
    """
    skip_list = []
    for service_name in service_list:
        service_lib_path = _LIB_PATH + "/" + service_name

        if os.path.isdir(service_lib_path) is False:
            print(f"\tSkipping {service_name}, because {service_lib_path} doesn't exist.")
            skip_list.append(service_name)

    # Remove skipped services from list
    for service_name in skip_list:
        service_list.remove(service_name)

    print("\tGenerating services.")
    _generate_cpp_service(_SERVICE_LIST_FULL_PATH, service_list)

def _generate_cpp_topic_handler(topic_handler_list_full_path, topic_handler_list):
    """Generate the Service.cpp source file.

    Args:
        topic_handler_list_full_path (str): Full path to where the source code shall be created.
        topic_handler_list (list): List of all topic handlers names
    """
    includes = ""
    instances = ""
    list_of_instances = ""

    for idx, topic_handler_name in enumerate(topic_handler_list):

        if idx > 0:
            includes += "\n"
            instances += "\n\n"
            list_of_instances += ",\n"

        includes += f"#include <{topic_handler_name}.h>"

        instances += f"/** Plugin topic handler {topic_handler_name} instance. */\n"
        instances += f"static {topic_handler_name} g{topic_handler_name};"

        list_of_instances += f"    &g{topic_handler_name}"

    data = {
        "INCLUDES": includes,
        "INSTANCES": instances,
        "LIST_OF_INSTANCES": list_of_instances,
    }

    with open(_TOPIC_HANDLER_LIST_TEMPLATE_FULL_PATH, "r", encoding="utf-8") as file_desc:
        src = Template(file_desc.read())
        result = src.substitute(data)

    with open(topic_handler_list_full_path, "w", encoding="utf-8") as file_desc:
        file_desc.write(result)

def _generate_topic_handlers(topic_handler_list):
    """Generate all topic handler related artifacts.

    Args:
        topic_handler_list (list): List of topic handler names
    """
    skip_list = []
    for topic_handler_name in topic_handler_list:
        topic_handler_lib_path = _LIB_PATH + "/" + topic_handler_name

        if os.path.isdir(topic_handler_lib_path) is False:
            print(f"\tSkipping {topic_handler_name}, because {topic_handler_lib_path} doesn't exist.")
            skip_list.append(topic_handler_name)

    # Remove skipped services from list
    for topic_handler_name in skip_list:
        topic_handler_list.remove(topic_handler_name)

    print("\tGenerating topic handlers.")
    _generate_cpp_topic_handler(_TOPIC_HANDLER_LIST_FULL_PATH, topic_handler_list)

def configure(config_full_path):
    """Configures the plugins according to configuration.

    Args:
        config_full_path (str): Full path to configuration file
    """
    print(f"Configure plugins and services: {config_full_path}")

    # Load configuration
    config = configparser.ConfigParser(empty_lines_in_values=True)

    try:
        config.read(config_full_path)
    except configparser.Error:
        print(f"Configuration file {config_full_path} is invalid.")

    # All plugins and services are listed in the section "config:<type>" under the key "lib_deps".
    config_section_name = _find_config_section(config)
    lib_deps = config[config_section_name]["lib_deps"].strip().splitlines()

    plugin_list = _get_plugin_list(lib_deps)
    service_list = _get_service_list(lib_deps)
    topic_handler_list = _get_topic_handler_list(lib_deps)

    _generate_plugins(plugin_list)
    _generate_services(service_list)
    _generate_topic_handlers(topic_handler_list)

################################################################################
# Main
################################################################################
