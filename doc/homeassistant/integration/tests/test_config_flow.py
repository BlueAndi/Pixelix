"""Tests of the config flow and entry handling of the Pixelix integration."""

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
from typing import Any

# Home Assistant is provided by the test environment only.
# pylint: disable=import-error
import aiohttp
import pytest
from homeassistant import config_entries
from homeassistant.const import (
    CONF_HOST,
    CONF_PASSWORD,
    CONF_PORT,
    CONF_USERNAME,
)
from homeassistant.core import HomeAssistant
from homeassistant.data_entry_flow import FlowResultType
from homeassistant.helpers import device_registry as dr
from pytest_homeassistant_custom_component.common import MockConfigEntry
from pytest_homeassistant_custom_component.test_util.aiohttp import (
    AiohttpClientMocker,
)

from custom_components.pixelix.const import DOMAIN

from .helpers import (
    DEVICE_ID_A,
    DEVICE_ID_B,
    HOST_A,
    HOST_B,
    HOSTNAME_A,
    PASSWORD,
    USERNAME,
    device_id,
    mock_device_status,
    setup_entry,
    zeroconf_info,
)

###############################################################################
# Variables
###############################################################################
USER_INPUT = {
    CONF_HOST: HOST_A,
    CONF_PORT: 80,
    CONF_USERNAME: USERNAME,
    CONF_PASSWORD: PASSWORD,
}

CREDENTIALS = {CONF_USERNAME: USERNAME, CONF_PASSWORD: PASSWORD}

###############################################################################
# Classes
###############################################################################

###############################################################################
# Functions
###############################################################################


async def add_manually(
    hass: HomeAssistant, user_input: dict[str, Any]
) -> dict[str, Any]:
    """Run the manual config flow with the given input.

    Args:
        hass: The Home Assistant instance.
        user_input: The input of the form.

    Returns:
        The result of the flow.
    """
    result = await hass.config_entries.flow.async_init(
        DOMAIN, context={"source": config_entries.SOURCE_USER}
    )
    assert result["type"] is FlowResultType.FORM

    return await hass.config_entries.flow.async_configure(
        result["flow_id"], user_input
    )


async def discover(
    hass: HomeAssistant, name: str, host: str
) -> dict[str, Any]:
    """Start the config flow by an mDNS discovery.

    Args:
        hass: The Home Assistant instance.
        name: The host name of the device.
        host: The address of the device.

    Returns:
        The result of the flow.
    """
    return await hass.config_entries.flow.async_init(
        DOMAIN,
        context={"source": config_entries.SOURCE_ZEROCONF},
        data=zeroconf_info(name, host),
    )


async def test_manual_flow(
    hass: HomeAssistant, aioclient_mock: AiohttpClientMocker
) -> None:
    """A device is added by its address, identified by its device id."""
    mock_device_status(aioclient_mock, HOST_A, DEVICE_ID_A)

    result = await add_manually(hass, USER_INPUT)

    assert result["type"] is FlowResultType.CREATE_ENTRY
    assert result["title"] == HOST_A
    assert result["data"] == USER_INPUT
    entry = hass.config_entries.async_entries(DOMAIN)[0]
    assert entry.unique_id == DEVICE_ID_A.lower()
    assert entry.version == 2


async def test_manual_flow_invalid_auth(
    hass: HomeAssistant, aioclient_mock: AiohttpClientMocker
) -> None:
    """A rejected login is not reported as a connection problem."""
    mock_device_status(aioclient_mock, HOST_A,
                       DEVICE_ID_A, status=401, text="")

    result = await add_manually(hass, USER_INPUT)

    assert result["type"] is FlowResultType.FORM
    assert result["errors"] == {"base": "invalid_auth"}
    assert not hass.config_entries.async_entries(DOMAIN)


@pytest.mark.parametrize(
    ("answer", "error"),
    [
        ({"exc": aiohttp.ClientError}, "cannot_connect"),
        ({"exc": TimeoutError}, "cannot_connect"),
        ({"text": "<html></html>"}, "unexpected_response"),
        ({"status": 500, "text": ""}, "unexpected_response"),
    ],
)
async def test_manual_flow_errors(
    hass: HomeAssistant,
    aioclient_mock: AiohttpClientMocker,
    answer: dict[str, Any],
    error: str,
) -> None:
    """Every kind of problem has its own message."""
    mock_device_status(aioclient_mock, HOST_A, DEVICE_ID_A, **answer)

    result = await add_manually(hass, USER_INPUT)

    assert result["type"] is FlowResultType.FORM
    assert result["errors"] == {"base": error}


