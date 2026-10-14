"""Pixelix integration, which shows notifications on Pixelix devices."""

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
import json
import logging
from collections.abc import Callable
from dataclasses import dataclass
from typing import Any

# Home Assistant and voluptuous are provided by the HA runtime only.
# pylint: disable=import-error
import voluptuous as vol

from homeassistant.components import mqtt
from homeassistant.config_entries import ConfigEntry
from homeassistant.const import (
    ATTR_DEVICE_ID,
    CONF_HOST,
    CONF_PASSWORD,
    CONF_PORT,
    CONF_USERNAME,
)
from homeassistant.core import HomeAssistant, ServiceCall
from homeassistant.exceptions import (
    ConfigEntryAuthFailed,
    HomeAssistantError,
    ServiceValidationError,
)
from homeassistant.helpers import config_validation as cv
from homeassistant.helpers import device_registry as dr
from homeassistant.helpers.aiohttp_client import async_get_clientsession
from homeassistant.helpers.typing import ConfigType

from .api import PixelixAuthError, PixelixClient, PixelixError
from .const import (
    ATTR_DURATION,
    ATTR_ICON_FILE_NAME,
    ATTR_MELODY,
    ATTR_MESSAGE,
    ATTR_VOLUME,
    DOMAIN,
    MANUFACTURER,
    MAX_MELODY_BYTES,
    MAX_TEXT_BYTES,
    MAX_UINT32,
    MAX_VOLUME,
    MODEL,
    SERVICE_NOTIFY,
)
from .validation import max_bytes, normalize_name

###############################################################################
# Variables
###############################################################################
_LOGGER = logging.getLogger(__name__)


@dataclass
class PixelixRuntimeData:
    """Runtime state of one Pixelix device."""

    client: PixelixClient
    hostname: str | None = None
    mqtt_online: bool = False
    mqtt_unsubscribe: Callable[[], None] | None = None


type PixelixConfigEntry = ConfigEntry[PixelixRuntimeData]

# Pixelix devices are only set up by config entries. Home Assistant requires
# this name.
# pylint: disable-next=invalid-name
CONFIG_SCHEMA = cv.config_entry_only_config_schema(DOMAIN)

# Service field name and the matching field of the REST API.
NOTIFY_PARAMS = (
    (ATTR_MESSAGE, "text"),
    (ATTR_DURATION, "duration"),
    (ATTR_ICON_FILE_NAME, "iconFileName"),
    (ATTR_MELODY, "melody"),
    (ATTR_VOLUME, "volume"),
)

# The ranges match what the Pixelix firmware accepts. A duration of 0 is
# handled by the firmware (defaults to 10 s).
NOTIFY_SCHEMA = vol.Schema(
    {
        vol.Optional(ATTR_DEVICE_ID): vol.All(cv.ensure_list, [cv.string]),
        vol.Required(ATTR_MESSAGE): vol.All(
            cv.string, vol.Length(min=1), max_bytes(MAX_TEXT_BYTES)
        ),
        vol.Optional(ATTR_DURATION): vol.All(
            vol.Coerce(int), vol.Range(min=0, max=MAX_UINT32)
        ),
        vol.Optional(ATTR_ICON_FILE_NAME): cv.string,
        vol.Optional(ATTR_MELODY): vol.All(
            cv.string, max_bytes(MAX_MELODY_BYTES)
        ),
        vol.Optional(ATTR_VOLUME): vol.All(
            vol.Coerce(int), vol.Range(min=0, max=MAX_VOLUME)
        ),
    },
    extra=vol.PREVENT_EXTRA,
)

###############################################################################
# Classes
###############################################################################

###############################################################################
# Functions
###############################################################################


