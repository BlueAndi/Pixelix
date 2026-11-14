#!/usr/bin/env python3
"""Assemble the Pages installer with verified per-board flash images of all supported releases."""

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
import re
import shutil
import sys
from datetime import datetime, timezone
from pathlib import Path
from typing import Any, Optional

################################################################################
# Variables
################################################################################
INSTALLER_ASSETS = Path("usb-installer/dist")

# Only releases with a major version greater or equal to this one are offered.
MIN_MAJOR_VERSION = 11

CATALOG_SCHEMA = 1

TAG_PATTERN = re.compile(
    r"^v?(?P<major>\d+)\.(?P<minor>\d+)\.(?P<patch>\d+)(?:-(?P<pre>[0-9A-Za-z.-]+))?$"
)

# Tags and environments are used as directory names, therefore restrict them.
SAFE_NAME_PATTERN = re.compile(r"^[0-9A-Za-z._-]+$")

SHA256_PATTERN = re.compile(r"^[0-9A-Fa-f]{64}$")

################################################################################
# Classes
################################################################################

################################################################################
# Functions
################################################################################


def warn(message: str) -> None:
    """Print a warning to stderr.

    Args:
        message: Warning text.

    Returns:
        None.
    """
    print(f"Warning: {message}", file=sys.stderr)


def parse_tag(tag: str) -> Optional[tuple[int, int, int, str]]:
    """Parse a release tag as semantic version.

    Args:
        tag: Release tag, optionally prefixed with "v".

    Returns:
        Tuple of major, minor, patch and pre-release string (empty if none),
        or None if the tag is not a valid version.
    """
    match = TAG_PATTERN.match(tag)
    result: Optional[tuple[int, int, int, str]] = None

    if match is not None:
        result = (
            int(match.group("major")),
            int(match.group("minor")),
            int(match.group("patch")),
            match.group("pre") or "",
        )

    return result


def version_sort_key(tag: str) -> tuple[Any, ...]:
    """Build a semantic version sort key (SemVer 2.0.0 precedence).

    Args:
        tag: Valid release tag.

    Returns:
        Sortable key; a higher key is a newer version.

    Raises:
        ValueError: If the tag is not a valid version.
    """
    parsed = parse_tag(tag)
    if parsed is None:
        raise ValueError(f"Invalid version tag: {tag}")

    major, minor, patch, pre = parsed
    identifiers: list[tuple[int, int, str]] = []

    for identifier in pre.split(".") if pre else []:
        if identifier.isdigit():
            identifiers.append((0, int(identifier), ""))
        else:
            identifiers.append((1, 0, identifier))

    # A version without pre-release suffix is newer than the same one with suffix.
    return (major, minor, patch, 0 if pre else 1, tuple(identifiers))


def tag_to_version(tag: str) -> str:
    """Strip the optional "v" prefix of a tag.

    Args:
        tag: Release tag.

    Returns:
        Version string.
    """
    return tag[1:] if tag.startswith("v") else tag


def select_releases(releases_meta: list[dict[str, Any]]) -> list[dict[str, Any]]:
    """Filter and sort the GitHub releases supported by the installer.

    Args:
        releases_meta: Entries with tagName, isPrerelease, publishedAt and optional isDraft.

    Returns:
        Supported releases, newest first.
    """
    selected: list[dict[str, Any]] = []

    for entry in releases_meta:
        tag = str(entry.get("tagName", ""))
        parsed = parse_tag(tag)

        if entry.get("isDraft", False):
            continue

        if parsed is None or not SAFE_NAME_PATTERN.match(tag):
            warn(f"Skipping release with invalid version tag: {tag!r}")
            continue

        if parsed[0] < MIN_MAJOR_VERSION:
            continue

        selected.append({
            "version": tag_to_version(tag),
            "tag": tag,
            "prerelease": bool(entry.get("isPrerelease", False)),
            "publishedAt": str(entry.get("publishedAt", "")),
        })

    selected.sort(
        key=lambda release: (version_sort_key(release["tag"]), release["publishedAt"]),
        reverse=True,
    )

    return selected


def determine_default(releases: list[dict[str, Any]], default_tag: str) -> dict[str, Any]:
    """Determine the preselected release.

    Args:
        releases: Releases with a valid package, newest first.
        default_tag: Explicitly requested tag, empty for the newest stable release.
            If only pre-releases exist, the newest pre-release is used.

    Returns:
        Catalog entry of the default release.

    Raises:
        ValueError: If the requested tag is unknown or no release with a USB flash package exists.
    """
    if default_tag:
        for release in releases:
            if default_tag == release["tag"]:
                return release

        raise ValueError(f"Default release is unknown: {default_tag}")

    for release in releases:
        if not release["prerelease"]:
            return release

    if releases:
        return releases[0]

    raise ValueError("No release with a USB flash package found.")


