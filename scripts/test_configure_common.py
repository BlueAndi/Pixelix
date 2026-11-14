"""Test the shared plugin and service configuration helpers."""

# MIT License
#
# Copyright (c) 2026 Andreas Merkle (web@blue-andi.de)
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
import hashlib
import json
from pathlib import Path
import tempfile
import unittest

import configure_common
import determine_layout

###############################################################################
# Variables
###############################################################################

###############################################################################
# Classes
###############################################################################


class ConfigureCommonTest(unittest.TestCase):
    """Verify shared helpers with temporary files."""

    def test_checksum_orders_files_by_basename(self) -> None:
        """Check directory and explicit paths use basename ordering.

        Args:
            self: Test case instance.

        Returns:
            None.
        """
        with tempfile.TemporaryDirectory() as temporary:
            directory = Path(temporary)
            (directory / "b").write_bytes(b"second")
            (directory / "a").write_bytes(b"first")
            digest = hashlib.md5(b"firstsecond").hexdigest()
            self.assertEqual(
                digest,
                configure_common.calculate_path_checksum([str(directory)]),
            )
            self.assertEqual(
                digest,
                configure_common.calculate_path_checksum(
                    [str(directory / "b"), str(directory / "a")]
                ),
            )
            self.assertEqual(
                hashlib.md5().hexdigest(),
                configure_common.calculate_path_checksum(
                    [str(directory / "missing")]
                ),
            )

    def test_load_json_file(self) -> None:
        """Check JSON loading and the missing-file behavior.

        Args:
            self: Test case instance.

        Returns:
            None.
        """
        with tempfile.TemporaryDirectory() as temporary:
            path = Path(temporary) / "data.json"
            self.assertIsNone(configure_common.load_json_file(str(path)))
            path.write_text('{"name": "example"}', encoding="utf-8")
            self.assertEqual(
                {"name": "example"},
                configure_common.load_json_file(str(path)),
            )
            path.write_text("{", encoding="utf-8")
            with self.assertRaises(ValueError):
                configure_common.load_json_file(str(path))

    def test_copy_and_cleanup(self) -> None:
        """Check copying and removal of obsolete destination directories.

        Args:
            self: Test case instance.

        Returns:
            None.
        """
        with tempfile.TemporaryDirectory() as temporary:
            directory = Path(temporary)
            source = directory / "source.txt"
            source.write_text("asset", encoding="utf-8")
            destination = directory / "web"
            destination.mkdir()
            retained = destination / "retained"
            obsolete = destination / "obsolete"
            obsolete.mkdir()
            configure_common.copy_files([str(source)], str(retained))
            self.assertEqual(
                "asset", (retained / source.name).read_text(encoding="utf-8")
            )
            self.assertTrue(
                configure_common.clean_up_folders(
                    ["retained"], str(destination)
                )
            )
            self.assertTrue(retained.is_dir())
            self.assertFalse(obsolete.exists())
            self.assertFalse(
                configure_common.clean_up_folders(
                    ["retained"], str(destination)
                )
            )
            self.assertFalse(
                configure_common.clean_up_folders(
                    [], str(directory / "missing")
                )
            )

    def test_sync_manifest_and_layout(self) -> None:
        """Check layout selection, idempotence and invalid metadata.

        Args:
            self: Test case instance.

        Returns:
            None.
        """
        with tempfile.TemporaryDirectory() as temporary:
            root = Path(temporary)
            lib = root / "example"
            web = lib / "web"
            web.mkdir(parents=True)
            (web / "base.txt").write_text("base", encoding="utf-8")
            (web / "layout.txt").write_text("layout", encoding="utf-8")
            (web / "generic.txt").write_text("generic", encoding="utf-8")
            dst = root / "destination"
            manifest = {
                "pixelix": {
                    "type": "plugin",
                    "name": "example",
                    "web": {
                        "files": ["web/base.txt"],
                        "layouts": [
                            {"name": "LAYOUT_GENERIC",
                             "files": ["web/generic.txt"]},
                            {"name": "LAYOUT_TEST",
                             "files": ["web/layout.txt"]}
                        ],
                    },
                },
            }
            manifest_path = lib / "pixelix.json"
            manifest_path.write_text(json.dumps(manifest), encoding="utf-8")
            args = ("example", "plugin", str(lib), str(dst), "LAYOUT_TEST")
            self.assertEqual(
                (False, True), configure_common.sync_web_files(*args)
            )
            self.assertEqual("layout", (dst / "layout.txt").read_text(
                encoding="utf-8"
            ))
            self.assertEqual(
                (False, False), configure_common.sync_web_files(*args)
            )
            other_dst = root / "other"
            self.assertEqual(
                (False, True),
                configure_common.sync_web_files(
                    "example", "plugin", str(lib), str(other_dst),
                    "LAYOUT_OTHER"
                ),
            )
            self.assertFalse((other_dst / "generic.txt").exists())
            manifest["pixelix"]["name"] = "other"
            manifest_path.write_text(json.dumps(manifest), encoding="utf-8")
            self.assertEqual(
                (False, False), configure_common.sync_web_files(*args)
            )
            manifest["pixelix"]["name"] = "example"
            manifest["pixelix"]["type"] = "service"
            manifest_path.write_text(json.dumps(manifest), encoding="utf-8")
            self.assertEqual(
                (True, False), configure_common.sync_web_files(*args)
            )
            manifest_path.unlink()
            self.assertEqual(
                (False, True), configure_common.sync_web_files(*args)
            )


class DetermineLayoutTest(unittest.TestCase):
    """Verify display layout selection from build flags."""

    def test_build_flags(self) -> None:
        """Check defaults, dimension parsing, and repeated definitions.

        Args:
            self: Test case instance.

        Returns:
            None.
        """
        self.assertEqual("LAYOUT_0X0", determine_layout.determine_layout([]))
        flags = [
            "-D CONFIG_LED_MATRIX_WIDTH=32",
            "-D CONFIG_LED_MATRIX_HEIGHT=8",
            "-D CONFIG_LED_MATRIX_WIDTH=64",
        ]
        self.assertEqual(
            "LAYOUT_64X8", determine_layout.determine_layout(flags)
        )


###############################################################################
# Functions
###############################################################################

###############################################################################
# Main
###############################################################################

if __name__ == "__main__":
    unittest.main()