async def async_setup(hass: HomeAssistant, config: ConfigType) -> bool:
    """Register the notify service.

    Args:
        hass: The Home Assistant instance.
        config: The complete Home Assistant configuration, which is not
            used because devices are set up by config entries.

    Returns:
        True, because the setup can not fail.
    """
    del config

    hass.services.async_register(
        DOMAIN, SERVICE_NOTIFY, _async_notify, schema=NOTIFY_SCHEMA
    )
    return True


async def async_setup_entry(
    hass: HomeAssistant, entry: PixelixConfigEntry
) -> bool:
    """Set up a Pixelix device.

    Args:
        hass: The Home Assistant instance.
        entry: The config entry of the device.

    Returns:
        True, because an offline device during startup does not block setup.

    Raises:
        ConfigEntryAuthFailed: The web login was rejected.
    """
    client = _create_client(hass, entry)
    runtime_data = PixelixRuntimeData(client=client)
    entry.runtime_data = runtime_data

    # Verify authentication if the device is reachable. If rejected, request
    # reauthentication right at startup. An offline device does not block.
    try:
        device_id = await client.async_get_device_id()
    except PixelixAuthError as err:
        raise ConfigEntryAuthFailed("The web login was rejected.") from err
    except PixelixError as err:
        _LOGGER.debug(
            "Device %s not reachable during startup (%s), setup continues.",
            entry.title,
            type(err).__name__,
        )
    else:
        new_unique_id = normalize_name(device_id)
        if entry.unique_id != new_unique_id:
            old_unique_id = entry.unique_id
            if not any(
                other.entry_id != entry.entry_id
                and other.unique_id == new_unique_id
                for other in hass.config_entries.async_entries(DOMAIN)
            ):
                hass.config_entries.async_update_entry(
                    entry, unique_id=new_unique_id
                )
                _migrate_device_identifier(
                    hass, entry, old_unique_id, new_unique_id
                )

    runtime_data.hostname = client.hostname or entry.title
    await _async_subscribe_to_availability(hass, runtime_data)

    dr.async_get(hass).async_get_or_create(
        config_entry_id=entry.entry_id,
        identifiers={(DOMAIN, entry.unique_id or entry.entry_id)},
        name=entry.title,
        manufacturer=MANUFACTURER,
        model=MODEL,
        configuration_url=client.base_url,
        **_device_details(client),
    )
    return True


async def async_unload_entry(
    hass: HomeAssistant, entry: PixelixConfigEntry
) -> bool:
    """Unload a Pixelix device.

    Args:
        hass: The Home Assistant instance.
        entry: The config entry of the device.

    Returns:
        True, because there is nothing to release.
    """
    del hass

    if entry.runtime_data.mqtt_unsubscribe is not None:
        entry.runtime_data.mqtt_unsubscribe()

    return True


async def async_migrate_entry(
    hass: HomeAssistant, entry: PixelixConfigEntry
) -> bool:
    """Migrate a config entry to the current version.

    Version 1 used the address which the user entered as unique id, if the
    device was not added by discovery. Version 2 uses the host name which the
    device reports. The device of the entry keeps its id, therefore
    automations and the device assignment stay intact.

    If the device is not reachable, the old unique id is kept. It is already
    correct, if the device was discovered or added by its host name.

    Args:
        hass: The Home Assistant instance.
        entry: The config entry to migrate.

    Returns:
        True if the entry is on the current version, False for an entry of a
        newer version which can not be used.
    """
    if entry.version > 2:
        return False

    if entry.version == 1:
        old_unique_id = entry.unique_id
        new_unique_id = old_unique_id

        try:
            device_id = await _create_client(hass, entry).async_get_device_id()
        except PixelixError as err:
            _LOGGER.warning(
                "Cannot read the device id of %s (%s), its unique id is not "
                "migrated.",
                entry.title,
                type(err).__name__,
            )
        else:
            candidate = normalize_name(device_id)
            if any(
                other.entry_id != entry.entry_id
                and other.unique_id == candidate
                for other in hass.config_entries.async_entries(DOMAIN)
            ):
                _LOGGER.warning(
                    "%s is configured twice. Remove the duplicate entry.",
                    device_id,
                )
            else:
                new_unique_id = candidate

        hass.config_entries.async_update_entry(
            entry, unique_id=new_unique_id, version=2
        )
        if new_unique_id != old_unique_id:
            _migrate_device_identifier(
                hass, entry, old_unique_id, new_unique_id
            )

    return True


