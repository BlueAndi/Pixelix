"""Config flow of the Pixelix integration."""

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
from collections.abc import Mapping

# Home Assistant and voluptuous are provided by the HA runtime only.
# pylint: disable=import-error
import voluptuous as vol

from homeassistant.config_entries import ConfigFlow, ConfigFlowResult
from homeassistant.const import (
    CONF_HOST,
    CONF_PASSWORD,
    CONF_PORT,
    CONF_USERNAME,
)
from homeassistant.helpers import config_validation as cv
from homeassistant.helpers.aiohttp_client import async_get_clientsession
from homeassistant.helpers.selector import (
    TextSelector,
    TextSelectorConfig,
    TextSelectorType,
)
from homeassistant.helpers.service_info.zeroconf import ZeroconfServiceInfo

from .api import (
    PixelixAuthError,
    PixelixClient,
    PixelixConnectionError,
    PixelixError,
)
from .const import DEFAULT_PASSWORD, DEFAULT_PORT, DEFAULT_USERNAME, DOMAIN
from .validation import normalize_name

###############################################################################
# Variables
###############################################################################
PASSWORD_SELECTOR = TextSelector(
    TextSelectorConfig(type=TextSelectorType.PASSWORD)
)

CREDENTIALS_SCHEMA = vol.Schema(
    {
        vol.Required(CONF_USERNAME, default=DEFAULT_USERNAME): str,
        vol.Required(CONF_PASSWORD, default=DEFAULT_PASSWORD): (
            PASSWORD_SELECTOR
        ),
    }
)

# The default web login is not offered again, because it was rejected.
REAUTH_SCHEMA = vol.Schema(
    {
        vol.Required(CONF_USERNAME): str,
        vol.Required(CONF_PASSWORD): PASSWORD_SELECTOR,
    }
)

USER_SCHEMA = vol.Schema(
    {
        vol.Required(CONF_HOST): str,
        vol.Required(CONF_PORT, default=DEFAULT_PORT): cv.port,
    }
).extend(CREDENTIALS_SCHEMA.schema)

###############################################################################
# Classes
###############################################################################


