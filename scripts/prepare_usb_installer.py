#!/usr/bin/env python3
"""Assemble the Pages installer and verified per-board flash images."""

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
import argparse
import json
import shutil
import sys
from pathlib import Path
from typing import Any

################################################################################
# Variables
################################################################################
INSTALLER_ASSETS = Path("usb-installer/dist")

################################################################################
# Classes
################################################################################

################################################################################
# Functions
################################################################################


def read_package(package_directory: Path) -> dict[str, Any]:
    """Load and validate one flash package manifest."""
    manifest_path = package_directory / "manifest.json"
    with manifest_path.open("r", encoding="utf-8") as stream:
        manifest = json.load(stream)

    if (not manifest.get("environment")) or (not manifest.get("images")):
        raise ValueError(f"Invalid flash package manifest: {manifest_path}")

    return manifest


def copy_package_images(
    package_directory: Path,
    destination_directory: Path,
    manifest: dict[str, Any],
) -> None:
    """Copy one target's images and add their Pages-relative URLs."""
    for image in manifest["images"]:
        filename = str(image["file"])
        source = package_directory / filename
        if not source.is_file():
            raise FileNotFoundError(
                f"Flash package image is missing: {source}")

        shutil.copyfile(source, destination_directory / filename)
        image["url"] = f"firmware/{manifest['environment']}/{filename}"


def assemble(packages_directory: Path, site_directory: Path, version: str) -> None:
    """Copy installer assets and all board images into the Pages root."""
    if not INSTALLER_ASSETS.is_dir():
        raise FileNotFoundError(
            f"Installer build output is missing: {INSTALLER_ASSETS}"
        )

    site_directory.mkdir(parents=True, exist_ok=True)
    shutil.copytree(INSTALLER_ASSETS, site_directory, dirs_exist_ok=True)

    targets: list[dict[str, Any]] = []
    target_names: set[str] = set()
    package_manifests = sorted(
        packages_directory.glob("usb-flash-*/manifest.json")
    )

    for manifest_path in package_manifests:
        package_directory = manifest_path.parent
        manifest = read_package(package_directory)
        environment = str(manifest["environment"])

        if environment in target_names:
            raise ValueError(
                f"Duplicate flash package for environment: {environment}")
        target_names.add(environment)

        destination_directory = site_directory / "firmware" / environment
        destination_directory.mkdir(parents=True, exist_ok=True)
        copy_package_images(package_directory, destination_directory, manifest)
        targets.append(manifest)

    if not targets:
        raise ValueError(f"No USB flash packages found in {packages_directory}")

    index = {
        "version": version,
        "targets": sorted(
            targets,
            key=lambda target: target["environment"].lower(),
        ),
    }

    manifest_path = site_directory / "firmware-manifest.json"
    with manifest_path.open("w", encoding="utf-8", newline="\n") as stream:
        json.dump(index, stream, indent=2)
        stream.write("\n")


def build_parser() -> argparse.ArgumentParser:
    """Build the command-line argument parser."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--packages-dir", type=Path, required=True)
    parser.add_argument("--site-dir", type=Path, required=True)
    parser.add_argument("--version", required=True)
    return parser


def main() -> int:
    """Parse arguments and assemble the Pages deployment."""
    args = build_parser().parse_args()

    try:
        assemble(args.packages_dir, args.site_dir, args.version)
    except (OSError, ValueError, json.JSONDecodeError) as error:
        print(f"USB installer assembly error: {error}", file=sys.stderr)
        return 1

    return 0


################################################################################
# Main
################################################################################
if __name__ == "__main__":
    raise SystemExit(main())
