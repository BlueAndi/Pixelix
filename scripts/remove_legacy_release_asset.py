#!/usr/bin/env python3
"""Remove the obsolete firmware archive from a release after replacement."""

# MIT License
#
# Copyright (c) 2026 Andreas Merkle
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
import argparse
import json
import subprocess


###############################################################################
# Variables
###############################################################################

###############################################################################
# Classes
###############################################################################

###############################################################################
# Functions
###############################################################################


def remove_legacy_asset(repository: str, tag: str) -> None:
    """Remove only the exact old archive; propagate GitHub CLI failures.

    Args:
        repository: GitHub repository in owner/name form.
        tag: Release tag whose obsolete asset should be removed.

    Raises:
        subprocess.CalledProcessError: The GitHub CLI request fails.
        ValueError: The GitHub CLI response is invalid JSON.
    """
    result = subprocess.run(
        ["gh", "release", "view", tag, "--repo", repository,
         "--json", "assets"],
        check=True, capture_output=True, text=True
    )
    assets = json.loads(result.stdout)["assets"]
    old_name = f"{repository.rsplit('/', 1)[-1]}-{tag}.zip"
    if any(asset["name"] == old_name for asset in assets):
        subprocess.run(
            ["gh", "release", "delete-asset", tag, old_name,
             "--repo", repository, "--yes"],
            check=True
        )
    else:
        print(f"No obsolete firmware archive found: {old_name}")


def main() -> None:
    """Parse release arguments."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--repository", required=True)
    parser.add_argument("--tag", required=True)
    args = parser.parse_args()
    remove_legacy_asset(args.repository, args.tag)


###############################################################################
# Main
###############################################################################

if __name__ == "__main__":
    main()
