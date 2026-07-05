"""Configures the plugins."""

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
import os

from config_model import ConfigModel
from configure_plugins import configure_plugins
from configure_services import configure_services
from configure_topic_handlers import configure_topic_handlers

################################################################################
# Variables
################################################################################
_MENU_BASE_FULL_PATH = "./scripts/menu_base.js"
_PLUGIN_SUB_MENU_FULL_PATH = "./scripts/pluginsSubMenu.js"
_SERVICE_SUB_MENU_FULL_PATH = "./scripts/servicesSubMenu.js"
_APP_MENU_FULL_PATH = "./data/js/app-menu.js"

################################################################################
# Classes
################################################################################

################################################################################
# Functions
################################################################################


def _generate_app_menu():
    """Generate app-menu.js from static and generated menu fragments."""
    source_files = [
        _MENU_BASE_FULL_PATH,
        _PLUGIN_SUB_MENU_FULL_PATH,
        _SERVICE_SUB_MENU_FULL_PATH
    ]
    fragments = []

    for source_file in source_files:
        if os.path.exists(source_file) is False:
            raise FileNotFoundError(f"Missing menu source file: {source_file}")

        with open(source_file, "r", encoding="utf-8") as file_desc:
            fragments.append(file_desc.read().strip())

    app_menu = "\n\n".join(fragments).strip() + "\n"

    with open(_APP_MENU_FULL_PATH, "w", encoding="utf-8") as file_desc:
        file_desc.write(app_menu)


def configure(config_full_path, layout):
    """Configures the plugins according to configuration.

    Args:
        config_full_path (str): Full path to configuration file.
        layout (str): The display layout.
    """
    print(
        f"Configure plugins and services: {config_full_path} with layout {layout}")

    # Load configuration
    config_model = ConfigModel()

    if config_model.load(config_full_path) is False:
        print(f"Configuration file {config_full_path} is invalid.")
    else:
        plugin_list = config_model.get_plugin_list()
        service_list = config_model.get_service_list()
        topic_handler_list = config_model.get_topic_handler_list()

        configure_plugins(plugin_list, layout)
        configure_services(service_list, layout)
        _generate_app_menu()
        configure_topic_handlers(topic_handler_list)

################################################################################
# Main
################################################################################