def validate_package(manifest: dict[str, Any], manifest_path: Path) -> None:
    """Validate the required fields of a flash package manifest.

    Args:
        manifest: Parsed package manifest.
        manifest_path: Manifest file, used for error messages.

    Returns:
        None.

    Raises:
        ValueError: If a required field is missing or has an invalid value.
    """
    error = f"Invalid flash package manifest: {manifest_path}"
    images = manifest.get("images")

    if (not isinstance(manifest.get("environment"), str)) or (not SAFE_NAME_PATTERN.match(manifest["environment"])):
        raise ValueError(f"{error} (environment)")

    for key in ("chip", "board"):
        if (not isinstance(manifest.get(key), str)) or (not manifest[key]):
            raise ValueError(f"{error} ({key})")

    if (not isinstance(images, list)) or (not images):
        raise ValueError(f"{error} (images)")

    for image in images:
        if not isinstance(image, dict):
            raise ValueError(f"{error} (image)")

        offset = image.get("offset")
        if (not isinstance(image.get("file"), str)) or (not SAFE_NAME_PATTERN.match(image["file"])):
            raise ValueError(f"{error} (image file)")
        if (not isinstance(image.get("sha256"), str)) or (not SHA256_PATTERN.match(image["sha256"])):
            raise ValueError(f"{error} (image sha256)")
        if (not isinstance(offset, int)) or isinstance(offset, bool) or (0 > offset):
            raise ValueError(f"{error} (image offset)")

    if not isinstance(manifest.get("eraseAll"), bool):
        raise ValueError(f"{error} (eraseAll)")

    for key in ("flashMode", "flashFrequency", "flashSize"):
        if (not isinstance(manifest.get(key), str)) or (not manifest[key]):
            raise ValueError(f"{error} ({key})")


def read_package(package_directory: Path) -> dict[str, Any]:
    """Load and validate one flash package manifest.

    Args:
        package_directory: Directory of one board's flash package.

    Returns:
        Package manifest.

    Raises:
        ValueError: If the manifest is missing or invalid.
    """
    manifest_path = package_directory / "manifest.json"
    if not manifest_path.is_file():
        raise ValueError(f"Flash package manifest is missing: {manifest_path}")

    with manifest_path.open("r", encoding="utf-8") as stream:
        manifest = json.load(stream)

    if not isinstance(manifest, dict):
        raise ValueError(f"Invalid flash package manifest: {manifest_path}")

    validate_package(manifest, manifest_path)

    return manifest


def copy_package_images(
    package_directory: Path,
    destination_directory: Path,
    url_prefix: str,
    manifest: dict[str, Any],
) -> None:
    """Copy one target's images and add their Pages-relative URLs.

    Args:
        package_directory: Source directory of the flash package.
        destination_directory: Destination directory of the images.
        url_prefix: Pages-relative URL of the destination directory.
        manifest: Package manifest, updated with the image URLs.

    Returns:
        None.

    Raises:
        FileNotFoundError: If an image is missing.
    """
    for image in manifest["images"]:
        filename = str(image["file"])
        source = package_directory / filename
        if not source.is_file():
            raise FileNotFoundError(
                f"Flash package image is missing: {source}")

        shutil.copyfile(source, destination_directory / filename)
        image["url"] = f"{url_prefix}/{filename}"


def assemble_release(packages_directory: Path, site_directory: Path, release: dict[str, Any]) -> bool:
    """Copy all board images of one release and write its manifest.

    Args:
        packages_directory: Directory with the extracted flash packages of the release.
        site_directory: Pages root.
        release: Catalog entry of the release; the manifest path is added on success.

    Returns:
        True if the release has been added, False if it has no USB flash package.

    Raises:
        ValueError: If a package is invalid or duplicated.
        FileNotFoundError: If an image is missing.
    """
    package_directories = sorted(path for path in packages_directory.glob("usb-flash-*") if path.is_dir())
    if not package_directories:
        return False

    release_url = f"firmware/{release['tag']}"
    targets: list[dict[str, Any]] = []
    target_names: set[str] = set()

    for package_directory in package_directories:
        manifest = read_package(package_directory)
        environment = str(manifest["environment"])

        if environment in target_names:
            raise ValueError(
                f"Duplicate flash package for environment {environment} in {release['tag']}")
        target_names.add(environment)

        destination_directory = site_directory / release_url / environment
        destination_directory.mkdir(parents=True, exist_ok=True)
        copy_package_images(package_directory, destination_directory, f"{release_url}/{environment}", manifest)
        targets.append(manifest)

    index = {
        "version": release["version"],
        "targets": sorted(
            targets,
            key=lambda target: target["environment"].lower(),
        ),
    }

    write_json(site_directory / release_url / "manifest.json", index)
    release["manifest"] = f"{release_url}/manifest.json"

    return True


