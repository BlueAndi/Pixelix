"""Short description of the module, one sentence."""

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
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

###############################################################################
# Imports
###############################################################################
import logging
from typing import Any

# Home Assistant is provided by the HA runtime only.
# pylint: disable=import-error
from homeassistant.core import HomeAssistant

from .const import DOMAIN

###############################################################################
# Variables
###############################################################################
_LOGGER = logging.getLogger(__name__)

###############################################################################
# Classes
###############################################################################


class ExampleError(Exception):
    """Base class of all errors of this module."""


class Example:
    """Short description of what the class represents."""

    def __init__(self, hass: HomeAssistant, name: str) -> None:
        """Create the example.

        Args:
            hass: The Home Assistant instance.
            name: The name of the device.

        Returns:
            None.
        """
        self._hass = hass
        self._name = name

    @property
    def name(self) -> str:
        """Get the name of the device.

        Returns:
            The name of the device.
        """
        return self._name

    async def async_do_something(self, value: int) -> dict[str, Any]:
        """Do something with the device.

        Args:
            value: The value to send to the device.

        Returns:
            The answer of the device.

        Raises:
            ExampleError: The device did not answer or rejected the value.
        """
        _LOGGER.debug("%s: Sending %d.", self._name, value)

        return {"domain": DOMAIN, "value": value}


###############################################################################
# Functions
###############################################################################


def example_helper(text: str) -> str:
    """Short description of what the helper does.

    Args:
        text: The text to process.

    Returns:
        The processed text.
    """
    return text.strip()
