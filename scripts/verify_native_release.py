#!/usr/bin/env python3
"""Verify an extracted native release independently of the build tree."""

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
import subprocess
import tarfile
import tempfile
import time
import urllib.error
import urllib.request
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


def check_data(root: Path, source_data: Path) -> None:
    """Compare packaged generated data with the build output.

    Args:
        root: Extracted package directory.
        source_data: Generated source filesystem directory.

    Raises:
        ValueError: A generated file or directory is incorrect.
    """
    for relative in ("js/app-menu.js", "cp/captivePortal.html",
                     "configuration/smiley.bmp", "images/LogoStartup.gif",
                     "plugins/DateTimePlugin/DateTimePlugin.html",
                     "services/MqttService/MqttService.html"):
        packaged = (root / "data" / relative).read_bytes()
        original = (source_data / relative).read_bytes()
        if packaged != original:
            raise ValueError(f"Incorrect packaged data: {relative}")


def stop_process(process: subprocess.Popen, kind: str) -> None:
    """Stop the launcher and its application process.

    Args:
        process: Running launcher process.
        kind: Target operating system.
    """
    if kind == "windows":
        powershell = (Path(os.environ["SystemRoot"]) / "System32" /
                      "WindowsPowerShell" / "v1.0" / "powershell.exe")
        # Stop-Process returns before the application has exited. Wait for
        # the children, otherwise the temporary directory cleanup races with
        # the application that still holds files open below it. A child that
        # survives the timeout is reported through the exit code.
        command = (
            "$children = @(Get-CimInstance Win32_Process -Filter "
            f"'ParentProcessId={process.pid}' | "
            "ForEach-Object { $_.ProcessId }); "
            "foreach ($id in $children) { "
            "Stop-Process -Id $id -ErrorAction SilentlyContinue }; "
            f"Stop-Process -Id {process.pid} -ErrorAction SilentlyContinue; "
            "foreach ($id in $children) { "
            "$child = Get-Process -Id $id -ErrorAction SilentlyContinue; "
            "if ($child -and -not $child.WaitForExit(15000)) { exit 2 } }; "
            "exit 0"
        )
        subprocess.run(
            [str(powershell), "-NoProfile", "-Command", command], check=True
        )
    elif process.poll() is None:
        process.terminate()
    process.wait(timeout=15)


def check_startup(root: Path, work: Path, kind: str) -> None:
    """Start the launcher outside the package and check the captive portal.

    Args:
        root: Extracted package directory.
        work: Unrelated current working directory and log location.
        kind: Target operating system.

    Raises:
        RuntimeError: Startup fails or the expected page is unavailable.
        ValueError: A Linux executable lacks execute permissions.
    """
    env = os.environ.copy()
    env["SDL_VIDEODRIVER"] = "dummy"
    env["SDL_AUDIODRIVER"] = "dummy"
    env.pop("LD_LIBRARY_PATH", None)
    if kind == "linux":
        launcher = root / "start.sh"
        if (not os.access(launcher, os.X_OK) or
                not os.access(root / "pixelix", os.X_OK)):
            raise ValueError("Native executable or launcher is not executable")
        command = [str(launcher)]
    else:
        env["PATH"] = str(Path(os.environ["SystemRoot"]) / "System32")
        command = f'"{root / "start.cmd"}"'
    log_path = work / "pixelix.log"
    with log_path.open("w+", encoding="utf-8") as log:
        with subprocess.Popen(
            command, cwd=work, env=env, stdout=log, stderr=subprocess.STDOUT,
            shell=kind == "windows"
        ) as process:
            try:
                response = None
                for _ in range(60):
                    if process.poll() is not None:
                        break
                    try:
                        with urllib.request.urlopen(
                            "http://127.0.0.1:8080/", timeout=1
                        ) as request:
                            response = request.read()
                        break
                    except (OSError, urllib.error.URLError):
                        time.sleep(1)
                if (response is None or
                        b"Wifi network configuration" not in response):
                    log.flush()
                    details = log_path.read_text(
                        encoding="utf-8", errors="replace"
                    )
                    raise RuntimeError(
                        f"Native release did not start correctly:\n{details}"
                    )
            finally:
                stop_process(process, kind)


def verify(archive_path: Path, source_data: Path, kind: str) -> None:
    """Extract and verify a release archive.

    Args:
        archive_path: Release archive to test.
        source_data: Generated source filesystem directory.
        kind: Target operating system.
    """
    with tempfile.TemporaryDirectory(prefix="pixelix release ") as temporary:
        work = Path(temporary)
        if kind == "linux":
            with tarfile.open(archive_path, "r:gz") as archive:
                archive.extractall(work, filter="data")
        else:
            with zipfile.ZipFile(archive_path) as archive:
                archive.extractall(work)
        root = work / "pixelix"
        check_data(root, source_data)
        check_startup(root, work, kind)


def main() -> None:
    """Parse verification arguments."""
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--archive", type=Path, required=True)
    parser.add_argument("--data", type=Path, required=True)
    parser.add_argument("--kind", choices=("linux", "windows"), required=True)
    args = parser.parse_args()
    verify(args.archive.resolve(), args.data.resolve(), args.kind)


###############################################################################
# Main
###############################################################################

if __name__ == "__main__":
    main()
