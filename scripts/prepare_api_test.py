"""Prepares a running Pixelix instance for the OpenAPI based REST API test.

Every plugin specific REST API endpoint is addressed by the unique id (UID) of a
plugin instance. A UID is assigned at runtime, therefore it can not be part of
the OpenAPI specification. This script installs the plugins which the
specification describes, takes the UID out of the installation response and
writes it into a schemathesis configuration file. Schemathesis uses the UIDs
instead of generated values, which makes it possible to test the plugin
endpoints for real.

A plugin which is not part of the firmware build or which does not fit into a
display slot anymore is reported and skipped. Its endpoints are still tested,
but they will answer with 404 like any unknown plugin UID.
"""

# MIT License
#
# Copyright (c) 2019 - 2026 Andreas Merkle (web@blue-andi.de)
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
import base64
import json
import sys
import urllib.error
import urllib.parse
import urllib.request

import yaml

################################################################################
# Variables
################################################################################

# Path prefix of all plugin specific endpoints in the OpenAPI specification.
_PLUGIN_PATH_PREFIX = "/display/uid/{uid}"

# Suffix of every OpenAPI tag which names a plugin.
_PLUGIN_TAG_SUFFIX = "Plugin"

# Path parameters which are not a plugin UID. The first slot resp. the first
# lamp exists in every plugin instance, therefore 0 is a valid value.
_DEFAULT_PARAMETERS = {
    "id": 0,
    "lampId": 0,
    "bitmapSlotId": 0
}

################################################################################
# Classes
################################################################################

class Pixelix:
    """Minimal REST API client for a running Pixelix instance."""

    def __init__(self, url, user, password):
        self._url = url.rstrip("/")
        credentials = f"{user}:{password}".encode("utf-8")
        self._authorization = "Basic " + base64.b64encode(credentials).decode("ascii")

    def _request(self, endpoint, method="GET"):
        """Send a request and return the "data" member of the JSON response.

        Args:
            endpoint (str): Endpoint incl. query parameters, relative to the base URL.
            method (str): HTTP method.

        Returns:
            dict: The "data" member of the response or None in case of an error.
        """
        result = None
        request = urllib.request.Request(f"{self._url}{endpoint}", method=method)
        request.add_header("Authorization", self._authorization)

        try:
            with urllib.request.urlopen(request, timeout=10) as response:
                payload = json.loads(response.read().decode("utf-8"))

                if payload.get("status") == "ok":
                    result = payload.get("data", {})
                else:
                    print(f"    {method} {endpoint} failed: {payload}")

        except (urllib.error.URLError, json.JSONDecodeError, TimeoutError) as error:
            print(f"    {method} {endpoint} failed: {error}")

        return result

    def get_available_plugins(self):
        """Get the names of all plugins which are part of the firmware.

        Returns:
            list: Plugin names.
        """
        data = self._request("/plugins")

        return [] if data is None else data.get("plugins", [])

    def install_plugin(self, name):
        """Install one plugin instance in the next free display slot.

        Args:
            name (str): Plugin name.

        Returns:
            int: UID of the installed plugin instance or None if it failed.
        """
        endpoint = "/plugin/install?name=" + urllib.parse.quote(name)
        data = self._request(endpoint, method="POST")

        return None if data is None else data.get("uid")

################################################################################
# Functions
################################################################################

def collect_plugin_tags(spec_path):
    """Collect every plugin which the OpenAPI specification describes.

    Args:
        spec_path (str): Path to the OpenAPI specification.

    Returns:
        dict: Plugin name to the number of its endpoints.
    """
    with open(spec_path, encoding="utf-8") as file_handle:
        spec = yaml.safe_load(file_handle)

    plugins = {}

    for path, operations in spec.get("paths", {}).items():
        if not path.startswith(_PLUGIN_PATH_PREFIX):
            continue

        for method, operation in operations.items():
            if method not in ("get", "post", "put", "delete", "patch"):
                continue

            for tag in operation.get("tags", []):
                if tag.endswith(_PLUGIN_TAG_SUFFIX):
                    plugins[tag] = plugins.get(tag, 0) + 1

    return plugins


def install_plugins(pixelix, required):
    """Install every required plugin and provide the UID of its instance.

    Args:
        pixelix (Pixelix): The REST API client.
        required (dict): Plugin name to the number of its endpoints.

    Returns:
        dict: Plugin name to UID, containing only the plugins which are available.
    """
    available = pixelix.get_available_plugins()
    result = {}

    for name in sorted(required):
        if name not in available:
            print(f"  {name}: not part of the firmware, its endpoints will answer with 404")

        else:
            uid = pixelix.install_plugin(name)

            if uid is None:
                print(f"  {name}: no free display slot, its endpoints will answer with 404")
            else:
                result[name] = uid
                print(f"  {name}: installed with UID {uid}")

    return result


def write_config(config_path, plugin_uids):
    """Write the schemathesis configuration file.

    Args:
        config_path (str): Path of the configuration file to write.
        plugin_uids (dict): Plugin name to UID.
    """
    lines = [
        "# Generated by scripts/prepare_api_test.py - do not edit.",
        "#",
        "# The plugin UIDs are assigned at runtime, therefore they can not be part of",
        "# the OpenAPI specification itself.",
        "",
        "[parameters]"
    ]

    for name, value in _DEFAULT_PARAMETERS.items():
        lines.append(f"{name} = {value}")

    for name in sorted(plugin_uids):
        lines.append("")
        lines.append("[[operations]]")
        lines.append(f'include-tag = "{name}"')
        lines.append(f"parameters = {{ uid = {plugin_uids[name]} }}")

    lines.append("")

    with open(config_path, "w", encoding="utf-8") as file_handle:
        file_handle.write("\n".join(lines))


def main():
    """Install the plugins of the OpenAPI specification and write their UIDs."""
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--spec", required=True, help="Path to the OpenAPI specification.")
    parser.add_argument("--url", required=True, help="Base URL of the REST API.")
    parser.add_argument("--user", required=True, help="Web login user.")
    parser.add_argument("--password", required=True, help="Web login password.")
    parser.add_argument("--output", required=True, help="Path of the schemathesis configuration to write.")
    args = parser.parse_args()

    required = collect_plugin_tags(args.spec)
    print(f"{len(required)} plugins are described by the OpenAPI specification:")

    pixelix = Pixelix(args.url, args.user, args.password)
    plugin_uids = install_plugins(pixelix, required)

    write_config(args.output, plugin_uids)

    tested = sum(count for name, count in required.items() if name in plugin_uids)
    total = sum(required.values())
    print(f"{len(plugin_uids)}/{len(required)} plugins available, {tested}/{total} plugin endpoints will be tested for real.")

    return 0

################################################################################
# Main
################################################################################

if __name__ == "__main__":
    sys.exit(main())
