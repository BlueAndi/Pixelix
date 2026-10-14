"""REST client of a Pixelix device."""

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
import asyncio
import base64
import json
import logging
from http import HTTPStatus
from typing import Any

# Home Assistant and its libraries are provided by the HA runtime only.
# pylint: disable=import-error
import aiohttp
from yarl import URL

from .const import REQUEST_TIMEOUT, REST_API_PATH

###############################################################################
# Variables
###############################################################################
# Neither the web login nor the content of a notification is ever logged.
_LOGGER = logging.getLogger(__name__)

###############################################################################
# Classes
###############################################################################


class PixelixError(Exception):
    """Base class of all errors of the Pixelix client."""


class PixelixConnectionError(PixelixError):
    """The device is not reachable, or it did not answer in time."""


class PixelixAuthError(PixelixError):
    """The web login was rejected by the device (HTTP 401)."""


class PixelixRequestError(PixelixError):
    """The device answered with an HTTP error status."""

    def __init__(self, message: str, status: int) -> None:
        """Create the error.

        Args:
            message: The reason, taken from the answer of the device if it
                provides one.
            status: The HTTP status code of the answer.

        Returns:
            None.
        """
        super().__init__(message)
        self.status = status


class PixelixResponseError(PixelixError):
    """The answer of the device is invalid or not compatible."""


