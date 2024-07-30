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
from string import Template

################################################################################
# Variables
################################################################################
_LIB_PATH = "./lib"

_SERVICE_LIST_FULL_PATH = "./src/Generated/Services.cpp"
_SERVICE_LIST_TEMPLATE_FULL_PATH = "./scripts/Services.cpp"

################################################################################
# Classes
################################################################################

################################################################################
# Functions
################################################################################

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

def configure_services(service_list):
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

################################################################################
# Main
################################################################################
