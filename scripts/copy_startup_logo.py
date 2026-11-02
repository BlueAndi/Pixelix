"""Copy the startup logo which fits to the display layout."""

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
import os
import re
import shutil

################################################################################
# Variables
################################################################################
_LOGO_SRC_PATH = "./doc/images/startup"
_LOGO_DST_PATH = "./data/images"
_LOGO_FILE_NAME = "LogoStartup.gif"
_LAYOUT_PATTERN = r"LAYOUT_([0-9]+)X([0-9]+)"

################################################################################
# Classes
################################################################################

################################################################################
# Functions
################################################################################


def copy_startup_logo(layout):
    """Copy the startup logo which fits to the display layout.

    The logo is not scaled by the firmware, therefore every display layout
    needs its own one. It is a generated part of the filesystem image and
    not under version control.

    If no logo exists for the layout, a logo left over from another layout is
    removed. Otherwise it would be shown cropped. Without a logo the firmware
    falls back to the plain text one.

    Args:
        layout (str): Display layout, e.g. "LAYOUT_32X8".

    Returns:
        bool: If a logo was copied, it will return True otherwise False.
    """
    is_successful = False
    match = re.fullmatch(_LAYOUT_PATTERN, layout)
    dst_full_path = os.path.join(_LOGO_DST_PATH, _LOGO_FILE_NAME)

    if match is None:
        print(f"Startup logo: display layout {layout} is unknown.")
    else:
        size = f"{match.group(1)}x{match.group(2)}"
        src_full_path = os.path.join(_LOGO_SRC_PATH, size, _LOGO_FILE_NAME)

        if os.path.exists(src_full_path) is False:
            print(f"Startup logo: none available for {size}, expected {src_full_path}.")

            if os.path.exists(dst_full_path) is True:
                os.remove(dst_full_path)

        else:
            os.makedirs(_LOGO_DST_PATH, exist_ok=True)
            shutil.copyfile(src_full_path, dst_full_path)

            print(f"Startup logo: {size} copied to {dst_full_path}.")

            is_successful = True

    return is_successful

################################################################################
# Main
################################################################################
