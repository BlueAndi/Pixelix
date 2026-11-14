"""Configuration model."""

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

import configparser
import re

################################################################################
# Variables
################################################################################

################################################################################
# Classes
################################################################################


class ConfigModel():
    """Loads a configuration and provides necessary access functions.
    """
    def __init__(self) -> None:
        """Initialize an empty library dependency list.

        Args:
            self: Configuration model instance.

        Returns:
            None.
        """
        self._lib_deps: list[str] = []

    def _get_config_section(self, config: configparser.ConfigParser) -> str:
        """Find the config:<type> section in the configuration.

        Args:
            self: Configuration model instance.
            config: Configuration to inspect.

        Returns:
            Section name, or an empty string if no section matches.
        """
        name = ""
        sections = config.sections()

        for section in sections:
            if section.startswith("config:") is True:
                name = section
                break

        return name

    def load(self, full_path: str) -> bool:
        """Load the configuration file.

        Args:
            self: Configuration model instance.
            full_path: Full path to configuration file.

        Returns:
            Whether the configuration was loaded successfully.
        """
        is_successful = True

        try:
            config = configparser.ConfigParser(empty_lines_in_values=True)
            config.read(full_path)
            config_section_name = self._get_config_section(config)
            self._lib_deps = config[config_section_name]["lib_deps"].strip().splitlines()

        except configparser.Error:
            is_successful = False

        return is_successful

    def _get_lib_list(self, postfix: str) -> list[str]:
        """Get list of libraries from library dependencies, which name has the
            given postfix.

        Args:
            self: Configuration model instance.
            postfix: Postfix which identifies the libraries.

        Returns:
            Matching library names.
        """
        lib_list = []
        for lib_name in self._lib_deps:
            result = re.match(f"([a-zA-Z0-9_\\-]*{postfix})[ ]*@.*", lib_name)
            if result:
                lib_list.append(result.group(1))

        return lib_list

    def get_plugin_list(self) -> list[str]:
        """Get list of plugins from library dependencies.

        Args:
            self: Configuration model instance.

        Returns:
            Plugin library names.
        """
        return self._get_lib_list("Plugin")

    def get_service_list(self) -> list[str]:
        """Get list of services from library dependencies.

        Args:
            self: Configuration model instance.

        Returns:
            Service library names.
        """
        return self._get_lib_list("Service")

    def get_topic_handler_list(self) -> list[str]:
        """Get list of topic handlers from library dependencies.

        Args:
            self: Configuration model instance.

        Returns:
            Topic handler library names.
        """
        return self._get_lib_list("TopicHandler")

################################################################################
# Functions
################################################################################

################################################################################
# Main
################################################################################
