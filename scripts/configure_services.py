"""Service handler."""

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

_WEB_DATA_PATH = "./data/services"

_MENU_FULL_PATH = "./scripts/servicesSubMenu.js"

_SERVICE_LIST_FULL_PATH = "./src/Generated/Services.cpp"
_SERVICE_LIST_TEMPLATE_FULL_PATH = "./scripts/Services.cpp"

################################################################################
# Classes
################################################################################

################################################################################
# Functions
################################################################################


def _generate_web_menu(menu_full_path: str, service_list: list[str]) -> None:
    """Generate the service web menu.

    Args:
        menu_full_path: Destination for the JavaScript menu.
        service_list: Service names to include.

    Returns:
        None.
    """
    with open(menu_full_path, 'w', encoding="utf-8") as file_desc:
        file_desc.write("var serviceSubMenu = [\n")

        for idx, service_name in enumerate(service_list):

            file_desc.write("    {\n")
            file_desc.write(f"        title: \"{service_name}\",\n")
            file_desc.write(
                f"        hyperRef: "
                f"\"/services/{service_name}/{service_name}.html\"\n"
            )
            file_desc.write("    }")

            if idx == (len(service_list) - 1):
                file_desc.write("\n")
            else:
                file_desc.write(",\n")

        file_desc.write("];\n")


def _generate_cpp_service(
    service_list_full_path: str, service_list: list[str]
) -> None:
    """Generate the service C++ source file.

    Args:
        service_list_full_path: Destination for the generated source file.
        service_list: Service names to include.

    Returns:
        None.
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
        start_calls += "    {\n"
        start_calls += "        isSuccessful = false;\n"
        start_calls += "    }\n"

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


def configure_services(service_list: list[str], layout: str) -> None:
    """Generate service artifacts, removing skipped names from the input list.

    Args:
        service_list: Mutable list of service names.
        layout: Display layout.

    Returns:
        None.
    """
    # Avoid generation if possible, because it will cause a compilation step.
    is_generation_required = False
    service_list_web = service_list.copy()

    if os.path.isdir(_WEB_DATA_PATH) is False:
        os.mkdir(_WEB_DATA_PATH)
        is_generation_required = True

    else:
        # Remove all obsolete services in the web data if there are any.
        if _clean_up_folders(service_list, _WEB_DATA_PATH) is True:
            is_generation_required = True

    skip_list = []
    skip_list_web = []
    for service_name in service_list:
        service_lib_path = _LIB_PATH + "/" + service_name
        service_lib_web_path = service_lib_path + "/web"

        if os.path.isdir(service_lib_path) is False:
            print(f"\tSkipping {service_name}, because "
                  f"{service_lib_path} doesn't exist.")
            skip_list.append(service_name)

        elif os.path.isdir(service_lib_web_path) is False:
            print(f"\tSkipping {service_name}, because "
                  f"{service_lib_web_path} doesn't exist.")
            skip_list_web.append(service_name)

        else:
            invalid_type, changed = sync_web_files(
                service_name, "service", service_lib_path,
                _WEB_DATA_PATH + "/" + service_name, layout
            )
            if invalid_type:
                skip_list.append(service_name)
            if changed:
                is_generation_required = True

    # Remove skipped services from list
    for service_name in skip_list:
        service_list.remove(service_name)

    for service_name in skip_list_web:
        service_list_web.remove(service_name)

    if (is_generation_required is True) or \
        (os.path.exists(_MENU_FULL_PATH) is False) or \
            (os.path.exists(_SERVICE_LIST_FULL_PATH) is False):

        print("\tGenerating services web menu.")
        _generate_web_menu(_MENU_FULL_PATH, service_list_web)

    print("\tGenerating services.")
    _generate_cpp_service(_SERVICE_LIST_FULL_PATH, service_list)

################################################################################
# Main
################################################################################
