"""Input validators and identity helpers of the Pixelix integration."""

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
from collections.abc import Callable

# Voluptuous is provided by the HA runtime only.
# pylint: disable=import-error
import voluptuous as vol

###############################################################################
# Variables
###############################################################################

###############################################################################
# Classes
###############################################################################

###############################################################################
# Functions
###############################################################################


def normalize_name(name: str) -> str:
    """Normalize the host name of a device to get its unique id.

    The name which a device announces by mDNS and the name which it reports
    by its REST API differ at most in the case and a ``.local`` suffix.

    Args:
        name: The host name of the device.

    Returns:
        The name in lower case without a ``.local`` suffix.
    """
    return name.strip().lower().removesuffix(".local")


def max_bytes(limit: int) -> Callable[[str], str]:
    """Create a validator which limits the length of a string in bytes.

    The Pixelix firmware measures text and melody in bytes of the UTF-8
    encoding, not in characters. A too long value is rejected, never cut.

    Args:
        limit: The maximum number of bytes.

    Returns:
        The validator, which returns the value if it is short enough.
    """

    def validate(value: str) -> str:
        """Check the length of a string.

        Args:
            value: The string to check.

        Returns:
            The unchanged string.

        Raises:
            vol.Invalid: The string needs more than the allowed bytes.
        """
        length = len(value.encode("utf-8"))
        if length > limit:
            raise vol.Invalid(
                f"must not be longer than {limit} bytes in UTF-8, "
                f"but has {length} bytes"
            )
        return value

    return validate
