"""Shared helpers for configuring plugin and service web assets."""

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
from __future__ import annotations

import hashlib
import json
import os
import shutil
from typing import Any

###############################################################################
# Variables
###############################################################################

###############################################################################
# Classes
###############################################################################

###############################################################################
# Functions
###############################################################################


def calculate_path_checksum(paths: list[str]) -> str:
    """Calculate an MD5 checksum of files in the supplied paths.

    Directories are inspected one level deep, and files are ordered by name.

    Args:
        paths: File and directory paths to inspect.

    Returns:
        Hexadecimal MD5 digest of the file contents.
    """
    hasher = hashlib.md5()
    file_list: list[str] = []
    for path in paths:
        if os.path.isdir(path):
            for filename in os.listdir(path):
                file_list.append(os.path.normpath(path + "/" + filename))
        elif os.path.isfile(path):
            file_list.append(path)

    for file_path in sorted(file_list, key=os.path.basename):
        with open(file_path, "rb") as file:
            while chunk := file.read(8192):
                hasher.update(chunk)
    return hasher.hexdigest()


def load_json_file(full_path: str) -> Any:
    """Read an optional JSON file.

    Args:
        full_path: Path of the JSON file.

    Returns:
        Parsed JSON value, or None if the file does not exist.
    """
    try:
        with open(full_path, encoding="utf-8") as json_data:
            return json.load(json_data)
    except FileNotFoundError:
        return None


def clean_up_folders(names: list[str], dst_path: str) -> bool:
    """Remove destination subdirectories not present in the name list.

    Args:
        names: Plugin or service names to retain.
        dst_path: Destination directory.

    Returns:
        Whether an obsolete directory was found.
    """
    is_cleaned_up = False
    if os.path.isdir(dst_path):
        dst_subfolders = [
            name for name in os.listdir(dst_path)
            if os.path.isdir(dst_path + "/" + name)
        ]
        for folder in dst_subfolders:
            if folder not in names:
                dst_full_path = dst_path + "/" + folder
                try:
                    shutil.rmtree(dst_full_path)
                    print(f'\t"{dst_full_path}" removed.')
                except FileNotFoundError:
                    pass
                is_cleaned_up = True
    return is_cleaned_up


def copy_files(src_files: list[str], dst_path: str) -> None:
    """Copy web files into the destination, creating it if necessary.

    Args:
        src_files: Files to copy.
        dst_path: Destination directory.

    Returns:
        None.
    """
    if not os.path.exists(dst_path):
        os.mkdir(dst_path)
    for src_file in src_files:
        print(f"\t\t{src_file} -> {dst_path}", end="")
        try:
            shutil.copy2(src_file, dst_path)
            print("")
        except FileNotFoundError:
            print(" -> file not found!")


def _web_files(
    name: str, kind: str, lib_path: str, layout: str
) -> tuple[list[str], bool]:
    """Find source files and indicate whether the component type is invalid.

    Args:
        name: Component name.
        kind: Expected component type.
        lib_path: Component library directory.
        layout: Requested display layout.

    Returns:
        Source paths and whether the manifest has an incorrect type.
    """
    manifest = load_json_file(lib_path + "/pixelix.json")
    if manifest is None:
        web_path = lib_path + "/web"
        return [web_path + "/" + item for item in os.listdir(web_path)], False

    component = manifest["pixelix"]
    if component["type"] != kind:
        print(
            f"\tSkipping {name}, because its type isn't {kind} "
            "in pixelix.json."
        )
        return [], True
    if component["name"] != name:
        print(
            f"\tSkipping {name}, because {kind} name doesn't match "
            "with pixelix.json."
        )
        return [], False

    files = component["web"]["files"]
    layout_files = []
    for lib_layout in component["web"]["layouts"]:
        if lib_layout["name"] == layout:
            layout_files = lib_layout["files"]
    return [lib_path + "/" + path for path in files + layout_files], False


def sync_web_files(
    name: str, kind: str, lib_path: str, dst_path: str, layout: str
) -> tuple[bool, bool]:
    """Synchronize component files and report invalid type and changes.

    Args:
        name: Component name.
        kind: Expected component type.
        lib_path: Component library directory.
        dst_path: Destination directory.
        layout: Requested display layout.

    Returns:
        Whether the manifest has an incorrect type, and whether files changed.
    """
    checksum = calculate_path_checksum([dst_path])
    files, invalid_type = _web_files(name, kind, lib_path, layout)
    if files and checksum != calculate_path_checksum(files):
        print("\tCopy web data:")
        try:
            shutil.rmtree(dst_path)
        except FileNotFoundError:
            pass
        copy_files(files, dst_path)
        return invalid_type, True
    return invalid_type, False
