# Home Assistant Framework Reference

Condensed from <https://developers.home-assistant.io/docs/creating_component_index/>
and the pages it links. Use this to find the right API fast; follow the links
for the full upstream text.

---

## 1. manifest.json

Every integration is a directory under `custom_components/` containing at
minimum `manifest.json` and `__init__.py`. The directory name **must** equal
`domain`.

<https://developers.home-assistant.io/docs/creating_integration_manifest/>

| Key | Required | Meaning |
| --- | --- | --- |
| `domain` | yes | Short name, lowercase letters and underscores only. |
| `name` | yes | Display name. |
| `version` | **yes for custom** | SemVer or CalVer. Core integrations omit it. |
| `documentation` | yes | URL to the docs. |
| `codeowners` | yes | GitHub handles, e.g. `["@BlueAndi"]`. |
| `requirements` | yes | pip requirement strings. `[]` when none. |
| `integration_type` | recommended | `device`, `entity`, `hardware`, `helper`, `hub`, `service`, `system`, `virtual`. Custom integrations default to `hub`. |
| `iot_class` | recommended | `assumed_state`, `cloud_polling`, `cloud_push`, `local_polling`, `local_push`, `calculated`. |
| `config_flow` | no | `true` requires a `config_flow.py`. |
| `dependencies` | no | Integrations that must load first. |
| `after_dependencies` | no | Load before this one **if** configured (e.g. `mqtt`). |
| `issue_tracker` | no | Issue URL shown to users. |
| `single_config_entry` | no | `true` allows exactly one entry. |
| `quality_scale` | no | `bronze` … `platinum`. |
| `loggers` | no | Logger names used by the requirements. |
| `zeroconf` / `dhcp` / `ssdp` / `bluetooth` / `mqtt` / `usb` | no | Discovery matchers. |

Pixelix uses `zeroconf: ["_pixelix._tcp.local."]`, `integration_type: "device"`,
`iot_class: "assumed_state"` and `after_dependencies: ["mqtt"]` — MQTT is used
when present but is not required.

---

## 2. Setting up

<https://developers.home-assistant.io/docs/config_entries_index/>

An integration configured through the UI is set up **per config entry**. The
YAML `async_setup` is only needed to declare that YAML configuration is not
supported:

```python
CONFIG_SCHEMA = cv.config_entry_only_config_schema(DOMAIN)
```

### Lifecycle

```python
async def async_setup_entry(
    hass: HomeAssistant, entry: MyConfigEntry
) -> bool:
    """Set up one device from a config entry."""


async def async_unload_entry(
    hass: HomeAssistant, entry: MyConfigEntry
) -> bool:
    """Unload a config entry."""


async def async_migrate_entry(
    hass: HomeAssistant, entry: ConfigEntry
) -> bool:
    """Migrate an entry of an older version."""


async def async_remove_entry(
    hass: HomeAssistant, entry: ConfigEntry
) -> None:
    """Clean up after the entry was deleted."""
```

### runtime_data

Per-entry state belongs in `entry.runtime_data`, typed with a generic
`ConfigEntry` alias — not in `hass.data[DOMAIN]`:

```python
@dataclass
class PixelixRuntimeData:
    """Runtime state of one Pixelix device."""

    client: PixelixClient
    hostname: str | None = None


type PixelixConfigEntry = ConfigEntry[PixelixRuntimeData]
```

`runtime_data` is dropped automatically on unload.

### Platforms

```python
await hass.config_entries.async_forward_entry_setups(
    entry, [Platform.SENSOR, Platform.SWITCH]
)
...
return await hass.config_entries.async_unload_platforms(
    entry, [Platform.SENSOR, Platform.SWITCH]
)
```

### Cleanup

Register every teardown at setup time so unload cannot forget it:

```python
entry.async_on_unload(unsubscribe_callback)
entry.async_on_unload(entry.add_update_listener(_async_update_listener))
```

### Exceptions during setup

| Raise | Effect |
| --- | --- |
| `ConfigEntryNotReady` | Entry goes to `SETUP_RETRY`, HA retries with backoff. |
| `ConfigEntryAuthFailed` | Starts the reauth flow. |
| `ConfigEntryError` | Permanent failure, `SETUP_ERROR`. |

Never mutate `entry` directly — use
`hass.config_entries.async_update_entry(entry, data=..., version=...)`.

---

## 3. Config flow

<https://developers.home-assistant.io/docs/config_entries_config_flow_handler/>

```python
class PixelixConfigFlow(ConfigFlow, domain=DOMAIN):
    """Add a device."""

    VERSION = 2
    MINOR_VERSION = 1

    async def async_step_user(
        self, user_input: dict[str, Any] | None = None
    ) -> ConfigFlowResult:
        errors: dict[str, str] = {}

        if user_input is not None:
            try:
                info = await _validate(self.hass, user_input)
            except PixelixConnectionError:
                errors["base"] = "cannot_connect"
            except PixelixAuthError:
                errors["base"] = "invalid_auth"
            else:
                await self.async_set_unique_id(info["device_id"])
                self._abort_if_unique_id_configured()
                return self.async_create_entry(
                    title=info["name"], data=user_input
                )

        return self.async_show_form(
            step_id="user", data_schema=USER_SCHEMA, errors=errors
        )
```

