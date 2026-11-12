#!/usr/bin/env python3
"""Package a native build and its generated filesystem for distribution."""

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
import os
from pathlib import Path
import re
import shutil
import subprocess
import tarfile
import tempfile
import zipfile


###############################################################################
# Variables
###############################################################################

###############################################################################
# Classes
###############################################################################

###############################################################################
# Functions
###############################################################################


def windows_dependencies(executable: Path, runtime_dir: Path) -> list[Path]:
    """Resolve non-system DLL imports recursively from the MSYS2 runtime.

    Args:
        executable: Native Windows executable.
        runtime_dir: Directory containing the MSYS2 runtime DLLs.

    Returns:
        Paths to all required non-system DLLs.

    Raises:
        FileNotFoundError: A DLL cannot be resolved.
        subprocess.CalledProcessError: The binary cannot be inspected.
    """
    system_dir = Path(os.environ["SystemRoot"]) / "System32"
    pending = [executable]
    found: dict[str, Path] = {}
    while pending:
        binary = pending.pop()
        output = subprocess.run(
            ["objdump", "-p", str(binary)], check=True,
            capture_output=True, text=True
        ).stdout
        for name in re.findall(r"^\s*DLL Name:\s*(\S+)", output, re.MULTILINE):
            key = name.lower()
            if key in found or key.startswith(("api-ms-win-", "ext-ms-win-")):
                continue
            runtime = runtime_dir / name
            if runtime.is_file():
                found[key] = runtime
                pending.append(runtime)
            elif not (system_dir / name).is_file():
                raise FileNotFoundError(
                    f"Unresolved DLL import {name} in {binary}"
                )
    return list(found.values())


def archive_mode(info: tarfile.TarInfo) -> tarfile.TarInfo:
    """Preserve executable permissions independently of the packaging host.

    Args:
        info: Archive member metadata.

    Returns:
        Metadata with executable permissions for the executable and launcher.
    """
    if info.name in ("pixelix/start.sh", "pixelix/pixelix"):
        info.mode |= 0o111
    return info


def copy_project_licenses(project: Path, destination: Path) -> None:
    """Include project and downloaded library license notices.

    Args:
        project: Release source checkout with installed native dependencies.
        destination: License directory in the staged archive.

    Raises:
        FileNotFoundError: The project license or dependency tree is missing.
    """
    shutil.copy2(project / "LICENSE", destination / "Pixelix-LICENSE")
    dependencies = project / ".pio" / "libdeps" / "native"
    if not dependencies.is_dir():
        raise FileNotFoundError(
            f"Native dependency directory is missing: {dependencies}"
        )
    for dependency in dependencies.iterdir():
        if dependency.is_dir():
            for notice in dependency.iterdir():
                if (notice.is_file() and
                        notice.name.upper().startswith(
                            ("LICENSE", "COPYING", "NOTICE"))):
                    target = destination / "platformio" / dependency.name
                    target.mkdir(parents=True, exist_ok=True)
                    shutil.copy2(notice, target / notice.name)


# pylint: disable-next=too-many-arguments
def package(*, kind: str, executable: Path, data: Path, runtime: Path,
            licenses: Path, project: Path, output: Path) -> None:
    """Create a platform archive from the build outputs.

    Args:
        kind: Target operating system, either linux or windows.
        executable: Built native executable.
        data: Generated filesystem tree.
        runtime: SDL3 library on Linux, or MSYS2 binary directory on Windows.
        licenses: Directory containing third-party license notices.
        project: Release source checkout containing installed dependencies.
        output: Destination archive path.

    Raises:
        FileNotFoundError: A required input is missing.
    """
    for required in (executable, data / "js" / "app-menu.js",
                     data / "cp" / "captivePortal.html",
                     data / "configuration" / "smiley.bmp",
                     data / "images" / "LogoStartup.gif",
                     data / "plugins" / "DateTimePlugin" /
                     "DateTimePlugin.html",
                     data / "services" / "MqttService" / "MqttService.html",
                     licenses):
        if not required.exists():
            raise FileNotFoundError(
                f"Required native release input is missing: {required}"
            )
    output.parent.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="pixelix-native-") as temporary:
        root = Path(temporary) / "pixelix"
        root.mkdir()
        shutil.copytree(data, root / "data")
        shutil.copytree(licenses, root / "licenses")
        copy_project_licenses(project, root / "licenses")
        if kind == "linux":
            binary = root / "pixelix"
            shutil.copy2(executable, binary)
            binary.chmod(0o755)
            library = runtime.resolve(strict=True)
            if not library.is_file():
                raise FileNotFoundError(
                    f"SDL3 runtime is not a file: {runtime}"
                )
            (root / "lib").mkdir()
            shutil.copy2(library, root / "lib" / "libSDL3.so.0")
            launcher = root / "start.sh"
            launcher.write_text(
                '#!/bin/sh\n'
                'set -eu\n'
                'DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)\n'
                'export LD_LIBRARY_PATH='
                '"$DIR/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"\n'
                'exec "$DIR/pixelix" --work-dir "$DIR" "$@"\n',
                encoding="utf-8", newline="\n"
            )
            launcher.chmod(0o755)
            with tarfile.open(output, "w:gz") as archive:
                archive.add(root, arcname=root.name, filter=archive_mode)
        else:
            shutil.copy2(executable, root / "pixelix.exe")
            for library in windows_dependencies(executable, runtime):
                shutil.copy2(library, root / library.name)
            (root / "start.cmd").write_text(
                '@echo off\r\n"%~dp0pixelix.exe" --work-dir "%~dp0." %*\r\n',
                encoding="utf-8", newline=""
            )
            with zipfile.ZipFile(output, "w", zipfile.ZIP_DEFLATED) as archive:
                for path in sorted(root.rglob("*")):
                    if path.is_file():
                        archive.write(path, path.relative_to(root.parent))


def main() -> None:
    """Parse packaging arguments and create the archive."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--kind", choices=("linux", "windows"), required=True)
    parser.add_argument("--executable", type=Path, required=True)
    parser.add_argument("--data", type=Path, required=True)
    parser.add_argument(
        "--runtime", type=Path, required=True,
        help="Linux SDL3 library or Windows MSYS2 bin directory"
    )
    parser.add_argument("--licenses", type=Path, required=True)
    parser.add_argument("--project", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    package(kind=args.kind, executable=args.executable, data=args.data,
            runtime=args.runtime, licenses=args.licenses, project=args.project,
            output=args.output)


###############################################################################
# Main
###############################################################################

if __name__ == "__main__":
    main()
