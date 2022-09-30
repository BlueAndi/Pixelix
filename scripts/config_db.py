"""
MIT License

Copyright (c) 2019 - 2022 Andreas Merkle <web@blue-andi.de>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

================================================================================
Handles the access to a configuration database file in JSON format.

The configuration database contains a list of items. A item is just a configuration
parameter, which provides different options.

"""

################################################################################
# Imports
################################################################################
import json

################################################################################
# Classes
################################################################################

class ConfigDb():
    """The configuration database provides a API to read from it.
    """
    def __init__(self) -> None:
        self._db = {}

    def load(self, file_name):
        """Load a configuration database file in JSON format.

        Args:
            file_name (str): File name of the configuration database

        Returns:
            bool: If successful loaded, it will return True otherwise False.
        """
        status = True

        try:
            with open(file_name, encoding="utf-8") as file_desc:
                self._db = json.load(file_desc)
        except FileNotFoundError:
            status = False

        return status

    def _find_item_by_name(self, item_name):
        found_item = {}

        if "configItems" in self._db:
            for item in self._db["configItems"]:
                if "name" in item:
                    if item["name"] == item_name:
                        found_item = item
                        break

        return found_item

    def _find_option_by_name(self, item_name, option_name):
        found_option = {}
        item = self._find_item_by_name(item_name)

        if "options" in item:
            for option in item["options"]:
                if "name" in option:
                    if option["name"] == option_name:
                        found_option = option
                        break

        return found_option

    def get_item_names(self):
        """Get a list of the item names.

        Returns:
            list: List of item names
        """
        item_names = []

        if "configItems" in self._db:
            for item in self._db["configItems"]:
                if "name" in item:
                    item_names.append(item["name"])

        return item_names

    def get_item_default_name(self, item_name):
        """Get the default option of a item.

        Args:
            item_name (str): The name of ite

        Returns:
            str: Item default option
        """
        default_option = ""
        item = self._find_item_by_name(item_name)

        if "default" in item:
            default_option = item["default"]

        return default_option

    def get_item_label(self, item_name):
        """Get the item label, which is used in the GUI.

        Args:
            item_name (str): The item name

        Returns:
            str: Item label
        """
        default_option = ""
        item = self._find_item_by_name(item_name)

        if "label" in item:
            default_option = item["label"]

        return default_option

    def get_item_option_names(self, item_name):
        """Get a list of option names which a item provides.

        Args:
            item_name (str): The item name

        Returns:
            list: List of option names
        """
        option_names = []
        item = self._find_item_by_name(item_name)

        if "options" in item:
            for option in item["options"]:
                if "name" in option:
                    option_names.append(option["name"])

        return option_names

    def get_item_option(self, item_name, option_name):
        """Get the option of item and identify the option by its name.

        Args:
            item_name (str): The item name
            option_name (str): The option name

        Returns:
            obj: Item option
        """
        return self._find_option_by_name(item_name, option_name)

################################################################################
# Functions
################################################################################