def _create_client(
    hass: HomeAssistant, entry: PixelixConfigEntry
) -> PixelixClient:
    """Create the REST client of a config entry.

    Args:
        hass: The Home Assistant instance.
        entry: The config entry of the device.

    Returns:
        The client, which uses the HTTP session of Home Assistant.
    """
    return PixelixClient(
        async_get_clientsession(hass),
        entry.data[CONF_HOST],
        entry.data[CONF_PORT],
        entry.data[CONF_USERNAME],
        entry.data[CONF_PASSWORD],
    )


def _device_details(client: PixelixClient) -> dict[str, Any]:
    """Collect the device details which the device reported, if any.

    Only details which the device actually reported are returned. Passing
    None to the device registry would clear a value which an earlier, online
    setup had stored, so a device which is offline or runs an older firmware
    keeps what is already known about it.

    The MAC address is reported as a connection. Home Assistant merges
    devices which share a connection, so the device of this integration and
    the one created by the MQTT discovery of the firmware become one device.

    Args:
        client: The client of the device, after its status was read.

    Returns:
        The keyword arguments for the device registry, which may be empty.
    """
    details: dict[str, Any] = {}

    if client.sw_version is not None:
        details["sw_version"] = client.sw_version

    if client.hw_version is not None:
        details["hw_version"] = client.hw_version

    if client.mac is not None:
        details["connections"] = {
            (dr.CONNECTION_NETWORK_MAC, dr.format_mac(client.mac))
        }

    return details


async def _async_subscribe_to_availability(
    hass: HomeAssistant, runtime_data: PixelixRuntimeData
) -> None:
    """Track the retained MQTT availability state of the device.

    Args:
        hass: The Home Assistant instance.
        runtime_data: The runtime state of the device, which receives the
            availability and the unsubscribe callback.

    Returns:
        None.
    """
    if (
        runtime_data.hostname is not None
        and "mqtt" in hass.config.components
    ):
        async def _async_handle_message(message: Any) -> None:
            """Store whether the device reports itself as online.

            Args:
                message: The received MQTT message.

            Returns:
                None.
            """
            runtime_data.mqtt_online = message.payload == "online"

        try:
            runtime_data.mqtt_unsubscribe = await mqtt.async_subscribe(
                hass,
                f"{runtime_data.hostname}/status",
                _async_handle_message,
                qos=0,
            )
        except HomeAssistantError as err:
            _LOGGER.debug(
                "Cannot subscribe to MQTT availability of %s (%s).",
                runtime_data.hostname,
                type(err).__name__,
            )


def _migrate_device_identifier(
    hass: HomeAssistant,
    entry: PixelixConfigEntry,
    old_unique_id: str | None,
    new_unique_id: str | None,
) -> None:
    """Let the device follow the new unique id of its config entry.

    Args:
        hass: The Home Assistant instance.
        entry: The config entry of the device.
        old_unique_id: The unique id before the migration.
        new_unique_id: The unique id after the migration.

    Returns:
        None.
    """
    registry = dr.async_get(hass)
    identifier = (DOMAIN, old_unique_id or entry.entry_id)
    device = next(
        (
            candidate
            for candidate in dr.async_entries_for_config_entry(
                registry, entry.entry_id
            )
            if identifier in candidate.identifiers
        ),
        None,
    )

    if device is not None:
        try:
            registry.async_update_device(
                device.id,
                new_identifiers={(DOMAIN, new_unique_id or entry.entry_id)},
            )
        except HomeAssistantError:
            _LOGGER.warning(
                "The device of %s keeps its old identifier.", entry.title
            )