async def test_zeroconf_flow(
    hass: HomeAssistant, aioclient_mock: AiohttpClientMocker
) -> None:
    """A discovered device is added after the web login was entered."""
    mock_device_status(aioclient_mock, HOST_A, DEVICE_ID_A)

    result = await discover(hass, HOSTNAME_A, HOST_A)
    assert result["type"] is FlowResultType.FORM
    assert result["step_id"] == "discovery_confirm"

    result = await hass.config_entries.flow.async_configure(
        result["flow_id"], CREDENTIALS
    )

    assert result["type"] is FlowResultType.CREATE_ENTRY
    assert result["data"][CONF_HOST] == HOST_A
    entry = hass.config_entries.async_entries(DOMAIN)[0]
    assert entry.unique_id == DEVICE_ID_A.lower()


async def test_zeroconf_flow_invalid_auth(
    hass: HomeAssistant, aioclient_mock: AiohttpClientMocker
) -> None:
    """A wrong login of a discovered device asks again."""
    mock_device_status(aioclient_mock, HOST_A,
                       DEVICE_ID_A, status=401, text="")

    result = await discover(hass, HOSTNAME_A, HOST_A)
    result = await hass.config_entries.flow.async_configure(
        result["flow_id"], CREDENTIALS
    )

    assert result["type"] is FlowResultType.FORM
    assert result["errors"] == {"base": "invalid_auth"}


async def test_duplicate_discovery_in_progress(
    hass: HomeAssistant, aioclient_mock: AiohttpClientMocker
) -> None:
    """The same device announced twice results in one flow."""
    mock_device_status(aioclient_mock, HOST_A, DEVICE_ID_A)

    first = await discover(hass, HOSTNAME_A, HOST_A)
    second = await discover(hass, HOSTNAME_A, HOST_A)

    assert first["type"] is FlowResultType.FORM
    assert second["type"] is FlowResultType.ABORT
    assert second["reason"] == "already_in_progress"


async def test_duplicate_discovery_after_setup(
    hass: HomeAssistant, aioclient_mock: AiohttpClientMocker
) -> None:
    """A configured device is not offered again, but its address follows."""
    mock_device_status(aioclient_mock, HOST_A, DEVICE_ID_A)
    mock_device_status(aioclient_mock, HOST_B, DEVICE_ID_A)
    entry = await setup_entry(hass, HOST_A, DEVICE_ID_A, HOSTNAME_A)

    result = await discover(hass, HOSTNAME_A, HOST_B)

    assert result["type"] is FlowResultType.FORM
    result = await hass.config_entries.flow.async_configure(
        result["flow_id"], CREDENTIALS
    )

    assert result["type"] is FlowResultType.ABORT
    assert result["reason"] == "already_configured"
    assert entry.data[CONF_HOST] == HOST_B
    assert len(hass.config_entries.async_entries(DOMAIN)) == 1


async def test_manual_then_discovery_is_one_device(
    hass: HomeAssistant, aioclient_mock: AiohttpClientMocker
) -> None:
    """Manual setup by IP and discovery produce the same unique id."""
    mock_device_status(aioclient_mock, HOST_A, DEVICE_ID_A)
    mock_device_status(aioclient_mock, HOST_B, DEVICE_ID_A)
    await add_manually(hass, USER_INPUT)
    unique_id = hass.config_entries.async_entries(DOMAIN)[0].unique_id

    result = await discover(hass, HOSTNAME_A, HOST_B)
    result = await hass.config_entries.flow.async_configure(
        result["flow_id"], CREDENTIALS
    )

    assert result["type"] is FlowResultType.ABORT
    assert result["reason"] == "already_configured"
    entries = hass.config_entries.async_entries(DOMAIN)
    assert [entry.unique_id for entry in entries] == [unique_id]
    assert entries[0].data[CONF_HOST] == HOST_B


async def test_discovery_then_manual_is_one_device(
    hass: HomeAssistant, aioclient_mock: AiohttpClientMocker
) -> None:
    """Discovery and a later manual setup by IP produce one entry."""
    mock_device_status(aioclient_mock, HOST_A, DEVICE_ID_A)
    result = await discover(hass, HOSTNAME_A, HOST_A)
    await hass.config_entries.flow.async_configure(
        result["flow_id"], CREDENTIALS
    )

    result = await add_manually(hass, USER_INPUT)

    assert result["type"] is FlowResultType.ABORT
    assert result["reason"] == "already_configured"
    assert len(hass.config_entries.async_entries(DOMAIN)) == 1