class PixelixClient:
    """Client for the REST API of one Pixelix device."""

    def __init__(
        self,
        session: aiohttp.ClientSession,
        host: str,
        port: int,
        username: str,
        password: str,
    ) -> None:
        """Create the client.

        Args:
            session: The HTTP session to use. It is owned by the caller.
            host: Host name or IP address of the device.
            port: Port of the web server of the device.
            username: Web login user of the device.
            password: Web login password of the device.

        Returns:
            None.
        """
        self._session = session
        self._base_url = URL.build(scheme="http", host=host, port=port)
        self._hostname: str | None = None
        self._sw_version: str | None = None
        self._hw_version: str | None = None
        self._mac: str | None = None
        # Built by hand, because aiohttp deprecates its BasicAuth class. The
        # header is never logged.
        token = base64.b64encode(f"{username}:{password}".encode("utf-8"))
        self._headers = {"Authorization": f"Basic {token.decode('ascii')}"}

    @property
    def base_url(self) -> str:
        """Get the URL of the web interface of the device.

        Returns:
            The URL, e.g. ``http://192.168.1.10``.
        """
        return str(self._base_url)

    @property
    def hostname(self) -> str | None:
        """Get the device hostname reported by its device endpoint.

        Returns:
            The hostname, or None if the device did not report one.
        """
        return self._hostname

    @property
    def sw_version(self) -> str | None:
        """Get the firmware version reported by the device.

        Returns:
            The firmware version, e.g. ``"7.4.0"``, or None if the device
            did not report one. It is read by async_get_device_id().
        """
        return self._sw_version

    @property
    def hw_version(self) -> str | None:
        """Get the hardware of the device.

        The firmware reports the PlatformIO target name here, which
        identifies the board, e.g. ``"ulanzi-TC001"``.

        Returns:
            The hardware name, or None if the device did not report one.
            It is read by async_get_device_id().
        """
        return self._hw_version

    @property
    def mac(self) -> str | None:
        """Get the WiFi MAC address of the device.

        Older firmware versions do not report it.

        Returns:
            The MAC address as the device spells it, e.g.
            ``"A1:B2:C3:D4:E5:F6"``, or None if the device did not report
            one. It is read by async_get_device_id().
        """
        return self._mac

    async def async_get_device(self) -> dict[str, Any]:
        """Read the device information of the device.

        Returns:
            The JSON response of the device.

        Raises:
            PixelixConnectionError: The device is not reachable.
            PixelixAuthError: The web login was rejected.
            PixelixRequestError: The device answered with an HTTP error.
            PixelixResponseError: The answer is invalid or incompatible.
        """
        return await self._async_request("GET", "/device")

    async def async_get_device_id(self) -> str:
        """Read the unique device id from the device information.

        The device endpoint returns the chip id under ``device.id``, which
        uniquely identifies the physical device. It also verifies host, port
        and web login.

        Returns:
            The device id (chip id as hex string, e.g. ``"1a2b3c4d5e6f"``).

        Raises:
            PixelixConnectionError: The device is not reachable.
            PixelixAuthError: The web login was rejected.
            PixelixRequestError: The device answered with an HTTP error.
            PixelixResponseError: The answer is invalid or incompatible.
        """
        payload = await self.async_get_device()
        data = payload.get("data")
        device = data.get("device") if isinstance(data, dict) else None
        device_id = device.get("id") if isinstance(device, dict) else None

        if not isinstance(device, dict):
            device = {}

        self._hostname = _text_or_none(device.get("name"))
        self._sw_version = _text_or_none(device.get("swVersion"))
        self._hw_version = _text_or_none(device.get("hwVersion"))
        self._mac = _text_or_none(device.get("mac"))

        if not isinstance(device_id, str) or not device_id.strip():
            raise PixelixResponseError(
                "The device status did not contain a device id."
            )

        return device_id.strip()

    async def async_notify(self, payload: dict[str, str | int]) -> None:
        """Show a notification on the device.

        The payload is sent as JSON. The REST API converts values of URL
        parameters to strings, which the notification handler does not
        accept for ``duration`` and ``volume``. JSON keeps the types.

        Args:
            payload: The notification, which is ``text`` and optionally
                ``duration``, ``iconFileName``, ``melody`` and ``volume``.

        Returns:
            None.

        Raises:
            PixelixConnectionError: The device is not reachable.
            PixelixAuthError: The web login was rejected.
            PixelixRequestError: The device rejected the notification.
            PixelixResponseError: The answer is invalid or incompatible.
        """
        await self._async_request("POST", "/notify", payload=payload)

    async def _async_request(
        self,
        method: str,
        path: str,
        params: dict[str, str] | None = None,
        payload: dict[str, str | int] | None = None,
    ) -> dict[str, Any]:
        """Send a request to the REST API of the device.

        Args:
            method: The HTTP method.
            path: The path below the REST API base path.
            params: The URL parameters, if any.
            payload: The JSON body, if any.

        Returns:
            The JSON response of the device.

        Raises:
            PixelixConnectionError: The device is not reachable.
            PixelixAuthError: The web login was rejected.
            PixelixRequestError: The device answered with an HTTP error.
            PixelixResponseError: The answer is invalid or incompatible.
        """
        url = self._base_url.with_path(REST_API_PATH + path)

        try:
            async with asyncio.timeout(REQUEST_TIMEOUT):
                async with self._session.request(
                    method,
                    url,
                    params=params,
                    json=payload,
                    headers=self._headers,
                ) as response:
                    status = response.status
                    body = await response.text(errors="replace")
        except (aiohttp.ClientError, TimeoutError) as err:
            _LOGGER.debug(
                "%s %s%s: connection failed (%s).",
                method,
                self._base_url,
                path,
                type(err).__name__,
            )
            raise PixelixConnectionError(
                f"Cannot connect to {self._base_url}."
            ) from err

        _LOGGER.debug("%s %s%s: HTTP %s", method, self._base_url, path, status)

        # The Pixelix web server rejects a wrong login with 401 only.
        if status == HTTPStatus.UNAUTHORIZED:
            raise PixelixAuthError("The web login was rejected.")

        return self._parse_response(status, body)

    @staticmethod
    def _parse_response(status: int, body: str) -> dict[str, Any]:
        """Check the response of the device.

        Args:
            status: The HTTP status code.
            body: The response body, which is expected to be JSON.

        Returns:
            The decoded JSON response.

        Raises:
            PixelixRequestError: The device answered with an HTTP error
                status.
            PixelixResponseError: The answer is not a valid positive answer
                of the Pixelix REST API.
        """
        try:
            payload = json.loads(body)
        except ValueError:
            payload = None

        if status != HTTPStatus.OK:
            error = payload.get("error") if isinstance(payload, dict) else None
            message = error.get("msg") if isinstance(error, dict) else None
            raise PixelixRequestError(
                message
                if isinstance(message, str) and message
                else f"The device answered with HTTP {status}.",
                status,
            )

        if not isinstance(payload, dict) or payload.get("status") != "ok":
            raise PixelixResponseError(
                "The answer is not a valid Pixelix REST API response."
            )

        return payload


###############################################################################
# Functions
###############################################################################


def _text_or_none(value: Any) -> str | None:
    """Reduce a value of the device status to a non-empty string.

    The device may omit a field, report it as null or as an empty string.
    All of them mean that the information is not available.

    Args:
        value: The value as it was decoded from the JSON answer.

    Returns:
        The stripped text, or None if it is not a non-empty string.
    """
    if not isinstance(value, str) or not value.strip():
        return None

    return value.strip()