def write_json(path: Path, data: dict[str, Any]) -> None:
    """Write a JSON file with LF line endings.

    Args:
        path: Destination file.
        data: JSON serializable content.

    Returns:
        None.
    """
    with path.open("w", encoding="utf-8", newline="\n") as stream:
        json.dump(data, stream, indent=2)
        stream.write("\n")


def assemble(
    releases_directory: Path,
    releases_meta: list[dict[str, Any]],
    site_directory: Path,
    default_tag: str,
) -> None:
    """Copy installer assets, all release images and write the release catalog.

    Args:
        releases_directory: Directory with one subdirectory per release tag.
        releases_meta: GitHub release metadata.
        site_directory: Pages root.
        default_tag: Tag to preselect, empty for the newest stable release.

    Returns:
        None.

    Raises:
        FileNotFoundError: If the installer build output or an image is missing.
        ValueError: If a package is invalid or no stable release is available.
    """
    if not INSTALLER_ASSETS.is_dir():
        raise FileNotFoundError(
            f"Installer build output is missing: {INSTALLER_ASSETS}"
        )

    site_directory.mkdir(parents=True, exist_ok=True)
    shutil.copytree(INSTALLER_ASSETS, site_directory, dirs_exist_ok=True)

    releases: list[dict[str, Any]] = []

    for release in select_releases(releases_meta):
        if assemble_release(releases_directory / release["tag"], site_directory, release):
            releases.append(release)
        else:
            warn(f"Skipping release without USB flash packages: {release['tag']}")

    default_release = determine_default(releases, default_tag)
    catalog = {
        "schema": CATALOG_SCHEMA,
        "generated": datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"),
        "default": default_release["version"],
        "defaultTag": default_release["tag"],
        "releases": releases,
    }

    write_json(site_directory / "releases.json", catalog)


def load_releases_meta(path: Path) -> list[dict[str, Any]]:
    """Load the GitHub release metadata.

    Args:
        path: JSON file written by "gh release list --json".

    Returns:
        Release metadata entries.

    Raises:
        ValueError: If the file does not contain a list.
    """
    with path.open("r", encoding="utf-8") as stream:
        data = json.load(stream)

    if not isinstance(data, list):
        raise ValueError(f"Invalid release metadata: {path}")

    return data


def build_parser() -> argparse.ArgumentParser:
    """Build the command-line argument parser.

    Returns:
        Argument parser.
    """
    parser = argparse.ArgumentParser(description=__doc__)
    subparsers = parser.add_subparsers(dest="command", required=True)

    list_tags = subparsers.add_parser("list-tags", help="Print the tags of all supported releases.")
    list_tags.add_argument("--releases-meta", type=Path, required=True)

    assemble_parser = subparsers.add_parser("assemble", help="Assemble the Pages site.")
    assemble_parser.add_argument("--releases-dir", type=Path, required=True)
    assemble_parser.add_argument("--releases-meta", type=Path, required=True)
    assemble_parser.add_argument("--site-dir", type=Path, required=True)
    assemble_parser.add_argument("--default-tag", default="")

    return parser


def main() -> int:
    """Parse arguments and run the requested command.

    Returns:
        Process exit code.
    """
    args = build_parser().parse_args()

    try:
        releases_meta = load_releases_meta(args.releases_meta)

        if "list-tags" == args.command:
            for release in select_releases(releases_meta):
                print(release["tag"])
        else:
            assemble(args.releases_dir, releases_meta, args.site_dir, args.default_tag)
    except (OSError, ValueError, json.JSONDecodeError) as error:
        print(f"USB installer assembly error: {error}", file=sys.stderr)
        return 1

    return 0


################################################################################
# Main
################################################################################
if __name__ == "__main__":
    raise SystemExit(main())
