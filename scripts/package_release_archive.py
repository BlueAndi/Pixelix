#!/usr/bin/env python3
"""Repackage existing release firmware artifacts without rebuilding them."""

# The CLI entry points intentionally share a small amount of argument handling
# with package_flash_images.py.
# pylint: disable=duplicate-code

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
import tempfile
import zipfile
from dataclasses import dataclass
from pathlib import Path, PurePosixPath
from typing import Any

from package_flash_images import package_environment, read_platformio_sections

################################################################################
# Variables
################################################################################
FIRMWARE_SUFFIX = "_firmware"
FILESYSTEM_SUFFIX = "_filesystem"
FIRMWARE_IMAGES = ("bootloader.bin", "partitions.bin", "firmware.bin")

################################################################################
# Classes
################################################################################


@dataclass(frozen=True)
class PackageContext:
    """Paths and PlatformIO metadata shared while repackaging targets."""

    sections: dict[str, dict[str, Any]]
    project_directory: Path
    temp_root: Path
    output_directory: Path

################################################################################
# Functions
################################################################################


def normalized_parts(member_name: str) -> tuple[str, ...]:
    """Return safe, non-empty POSIX archive path components."""
    return tuple(
        part
        for part in PurePosixPath(member_name).parts
        if part not in {"", ".", "/"}
    )


def find_artifact_directories(archive: zipfile.ZipFile) -> dict[str, set[str]]:
    """Find release artifact directories for firmware and filesystem images."""
    artifacts: dict[str, set[str]] = {
        "firmware": set(),
        "filesystem": set(),
    }

    for member in archive.namelist():
        for part in normalized_parts(member):
            if part.endswith(FIRMWARE_SUFFIX):
                artifacts["firmware"].add(part)
            elif part.endswith(FILESYSTEM_SUFFIX):
                artifacts["filesystem"].add(part)

    return artifacts


def find_artifact_member(
    archive: zipfile.ZipFile,
    artifact_directory: str,
    filename: str,
) -> str:
    """Find one named image below its release artifact directory."""
    candidates = [
        member
        for member in archive.namelist()
        if (artifact_directory in normalized_parts(member))
        and (PurePosixPath(member).name == filename)
    ]

    if 1 != len(candidates):
        raise ValueError(
            f"Expected one {filename} in {artifact_directory}, found {len(candidates)}"
        )

    return candidates[0]


def copy_archive_image(
    archive: zipfile.ZipFile,
    artifact_directory: str,
    filename: str,
    destination: Path,
) -> None:
    """Copy one named image from the release ZIP into a temporary directory."""
    member_name = find_artifact_member(archive, artifact_directory, filename)
    with archive.open(member_name, "r") as source:
        with destination.open("wb") as output:
            shutil.copyfileobj(source, output)


def release_environments(archive: zipfile.ZipFile) -> list[str]:
    """Return the matching firmware/filesystem targets present in the release."""
    artifacts = find_artifact_directories(archive)
    firmware_environments = {
        artifact.removesuffix(FIRMWARE_SUFFIX)
        for artifact in artifacts["firmware"]
    }
    filesystem_environments = {
        artifact.removesuffix(FILESYSTEM_SUFFIX)
        for artifact in artifacts["filesystem"]
    }

    if not firmware_environments:
        raise ValueError("Release ZIP contains no firmware artifacts")
    if firmware_environments != filesystem_environments:
        raise ValueError(
            "Release ZIP firmware/filesystem target sets do not match"
        )

    return sorted(firmware_environments)


def package_archive_environment(
    archive: zipfile.ZipFile,
    environment: str,
    context: PackageContext,
) -> None:
    """Package one target using the binaries stored in a release archive."""
    config = context.sections.get(f"env:{environment}")
    if config is None:
        raise ValueError(
            f"Release environment is missing from checked-out tag: {environment}"
        )

    firmware_artifact = f"{environment}{FIRMWARE_SUFFIX}"
    filesystem_artifact = f"{environment}{FILESYSTEM_SUFFIX}"
    filesystem_name = str(config["board_build.filesystem"]).lower()
    filesystem_image = (
        "littlefs.bin" if "littlefs" == filesystem_name else "spiffs.bin"
    )
    build_directory = context.temp_root / environment
    build_directory.mkdir(parents=True, exist_ok=True)

    for filename in FIRMWARE_IMAGES:
        copy_archive_image(
            archive,
            firmware_artifact,
            filename,
            build_directory / filename,
        )

    copy_archive_image(
        archive,
        filesystem_artifact,
        filesystem_image,
        build_directory / filesystem_image,
    )

    factory_name = Path(str(config["custom_factory_binary"])).name
    factory_path = build_directory / factory_name
    copy_archive_image(archive, firmware_artifact, factory_name, factory_path)

    config["custom_factory_binary"] = str(factory_path)
    package_environment(
        environment,
        build_directory,
        context.output_directory / f"usb-flash-{environment}",
        context.sections,
        context.project_directory,
    )
    print(f"Packaged {environment}")


def package_release_archive(
    release_archive: Path,
    output_directory: Path,
    project_directory: Path,
    platformio_command: str,
) -> int:
    """Build per-board USB packages from the binaries in an existing release ZIP."""
    sections = read_platformio_sections(platformio_command, project_directory)
    output_directory.mkdir(parents=True, exist_ok=True)

    with zipfile.ZipFile(release_archive, "r") as archive:
        environments = release_environments(archive)

        with tempfile.TemporaryDirectory(prefix="pixelix-release-usb-") as temp_name:
            context = PackageContext(
                sections,
                project_directory,
                Path(temp_name),
                output_directory,
            )
            for environment in environments:
                package_archive_environment(
                    archive,
                    environment,
                    context,
                )

    return len(environments)


def build_parser() -> argparse.ArgumentParser:
    """Build the command-line argument parser."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--release-archive", type=Path, required=True)
    parser.add_argument("--project-dir", type=Path, required=True)
    parser.add_argument("--output-dir", type=Path, required=True)
    parser.add_argument("--platformio", default="platformio")
    return parser


def main() -> int:
    """Parse arguments and repackage an existing release's firmware images."""
    args = build_parser().parse_args()

    try:
        count = package_release_archive(
            args.release_archive,
            args.output_dir,
            args.project_dir,
            args.platformio,
        )
    except (
        OSError,
        ValueError,
        zipfile.BadZipFile,
        json.JSONDecodeError,
    ) as error:
        print(f"Release USB package backfill error: {error}", file=sys.stderr)
        return 1

    print(f"Packaged {count} board targets from {args.release_archive}")
    return 0


################################################################################
# Main
################################################################################
if __name__ == "__main__":
    raise SystemExit(main())
