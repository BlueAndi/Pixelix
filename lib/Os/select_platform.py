"""Select the operating system abstraction implementation.

The Os library provides the very same interfaces (Mutex, MutexRecursive,
CriticalSection, Queue and Task) for two platforms:

* src/esp32   - Implementation on top of the ESP32 freeRTOS port.
* src/native  - Implementation on top of the C++ standard library, used to run
                and test the software on the host.

Both provide equally named headers, therefore only the include path of the
matching implementation is added. Platformio re-exports the include path which
a library adds to its own build environment to every dependent library, so a
library which declares a dependency to "Os" finds the right implementation
without knowing about the platform.

Author: Andreas Merkle <web@blue-andi.de>
"""

# MIT License
#
# Copyright (c) 2019 - 2026 Andreas Merkle <web@blue-andi.de>
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

Import("env") # pylint: disable=undefined-variable

################################################################################
# Variables
################################################################################

# Platform name of the host.
NATIVE_PLATFORM = "native"

# Sub-directory with the implementation for the host.
NATIVE_SUB_DIR = "native"

# Sub-directory with the implementation for the ESP32 target.
ESP32_SUB_DIR = "esp32"

################################################################################
# Classes
################################################################################

################################################################################
# Functions
################################################################################

def _select_implementation(build_env):
    """Add the include path of the implementation which fits to the platform.

    Args:
        build_env (SCons.Environment): The library build environment.
    """
    if build_env["PIOPLATFORM"] == NATIVE_PLATFORM:
        sub_dir = NATIVE_SUB_DIR
    else:
        sub_dir = ESP32_SUB_DIR

    # The current working directory is the library root directory.
    build_env.Append(CPPPATH=[os.path.join(os.getcwd(), "src", sub_dir)])

################################################################################
# Main
################################################################################

_select_implementation(env) # pylint: disable=undefined-variable