### Results

| Call | Use |
| --- | --- |
| `self.async_show_form(step_id=…, data_schema=…, errors=…)` | Ask the user. |
| `self.async_create_entry(title=…, data=…)` | Finish, create the entry. |
| `self.async_abort(reason=…)` | Stop; `reason` is a key under `config.abort`. |
| `self.async_update_reload_and_abort(entry, data_updates=…)` | Finish a reconfigure or reauth. |

### Unique ID

Use a stable identifier — a chip id, serial or MAC. **Never** a host name or
IP address; those change and would create duplicate devices.

```python
await self.async_set_unique_id(device_id)
self._abort_if_unique_id_configured()
# For reconfigure / reauth, assert it did not change instead:
self._abort_if_unique_id_mismatch()
```

`_abort_if_unique_id_configured(updates={CONF_HOST: host})` additionally
refreshes the stored address of an already known device.

### Discovery steps

Reserved step names: `bluetooth`, `dhcp`, `homekit`, `mqtt`, `ssdp`, `usb`,
`zeroconf`. A discovery step sets the unique id, aborts if already configured,
then hands over to a confirmation step that asks for whatever is still missing
(credentials, typically).

```python
async def async_step_zeroconf(
    self, discovery_info: ZeroconfServiceInfo
) -> ConfigFlowResult:
```

### Reauth and reconfigure

- `async_step_reauth(entry_data)` → `async_step_reauth_confirm(user_input)`.
  Triggered by raising `ConfigEntryAuthFailed`. Updates the existing entry;
  it never creates a second one.
- `async_step_reconfigure(user_input)` lets the user change settings of an
  existing entry; use `self._get_reconfigure_entry()`.

### Migration

```python
async def async_migrate_entry(hass, config_entry) -> bool:
    if config_entry.version == 1:
        new_data = {**config_entry.data}
        # adapt new_data
        hass.config_entries.async_update_entry(
            config_entry, data=new_data, version=2
        )
    return True
```

---

## 4. Fetching data

<https://developers.home-assistant.io/docs/integration_fetching_data/>

**Push** (device notifies, e.g. MQTT) is preferred over **poll**. For polling,
use a coordinator so one request feeds all entities:

```python
class MyCoordinator(DataUpdateCoordinator[MyData]):
    """Fetch data from the device."""

    def __init__(self, hass, config_entry, client) -> None:
        super().__init__(
            hass,
            _LOGGER,
            name=DOMAIN,
            config_entry=config_entry,
            update_interval=timedelta(seconds=30),
            always_update=False,
        )
        self._client = client

    async def _async_update_data(self) -> MyData:
        try:
            return await self._client.fetch()
        except MyAuthError as err:
            raise ConfigEntryAuthFailed from err
        except MyError as err:
            raise UpdateFailed(f"Error: {err}") from err
```

Prime it in `async_setup_entry` with
`await coordinator.async_config_entry_first_refresh()` — it raises
`ConfigEntryNotReady` on failure, so HA retries.

Entities subclass `CoordinatorEntity`, which supplies `should_poll`,
`async_update`, `async_added_to_hass` and `available`:

```python
@callback
def _handle_coordinator_update(self) -> None:
    self._attr_is_on = self.coordinator.data[self.idx]["state"]
    self.async_write_ha_state()
```

---

## 5. Entities

<https://developers.home-assistant.io/docs/core/entity/>

```python
class MyEntity(CoordinatorEntity[MyCoordinator], SensorEntity):
    """A sensor of the device."""

    _attr_has_entity_name = True       # mandatory for new integrations
    _attr_name = None                  # entity IS the main feature of the device

    def __init__(self, coordinator, entry) -> None:
        super().__init__(coordinator)
        self._attr_unique_id = f"{entry.unique_id}_brightness"
        self._attr_device_info = DeviceInfo(
            identifiers={(DOMAIN, entry.unique_id)},
            manufacturer=MANUFACTURER,
            model=MODEL,
            name=entry.title,
        )
```

- `_attr_unique_id` — stable, unique within the platform, never user-editable.
  Without it the entity cannot be renamed or customised in the UI.
- Prefer `_attr_x` over overriding the property. Read back via `self.x`, not
  `self._attr_x`.
- `available` — `False` while the device cannot be reached.
- `entity_category` — `CONFIG` or `DIAGNOSTIC` for secondary entities.
- `should_poll = False` for push integrations; call `async_write_ha_state()`
  when new data arrives.
- Subscribe in `async_added_to_hass()`, undo it in
  `async_will_remove_from_hass()`.

---

## 6. Actions (services)

<https://developers.home-assistant.io/docs/dev_101_services/>

