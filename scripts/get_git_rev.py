"""git related helper scripts, used during build process."""

# MIT License
#
# Copyright (c) 2022 - 2023 Andreas Merkle (web@blue-andi.de)
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
import subprocess
import json

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

def get_git_revision_hash():
    """Get the git revision hash number.

    Returns:
        str: Revision hash
    """
    rev_hash = "0"

    try:
        rev_hash = subprocess.check_output(['git', 'rev-parse', 'HEAD']).decode('ascii').strip()
    except subprocess.CalledProcessError:
        pass

    return rev_hash

def get_git_revision_short_hash():
    """Get the short git revision hash number.

    Returns:
        str: Short revision hash
    """
    rev_short_hash = "0"

    try:
        rev_short_hash = subprocess.check_output(['git', 'rev-parse', '--short', 'HEAD']).decode('ascii').strip()
    except subprocess.CalledProcessError:
        pass

    return rev_short_hash

def get_current_git_branch():
    """Get current git branch name.

    Returns:
        str: Branch name
    """
    branch_name = "unknown"

    try:
        branch_name = subprocess.check_output(['git', 'rev-parse', '--abbrev-ref', 'HEAD']).decode('ascii').strip()
    except subprocess.CalledProcessError:
        pass

    return branch_name

def any_local_change():
    """Checks whether there is any local change in the sources,
    which are under revision control.

    Returns:
        bool: If any local change is available, it will return True otherwise False.
    """
    is_src_changed = False
    result = ""

    try:
        result = subprocess.check_output(['git', 'status', '--porcelain', '--untracked-files=no']).decode('ascii').strip()
    except subprocess.CalledProcessError:
        pass

    if len(result) > 0:
        is_src_changed = True

    return is_src_changed

def any_unpushed_commit(upstream):
    """Checks whether there is any commit, which is not pushed to the remote repository.

    Args:
        upstream (str): Upstream repository path

    Returns:
        bool: If there is any unpushed commit, it will return True otherwise False.
    """
    is_unpushed_commit_available = False
    result = ""

    try:
        result = subprocess.check_output(['git', 'cherry', '-v', upstream], stderr=subprocess.STDOUT).decode('ascii').strip()
    except subprocess.CalledProcessError:
        pass

    if len(result) > 0:
        is_unpushed_commit_available = True

    return is_unpushed_commit_available

def get_version():
    """Get the version identification from the definition file.

    Returns:
        str: Version
    """
    version_filename = "./data/version.json"
    version = "vX.X.X"
    branch_name = get_current_git_branch()

    try:
        with open(version_filename, encoding="utf-8") as json_file:
            data = json.load(json_file)
            version = data["version"]

        if True is any_unpushed_commit("origin/" + branch_name):
            version = version + ":uc"

        if True is any_local_change():
            version = version + ":lc"

        if branch_name != "master":
            version = version + ":b"

    except FileNotFoundError:
        pass

    return version

################################################################################
# Main
################################################################################

SOFTWARE_VERSION        = "-DSW_VERSION=" + get_version()
SOFTWARE_REVISION       = "-DSW_REV=" + get_git_revision_hash()
SOFTWARE_REVISION_SHORT = "-DSW_REV_SHORT=" + get_git_revision_short_hash()
SOFTWARE_BRANCH         = "-DSW_BRANCH=" + get_current_git_branch()

# pylint: disable=undefined-variable
env.Append(BUILD_FLAGS=[ SOFTWARE_VERSION, SOFTWARE_REVISION, SOFTWARE_REVISION_SHORT, SOFTWARE_BRANCH ]) # type: ignore

print("Software version       : " + get_version())
print("Software revision long : " + get_git_revision_hash())
print("Software revision short: " + get_git_revision_short_hash())
print("Software branch        : " + get_current_git_branch())
