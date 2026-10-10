"""Link the platform specific socket library.

The network of the host is based on sockets, see SocketCompat.hpp. On
Windows they are provided by winsock, which must be linked explicitly. On Linux
the sockets are part of the C library, therefore nothing has to be linked.

Author: Andreas Merkle <web@blue-andi.de>
"""

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
import sys

Import("env") # pylint: disable=undefined-variable

################################################################################
# Variables
################################################################################

################################################################################
# Classes
################################################################################

################################################################################
# Functions
################################################################################

def _link_socket_library(build_env):
    """Add the socket library of the host to the linker.

    Args:
        build_env (SCons.Environment): The library build environment.
    """
    if sys.platform.startswith("win"):
        # The library is appended to the linker command of the whole program,
        # not only of this library, therefore the global environment is used.
        build_env.Append(LIBS=["ws2_32"])

################################################################################
# Main
################################################################################

_link_socket_library(env) # pylint: disable=undefined-variable
