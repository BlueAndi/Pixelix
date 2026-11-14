"""Tests of the USB installer assembly script."""

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
import json
from pathlib import Path
from typing import Any

import pytest

import prepare_usb_installer as pui

################################################################################
# Variables
################################################################################

################################################################################
# Classes
################################################################################

################################################################################
# Functions
################################################################################


def meta(tag: str, prerelease: bool = False, published: str = "2026-01-01T00:00:00Z") -> dict[str, Any]:
    """Create one release metadata entry.

    Args:
        tag: Release tag.
        prerelease: Whether the release is a prerelease.
        published: Publication timestamp.

    Returns:
        Release metadata.
    """
    return {"tagName": tag, "isPrerelease": prerelease, "publishedAt": published}


def create_package(releases_dir: Path, tag: str, environment: str, with_image: bool = True) -> None:
    """Create one extracted flash package of a release.

    Args:
        releases_dir: Release package root.
        tag: Release tag.
        environment: Board environment name.
        with_image: Whether to create the image referenced by the manifest.

    Returns:
        None.
    """
    package = releases_dir / tag / f"usb-flash-{environment}"
    package.mkdir(parents=True)
    (package / "manifest.json").write_text(json.dumps({
        "environment": environment,
        "chip": "ESP32",
        "board": "board",
        "eraseAll": True,
        "flashMode": "keep",
        "flashFrequency": "keep",
        "flashSize": "detect",
        "images": [{"file": "firmware.bin", "sha256": "0" * 64, "offset": 0}],
    }), encoding="utf-8")
    if with_image:
        (package / "firmware.bin").write_bytes(b"fw")


################################################################################
# Tests
################################################################################


@pytest.mark.parametrize("tag,expected", [
    ("11.0.0", True), ("v11.1.0", True), ("11.0.0-rc.1", True), ("12.3.4", True),
    ("10.9.9", False), ("9.0.0", False),
])
def test_minimum_version(tag: str, expected: bool) -> None:
    """Only versions with major >= 11 are selected.

    Args:
        tag: Release tag under test.
        expected: Whether the release should be selected.

    Returns:
        None.
    """
    assert expected == bool(pui.select_releases([meta(tag)]))


def test_invalid_tags_and_drafts_are_skipped() -> None:
    """Invalid tags, path injection and drafts are dropped.

    Returns:
        None.
    """
    entries = [meta("latest"), meta("../11.0.0"), meta("11.0"), {**meta("11.0.1"), "isDraft": True}]
    assert not pui.select_releases(entries)


def test_sorting_and_prerelease_flag() -> None:
    """Releases are sorted by SemVer; the flag comes from the metadata.

    Returns:
        None.
    """
    entries = [
        meta("11.2.0"), meta("11.3.0-rc.2", True), meta("11.3.0-rc.10", True),
        meta("11.3.0-beta", True), meta("11.10.0"), meta("11.3.0-rc.1", True),
    ]
    selected = pui.select_releases(entries)
    assert ["11.10.0", "11.3.0-rc.10", "11.3.0-rc.2", "11.3.0-rc.1", "11.3.0-beta", "11.2.0"] == \
        [release["version"] for release in selected]
    assert [False, True, True, True, True, False] == [release["prerelease"] for release in selected]


def test_stable_is_newer_than_its_prerelease() -> None:
    """A stable version ranks above its own pre-releases.

    Returns:
        None.
    """
    selected = pui.select_releases([meta("11.1.0-rc.1", True), meta("11.1.0")])
    assert "11.1.0" == selected[0]["version"]


def test_default_ignores_newer_prerelease() -> None:
    """The default is the newest stable release.

    Returns:
        None.
    """
    releases = pui.select_releases([meta("11.3.0-rc.1", True), meta("11.2.0"), meta("11.1.0")])
    assert "11.2.0" == pui.determine_default(releases, "")["version"]


def test_default_explicit_tag() -> None:
    """An explicit tag is accepted, including a pre-release; an unknown tag is rejected.

    Returns:
        None.
    """
    releases = pui.select_releases([meta("11.3.0-rc.1", True), meta("11.2.0"), meta("11.1.0")])
    assert "11.1.0" == pui.determine_default(releases, "11.1.0")["tag"]
    assert "11.3.0-rc.1" == pui.determine_default(releases, "11.3.0-rc.1")["tag"]
    with pytest.raises(ValueError):
        pui.determine_default(releases, "11.9.9")


def test_default_falls_back_to_prerelease() -> None:
    """Without a stable release the newest pre-release is the default.

    Returns:
        None.
    """
    releases = pui.select_releases([meta("11.3.0-rc.1", True), meta("11.3.0-rc.2", True)])
    assert "11.3.0-rc.2" == pui.determine_default(releases, "")["tag"]


def test_default_requires_release() -> None:
    """Without any release there is no default.

    Returns:
        None.
    """
    with pytest.raises(ValueError):
        pui.determine_default([], "")


def test_assemble(tmp_path: Path, monkeypatch: pytest.MonkeyPatch) -> None:
    """The site contains a catalog and per-release manifests and images.

    Args:
        tmp_path: Temporary test directory.
        monkeypatch: Pytest fixture used to replace the installer assets path.

    Returns:
        None.
    """
    dist = tmp_path / "dist"
    dist.mkdir()
    (dist / "index.html").write_text("x", encoding="utf-8")
    monkeypatch.setattr(pui, "INSTALLER_ASSETS", dist)

    releases_dir = tmp_path / "releases"
    create_package(releases_dir, "11.2.0", "board-a")
    create_package(releases_dir, "11.3.0-rc.1", "board-a")
    site = tmp_path / "site"

    pui.assemble(releases_dir, [meta("11.2.0"), meta("11.3.0-rc.1", True), meta("11.1.0")], site, "")

    catalog = json.loads((site / "releases.json").read_text(encoding="utf-8"))
    assert "11.2.0" == catalog["default"]
    assert "11.2.0" == catalog["defaultTag"]
    assert ["11.3.0-rc.1", "11.2.0"] == [release["version"] for release in catalog["releases"]]
    manifest = json.loads((site / catalog["releases"][1]["manifest"]).read_text(encoding="utf-8"))
    assert "firmware/11.2.0/board-a/firmware.bin" == manifest["targets"][0]["images"][0]["url"]
    assert (site / "firmware/11.2.0/board-a/firmware.bin").is_file()


