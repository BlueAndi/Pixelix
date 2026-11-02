"""Helpers of the Pixelix tests."""

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
from ipaddress import ip_address
from typing import Any

# Home Assistant is provided by the test environment only.
# pylint: disable=import-error
from homeassistant.const import (
    CONF_HOST,
    CONF_PASSWORD,
    CONF_PORT,
    CONF_USERNAME,
)
from homeassistant.core import HomeAssistant
from homeassistant.helpers import device_registry as dr
from homeassistant.helpers.service_info.zeroconf import ZeroconfServiceInfo
from pytest_homeassistant_custom_component.common import MockConfigEntry
from pytest_homeassistant_custom_component.test_util.aiohttp import (
    AiohttpClientMocker,
)

from custom_components.pixelix.const import DOMAIN
from custom_components.pixelix.validation import normalize_name

###############################################################################
# Variables
###############################################################################
HOST_A = "192.168.1.50"
HOST_B = "192.168.1.51"
HOSTNAME_A = "pixelix-AAAAAAAA"
HOSTNAME_B = "pixelix-BBBBBBBB"
DEVICE_ID_A = "112233445566"
DEVICE_ID_B = "AABBCCDDEEFF"
USERNAME = "luke"
PASSWORD = "skywalker"

# Device details which the status of a device reports.
MAC_A = "A1:B2:C3:D4:E5:F6"
MAC_B = "A6:B5:C4:D3:E2:F1"
SW_VERSION = "7.4.0"
HW_VERSION = "ulanzi-TC001"

OK_RESPONSE = {"data": {}, "status": "ok"}

# The answer of a device which rejects a notification, see the REST API.
INVALID_DATA_ANSWER = {
    "status": 404,
    "json": {"status": "error", "error": {"msg": "Invalid data."}},
}

###############################################################################
# Classes
###############################################################################

###############################################################################
# Functions
###############################################################################


def api_url(host: str, path: str) -> str:
    """Build the URL of a REST API request.

    Args:
        host: The address of the device.
        path: The path below the REST API base path.

    Returns:
        The URL.
    """
    return f"http://{host}/rest/api/v1{path}"


def mock_device_status(
    aioclient_mock: AiohttpClientMocker,
    host: str,
    chip_id: str,
    *,
    hostname: str | None = None,
    omit: tuple[str, ...] = (),
    **kwargs: Any,
) -> None:
    """Mock the /device endpoint which reports device.id.

    Args:
        aioclient_mock: The HTTP client mock.
        host: The address of the device.
        chip_id: The chip id which the device reports.
        hostname: The hostname which the device reports.
        omit: Names of fields of the device block which the device does not
            report, e.g. ``("mac",)`` for an older firmware.
        **kwargs: Additional arguments of the mocked response, e.g. the
            status or an exception. They replace the successful answer.

    Returns:
        None.
    """
    if not kwargs:
        if hostname is None:
            hostname = (
                HOSTNAME_A
                if host == HOST_A
                else HOSTNAME_B
                if host == HOST_B
                else host
            )
        mac = MAC_B if host == HOST_B else MAC_A
        device = {
            "id": chip_id,
            "name": hostname,
            "model": "Pixelix",
            "manufacturer": "BlueAndi & Friends",
            "swVersion": SW_VERSION,
            "hwVersion": HW_VERSION,
            "mac": mac,
        }
        for field in omit:
            device.pop(field, None)

        kwargs = {
            "json": {"data": {"device": device}, "status": "ok"}
        }

    aioclient_mock.get(api_url(host, "/device"), **kwargs)


def mock_notify(
    aioclient_mock: AiohttpClientMocker, host: str, **kwargs: Any
) -> None:
    """Mock the notify request of a device.

    Args:
        aioclient_mock: The HTTP client mock.
        host: The address of the device.
        **kwargs: Arguments of the mocked response. Without any, the device
            accepts the notification.

    Returns:
        None.
    """
    aioclient_mock.post(
        api_url(host, "/notify"), **(kwargs or {"json": OK_RESPONSE})
    )


def notify_payloads(
    aioclient_mock: AiohttpClientMocker, host: str
) -> list[dict[str, Any]]:
    """Get the notifications which were sent to a device.

    Args:
        aioclient_mock: The HTTP client mock.
        host: The address of the device.

    Returns:
        The JSON bodies of all notify requests to the device.
    """
    url = api_url(host, "/notify")

    return [
        data
        for method, called_url, data, _ in aioclient_mock.mock_calls
        if method.lower() == "post" and str(called_url) == url
    ]


# pylint: disable-next=too-many-arguments,too-many-positional-arguments
async def setup_entry(
    hass: HomeAssistant,
    host: str = HOST_A,
    device_id_value: str = DEVICE_ID_A,
    title: str = HOSTNAME_A,
    *,
    version: int = 2,
    unique_id: str | None = None,
) -> MockConfigEntry:
    """Add a Pixelix config entry and set it up.

    Args:
        hass: The Home Assistant instance.
        host: The address of the device.
        device_id_value: The chip id of the device.
        title: The title of the entry (e.g. host name).
        version: The version of the config entry.
        unique_id: The unique id. Without one, it is derived from the device
            id as the config flow does.

    Returns:
        The config entry.
    """
    entry = MockConfigEntry(
        domain=DOMAIN,
        version=version,
        unique_id=unique_id or normalize_name(device_id_value),
        title=title,
        data={
            CONF_HOST: host,
            CONF_PORT: 80,
            CONF_USERNAME: USERNAME,
            CONF_PASSWORD: PASSWORD,
        },
    )
    entry.add_to_hass(hass)

    assert await hass.config_entries.async_setup(entry.entry_id)
    await hass.async_block_till_done()

    return entry


def device_id(hass: HomeAssistant, entry: MockConfigEntry) -> str:
    """Get the id of the device of a config entry.

    Args:
        hass: The Home Assistant instance.
        entry: The config entry.

    Returns:
        The device id in the device registry.
    """
    devices = dr.async_entries_for_config_entry(
        dr.async_get(hass), entry.entry_id
    )
    device = next(
        (d for d in devices if (DOMAIN, entry.unique_id) in d.identifiers),
        None,
    )
    assert device is not None

    return device.id


def zeroconf_info(
    name: str, host: str = HOST_A, port: int = 80
) -> ZeroconfServiceInfo:
    """Create the mDNS information of a Pixelix device.

    Args:
        name: The host name, which is the mDNS instance name.
        host: The IPv4 address of the device.
        port: The port of the web server.

    Returns:
        The discovery information.
    """
    return ZeroconfServiceInfo(
        ip_address=ip_address(host),
        ip_addresses=[ip_address(host)],
        port=port,
        hostname=f"{name.lower()}.local.",
        type="_pixelix._tcp.local.",
        name=f"{name}._pixelix._tcp.local.",
        properties={},
    )