async def test_two_devices(
    hass: HomeAssistant, aioclient_mock: AiohttpClientMocker
) -> None:
    """Different devices are different entries."""
    mock_device_status(aioclient_mock, HOST_A, DEVICE_ID_A)
    mock_device_status(aioclient_mock, HOST_B, DEVICE_ID_B)

    await add_manually(hass, USER_INPUT)
    result = await add_manually(hass, {**USER_INPUT, CONF_HOST: HOST_B})

    assert result["type"] is FlowResultType.CREATE_ENTRY
    assert len(hass.config_entries.async_entries(DOMAIN)) == 2


async def start_reauth(hass: HomeAssistant, entry: Any) -> dict[str, Any]:
    """Start the reauthentication flow of an entry.

    Args:
        hass: The Home Assistant instance.
        entry: The config entry.

    Returns:
        The first result of the flow, which is the form.
    """
    result = await entry.start_reauth_flow(hass)
    assert result["type"] is FlowResultType.FORM
    assert result["step_id"] == "reauth_confirm"

    return result


async def test_reauthentication(
    hass: HomeAssistant, aioclient_mock: AiohttpClientMocker
) -> None:
    """New credentials replace the old ones, the entry stays."""
    mock_device_status(aioclient_mock, HOST_A, DEVICE_ID_A)
    entry = await setup_entry(hass, HOST_A, DEVICE_ID_A, HOSTNAME_A)
    entry_id = entry.entry_id
    device = device_id(hass, entry)
    result = await start_reauth(hass, entry)

    aioclient_mock.clear_requests()
    mock_device_status(aioclient_mock, HOST_A,
                       DEVICE_ID_A, status=401, text="")
    result = await hass.config_entries.flow.async_configure(
        result["flow_id"],
        {CONF_USERNAME: "han", CONF_PASSWORD: "wrong"},
    )
    assert result["type"] is FlowResultType.FORM
    assert result["errors"] == {"base": "invalid_auth"}

    aioclient_mock.clear_requests()
    mock_device_status(aioclient_mock, HOST_A, DEVICE_ID_A)
    result = await hass.config_entries.flow.async_configure(
        result["flow_id"],
        {CONF_USERNAME: "han", CONF_PASSWORD: "solo"},
    )
    await hass.async_block_till_done()

    assert result["type"] is FlowResultType.ABORT
    assert result["reason"] == "reauth_successful"
    entries = hass.config_entries.async_entries(DOMAIN)
    assert [e.entry_id for e in entries] == [entry_id]
    assert entries[0].data[CONF_USERNAME] == "han"
    assert entries[0].data[CONF_PASSWORD] == "solo"
    assert entries[0].data[CONF_HOST] == HOST_A
    assert entries[0].unique_id == DEVICE_ID_A.lower()
    assert entries[0].state is config_entries.ConfigEntryState.LOADED
    assert device_id(hass, entries[0]) == device


async def test_reauthentication_other_device(
    hass: HomeAssistant, aioclient_mock: AiohttpClientMocker
) -> None:
    """The login of another device is never stored in the entry."""
    mock_device_status(aioclient_mock, HOST_A, DEVICE_ID_A)
    entry = await setup_entry(hass, HOST_A, DEVICE_ID_A, HOSTNAME_A)
    result = await start_reauth(hass, entry)

    aioclient_mock.clear_requests()
    mock_device_status(aioclient_mock, HOST_A, DEVICE_ID_B)

    result = await hass.config_entries.flow.async_configure(
        result["flow_id"], {CONF_USERNAME: "han", CONF_PASSWORD: "solo"}
    )

    assert result["type"] is FlowResultType.ABORT
    assert result["reason"] == "wrong_device"
    assert entry.data[CONF_USERNAME] == USERNAME


async def test_migration_of_address_based_unique_id(
    hass: HomeAssistant, aioclient_mock: AiohttpClientMocker
) -> None:
    """A version 1 entry gets the device id as unique id, device stays."""
    mock_device_status(aioclient_mock, HOST_A, DEVICE_ID_A)
    registry = dr.async_get(hass)
    entry = await setup_entry(
        hass, HOST_A, DEVICE_ID_A, HOSTNAME_A, version=1, unique_id=HOST_A
    )

    assert entry.version == 2
    assert entry.unique_id == DEVICE_ID_A.lower()
    devices = dr.async_entries_for_config_entry(registry, entry.entry_id)
    assert len(devices) == 1
    assert (DOMAIN, DEVICE_ID_A.lower()) in devices[0].identifiers