def test_assemble_defective_package_fails(tmp_path: Path, monkeypatch: pytest.MonkeyPatch) -> None:
    """A package with a missing image aborts the assembly.

    Args:
        tmp_path: Temporary test directory.
        monkeypatch: Pytest fixture used to replace the installer assets path.

    Returns:
        None.
    """
    dist = tmp_path / "dist"
    dist.mkdir()
    monkeypatch.setattr(pui, "INSTALLER_ASSETS", dist)

    releases_dir = tmp_path / "releases"
    create_package(releases_dir, "11.2.0", "board-a", with_image=False)

    with pytest.raises(FileNotFoundError):
        pui.assemble(releases_dir, [meta("11.2.0")], tmp_path / "site", "")


def assemble_single(tmp_path: Path, monkeypatch: pytest.MonkeyPatch, releases_dir: Path) -> None:
    """Assemble a site with the single release 11.2.0.

    Args:
        tmp_path: Temporary test directory.
        monkeypatch: Pytest fixture used to replace the installer assets path.
        releases_dir: Directory containing release packages.

    Returns:
        None.
    """
    dist = tmp_path / "dist"
    dist.mkdir()
    monkeypatch.setattr(pui, "INSTALLER_ASSETS", dist)
    pui.assemble(releases_dir, [meta("11.2.0")], tmp_path / "site", "")


@pytest.mark.parametrize("field", ["chip", "board", "environment", "images", "eraseAll", "flashMode", "flashFrequency", "flashSize"])
def test_assemble_missing_manifest_field_fails(tmp_path: Path, monkeypatch: pytest.MonkeyPatch, field: str) -> None:
    """A package without a required manifest field aborts the assembly.

    Args:
        tmp_path: Temporary test directory.
        monkeypatch: Pytest fixture used to replace the installer assets path.
        field: Required manifest field to remove.

    Returns:
        None.
    """
    releases_dir = tmp_path / "releases"
    create_package(releases_dir, "11.2.0", "board-a")
    manifest_path = releases_dir / "11.2.0" / "usb-flash-board-a" / "manifest.json"
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    del manifest[field]
    manifest_path.write_text(json.dumps(manifest), encoding="utf-8")

    with pytest.raises(ValueError):
        assemble_single(tmp_path, monkeypatch, releases_dir)


@pytest.mark.parametrize("key,value", [
    ("sha256", "00"), ("sha256", 5), ("offset", "0"), ("offset", -1), ("offset", True), ("file", "../x.bin"),
])
def test_assemble_invalid_image_field_fails(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch, key: str, value: Any
) -> None:
    """A package with an invalid image field aborts the assembly.

    Args:
        tmp_path: Temporary test directory.
        monkeypatch: Pytest fixture used to replace the installer assets path.
        key: Image field to replace.
        value: Invalid field value.

    Returns:
        None.
    """
    releases_dir = tmp_path / "releases"
    create_package(releases_dir, "11.2.0", "board-a")
    manifest_path = releases_dir / "11.2.0" / "usb-flash-board-a" / "manifest.json"
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    manifest["images"][0][key] = value
    manifest_path.write_text(json.dumps(manifest), encoding="utf-8")

    with pytest.raises(ValueError):
        assemble_single(tmp_path, monkeypatch, releases_dir)


def test_assemble_package_without_manifest_fails(tmp_path: Path, monkeypatch: pytest.MonkeyPatch) -> None:
    """A package directory without manifest is defective, even next to valid ones.

    Args:
        tmp_path: Temporary test directory.
        monkeypatch: Pytest fixture used to replace the installer assets path.

    Returns:
        None.
    """
    releases_dir = tmp_path / "releases"
    create_package(releases_dir, "11.2.0", "board-a")
    (releases_dir / "11.2.0" / "usb-flash-board-b").mkdir()

    with pytest.raises(ValueError):
        assemble_single(tmp_path, monkeypatch, releases_dir)


def test_same_version_with_different_tags(tmp_path: Path, monkeypatch: pytest.MonkeyPatch) -> None:
    """Tags 11.0.0 and v11.0.0 get separate entries and directories.

    Args:
        tmp_path: Temporary test directory.
        monkeypatch: Pytest fixture used to replace the installer assets path.

    Returns:
        None.
    """
    dist = tmp_path / "dist"
    dist.mkdir()
    monkeypatch.setattr(pui, "INSTALLER_ASSETS", dist)
    releases_dir = tmp_path / "releases"
    create_package(releases_dir, "11.0.0", "board-a")
    create_package(releases_dir, "v11.0.0", "board-a")
    site = tmp_path / "site"

    pui.assemble(releases_dir, [meta("11.0.0"), meta("v11.0.0")], site, "")

    catalog = json.loads((site / "releases.json").read_text(encoding="utf-8"))
    assert {"11.0.0", "v11.0.0"} == {release["tag"] for release in catalog["releases"]}
    assert (site / "firmware/v11.0.0/manifest.json").is_file()
