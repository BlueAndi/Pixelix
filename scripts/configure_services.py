"""Service handler."""

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

_WEB_DATA_PATH = "./data/services"

_MENU_FULL_PATH = "./data/js/servicesSubMenu.js"

_SERVICE_LIST_FULL_PATH = "./src/Generated/Services.cpp"
_SERVICE_LIST_TEMPLATE_FULL_PATH = "./scripts/Services.cpp"

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

def _clean_up_folders(service_list, dst_path):
    """Remove folders in destination path, which are not present in the source path.

    Args:
        service_list (list): List with all service names
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
            if folder not in service_list:
                dst_full_path = dst_path + "/" + folder
                try:
                    shutil.rmtree(dst_full_path)
                    print(f"\t\"{dst_full_path}\" removed.")
                except FileNotFoundError:
                    pass
                is_cleaned_up = True

    return is_cleaned_up

def _copy_files(src_files, dst_path):
    """Copy service web related files from /lib/<service-name> to /data/services/<service-name>.
        If no destination folder exists, it will be created.

    Args:
        service_name (str): service name
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

def _generate_web_menu(menu_full_path, service_list):
    """Generate the menu.json file.

    Args:
        menu_full_path (str): Full path to menu.json where it shall be created.
        service_list (list): List of all service names
    """
    with open(menu_full_path, 'w', encoding="utf-8") as file_desc:
        file_desc.write("var serviceSubMenu = [\n")

        for idx, service_name in enumerate(service_list):

            file_desc.write("    {\n")
            file_desc.write(f"        title: \"{service_name}\",\n")
            file_desc.write(f"        hyperRef: \"/services/{service_name}/{service_name}.html\"\n")
            file_desc.write("    }")

            if idx == (len(service_list) - 1):
                file_desc.write("\n")
            else:
                file_desc.write(",\n")

        file_desc.write("]\n")

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
    list_entries = ""

    # Handle includes, start and process calls.
    for idx, service_name in enumerate(service_list):

        if idx > 0:
            includes += "\n"
            start_calls += "\n"
            process_calls += "\n"
            list_entries += ",\n"

        includes += f"#include <{service_name}.h>"

        start_calls += f"    if (false == {service_name}::getInstance().start())\n"
        start_calls +=  "    {\n"
        start_calls +=  "        isSuccessful = false;\n"
        start_calls +=  "    }\n"

        process_calls += f"    {service_name}::getInstance().process();"

        list_entries += f"    \"{service_name}\""

    # Handle stop calls in reverse order.
    for idx, service_name in enumerate(reversed(service_list)):

        if idx > 0:
            stop_calls += "\n"

        stop_calls += f"    {service_name}::getInstance().stop();"

    data = {
        "INCLUDES": includes,
        "START_SERVICES": start_calls,
        "STOP_SERVICES": stop_calls,
        "PROCESS_SERVICES": process_calls,
        "LIST_ENTRIES": list_entries
    }

    with open(_SERVICE_LIST_TEMPLATE_FULL_PATH, "r", encoding="utf-8") as file_desc:
        src = Template(file_desc.read())
        result = src.substitute(data)

    with open(service_list_full_path, "w", encoding="utf-8") as file_desc:
        file_desc.write(result)

def configure_services(service_list, layout):
    """Generate all service related artifacts.

    Args:
        service_list (list): List of service names
        layout (str): The display layout.
    """
    # Avoid generation if possible, because it will cause a compilation step.
    is_generation_required = False

    if os.path.isdir(_WEB_DATA_PATH) is False:
        os.mkdir(_WEB_DATA_PATH)
        is_generation_required = True

    else:
        # Remove all obsolete services in the web data if there are any.
        if _clean_up_folders(service_list, _WEB_DATA_PATH) is True:
            is_generation_required = True

    skip_list = []
    for service_name in service_list:
        service_lib_path = _LIB_PATH + "/" + service_name
        service_lib_web_path = service_lib_path + "/web"

        if os.path.isdir(service_lib_path) is False:
            print(f"\tSkipping {service_name}, because {service_lib_path} doesn't exist.")
            skip_list.append(service_name)

        elif os.path.isdir(service_lib_web_path) is False:
            pass

        else:
            data_web_service_path = _WEB_DATA_PATH + "/" + service_name

            data_web_service_path_checksum = calculate_path_checksum([data_web_service_path])

            src_files = []

            service_lib_data = _load_json_file(service_lib_path + "/pixelix.json")

            if service_lib_data is None:
                src_files = os.listdir(service_lib_web_path)
                src_files = [service_lib_web_path + "/" + src_file for src_file in src_files]
            else:

                if service_lib_data["pixelix"]["type"] != "service":
                    print(f"\tSkipping {service_name}, because its type isn't service in pixelix.json.")
                    skip_list.append(service_name)

                elif service_lib_data["pixelix"]["name"] != service_name:
                    print(f"\tSkipping {service_name}, because service name doesn't match with pixelix.json.")

                else:
                    web_files = service_lib_data["pixelix"]["web"]["files"]
                    layout_specific_files = []
                    src_files = [service_lib_path + "/" + file_rel_path for file_rel_path in web_files]

                    # Search for layout specific files or use generic file list.
                    for lib_layout in service_lib_data["pixelix"]["web"]["layouts"]:

                        if lib_layout["name"] == layout:
                            layout_specific_files = lib_layout["files"]

                        elif lib_layout["name"] == "LAYOUT_GENERIC":
                            if layout_specific_files is None:
                                layout_specific_files = lib_layout["files"]

                    layout_specific_files_full_path = [service_lib_path + "/" + file_rel_path for file_rel_path in layout_specific_files]
                    src_files = src_files + layout_specific_files_full_path

            if src_files:
                service_lib_web_path_checksum = calculate_path_checksum(src_files)

                if data_web_service_path_checksum != service_lib_web_path_checksum:
                    print("\tCopy web data:")
                    try:
                        shutil.rmtree(data_web_service_path)
                    except FileNotFoundError:
                        pass
                    _copy_files(src_files, data_web_service_path)
                    is_generation_required = True

    # Remove skipped services from list
    for service_name in skip_list:
        service_list.remove(service_name)

    if (is_generation_required is True) or \
        (os.path.exists(_MENU_FULL_PATH) is False) or \
        (os.path.exists(_SERVICE_LIST_FULL_PATH) is False):

        print("\tGenerating services web menu.")
        _generate_web_menu(_MENU_FULL_PATH, service_list)

    print("\tGenerating services.")
    _generate_cpp_service(_SERVICE_LIST_FULL_PATH, service_list)

################################################################################
# Main
################################################################################
