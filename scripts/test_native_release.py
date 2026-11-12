"""Tests for native release archives and legacy asset cleanup."""

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
import json
from pathlib import Path
import subprocess
import tarfile
import tempfile
import unittest
from unittest import mock
import zipfile

import package_native_release
import remove_legacy_release_asset


###############################################################################
# Variables
###############################################################################

###############################################################################
# Classes
###############################################################################


class NativePackageTest(unittest.TestCase):
    """Exercise platform archive layout and required input validation."""

    def setUp(self) -> None:
        """Create isolated input files for each archive test."""
        # The cleanup callback owns this directory for the entire test.
        # pylint: disable=consider-using-with
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.root = Path(self.temporary.name)
        self.data = self.root / "data"
        for name in ("js/app-menu.js", "cp/captivePortal.html",
                     "configuration/smiley.bmp", "images/LogoStartup.gif",
                     "plugins/DateTimePlugin/DateTimePlugin.html",
                     "services/MqttService/MqttService.html"):
            path = self.data / name
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(name, encoding="utf-8")
        self.binary = self.root / "program"
        self.binary.write_bytes(b"binary")
        self.library = self.root / "libSDL3.so.0"
        self.library.write_bytes(b"library")
        self.licenses = self.root / "notices"
        self.licenses.mkdir()
        (self.licenses / "LICENSE.txt").write_text("notice", encoding="utf-8")
        (self.root / "LICENSE").write_text("project license", encoding="utf-8")
        dependency = self.root / ".pio" / "libdeps" / "native" / "library"
        dependency.mkdir(parents=True)
        (dependency / "LICENSE").write_text(
            "library license", encoding="utf-8"
        )

    def test_linux_archive_has_generated_data_and_executable_permissions(
        self
    ) -> None:
        """Check generated content, launcher and archive permissions."""
        output = self.root / "native.tar.gz"
        package_native_release.package(
            kind="linux", executable=self.binary, data=self.data,
            runtime=self.library, licenses=self.licenses, project=self.root,
            output=output
        )
        with tarfile.open(output, "r:gz") as archive:
            names = archive.getnames()
            self.assertIn("pixelix/data/js/app-menu.js", names)
            self.assertIn("pixelix/data/configuration/smiley.bmp", names)
            self.assertIn(
                "pixelix/data/plugins/DateTimePlugin/DateTimePlugin.html",
                names
            )
            self.assertIn(
                "pixelix/data/services/MqttService/MqttService.html", names
            )
            self.assertIn("pixelix/lib/libSDL3.so.0", names)
            self.assertIn("pixelix/licenses/LICENSE.txt", names)
            self.assertIn("pixelix/licenses/Pixelix-LICENSE", names)
            self.assertIn("pixelix/licenses/platformio/library/LICENSE", names)
            self.assertEqual(
                archive.getmember("pixelix/start.sh").mode & 0o111, 0o111
            )
            self.assertEqual(
                archive.getmember("pixelix/pixelix").mode & 0o111, 0o111
            )
            with archive.extractfile("pixelix/start.sh") as launcher:
                self.assertIn(b'--work-dir "$DIR"', launcher.read())

    def test_windows_archive_includes_runtime_and_launcher(self) -> None:
        """Check that the Windows archive contains its DLL and launcher."""
        output = self.root / "native.zip"
        with mock.patch.object(
            package_native_release, "windows_dependencies",
            return_value=[self.library]
        ):
            package_native_release.package(
                kind="windows", executable=self.binary, data=self.data,
                runtime=self.root, licenses=self.licenses, project=self.root,
                output=output
            )
        with zipfile.ZipFile(output) as archive:
            self.assertIn("pixelix/pixelix.exe", archive.namelist())
            self.assertIn("pixelix/libSDL3.so.0", archive.namelist())
            self.assertIn("pixelix/data/js/app-menu.js", archive.namelist())
            self.assertIn(
                b'--work-dir "%~dp0."', archive.read("pixelix/start.cmd")
            )

    def test_missing_generated_data_fails(self) -> None:
        """Fail instead of shipping an incomplete filesystem."""
        (self.data / "js" / "app-menu.js").unlink()
        with self.assertRaises(FileNotFoundError):
            package_native_release.package(
                kind="linux", executable=self.binary, data=self.data,
                runtime=self.library, licenses=self.licenses,
                project=self.root, output=self.root / "native.tar.gz"
            )

    def test_missing_sdl_library_fails(self) -> None:
        """Fail instead of shipping an archive without SDL."""
        self.library.unlink()
        with self.assertRaises(FileNotFoundError):
            package_native_release.package(
                kind="linux", executable=self.binary, data=self.data,
                runtime=self.library, licenses=self.licenses,
                project=self.root, output=self.root / "native.tar.gz"
            )

    @mock.patch.dict("os.environ", {"SystemRoot": "C:\\Windows"})
    @mock.patch("package_native_release.subprocess.run")
    def test_windows_dependencies_are_resolved_recursively(
        self, run: mock.MagicMock
    ) -> None:
        """Include non-system DLLs required by other DLLs.

        Args:
            run: Mock binary inspection command.
        """
        first = self.root / "first.dll"
        second = self.root / "second.dll"
        first.touch()
        second.touch()
        run.side_effect = [
            subprocess.CompletedProcess(
                "objdump", 0, stdout="DLL Name: first.dll"
            ),
            subprocess.CompletedProcess(
                "objdump", 0, stdout="DLL Name: second.dll"
            ),
            subprocess.CompletedProcess("objdump", 0, stdout="")
        ]
        dependencies = package_native_release.windows_dependencies(
            self.binary, self.root
        )
        self.assertEqual({first, second}, set(dependencies))

    @mock.patch.dict("os.environ", {"SystemRoot": "C:\\Windows"})
    @mock.patch("package_native_release.subprocess.run")
    def test_windows_missing_dependency_fails(
        self, run: mock.MagicMock
    ) -> None:
        """Reject an unresolved non-system DLL.

        Args:
            run: Mock binary inspection command.
        """
        run.return_value.stdout = "DLL Name: pixelix-unknown.dll"
        with self.assertRaises(FileNotFoundError):
            package_native_release.windows_dependencies(self.binary, self.root)


