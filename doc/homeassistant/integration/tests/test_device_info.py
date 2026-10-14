"""Tests of the device details which the Pixelix integration reports."""

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
# Home Assistant and pytest are provided by the test environment only.
# pylint: disable=import-error
import pytest
from homeassistant.core import HomeAssistant
from homeassistant.helpers import device_registry as dr
from pytest_homeassistant_custom_component.test_util.aiohttp import (
    AiohttpClientMocker,
)

from .helpers import (
    DEVICE_ID_A,
    HOST_A,
    HOSTNAME_A,
    HW_VERSION,
    MAC_A,
    SW_VERSION,
    mock_device_status,
    setup_entry,
)

###############################################################################
# Variables
###############################################################################

###############################################################################
# Classes
###############################################################################

###############################################################################
# Functions
###############################################################################


def _device(hass: HomeAssistant, entry_id: str) -> dr.DeviceEntry:
    """Get the single device of a config entry.

    Args:
        hass: The Home Assistant test instance.
        entry_id: The id of the config entry.

    Returns:
        The device entry.
    """
    devices = dr.async_entries_for_config_entry(dr.async_get(hass), entry_id)
    assert 1 == len(devices)

    return devices[0]


async def test_device_reports_firmware_hardware_and_mac(
    hass: HomeAssistant,
    aioclient_mock: AiohttpClientMocker,
) -> None:
    """The device shows the firmware, the hardware and the MAC address.

    Args:
        hass: The Home Assistant test instance.
        aioclient_mock: The mock of the aiohttp client.

    Returns:
        None.
    """
    mock_device_status(aioclient_mock, HOST_A, DEVICE_ID_A)

    entry = await setup_entry(hass)
    device = _device(hass, entry.entry_id)

    assert SW_VERSION == device.sw_version
    assert HW_VERSION == device.hw_version
    assert {
        (dr.CONNECTION_NETWORK_MAC, dr.format_mac(MAC_A))
    } == device.connections


async def test_mac_is_normalized(
    hass: HomeAssistant,
    aioclient_mock: AiohttpClientMocker,
) -> None:
    """The MAC address is stored in the format of Home Assistant.

    The firmware reports it in upper case with colons (MAC_A). Home
    Assistant normalizes it to lower case, so a device of the MQTT discovery
    and the device of this integration are recognized as the same one.

    Args:
        hass: The Home Assistant test instance.
        aioclient_mock: The mock of the aiohttp client.

    Returns:
        None.
    """
    mock_device_status(aioclient_mock, HOST_A, DEVICE_ID_A)

    entry = await setup_entry(hass)
    device = _device(hass, entry.entry_id)

    assert {
        (dr.CONNECTION_NETWORK_MAC, "a1:b2:c3:d4:e5:f6")
    } == device.connections


@pytest.mark.parametrize(
    "omitted",
    [
        ("mac",),
        ("swVersion",),
        ("hwVersion",),
        ("mac", "swVersion", "hwVersion"),
    ],
)
async def test_older_firmware_without_details_still_sets_up(
    hass: HomeAssistant,
    aioclient_mock: AiohttpClientMocker,
    omitted: tuple[str, ...],
) -> None:
    """A device which does not report a detail is still set up.

    Args:
        hass: The Home Assistant test instance.
        aioclient_mock: The mock of the aiohttp client.
        omitted: The fields of the device block which the device does not
            report.

    Returns:
        None.
    """
    mock_device_status(aioclient_mock, HOST_A, DEVICE_ID_A, omit=omitted)

    entry = await setup_entry(hass)
    device = _device(hass, entry.entry_id)

    assert HOSTNAME_A == device.name

    if "mac" in omitted:
        assert not device.connections
    if "swVersion" in omitted:
        assert device.sw_version is None
    if "hwVersion" in omitted:
        assert device.hw_version is None


async def test_offline_device_keeps_known_details(
    hass: HomeAssistant,
    aioclient_mock: AiohttpClientMocker,
) -> None:
    """An offline device keeps the details of an earlier, online setup.

    The device registry clears a value which is passed as None, so details
    which the device did not report must not be passed at all.

    Args:
        hass: The Home Assistant test instance.
        aioclient_mock: The mock of the aiohttp client.

    Returns:
        None.
    """
    mock_device_status(aioclient_mock, HOST_A, DEVICE_ID_A)

    entry = await setup_entry(hass)
    assert SW_VERSION == _device(hass, entry.entry_id).sw_version

    # The device is offline on the next setup.
    aioclient_mock.clear_requests()
    mock_device_status(
        aioclient_mock, HOST_A, DEVICE_ID_A, exc=TimeoutError()
    )

    assert await hass.config_entries.async_reload(entry.entry_id)
    await hass.async_block_till_done()

    device = _device(hass, entry.entry_id)

    assert SW_VERSION == device.sw_version
    assert HW_VERSION == device.hw_version
    assert {
        (dr.CONNECTION_NETWORK_MAC, dr.format_mac(MAC_A))
    } == device.connections