async def _async_notify(call: ServiceCall) -> None:
    """Show a notification on the selected Pixelix devices.

    Only the fields given in the service call are sent. Without a target
    device, all configured devices show the notification. The requests run
    in parallel, an unreachable device delays only its own request, which
    ends after the request timeout at the latest.

    Args:
        call: The service call, validated by ``NOTIFY_SCHEMA``. It provides
            ``message`` and optionally the target ``device_id``,
            ``duration`` in ms, ``icon_file_name``, ``melody`` in RTTTL
            format and ``volume``.

    Returns:
        None.

    Raises:
        ServiceValidationError: The target is empty or contains a device
            which is not a loaded Pixelix device. Nothing is sent then.
        HomeAssistantError: At least one device did not show the
            notification. The message names every failed device. A device
            which rejected the web login starts the reauthentication.
    """
    entries = _select_entries(call)
    payload = {}
    for field, field_name in NOTIFY_PARAMS:
        if field in call.data:
            val = call.data[field]
            if (
                field == ATTR_ICON_FILE_NAME
                and isinstance(val, str)
                and val
                and not val.startswith("/")
            ):
                val = f"/{val}"
            payload[field_name] = val

    requests = []
    for entry in entries:
        runtime_data = entry.runtime_data
        if (
            runtime_data.hostname is not None
            and runtime_data.mqtt_online
            and "mqtt" in call.hass.config.components
            and mqtt.is_connected(call.hass)
        ):
            requests.append(
                mqtt.async_publish(
                    call.hass,
                    f"{runtime_data.hostname}/notify/set",
                    json.dumps(payload),
                    qos=0,
                    retain=False,
                )
            )
        else:
            requests.append(runtime_data.client.async_notify(payload))

    results = await asyncio.gather(*requests, return_exceptions=True)

    errors = []
    for entry, result in zip(entries, results):
        if isinstance(result, PixelixAuthError):
            entry.async_start_reauth(call.hass)
            errors.append(
                f"{entry.title}: {result} Update the web login of the device."
            )
        elif isinstance(result, Exception):
            if not isinstance(result, PixelixError):
                _LOGGER.error(
                    "Unexpected error while notifying %s.",
                    entry.title,
                    exc_info=result,
                )
            errors.append(f"{entry.title}: {result}")

    if errors:
        raise HomeAssistantError("; ".join(errors))


def _select_entries(call: ServiceCall) -> list[PixelixConfigEntry]:
    """Get the config entries which a service call addresses.

    Args:
        call: The service call, which may contain target device ids.

    Returns:
        The loaded entries of the target devices, or of all devices if the
        call has no target at all.

    Raises:
        ServiceValidationError: There is no device, the target is empty or
            one of the target devices is unknown. It never falls back to
            all devices in these cases.
    """
    loaded = {
        entry.entry_id: entry
        for entry in call.hass.config_entries.async_loaded_entries(DOMAIN)
    }
    device_ids = call.data.get(ATTR_DEVICE_ID)

    if device_ids is None:
        entries = list(loaded.values())
        if not entries:
            raise ServiceValidationError("No Pixelix device is configured.")
        return entries

    if not device_ids:
        raise ServiceValidationError("No target device is selected.")

    registry = dr.async_get(call.hass)
    entries = []
    unknown = []

    for device_id in device_ids:
        device = registry.async_get(device_id)
        entry = None
        if device is not None:
            entry = next(
                (
                    candidate
                    for candidate in loaded.values()
                    if device in dr.async_entries_for_config_entry(
                        registry, candidate.entry_id
                    )
                ),
                None,
            )

        if entry is None:
            unknown.append(device_id)
        elif entry not in entries:
            entries.append(entry)

    if unknown:
        raise ServiceValidationError(
            "Not a Pixelix device or the device is not loaded: "
            + ", ".join(unknown)
        )

    return entries
