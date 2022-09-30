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

class Config():
    """The class provides read/write access to a configuration file.
        A configuration file is in JSON format and supports only a flat
        key/value pair structure.
    """
    def __init__(self) -> None:
        self._config_dict = {}

    def load(self, file_name):
        """Load a configuration from a file in JSON format.

        Args:
            file_name (str): Name of the configuration file

        Returns:
            bool: If successful loaded, it will return True otherwise False.
        """
        status = True

        try:
            with open(file_name, encoding="utf-8") as file_desc:
                self._config_dict = json.load(file_desc)
        except FileNotFoundError:
            status = False

        return status

    def save(self, file_name):
        """Saves a configuration to a file in JSON format.

        Args:
            file_name (str): Name of the configuration file

        Returns:
            bool: If successful saved, it will return True otherwise False.
        """
        status = True

        try:
            with open(file_name, "w", encoding="utf-8") as file_desc:
                json.dump(self._config_dict, file_desc, ensure_ascii=False, indent=4)
        except FileExistsError:
            status = False

        return status

    def get_value_by_key(self, key):
        """Get the value of configuration key.

        Args:
            key (str): Key

        Returns:
            str: Value of the key
        """
        value = None

        if key in self._config_dict:
            value = self._config_dict[key]

        return value

    def set_value_by_key(self, key, value):
        """Set the value for a configuration key.

        Args:
            key (str): Key
            value (str): Value of the key
        """
        self._config_dict[key] = value

################################################################################
# Functions
################################################################################
