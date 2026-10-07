#!/usr/bin/env python3
"""Package complete USB flash images and resolved board metadata."""

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
import csv
import hashlib
import json
import shutil
import subprocess
import sys
from pathlib import Path
from typing import Any

################################################################################
# Variables
################################################################################
PARTITION_TABLE_OFFSET = 0x8000
FILESYSTEM_PARTITION_NAME = "spiffs"

################################################################################
# Classes
################################################################################

################################################################################
# Functions
################################################################################


def read_platformio_sections(
    platformio_command: str,
    project_directory: Path | None = None,
) -> dict[str, dict[str, Any]]:
    """Return PlatformIO's resolved project configuration sections."""
    result = subprocess.run(
        [platformio_command, "project", "config", "--json-output"],
        check=True,
        capture_output=True,
        text=True,
        cwd=project_directory,
    )
    raw_sections = json.loads(result.stdout)
    return {section_name: dict(entries) for section_name, entries in raw_sections}


def read_partitions(partition_file: Path) -> dict[str, dict[str, int]]:
    """Read named partitions from an ESP-IDF CSV partition table."""
    partitions: dict[str, dict[str, int]] = {}

    with partition_file.open("r", encoding="utf-8", newline="") as stream:
        for row in csv.reader(stream):
            fields = [field.strip() for field in row]
            if (not fields) or fields[0].startswith("#") or (5 > len(fields)):
                continue

            partitions[fields[0]] = {
                "offset": int(fields[3], 0),
                "size": int(fields[4], 0),
            }

    return partitions


def chip_family(board_id: str) -> str:
    """Map a configured PlatformIO board ID to its ESP32 ROM family."""
    normalized_board_id = board_id.lower().replace("_", "-")

    if "s3" in normalized_board_id:
        family = "ESP32-S3"
    elif "s2" in normalized_board_id:
        family = "ESP32-S2"
    else:
        family = "ESP32"

    return family


def sha256_file(path: Path) -> str:
    """Compute a file's SHA-256 digest using bounded memory."""
    digest = hashlib.sha256()

    with path.open("rb") as stream:
        while True:
            chunk = stream.read(1024 * 1024)
            if not chunk:
                break
            digest.update(chunk)

    return digest.hexdigest()


def get_image_sources(
    config: dict[str, Any],
    build_directory: Path,
    project_directory: Path,
) -> tuple[str, str, list[tuple[str, Path, int]]]:
    """Resolve board family, display label, and flash image sources."""
    partition_file = project_directory / str(config["board_build.partitions"])
    partitions = read_partitions(partition_file)
    factory_partition = partitions.get("factory")
    filesystem_partition = partitions.get(FILESYSTEM_PARTITION_NAME)

    if (factory_partition is None) or (filesystem_partition is None):
        raise ValueError("Required factory/filesystem partitions are missing")

    board_id = str(config["board"])
    family = chip_family(board_id)
    bootloader_offset = 0x0 if ("ESP32-S3" == family) else 0x1000
    firmware_offset = int(str(config["board_upload.offset_address"]), 0)
    filesystem_name = str(config["board_build.filesystem"]).lower()
    filesystem_image = (
        "littlefs.bin" if ("littlefs" == filesystem_name) else "spiffs.bin"
    )
    factory_image = project_directory / str(config["custom_factory_binary"])
    image_sources = [
        ("bootloader.bin", build_directory / "bootloader.bin", bootloader_offset),
        ("partitions.bin", build_directory /
         "partitions.bin", PARTITION_TABLE_OFFSET),
        ("factory.bin", factory_image, factory_partition["offset"]),
        ("firmware.bin", build_directory / "firmware.bin", firmware_offset),
        (
            filesystem_image,
            build_directory / filesystem_image,
            filesystem_partition["offset"],
        ),
    ]

    return board_id, family, image_sources


def write_manifest(
    output_directory: Path,
    environment: str,
    board_id: str,
    family: str,
    images: list[dict[str, Any]],
) -> None:
    """Write a target's board metadata and image hashes as JSON."""
    manifest = {
        "environment": environment,
        "board": board_id,
        "chip": family,
        "flashMode": "keep",
        "flashFrequency": "keep",
        "flashSize": "detect",
        "eraseAll": True,
        "images": images,
    }

    manifest_path = output_directory / "manifest.json"
    with manifest_path.open("w", encoding="utf-8", newline="\n") as stream:
        json.dump(manifest, stream, indent=2)
        stream.write("\n")


def package_environment(
    environment: str,
    build_directory: Path,
    output_directory: Path,
    sections: dict[str, dict[str, Any]],
    project_directory: Path | None = None,
) -> None:
    """Copy build images and write their exact flash plan and hashes."""
    config = sections.get(f"env:{environment}")
    if config is None:
        raise ValueError(f"Unknown PlatformIO environment: {environment}")

    if project_directory is None:
        project_directory = Path.cwd()

    board_id, family, image_sources = get_image_sources(
        config, build_directory, project_directory)

    output_directory.mkdir(parents=True, exist_ok=True)
    images: list[dict[str, Any]] = []

    for filename, source, offset in image_sources:
        if not source.is_file():
            raise FileNotFoundError(
                f"Required flash image is missing: {source}")

        destination = output_directory / filename
        shutil.copyfile(source, destination)
        images.append(
            {
                "file": filename,
                "offset": offset,
                "size": destination.stat().st_size,
                "sha256": sha256_file(destination),
            }
        )

    write_manifest(output_directory, environment, board_id, family, images)


def build_parser() -> argparse.ArgumentParser:
    """Build the command-line argument parser."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--environment", required=True)
    parser.add_argument("--build-dir", type=Path, required=True)
    parser.add_argument("--output-dir", type=Path, required=True)
    parser.add_argument("--platformio", default="platformio")
    return parser


def main() -> int:
    """Parse arguments and package one PlatformIO target."""
    args = build_parser().parse_args()

    try:
        sections = read_platformio_sections(args.platformio)
        package_environment(
            args.environment,
            args.build_dir,
            args.output_dir,
            sections,
        )
    except (OSError, ValueError, subprocess.CalledProcessError, json.JSONDecodeError) as error:
        print(f"USB flash packaging error: {error}", file=sys.stderr)
        return 1

    return 0


################################################################################
# Main
################################################################################
if __name__ == "__main__":
    raise SystemExit(main())