```python
hass.services.async_register(
    DOMAIN,
    SERVICE_NOTIFY,
    _async_notify,
    schema=NOTIFY_SCHEMA,
)
```

- The schema is voluptuous; use `homeassistant.helpers.config_validation` (`cv`)
  helpers and `extra=vol.PREVENT_EXTRA` so typos are rejected.
- `supports_response=SupportsResponse.OPTIONAL` (acts *and* returns data) or
  `.ONLY` (returns data only).
- Invalid user input → `ServiceValidationError`; runtime failure →
  `HomeAssistantError`.
- A device-targeted action declares its target in `services.yaml`:

```yaml
notify:
  name: Send notification
  description: Send a Pixelix notification.
  target:
    device:
      integration: pixelix
  fields:
    message:
      name: Message
      description: Text displayed on Pixelix.
      required: true
      example: The washing machine is finished!
      selector:
        text:
          multiline: true
```

Each field needs a `selector` so the UI can render it.

---

## 7. Translations

<https://developers.home-assistant.io/docs/internationalization/custom_integration/>

> "Do not use `strings.json` for custom components."

Custom integrations skip HA Core's translation build step, so
`translations/en.json` must contain the **complete, flat** English text. One
file per language, named BCP47 (`en.json`, `de.json`).

```json
{
    "title": "Pixelix",
    "config": {
        "flow_title": "{name}",
        "step": {
            "user": {
                "title": "Add Pixelix device",
                "description": "Enter the host name or IP address.",
                "data": { "host": "Host" },
                "data_description": { "host": "Host name or IP address." }
            }
        },
        "error": { "cannot_connect": "Failed to connect" },
        "abort": { "already_configured": "Device is already configured" }
    },
    "services": {
        "notify": {
            "name": "Send notification",
            "description": "Send a Pixelix notification.",
            "fields": {
                "message": {
                    "name": "Message",
                    "description": "Text displayed on Pixelix."
                }
            }
        }
    }
}
```

Other top-level sections: `options`, `entity`, `exceptions`, `selector`,
`device_automation`.

---

## 8. Testing

<https://developers.home-assistant.io/docs/development_testing/>

Custom integrations test against `pytest-homeassistant-custom-component`,
pinned to the HA version the integration targets. The repo config:

```ini
[pytest]
asyncio_mode = auto
testpaths = tests
pythonpath = .
```

Custom components are not loaded by default — enable them once, in
`conftest.py`:

```python
@pytest.fixture(autouse=True)
def auto_enable_custom_integrations(enable_custom_integrations):
    """Load the integration from the custom_components folder."""
    del enable_custom_integrations

    yield
```

Useful fixtures: `hass`, `hass_client`, `aioclient_mock`
(`AiohttpClientMocker`), `snapshot` (syrupy), plus `MockConfigEntry` from
`pytest_homeassistant_custom_component.common`.

Test through the core interfaces, never through integration internals:
`hass.config_entries`, `hass.states`, `hass.services`, and the device/entity
registries.

```python
result = await hass.config_entries.flow.async_init(
    DOMAIN, context={"source": config_entries.SOURCE_USER}
)
result = await hass.config_entries.flow.async_configure(
    result["flow_id"], USER_INPUT
)
assert result["type"] is FlowResultType.CREATE_ENTRY
```

Cover at least: the happy path of every config-flow step, each error branch
(`cannot_connect`, `invalid_auth`), duplicate detection, reauth, setup and
unload, and every action field including its validation limits.

---

## 9. Quality scale

<https://developers.home-assistant.io/docs/core/integration-quality-scale/>

| Tier | Gist |
| --- | --- |
| 🥉 Bronze | UI setup, basic coding standards, config-flow tests, user docs. |
| 🥈 Silver | Recovers from connection errors, triggers reauth, active code owner, troubleshooting docs. |
| 🥇 Gold | Discovery, translations everywhere, diagnostics, reconfigure flow, full test coverage. |
| 🏆 Platinum | Fully async, fully type-hinted, efficient data handling, all standards met. |

Aim for Gold behaviour even though a custom integration is not graded: it is
the checklist users judge an integration by.

---

## 10. Further reading

- Creating a component: <https://developers.home-assistant.io/docs/creating_component_index/>
- Development checklist: <https://developers.home-assistant.io/docs/development_checklist/>
- Config entries: <https://developers.home-assistant.io/docs/config_entries_index/>
- Config flow: <https://developers.home-assistant.io/docs/config_entries_config_flow_handler/>
- Fetching data: <https://developers.home-assistant.io/docs/integration_fetching_data/>
- Entities: <https://developers.home-assistant.io/docs/core/entity/>
- Actions: <https://developers.home-assistant.io/docs/dev_101_services/>
- Device registry: <https://developers.home-assistant.io/docs/device_registry_index/>
- Translations: <https://developers.home-assistant.io/docs/internationalization/custom_integration/>
- Testing: <https://developers.home-assistant.io/docs/development_testing/>
