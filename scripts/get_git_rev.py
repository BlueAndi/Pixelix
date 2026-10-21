"""git related helper scripts, used during build process."""

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
import subprocess
import json
import os

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

def run_git(args):
    """Run a git command and return its output.

    Args:
        args (list): Git arguments, without the leading 'git'

    Returns:
        str: Stripped output or None if the command failed or git is not available.
    """
    output = None

    try:
        output = subprocess.check_output(['git'] + args, stderr=subprocess.DEVNULL).decode('utf-8', errors='replace').strip()
    except (subprocess.CalledProcessError, OSError):
        pass

    return output

def get_git_revision_hash():
    """Get the git revision hash number.

    Returns:
        str: Revision hash
    """
    rev_hash = run_git(['rev-parse', 'HEAD'])

    if rev_hash is None:
        rev_hash = "0"

    return rev_hash

def get_git_revision_short_hash():
    """Get the short git revision hash number.

    Returns:
        str: Short revision hash
    """
    rev_short_hash = run_git(['rev-parse', '--short', 'HEAD'])

    if rev_short_hash is None:
        rev_short_hash = "0"

    return rev_short_hash

def is_detached_head():
    """Checks whether HEAD is detached, i.e. no branch is checked out.

    Returns:
        bool: If HEAD is detached, it will return True otherwise False.
    """
    # Fails with exit code 1 if HEAD is detached.
    return run_git(['symbolic-ref', '--quiet', 'HEAD']) is None

def get_detached_branch_name():
    """Get a branch name for a detached HEAD.
    A local or remote branch pointing to HEAD is preferred, then the
    branch name provided by a CI environment.

    Returns:
        str: Branch name or "detached" if no branch can be determined.
    """
    branch_name = "detached"
    candidates = []

    # Local branches "refs/heads/<name>" and remote branches "refs/remotes/<remote>/<name>".
    for refs_path, strip_count in [('refs/heads', 2), ('refs/remotes', 3)]:
        refs = run_git(['for-each-ref', '--points-at', 'HEAD', f'--format=%(refname:lstrip={strip_count})', refs_path])

        if refs is not None:
            candidates += [ref for ref in refs.splitlines() if ref not in ("", "HEAD")]

    if "master" in candidates:
        branch_name = "master"
    elif len(candidates) > 0:
        branch_name = candidates[0]
    else:
        # GitHub Actions checks out pull requests and tags detached.
        for env_name in ['GITHUB_HEAD_REF', 'GITHUB_REF_NAME']:
            env_value = os.environ.get(env_name, "")

            if len(env_value) > 0:
                branch_name = env_value
                break

    return branch_name

def get_current_git_branch():
    """Get current git branch name.

    Returns:
        str: Branch name
    """
    branch_name = "unknown"

    if is_detached_head() is True:
        if run_git(['rev-parse', 'HEAD']) is not None:
            branch_name = get_detached_branch_name()
    else:
        result = run_git(['rev-parse', '--abbrev-ref', 'HEAD'])

        if result is not None:
            branch_name = result

    return branch_name

def any_local_change():
    """Checks whether there is any local change in the sources,
    which are under revision control.

    Returns:
        bool: If any local change is available, it will return True otherwise False.
    """
    result = run_git(['status', '--porcelain', '--untracked-files=no'])

    return (result is not None) and (len(result) > 0)

def any_unpushed_commit(upstream):
    """Checks whether there is any commit, which is not pushed to the remote repository.
    If HEAD is detached, it checks whether HEAD is reachable from any remote branch.

    Args:
        upstream (str): Upstream repository path

    Returns:
        bool: If there is any unpushed commit, it will return True otherwise False.
    """
    if is_detached_head() is True:
        result = run_git(['rev-list', '--count', 'HEAD', '--not', '--remotes'])
        is_unpushed_commit_available = (result is not None) and (result != "0")
    else:
        result = run_git(['cherry', '-v', upstream])
        is_unpushed_commit_available = (result is not None) and (len(result) > 0)

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