# Home Assistant deprecated is_matching(), a flow does not implement it.
# pylint: disable-next=abstract-method
class PixelixConfigFlow(ConfigFlow, domain=DOMAIN):
    """Add a Pixelix device by mDNS discovery or by its host name or IP.

    A device is identified by its device id (ESP chip id) which it reports
    in the REST status. The IP address and host name are never used as
    unique id. Therefore both manual setup and mDNS discovery produce the
    same unique id.
    """

    VERSION = 2

    def __init__(self) -> None:
        """Create the flow.

        Returns:
            None.
        """
        self._host = ""
        self._port = DEFAULT_PORT
        self._name = ""

    async def async_step_user(
        self, user_input: dict[str, Any] | None = None
    ) -> ConfigFlowResult:
        """Add a device by manual input.

        Args:
            user_input: The form input. It is None for the first call,
                which shows the form.

        Returns:
            The form, or the created config entry.
        """
        errors: dict[str, str] = {}

        if user_input is not None:
            host = user_input[CONF_HOST].strip()
            port = user_input[CONF_PORT]

            error, device_id = await self._async_authenticate(
                host,
                port,
                user_input[CONF_USERNAME],
                user_input[CONF_PASSWORD],
            )
            if error is None:
                await self.async_set_unique_id(normalize_name(device_id))
                self._abort_if_unique_id_configured(
                    updates={CONF_HOST: host, CONF_PORT: port}
                )
                return self.async_create_entry(
                    title=host,
                    data={**user_input, CONF_HOST: host},
                )
            errors["base"] = error

        return self.async_show_form(
            step_id="user",
            data_schema=self.add_suggested_values_to_schema(
                USER_SCHEMA, user_input
            ),
            errors=errors,
        )

    async def async_step_zeroconf(
        self, discovery_info: ZeroconfServiceInfo
    ) -> ConfigFlowResult:
        """Handle a device found by mDNS.

        The mDNS name is used as tentative title for the discovery step.
        The permanent unique id is determined from the device status when
        credentials are confirmed.

        Args:
            discovery_info: The mDNS service information of the device.

        Returns:
            The form to enter the web login.
        """
        name = discovery_info.name.removesuffix(f".{discovery_info.type}")
        host = _select_host(discovery_info)
        port = discovery_info.port or DEFAULT_PORT

        await self.async_set_unique_id(normalize_name(name))
        self._abort_if_unique_id_configured(
            updates={CONF_HOST: host, CONF_PORT: port}
        )
        self._async_abort_entries_match({CONF_HOST: host})

        self._host = host
        self._port = port
        self._name = name
        self.context["title_placeholders"] = {"name": name}

        return await self.async_step_discovery_confirm()

    async def async_step_discovery_confirm(
        self, user_input: dict[str, Any] | None = None
    ) -> ConfigFlowResult:
        """Ask for the web login of a discovered device.

        Args:
            user_input: The form input. It is None for the first call,
                which shows the form.

        Returns:
            The form, or the created config entry.
        """
        errors: dict[str, str] = {}

        if user_input is not None:
            error, device_id = await self._async_authenticate(
                self._host,
                self._port,
                user_input[CONF_USERNAME],
                user_input[CONF_PASSWORD],
            )
            if error is None:
                await self.async_set_unique_id(normalize_name(device_id))
                self._abort_if_unique_id_configured(
                    updates={CONF_HOST: self._host, CONF_PORT: self._port}
                )
                return self.async_create_entry(
                    title=self._name or device_id,
                    data={
                        CONF_HOST: self._host,
                        CONF_PORT: self._port,
                        CONF_USERNAME: user_input[CONF_USERNAME],
                        CONF_PASSWORD: user_input[CONF_PASSWORD],
                    },
                )
            errors["base"] = error

        return self.async_show_form(
            step_id="discovery_confirm",
            data_schema=self.add_suggested_values_to_schema(
                CREDENTIALS_SCHEMA, user_input
            ),
            description_placeholders={
                "name": self._name,
                "host": self._host,
            },
            errors=errors,
        )

    async def async_step_reauth(
        self, entry_data: Mapping[str, Any]
    ) -> ConfigFlowResult:
        """Start the reauthentication after the device rejected the login.

        Args:
            entry_data: The data of the config entry.

        Returns:
            The form to enter the new web login.
        """
        del entry_data

        return await self.async_step_reauth_confirm()

    async def async_step_reauth_confirm(
        self, user_input: dict[str, Any] | None = None
    ) -> ConfigFlowResult:
        """Ask for the new web login and update the config entry.

        The config entry, its device and its unique id are kept.

        Args:
            user_input: The form input. It is None for the first call,
                which shows the form.

        Returns:
            The form, or the abort result after the entry was updated.
        """
        entry = self._get_reauth_entry()
        errors: dict[str, str] = {}

        if user_input is not None:
            error, device_id = await self._async_authenticate(
                entry.data[CONF_HOST],
                entry.data[CONF_PORT],
                user_input[CONF_USERNAME],
                user_input[CONF_PASSWORD],
            )
            if error is None:
                # Do not store the login of another device in this entry.
                await self.async_set_unique_id(normalize_name(device_id))
                self._abort_if_unique_id_mismatch(reason="wrong_device")
                return self.async_update_reload_and_abort(
                    entry,
                    data_updates={
                        CONF_USERNAME: user_input[CONF_USERNAME],
                        CONF_PASSWORD: user_input[CONF_PASSWORD],
                    },
                )
            errors["base"] = error

        return self.async_show_form(
            step_id="reauth_confirm",
            data_schema=self.add_suggested_values_to_schema(
                REAUTH_SCHEMA,
                user_input or {CONF_USERNAME: entry.data[CONF_USERNAME]},
            ),
            description_placeholders={
                "name": entry.title,
                "host": entry.data[CONF_HOST],
            },
            errors=errors,
        )

    async def _async_authenticate(
        self, host: str, port: int, username: str, password: str
    ) -> tuple[str | None, str]:
        """Check the device and read its device id.

        Args:
            host: Host name or IP address of the device.
            port: Port of the web server of the device.
            username: Web login user.
            password: Web login password.

        Returns:
            The error key of the form and the device id of the device.
            The error is None if the device is usable. Otherwise it is
            ``invalid_auth`` (login rejected), ``cannot_connect`` (device
            not reachable) or ``unexpected_response`` (the device is not a
            compatible Pixelix device). The device id is empty on error.
        """
        client = PixelixClient(
            async_get_clientsession(self.hass), host, port, username, password
        )
        error = None
        device_id = ""

        try:
            device_id = await client.async_get_device_id()
        except PixelixAuthError:
            error = "invalid_auth"
        except PixelixConnectionError:
            error = "cannot_connect"
        except PixelixError:
            error = "unexpected_response"

        return error, device_id


###############################################################################
# Functions
###############################################################################


def _select_host(discovery_info: ZeroconfServiceInfo) -> str:
    """Select the address to talk to a discovered device.

    Args:
        discovery_info: The mDNS service information of the device.

    Returns:
        The first IPv4 address of the device. Without one, the host which
        Home Assistant selected.
    """
    for address in discovery_info.ip_addresses:
        if address.version == 4:
            return str(address)

    return discovery_info.host
