"""Remove unused pre-compiled ESP-IDF libraries from the firmware link."""

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

from typing import Any

################################################################################
# Variables
################################################################################

# pylint: disable=undefined-variable
Import("env")  # type: ignore

# The Arduino framework links all of its pre-compiled ESP-IDF libraries.
# Some of them contain weak C++ standard library template instances, e.g.
# std::string::find(char). If such a library is searched before the C++
# standard library, the linker resolves the symbol there and pulls the whole
# object file in, including its static objects and constructors.
# This happens with Matter, which costs about 110 kB flash and 13 kB RAM,
# although Pixelix doesn't use it.
_UNUSED_LIBS: list[str] = [
    "-lespressif__esp_matter"
]

################################################################################
# Classes
################################################################################

################################################################################
# Functions
################################################################################


def remove_unused_libs(libs: list[Any], unused_libs: list[str]) -> list[Any]:
    """Remove the unused libraries from the list of libraries to link.

    Args:
        libs: Libraries to link. Besides the library flags as strings, it may
            contain SCons nodes of libraries built from source, which are kept.
        unused_libs: Library flags of the libraries, which shall not be linked.

    Returns:
        list[Any]: Libraries to link without the unused libraries.
    """
    return [lib for lib in libs if not (isinstance(lib, str) and lib in unused_libs)]


################################################################################
# Main
################################################################################

env.Replace(LIBS=remove_unused_libs(env.get("LIBS", []), _UNUSED_LIBS))  # type: ignore
