"""Topic handler handler."""

# MIT License
#
# Copyright (c) 2019 - 2025 Andreas Merkle (web@blue-andi.de)
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

_TOPIC_HANDLER_LIST_FULL_PATH = "./src/Generated/TopicHandlers.cpp"
_TOPIC_HANDLER_LIST_TEMPLATE_FULL_PATH = "./scripts/TopicHandlers.cpp"

################################################################################
# Classes
################################################################################

################################################################################
# Functions
################################################################################

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

def configure_topic_handlers(topic_handler_list):
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

################################################################################
# Main
################################################################################
