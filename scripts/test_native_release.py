"""Tests for native release archives."""

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
from pathlib import Path
import subprocess
import tarfile
import tempfile
import unittest
from unittest import mock
import zipfile

import package_native_release


###############################################################################
# Variables
###############################################################################

###############################################################################
# Classes
###############################################################################


class NativePackageTest(unittest.TestCase):
    """Exercise platform archive layout and required input validation."""

    def setUp(self) -> None:
        """Create isolated input files for each archive test.

        Args:
            self: Test case instance.

        Returns:
            None.
        """
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
        """Check generated content, launcher and archive permissions.

        Args:
            self: Test case instance.

        Returns:
            None.
        """
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
        """Check that the Windows archive contains its DLL and launcher.

        Args:
            self: Test case instance.

        Returns:
            None.
        """
        output = self.root / "native.zip"
        notice = self.root / "sdl-notice.txt"
        notice.write_text("sdl notice", encoding="utf-8")
        with mock.patch.object(
            package_native_release, "windows_dependencies",
            return_value=[self.library]
        ), mock.patch.object(
            package_native_release, "windows_licenses",
            return_value={Path("SDL3") / "LICENSE.txt": notice}
        ) as licenses:
            package_native_release.package(
                kind="windows", executable=self.binary, data=self.data,
                runtime=self.root, msys2=self.root, project=self.root,
                output=output
            )
        licenses.assert_called_once_with([self.library], self.root)
        with zipfile.ZipFile(output) as archive:
            names = archive.namelist()
            self.assertIn("pixelix/pixelix.exe", names)
            self.assertIn("pixelix/libSDL3.so.0", names)
            self.assertIn("pixelix/data/js/app-menu.js", names)
            self.assertIn("pixelix/licenses/SDL3/LICENSE.txt", names)
            self.assertIn("pixelix/licenses/Pixelix-LICENSE", names)
            self.assertIn("pixelix/licenses/platformio/library/LICENSE", names)
            self.assertNotIn("pixelix/licenses/LICENSE.txt", names)
            self.assertIn(
                b'--work-dir "%~dp0."', archive.read("pixelix/start.cmd")
            )

    def test_windows_archive_requires_msys2_root(self) -> None:
        """Reject Windows packaging without the package database.

        Args:
            self: Test case instance.

        Returns:
            None.
        """
        with self.assertRaises(ValueError):
            package_native_release.package(
                kind="windows", executable=self.binary, data=self.data,
                runtime=self.root, licenses=self.licenses, project=self.root,
                output=self.root / "native.zip"
            )

    def test_linux_archive_requires_license_directory(self) -> None:
        """Reject Linux packaging without the SDL3 license notice.

        Args:
            self: Test case instance.

        Returns:
            None.
        """
        with self.assertRaises(ValueError):
            package_native_release.package(
                kind="linux", executable=self.binary, data=self.data,
                runtime=self.library, msys2=self.root, project=self.root,
                output=self.root / "native.tar.gz"
            )

    def test_missing_generated_data_fails(self) -> None:
        """Fail instead of shipping an incomplete filesystem.

        Args:
            self: Test case instance.

        Returns:
            None.
        """
        (self.data / "js" / "app-menu.js").unlink()
        with self.assertRaises(FileNotFoundError):
            package_native_release.package(
                kind="linux", executable=self.binary, data=self.data,
                runtime=self.library, licenses=self.licenses,
                project=self.root, output=self.root / "native.tar.gz"
            )

    def test_missing_sdl_library_fails(self) -> None:
        """Fail instead of shipping an archive without SDL.

        Args:
            self: Test case instance.

        Returns:
            None.
        """
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
            self: Test case instance.
            run: Mock binary inspection command.

        Returns:
            None.
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
            self: Test case instance.
            run: Mock binary inspection command.

        Returns:
            None.
        """
        run.return_value.stdout = "DLL Name: pixelix-unknown.dll"
        with self.assertRaises(FileNotFoundError):
            package_native_release.windows_dependencies(self.binary, self.root)


class WindowsLicenseTest(unittest.TestCase):
    """Exercise the license lookup through the MSYS2 package database."""

    def setUp(self) -> None:
        """Create a minimal MSYS2 tree with runtime DLLs and notices.

        Args:
            self: Test case instance.

        Returns:
            None.
        """
        # The cleanup callback owns this directory for the entire test.
        # pylint: disable=consider-using-with
        self.temporary = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary.cleanup)
        self.msys2 = Path(self.temporary.name) / "msys64"
        self.runtime = self.msys2 / "ucrt64" / "bin"
        self.runtime.mkdir(parents=True)
        self.sdl = self.runtime / "SDL3.dll"
        self.sdl.touch()
        self.gcc = self.runtime / "libgcc_s_seh-1.dll"
        self.gcc.touch()
        (self.runtime / "libstdc++-6.dll").touch()
        licenses = self.msys2 / "ucrt64" / "share" / "licenses"
        self.sdl_notice = licenses / "SDL3" / "LICENSE.txt"
        self.gcc_notice = licenses / "gcc-libs" / "COPYING.RUNTIME"
        for notice in (self.sdl_notice, self.gcc_notice):
            notice.parent.mkdir(parents=True)
            notice.write_text(notice.name, encoding="utf-8")

    @staticmethod
    def pacman(stdout: str) -> subprocess.CompletedProcess:
        """Build a package manager result.

        Args:
            stdout: Package manager output.

        Returns:
            Completed package manager invocation.
        """
        return subprocess.CompletedProcess("pacman", 0, stdout=stdout)

    @mock.patch("package_native_release.subprocess.run")
    def test_only_notices_of_bundled_packages_are_collected(
        self, run: mock.MagicMock
    ) -> None:
        """Map each DLL to its package and ship only those notices.

        Args:
            self: Test case instance.
            run: Mock package manager invocation.

        Returns:
            None.
        """
        run.side_effect = [
            self.pacman("mingw-w64-ucrt-x86_64-sdl3\n"),
            self.pacman("mingw-w64-ucrt-x86_64-gcc-libs\n"),
            self.pacman("mingw-w64-ucrt-x86_64-gcc-libs\n"),
            self.pacman(
                "/ucrt64/bin/SDL3.dll\n"
                "/ucrt64/share/licenses/SDL3/\n"
                "/ucrt64/share/licenses/SDL3/LICENSE.txt\n"
            ),
            self.pacman(
                "/ucrt64/bin/libgcc_s_seh-1.dll\n"
                "/ucrt64/bin/libstdc++-6.dll\n"
                "/ucrt64/share/licenses/gcc-libs/\n"
                "/ucrt64/share/licenses/gcc-libs/COPYING.RUNTIME\n"
            ),
        ]
        notices = package_native_release.windows_licenses(
            [self.sdl, self.gcc, self.runtime / "libstdc++-6.dll"], self.msys2
        )
        self.assertEqual({
            Path("SDL3") / "LICENSE.txt": self.sdl_notice,
            Path("gcc-libs") / "COPYING.RUNTIME": self.gcc_notice,
        }, notices)
        pacman = str(self.msys2 / "usr" / "bin" / "pacman.exe")
        self.assertEqual(
            [call.args[0] for call in run.call_args_list],
            [[pacman, "-Qqo", "/ucrt64/bin/SDL3.dll"],
             [pacman, "-Qqo", "/ucrt64/bin/libgcc_s_seh-1.dll"],
             [pacman, "-Qqo", "/ucrt64/bin/libstdc++-6.dll"],
             [pacman, "-Qlq", "mingw-w64-ucrt-x86_64-sdl3"],
             [pacman, "-Qlq", "mingw-w64-ucrt-x86_64-gcc-libs"]]
        )

    @mock.patch("package_native_release.subprocess.run")
    def test_import_name_case_is_matched_to_disk(
        self, run: mock.MagicMock
    ) -> None:
        """Query the package database with the file name as stored on disk.

        Args:
            self: Test case instance.
            run: Mock package manager invocation.

        Returns:
            None.
        """
        run.side_effect = [
            self.pacman("mingw-w64-ucrt-x86_64-sdl3\n"),
            self.pacman("/ucrt64/share/licenses/SDL3/LICENSE.txt\n"),
        ]
        package_native_release.windows_licenses(
            [self.runtime / "sdl3.DLL"], self.msys2
        )
        self.assertEqual(run.call_args_list[0].args[0][1:],
                         ["-Qqo", "/ucrt64/bin/SDL3.dll"])

    @mock.patch("package_native_release.subprocess.run")
    def test_package_without_notice_fails(self, run: mock.MagicMock) -> None:
        """Refuse to ship a DLL whose package has no license notice.

        Args:
            self: Test case instance.
            run: Mock package manager invocation.

        Returns:
            None.
        """
        run.side_effect = [
            self.pacman("mingw-w64-ucrt-x86_64-sdl3\n"),
            self.pacman("/ucrt64/bin/SDL3.dll\n"),
        ]
        with self.assertRaises(FileNotFoundError):
            package_native_release.windows_licenses([self.sdl], self.msys2)

    @mock.patch("package_native_release.subprocess.run")
    def test_unowned_library_fails(self, run: mock.MagicMock) -> None:
        """Refuse to ship a DLL that no package owns.

        Args:
            self: Test case instance.
            run: Mock package manager invocation.

        Returns:
            None.
        """
        run.return_value = self.pacman("")
        with self.assertRaises(FileNotFoundError):
            package_native_release.windows_licenses([self.sdl], self.msys2)

    @mock.patch("package_native_release.subprocess.run")
    def test_package_manager_failure_is_propagated(
        self, run: mock.MagicMock
    ) -> None:
        """Propagate package database errors instead of shipping silently.

        Args:
            self: Test case instance.
            run: Mock package manager invocation.

        Returns:
            None.
        """
        run.side_effect = subprocess.CalledProcessError(1, "pacman")
        with self.assertRaises(subprocess.CalledProcessError):
            package_native_release.windows_licenses([self.sdl], self.msys2)


###############################################################################
# Functions
###############################################################################

###############################################################################
# Main
###############################################################################

if __name__ == "__main__":
    unittest.main()
