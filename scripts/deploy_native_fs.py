"""Deploys the filesystem content for the native environment.

On a target the filesystem image is built from the data folder and uploaded
into the flash. The native environment has no flash, therefore the data folder
is copied to the working directory of the application in the build folder
before it is started. The application works inside that directory, so the
files it creates at runtime (plugin configurations, preferences, ...) never
litter the source tree.

Layout of the working directory:

    .pio/build/native/fs/
        data/               Copy of ./data, the filesystem content.
        preferences.txt     Non-volatile storage, see ArduinoNative Preferences.
        imgui.ini           Window layout of the simulator.

Files which the application created at runtime are kept, like the flash of a
target keeps them across a firmware update. Delete the folder to start from
scratch.
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
import os
import shutil

# pylint: disable=undefined-variable
Import("env") # type: ignore

################################################################################
# Variables
################################################################################

# Name of the working directory inside the build folder.
_WORK_DIR_NAME = "fs"

# Name of the filesystem content inside the working directory.
_FS_DIR_NAME = "data"

# Command line option of the application, which selects its working directory.
_WORK_DIR_OPTION = "--work-dir"

################################################################################
# Classes
################################################################################

################################################################################
# Functions
################################################################################

def get_work_dir() -> str:
    """Get the working directory of the native application.

    Returns:
        str: Absolute path of the working directory.
    """
    # pylint: disable=undefined-variable
    return os.path.join(env.subst("$BUILD_DIR"), _WORK_DIR_NAME) # type: ignore

def deploy_filesystem(source, target, env): # pylint: disable=unused-argument,redefined-outer-name
    """Copy the data folder into the working directory of the application.

    Existing files are overwritten, additional files are kept.

    Args:
        source: SCons source nodes, unused.
        target: SCons target nodes, unused.
        env: SCons environment.
    """
    src_dir = env.subst("$PROJECT_DATA_DIR")
    dst_dir = os.path.join(get_work_dir(), _FS_DIR_NAME)

    print(f"Deploy filesystem content from {src_dir} to {dst_dir}")
    shutil.copytree(src_dir, dst_dir, dirs_exist_ok=True)

def add_work_dir_argument() -> None:
    """Pass the working directory to the application via its command line.

    The path is relative to the project folder, which is the current directory
    while PlatformIO runs the application. That way the argument never contains
    a space, even if the project path does.
    """
    # pylint: disable=undefined-variable
    project_dir = env.subst("$PROJECT_DIR") # type: ignore
    rel_dir = os.path.relpath(get_work_dir(), project_dir).replace(os.sep, "/")
    program_args = env.get("PROGRAM_ARGS", "") # type: ignore

    if isinstance(program_args, (list, tuple)):
        program_args = " ".join(program_args)

    program_args = f"{program_args} {_WORK_DIR_OPTION} {rel_dir}".strip()

    env.Replace(PROGRAM_ARGS=program_args) # type: ignore

################################################################################
# Main
################################################################################

add_work_dir_argument()

# The native platform runs the application for both targets.
# pylint: disable=undefined-variable
env.AddPreAction("upload", deploy_filesystem) # type: ignore
env.AddPreAction("exec", deploy_filesystem) # type: ignore
