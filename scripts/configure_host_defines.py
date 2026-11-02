"""Configures the host specific pre-processor defines."""

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
import platform

################################################################################
# Variables
################################################################################

# pylint: disable=undefined-variable
Import("env") # type: ignore

################################################################################
# Classes
################################################################################

################################################################################
# Functions
################################################################################

def get_host_defines(system_name):
    """Determine the pre-processor defines which the given host system needs.

        PNGdec's bundled zlib declares inflateSetDictionary() with the BSD type
        "uint" in inflate.c. glibc's sys/types.h provides that type, the Windows
        host toolchain does not, therefore it is defined for Windows only.

        It must NOT be defined on glibc: sys/types.h holds "typedef unsigned int
        uint;" under __USE_MISC, which g++ enables by default. The macro would
        rewrite that typedef into "typedef unsigned int unsigned int;" and every
        translation unit which includes sys/types.h would fail to compile.

    Args:
        system_name (str): Host system name, as platform.system() returns it.

    Returns:
        list[tuple[str, str]]: Pre-processor defines as (name, value) tuples.
    """
    defines = []

    if "Windows" == system_name:
        defines.append(("uint", "unsigned int"))

    return defines

################################################################################
# Main
################################################################################

# pylint: disable=undefined-variable
env.Append(CPPDEFINES=get_host_defines(platform.system())) # type: ignore