class LegacyAssetTest(unittest.TestCase):
    """Verify exact matching and GitHub error propagation."""

    @mock.patch("remove_legacy_release_asset.subprocess.run")
    def test_removes_only_exact_legacy_asset(
        self, run: mock.MagicMock
    ) -> None:
        """Delete the old firmware archive and no other release asset.

        Args:
            run: Mock GitHub CLI invocation.
        """
        run.return_value.stdout = json.dumps({"assets": [
            {"name": "Pixelix-v11.0.0-rc.1.zip"},
            {"name": "Pixelix-v11.0.0-rc.1-usb-flash-packages.zip"},
        ]})
        remove_legacy_release_asset.remove_legacy_asset(
            "BlueAndi/Pixelix", "v11.0.0-rc.1"
        )
        self.assertEqual(run.call_count, 2)
        self.assertEqual(run.call_args.args[0][3:5],
                         ["v11.0.0-rc.1", "Pixelix-v11.0.0-rc.1.zip"])

    @mock.patch("remove_legacy_release_asset.subprocess.run")
    def test_does_not_remove_other_assets(self, run: mock.MagicMock) -> None:
        """Leave other assets alone when the old ZIP is absent.

        Args:
            run: Mock GitHub CLI invocation.
        """
        run.return_value.stdout = json.dumps({"assets": [
            {"name": "Pixelix-v11.0.0-rc.1-usb-flash-packages.zip"}
        ]})
        remove_legacy_release_asset.remove_legacy_asset(
            "BlueAndi/Pixelix", "v11.0.0-rc.1"
        )
        self.assertEqual(run.call_count, 1)

    @mock.patch("remove_legacy_release_asset.subprocess.run")
    def test_github_failure_is_not_treated_as_missing(
        self, run: mock.MagicMock
    ) -> None:
        """Propagate failures when looking up the release.

        Args:
            run: Mock GitHub CLI invocation.
        """
        run.side_effect = subprocess.CalledProcessError(1, "gh")
        with self.assertRaises(subprocess.CalledProcessError):
            remove_legacy_release_asset.remove_legacy_asset(
                "BlueAndi/Pixelix", "v11.0.0-rc.1"
            )

    @mock.patch("remove_legacy_release_asset.subprocess.run")
    def test_deletion_failure_is_reported(self, run: mock.MagicMock) -> None:
        """Propagate failures when deleting the old asset.

        Args:
            run: Mock GitHub CLI invocation.
        """
        run.side_effect = [
            subprocess.CompletedProcess("gh", 0, stdout=json.dumps(
                {"assets": [{"name": "Pixelix-v1.zip"}]})),
            subprocess.CalledProcessError(1, "gh")
        ]
        with self.assertRaises(subprocess.CalledProcessError):
            remove_legacy_release_asset.remove_legacy_asset(
                "BlueAndi/Pixelix", "v1"
            )


###############################################################################
# Functions
###############################################################################

###############################################################################
# Main
###############################################################################

if __name__ == "__main__":
    unittest.main()
