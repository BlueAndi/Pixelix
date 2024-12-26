"""Determine the display layout by the build flags."""

# MIT License
#
# Copyright (c) 2019 - 2024 Andreas Merkle (web@blue-andi.de)
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
import re

################################################################################
# Variables
################################################################################

################################################################################
# Classes
################################################################################

################################################################################
# Functions
################################################################################

def determine_layout(build_flags):
    """Determine the display layout by the build flags.
        It assumes that the build flags contain the following pre-processor defines:
        - CONFIG_LED_MATRIX_WIDTH
        - CONFIG_LED_MATRIX_HEIGHT

    Args:
        build_flags (list[str]): Build flags

    Returns:
        str: Display layout
    """
    width = 0
    height = 0

    for build_flag in build_flags:
        match = re.search("-D CONFIG_LED_MATRIX_WIDTH=([0-9]+).*", build_flag)
        if match:
            width = int(match.group(1))

        match = re.search("-D CONFIG_LED_MATRIX_HEIGHT=([0-9]+).*", build_flag)
        if match:
            height = int(match.group(1))

    return f"LAYOUT_{width}X{height}"

################################################################################
# Main
################################################################################