async def test_migration_keeps_device(
    hass: HomeAssistant, aioclient_mock: AiohttpClientMocker
) -> None:
    """The device of a version 1 entry keeps its id and its assignment."""
    mock_device_status(aioclient_mock, HOST_A, DEVICE_ID_A)
    registry = dr.async_get(hass)
    entry = MockConfigEntry(
        domain=DOMAIN,
        version=1,
        unique_id=HOST_A,
        title=HOST_A,
        data={**USER_INPUT},
    )
    entry.add_to_hass(hass)
    old_device = registry.async_get_or_create(
        config_entry_id=entry.entry_id, identifiers={(DOMAIN, HOST_A)}
    )

    assert await hass.config_entries.async_setup(entry.entry_id)
    await hass.async_block_till_done()

    new_device = registry.async_get(old_device.id)
    assert new_device is not None
    assert (DOMAIN, DEVICE_ID_A.lower()) in new_device.identifiers
    assert len(registry.devices) == 1


async def test_migration_of_offline_device_keeps_unique_id(
    hass: HomeAssistant, aioclient_mock: AiohttpClientMocker
) -> None:
    """Without an answer of the device, the old unique id stays."""
    mock_device_status(aioclient_mock, HOST_A, DEVICE_ID_A,
                       exc=aiohttp.ClientError)

    entry = await setup_entry(
        hass,
        HOST_A,
        DEVICE_ID_A,
        HOSTNAME_A,
        version=1,
        unique_id=DEVICE_ID_A.lower(),
    )

    assert entry.version == 2
    assert entry.unique_id == DEVICE_ID_A.lower()
    assert entry.state is config_entries.ConfigEntryState.LOADED


async def test_migration_does_not_create_duplicate_id(
    hass: HomeAssistant, aioclient_mock: AiohttpClientMocker
) -> None:
    """A duplicate entry of one device keeps its old unique id."""
    mock_device_status(aioclient_mock, HOST_A, DEVICE_ID_A)
    mock_device_status(aioclient_mock, HOST_B, DEVICE_ID_A)
    await setup_entry(hass, HOST_A, DEVICE_ID_A, HOSTNAME_A)

    duplicate = await setup_entry(
        hass, HOST_B, DEVICE_ID_A, HOSTNAME_A, version=1, unique_id=HOST_B
    )

    assert duplicate.version == 2
    assert duplicate.unique_id == HOST_B


async def test_setup_entry_auth_failed_triggers_reauth(
    hass: HomeAssistant, aioclient_mock: AiohttpClientMocker
) -> None:
    """A rejected login during startup triggers reauthentication."""
    mock_device_status(aioclient_mock, HOST_A,
                       DEVICE_ID_A, status=401, text="")
    entry = MockConfigEntry(
        domain=DOMAIN,
        version=2,
        unique_id=DEVICE_ID_A.lower(),
        title=HOSTNAME_A,
        data=USER_INPUT,
    )
    entry.add_to_hass(hass)

    await hass.config_entries.async_setup(entry.entry_id)
    await hass.async_block_till_done()

    assert entry.state is config_entries.ConfigEntryState.SETUP_ERROR
    flows = hass.config_entries.flow.async_progress_by_handler(DOMAIN)
    assert any(f["context"].get("source") == "reauth" for f in flows)


async def test_setup_entry_offline_succeeds(
    hass: HomeAssistant, aioclient_mock: AiohttpClientMocker
) -> None:
    """An unreachable device during startup does not prevent setup."""
    mock_device_status(aioclient_mock, HOST_A, DEVICE_ID_A,
                       exc=aiohttp.ClientError)
    entry = MockConfigEntry(
        domain=DOMAIN,
        version=2,
        unique_id=DEVICE_ID_A.lower(),
        title=HOSTNAME_A,
        data=USER_INPUT,
    )
    entry.add_to_hass(hass)

    assert await hass.config_entries.async_setup(entry.entry_id)
    await hass.async_block_till_done()

    assert entry.state is config_entries.ConfigEntryState.LOADED


async def test_setup_entry_online_updates_migrated_unique_id(
    hass: HomeAssistant, aioclient_mock: AiohttpClientMocker
) -> None:
    """An entry with old IP unique_id is updated when it comes online."""
    mock_device_status(aioclient_mock, HOST_A, DEVICE_ID_A)
    entry = MockConfigEntry(
        domain=DOMAIN,
        version=2,
        unique_id=HOST_A,
        title=HOST_A,
        data=USER_INPUT,
    )
    entry.add_to_hass(hass)

    assert await hass.config_entries.async_setup(entry.entry_id)
    await hass.async_block_till_done()

    assert entry.unique_id == DEVICE_ID_A.lower()
