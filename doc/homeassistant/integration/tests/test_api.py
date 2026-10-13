"""Tests of the REST client of the Pixelix integration."""

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
import base64
import logging

# Home Assistant is provided by the test environment only.
# pylint: disable=import-error
import aiohttp
import pytest
from homeassistant.core import HomeAssistant
from homeassistant.helpers.aiohttp_client import async_get_clientsession
from pytest_homeassistant_custom_component.test_util.aiohttp import (
    AiohttpClientMocker,
)

from custom_components.pixelix.api import (
    PixelixAuthError,
    PixelixClient,
    PixelixConnectionError,
    PixelixRequestError,
    PixelixResponseError,
)

from .helpers import (
    DEVICE_ID_A,
    HOST_A,
    INVALID_DATA_ANSWER,
    OK_RESPONSE,
    api_url,
    mock_device_status,
    mock_notify,
)

###############################################################################
# Variables
###############################################################################
SECRET = "s3cr3t-pa55word"

###############################################################################
# Classes
###############################################################################

###############################################################################
# Functions
###############################################################################


def create_client(hass: HomeAssistant) -> PixelixClient:
    """Create a client for the test device.

    Args:
        hass: The Home Assistant instance.

    Returns:
        The client, which uses a password that must never be logged.
    """
    return PixelixClient(
        async_get_clientsession(hass), HOST_A, 80, "luke", SECRET
    )


async def test_device_id(
    hass: HomeAssistant, aioclient_mock: AiohttpClientMocker
) -> None:
    """The unique device id is read from the status of the device."""
    mock_device_status(aioclient_mock, HOST_A, DEVICE_ID_A)

    assert await create_client(hass).async_get_device_id() == DEVICE_ID_A


@pytest.mark.parametrize(
    "answer",
    [
        {"json": {"data": {}, "status": "ok"}},
        {"json": {"data": {"device": {}}, "status": "ok"}},
        {"json": {"data": {"device": {"id": ""}}, "status": "ok"}},
        {"json": {"data": {"device": {"id": 5}}, "status": "ok"}},
    ],
)
async def test_device_id_missing(
    hass: HomeAssistant, aioclient_mock: AiohttpClientMocker, answer: dict
) -> None:
    """An answer without device.id is an incompatible response."""
    mock_device_status(aioclient_mock, HOST_A, DEVICE_ID_A, **answer)

    with pytest.raises(PixelixResponseError):
        await create_client(hass).async_get_device_id()


async def test_auth_error(
    hass: HomeAssistant, aioclient_mock: AiohttpClientMocker
) -> None:
    """HTTP 401 is the answer of the web server to a wrong login."""
    mock_notify(aioclient_mock, HOST_A, status=401, text="")

    with pytest.raises(PixelixAuthError):
        await create_client(hass).async_notify({"text": "Hello"})


async def test_forbidden_is_not_an_auth_error(
    hass: HomeAssistant, aioclient_mock: AiohttpClientMocker
) -> None:
    """The firmware never answers a login problem with HTTP 403."""
    mock_notify(aioclient_mock, HOST_A, status=403, text="Forbidden")

    with pytest.raises(PixelixRequestError) as err:
        await create_client(hass).async_notify({"text": "Hello"})

    assert err.value.status == 403


async def test_request_error_uses_message_of_device(
    hass: HomeAssistant, aioclient_mock: AiohttpClientMocker
) -> None:
    """The reason of the device is part of the error."""
    mock_notify(aioclient_mock, HOST_A, **INVALID_DATA_ANSWER)

    with pytest.raises(PixelixRequestError, match="Invalid data.") as err:
        await create_client(hass).async_notify({"text": "Hello"})

    assert err.value.status == 404


async def test_request_error_without_message(
    hass: HomeAssistant, aioclient_mock: AiohttpClientMocker
) -> None:
    """An unexpected HTTP status without a reason names the status."""
    mock_notify(aioclient_mock, HOST_A, status=500, text="<html></html>")

    with pytest.raises(PixelixRequestError, match="HTTP 500"):
        await create_client(hass).async_notify({"text": "Hello"})


@pytest.mark.parametrize(
    "answer",
    [
        {"text": "<html>not json</html>"},
        {"json": ["not", "an", "object"]},
        {"json": {"data": {}}},
        {"json": {"data": {}, "status": "error"}},
    ],
)
async def test_invalid_response(
    hass: HomeAssistant, aioclient_mock: AiohttpClientMocker, answer: dict
) -> None:
    """HTTP 200 with another body than the REST API is incompatible."""
    mock_notify(aioclient_mock, HOST_A, **answer)

    with pytest.raises(PixelixResponseError):
        await create_client(hass).async_notify({"text": "Hello"})


@pytest.mark.parametrize("exc", [aiohttp.ClientError, TimeoutError])
async def test_connection_error(
    hass: HomeAssistant, aioclient_mock: AiohttpClientMocker, exc: type
) -> None:
    """A not reachable device and a timeout are connection errors."""
    mock_notify(aioclient_mock, HOST_A, exc=exc)

    with pytest.raises(PixelixConnectionError, match=HOST_A):
        await create_client(hass).async_notify({"text": "Hello"})


async def test_json_body_keeps_types(
    hass: HomeAssistant, aioclient_mock: AiohttpClientMocker
) -> None:
    """Numbers are sent as numbers, which the firmware requires."""
    mock_notify(aioclient_mock, HOST_A, json=OK_RESPONSE)
    payload = {"text": "Hello", "duration": 5000, "volume": 3}

    await create_client(hass).async_notify(payload)

    assert aioclient_mock.mock_calls[0][2] == payload
    assert str(aioclient_mock.mock_calls[0][1]) == api_url(HOST_A, "/notify")


async def test_basic_auth_header(
    hass: HomeAssistant, aioclient_mock: AiohttpClientMocker
) -> None:
    """The web login is sent as HTTP basic authentication."""
    mock_notify(aioclient_mock, HOST_A, json=OK_RESPONSE)

    await create_client(hass).async_notify({"text": "Hello"})

    token = base64.b64encode(f"luke:{SECRET}".encode()).decode()
    assert aioclient_mock.mock_calls[0][3]["Authorization"] == f"Basic {token}"


async def test_secrets_are_not_logged(
    hass: HomeAssistant,
    aioclient_mock: AiohttpClientMocker,
    caplog: pytest.LogCaptureFixture,
) -> None:
    """Neither the password nor the notification text appears in a log."""
    caplog.set_level(logging.DEBUG)
    mock_notify(aioclient_mock, HOST_A, status=401, text="")

    with pytest.raises(PixelixAuthError) as err:
        await create_client(hass).async_notify({"text": "Private text"})

    assert SECRET not in str(err.value)
    assert SECRET not in caplog.text
    assert "Private text" not in caplog.text
