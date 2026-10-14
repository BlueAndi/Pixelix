"""Tests of the <feature> of the <domain> integration."""

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
from homeassistant import config_entries
from homeassistant.core import HomeAssistant
from homeassistant.data_entry_flow import FlowResultType
from pytest_homeassistant_custom_component.test_util.aiohttp import (
    AiohttpClientMocker,
)

from custom_components.pixelix.const import DOMAIN

from .helpers import (
    DEVICE_ID_A,
    HOST_A,
    PASSWORD,
    USERNAME,
    mock_device_status,
)

###############################################################################
# Variables
###############################################################################
USER_INPUT = {
    "host": HOST_A,
    "port": 80,
    "username": USERNAME,
    "password": PASSWORD,
}

###############################################################################
# Classes
###############################################################################

###############################################################################
# Functions
###############################################################################


async def test_user_flow_creates_entry(
    hass: HomeAssistant,
    aioclient_mock: AiohttpClientMocker,
) -> None:
    """A complete user flow creates one config entry.

    Args:
        hass: The Home Assistant test instance.
        aioclient_mock: The mock of the aiohttp client.

    Returns:
        None.
    """
    mock_device_status(aioclient_mock, HOST_A, DEVICE_ID_A)

    result = await hass.config_entries.flow.async_init(
        DOMAIN, context={"source": config_entries.SOURCE_USER}
    )
    assert result["type"] is FlowResultType.FORM
    assert result["step_id"] == "user"

    result = await hass.config_entries.flow.async_configure(
        result["flow_id"], USER_INPUT
    )
    await hass.async_block_till_done()

    assert result["type"] is FlowResultType.CREATE_ENTRY
    assert result["data"] == USER_INPUT


@pytest.mark.parametrize(
    ("status", "expected_error"),
    [
        (401, "invalid_auth"),
        (500, "cannot_connect"),
    ],
)
async def test_user_flow_shows_error(
    hass: HomeAssistant,
    aioclient_mock: AiohttpClientMocker,
    status: int,
    expected_error: str,
) -> None:
    """A failing device answer shows the matching error and keeps the form.

    Args:
        hass: The Home Assistant test instance.
        aioclient_mock: The mock of the aiohttp client.
        status: The HTTP status which the device answers with.
        expected_error: The error key which the flow is expected to show.

    Returns:
        None.
    """
    mock_device_status(aioclient_mock, HOST_A, DEVICE_ID_A, status=status)

    result = await hass.config_entries.flow.async_init(
        DOMAIN, context={"source": config_entries.SOURCE_USER}
    )
    result = await hass.config_entries.flow.async_configure(
        result["flow_id"], USER_INPUT
    )

    assert result["type"] is FlowResultType.FORM
    assert result["errors"] == {"base": expected_error}
