"""Tests of the notify action of the Pixelix integration."""

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
from types import SimpleNamespace
from typing import Any

# Home Assistant is provided by the test environment only.
# pylint: disable=import-error
import aiohttp
import pytest
import voluptuous as vol
from homeassistant.components import mqtt
from homeassistant.const import ATTR_DEVICE_ID
from homeassistant.core import HomeAssistant
from homeassistant.exceptions import (
    HomeAssistantError,
    ServiceValidationError,
)
from homeassistant.helpers import device_registry as dr
from pytest_homeassistant_custom_component.common import MockConfigEntry
from pytest_homeassistant_custom_component.test_util.aiohttp import (
    AiohttpClientMocker,
)

from custom_components.pixelix.const import DOMAIN, SERVICE_NOTIFY

from .helpers import (
    DEVICE_ID_A,
    DEVICE_ID_B,
    HOST_A,
    HOST_B,
    HOSTNAME_A,
    HOSTNAME_B,
    INVALID_DATA_ANSWER,
    device_id,
    mock_device_status,
    mock_notify,
    notify_payloads,
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


@pytest.fixture(autouse=True)
def auto_mock_devices(aioclient_mock: AiohttpClientMocker) -> None:
    """Mock device status responses so entries can be set up without errors."""
    mock_device_status(aioclient_mock, HOST_A, DEVICE_ID_A)
    mock_device_status(aioclient_mock, HOST_B, DEVICE_ID_B)


async def notify(hass: HomeAssistant, data: dict[str, Any]) -> None:
    """Call the notify action and wait until it is finished.

    Args:
        hass: The Home Assistant instance.
        data: The data of the service call.

    Returns:
        None.
    """
    await hass.services.async_call(
        DOMAIN, SERVICE_NOTIFY, data, blocking=True
    )


async def test_only_message(
    hass: HomeAssistant, aioclient_mock: AiohttpClientMocker
) -> None:
    """Without optional fields, only the text is sent."""
    await setup_entry(hass)
    mock_notify(aioclient_mock, HOST_A)

    await notify(hass, {"message": "Hello"})

    assert notify_payloads(aioclient_mock, HOST_A) == [{"text": "Hello"}]


async def test_notify_uses_mqtt_when_device_is_online(
    hass: HomeAssistant,
    aioclient_mock: AiohttpClientMocker,
    monkeypatch: pytest.MonkeyPatch,
) -> None:
    """An online device receives notifications through its MQTT topic."""
    availability_callbacks = []
    published = []

    async def subscribe(_hass, topic, callback, qos=0):
        assert topic == f"{HOSTNAME_A}/status"
        assert qos == 0
        availability_callbacks.append(callback)
        return lambda: None

    async def publish(_hass, topic, payload, qos=0, retain=False):
        published.append((topic, payload, qos, retain))

    hass.config.components.add("mqtt")
    monkeypatch.setattr(mqtt, "is_connected", lambda _hass: True)
    monkeypatch.setattr(mqtt, "async_subscribe", subscribe)
    monkeypatch.setattr(mqtt, "async_publish", publish)

    await setup_entry(hass)
    await availability_callbacks[0](SimpleNamespace(payload="online"))

    await notify(
        hass,
        {
            "message": "Hello",
            "duration": 5000,
            "icon_file_name": "smiley.bmp",
        },
    )

    assert published == [
        (
            f"{HOSTNAME_A}/notify/set",
            '{"text": "Hello", "duration": 5000, '
            '"iconFileName": "/smiley.bmp"}',
            0,
            False,
        )
    ]
    assert not notify_payloads(aioclient_mock, HOST_A)


async def test_notify_falls_back_to_rest_when_device_is_offline(
    hass: HomeAssistant,
    aioclient_mock: AiohttpClientMocker,
    monkeypatch: pytest.MonkeyPatch,
) -> None:
    """An offline MQTT device receives notifications through REST."""
    availability_callbacks = []

    async def subscribe(_hass, _topic, callback, qos=0):
        del qos
        availability_callbacks.append(callback)
        return lambda: None

    async def publish(*_args, **_kwargs):
        pytest.fail("MQTT must not be used for an offline device.")

    hass.config.components.add("mqtt")
    monkeypatch.setattr(mqtt, "is_connected", lambda _hass: True)
    monkeypatch.setattr(mqtt, "async_subscribe", subscribe)
    monkeypatch.setattr(mqtt, "async_publish", publish)

    await setup_entry(hass)
    await availability_callbacks[0](SimpleNamespace(payload="offline"))
    mock_notify(aioclient_mock, HOST_A)

    await notify(hass, {"message": "Hello"})

    assert notify_payloads(aioclient_mock, HOST_A) == [{"text": "Hello"}]


async def test_all_parameters(
    hass: HomeAssistant, aioclient_mock: AiohttpClientMocker
) -> None:
    """All fields are sent completely and with the names of the REST API."""
    await setup_entry(hass)
    mock_notify(aioclient_mock, HOST_A)

    await notify(
        hass,
        {
            "message": "The washing machine is finished!",
            "duration": 10000,
            "icon_file_name": "/configuration/smiley.bmp",
            "melody": "Finished:d=4,o=5,b=120:8c,8e,8g,c6",
            "volume": 5,
        },
    )

    assert notify_payloads(aioclient_mock, HOST_A) == [
        {
            "text": "The washing machine is finished!",
            "duration": 10000,
            "iconFileName": "/configuration/smiley.bmp",
            "melody": "Finished:d=4,o=5,b=120:8c,8e,8g,c6",
            "volume": 5,
        }
    ]


async def test_duration_zero_is_forwarded(
    hass: HomeAssistant, aioclient_mock: AiohttpClientMocker
) -> None:
    """The firmware maps a duration of 0 to its default, so it is sent."""
    await setup_entry(hass)
    mock_notify(aioclient_mock, HOST_A)

    await notify(hass, {"message": "Hello", "duration": 0})

    assert notify_payloads(aioclient_mock, HOST_A) == [
        {"text": "Hello", "duration": 0}
    ]


@pytest.mark.parametrize(
    ("field", "value"),
    [
        ("duration", -1),
        ("duration", 4294967296),
        ("duration", "soon"),
        ("volume", -1),
        ("volume", 11),
        ("volume", "loud"),
    ],
)
async def test_invalid_number(
    hass: HomeAssistant,
    aioclient_mock: AiohttpClientMocker,
    field: str,
    value: Any,
) -> None:
    """A value which the firmware can not parse is rejected."""
    await setup_entry(hass)
    mock_notify(aioclient_mock, HOST_A)

    with pytest.raises(vol.Invalid):
        await notify(hass, {"message": "Hello", field: value})

    assert not notify_payloads(aioclient_mock, HOST_A)


async def test_volume_maximum_is_accepted(
    hass: HomeAssistant, aioclient_mock: AiohttpClientMocker
) -> None:
    """The firmware constant MAX_VOLUME is 10, volume 10 is accepted."""
    await setup_entry(hass)
    mock_notify(aioclient_mock, HOST_A)

    await notify(hass, {"message": "Hello", "melody": "a:d=4:c", "volume": 10})

    assert notify_payloads(aioclient_mock, HOST_A)[0]["volume"] == 10


async def test_icon_file_name_relative_path_normalized(
    hass: HomeAssistant, aioclient_mock: AiohttpClientMocker
) -> None:
    """A relative icon path gets a leading slash so LittleFS finds it."""
    await setup_entry(hass)
    mock_notify(aioclient_mock, HOST_A)

    await notify(
        hass, {"message": "Hello", "icon_file_name": "washing_machine.gif"}
    )

    assert (
        notify_payloads(aioclient_mock, HOST_A)[0]["iconFileName"]
        == "/washing_machine.gif"
    )


@pytest.mark.parametrize(
    ("field", "value"),
    [
        ("message", "x" * 257),
        # 129 characters, but 258 bytes in UTF-8.
        ("message", "\u00e4" * 129),
        ("message", ""),
        ("melody", "m" * 257),
    ],
)
async def test_too_long_text_is_rejected(
    hass: HomeAssistant,
    aioclient_mock: AiohttpClientMocker,
    field: str,
    value: str,
) -> None:
    """Text and melody are limited in bytes and are never cut."""
    await setup_entry(hass)
    mock_notify(aioclient_mock, HOST_A)
    data = {"message": "Hello", field: value}

    with pytest.raises(vol.Invalid):
        await notify(hass, data)

    assert not notify_payloads(aioclient_mock, HOST_A)


async def test_limits_are_accepted(
    hass: HomeAssistant, aioclient_mock: AiohttpClientMocker
) -> None:
    """Exactly the maximum length of text and melody is valid."""
    await setup_entry(hass)
    mock_notify(aioclient_mock, HOST_A)

    await notify(hass, {"message": "x" * 256, "melody": "m" * 256})

    payload = notify_payloads(aioclient_mock, HOST_A)[0]
    assert len(payload["text"]) == 256
    assert len(payload["melody"]) == 256


async def test_message_is_mandatory(hass: HomeAssistant) -> None:
    """The message is the only mandatory field."""
    await setup_entry(hass)

    with pytest.raises(vol.Invalid):
        await notify(hass, {"duration": 1000})


async def test_device_offline(
    hass: HomeAssistant, aioclient_mock: AiohttpClientMocker
) -> None:
    """An offline device is reported with its name and a clear reason."""
    await setup_entry(hass)
    mock_notify(aioclient_mock, HOST_A, exc=aiohttp.ClientError)

    with pytest.raises(HomeAssistantError, match=HOSTNAME_A) as err:
        await notify(hass, {"message": "Hello"})

    assert f"Cannot connect to http://{HOST_A}" in str(err.value)


async def test_device_rejects_notification(
    hass: HomeAssistant, aioclient_mock: AiohttpClientMocker
) -> None:
    """The reason of the device is reported."""
    await setup_entry(hass)
    mock_notify(aioclient_mock, HOST_A, **INVALID_DATA_ANSWER)

    with pytest.raises(HomeAssistantError, match="Invalid data."):
        await notify(hass, {"message": "Hello"})


async def test_two_devices_one_offline(
    hass: HomeAssistant, aioclient_mock: AiohttpClientMocker
) -> None:
    """An offline device does not stop the other device."""
    await setup_entry(hass, HOST_A, DEVICE_ID_A, HOSTNAME_A)
    await setup_entry(hass, HOST_B, DEVICE_ID_B, HOSTNAME_B)
    mock_notify(aioclient_mock, HOST_A, exc=TimeoutError)
    mock_notify(aioclient_mock, HOST_B)

    with pytest.raises(HomeAssistantError) as err:
        await notify(hass, {"message": "Hello"})

    assert HOSTNAME_A in str(err.value)
    assert HOSTNAME_B not in str(err.value)
    assert notify_payloads(aioclient_mock, HOST_B) == [{"text": "Hello"}]


async def test_without_target_all_devices(
    hass: HomeAssistant, aioclient_mock: AiohttpClientMocker
) -> None:
    """Without a target, every device shows the notification."""
    await setup_entry(hass, HOST_A, DEVICE_ID_A, HOSTNAME_A)
    await setup_entry(hass, HOST_B, DEVICE_ID_B, HOSTNAME_B)
    mock_notify(aioclient_mock, HOST_A)
    mock_notify(aioclient_mock, HOST_B)

    await notify(hass, {"message": "Hello"})

    assert len(notify_payloads(aioclient_mock, HOST_A)) == 1
    assert len(notify_payloads(aioclient_mock, HOST_B)) == 1


async def test_target_device(
    hass: HomeAssistant, aioclient_mock: AiohttpClientMocker
) -> None:
    """Only the selected device shows the notification."""
    entry_a = await setup_entry(hass, HOST_A, DEVICE_ID_A, HOSTNAME_A)
    await setup_entry(hass, HOST_B, DEVICE_ID_B, HOSTNAME_B)
    mock_notify(aioclient_mock, HOST_A)
    mock_notify(aioclient_mock, HOST_B)

    await notify(
        hass, {"message": "Hello", ATTR_DEVICE_ID: device_id(hass, entry_a)}
    )

    assert len(notify_payloads(aioclient_mock, HOST_A)) == 1
    assert notify_payloads(aioclient_mock, HOST_B) == []


async def test_without_any_device(hass: HomeAssistant) -> None:
    """The action is available, but fails clearly without a device."""
    entry = await setup_entry(hass)
    await hass.config_entries.async_remove(entry.entry_id)

    with pytest.raises(ServiceValidationError, match="No Pixelix device"):
        await notify(hass, {"message": "Hello"})


@pytest.mark.parametrize(
    "target",
    [
        ["does-not-exist"],
        [],
        None,
    ],
)
async def test_invalid_target_sends_nothing(
    hass: HomeAssistant,
    aioclient_mock: AiohttpClientMocker,
    target: list[str] | None,
) -> None:
    """An invalid selection never falls back to all devices."""
    await setup_entry(hass, HOST_A, DEVICE_ID_A, HOSTNAME_A)
    await setup_entry(hass, HOST_B, DEVICE_ID_B, HOSTNAME_B)
    mock_notify(aioclient_mock, HOST_A)
    mock_notify(aioclient_mock, HOST_B)

    with pytest.raises(ServiceValidationError):
        await notify(hass, {"message": "Hello", ATTR_DEVICE_ID: target})

    assert not notify_payloads(aioclient_mock, HOST_A)
    assert not notify_payloads(aioclient_mock, HOST_B)


async def test_valid_and_invalid_target_sends_nothing(
    hass: HomeAssistant, aioclient_mock: AiohttpClientMocker
) -> None:
    """One unknown device prevents the notification for all targets."""
    entry_a = await setup_entry(hass, HOST_A, HOSTNAME_A)
    mock_notify(aioclient_mock, HOST_A)

    with pytest.raises(ServiceValidationError, match="does-not-exist"):
        await notify(
            hass,
            {
                "message": "Hello",
                ATTR_DEVICE_ID: [device_id(hass, entry_a), "does-not-exist"],
            },
        )

    assert not notify_payloads(aioclient_mock, HOST_A)


async def test_device_of_other_integration_is_invalid(
    hass: HomeAssistant, aioclient_mock: AiohttpClientMocker
) -> None:
    """A device of another integration is not a valid target."""
    await setup_entry(hass, HOST_A, HOSTNAME_A)
    mock_notify(aioclient_mock, HOST_A)
    other = MockConfigEntry(domain="other")
    other.add_to_hass(hass)
    foreign = dr.async_get(hass).async_get_or_create(
        config_entry_id=other.entry_id, identifiers={("other", "1")}
    )

    with pytest.raises(ServiceValidationError):
        await notify(
            hass, {"message": "Hello", ATTR_DEVICE_ID: foreign.id}
        )

    assert not notify_payloads(aioclient_mock, HOST_A)


async def test_rejected_login_starts_reauthentication(
    hass: HomeAssistant, aioclient_mock: AiohttpClientMocker
) -> None:
    """A rejected login is reported and asks the user for a new login."""
    entry = await setup_entry(hass)
    mock_notify(aioclient_mock, HOST_A, status=401, text="")

    with pytest.raises(HomeAssistantError, match="web login"):
        await notify(hass, {"message": "Hello"})
    await hass.async_block_till_done()

    flows = hass.config_entries.flow.async_progress_by_handler(DOMAIN)
    assert [flow["context"]["source"] for flow in flows] == ["reauth"]
    assert flows[0]["context"]["entry_id"] == entry.entry_id
